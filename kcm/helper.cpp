/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "helper.h"

#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

#include <QDBusUnixFileDescriptor>
#include <QDir>
#include <QFile>
#include <QUrl>

#include "config.h"

using namespace Qt::StringLiterals;

struct Helper::ParsedKey {

    static ParsedKey parse(const QString &key, QString &errorMessage)
    {
        ParsedKey result;
        QStringList lst = key.split(QLatin1Char('/'));
        if (lst.size() != 3)
        {
            errorMessage = u"Invalid key format: %1"_s.arg(key);
            return result;
        }
        result.fileName = lst[0];
        result.groupName = lst[1];
        result.keyName = lst[2];
        return result;
    }

    QString fileName;
    QString groupName;
    QString keyName;
};

class Helper::Configs {

public:
    Configs() {
        m_coreConfig = openConfig(CoreConfig);
        m_greeterConfig = openConfig(GreeterConfig);
    };

    QSharedPointer<KConfig> getByFilename(const QString &fileName, QString &errorMessage)
    {
        if (fileName == u"core"_s) {
            return m_coreConfig;
        } else if (fileName == u"greeter"_s) {
            return m_greeterConfig;
        } else {
            errorMessage = u"Unknown config file: %1"_s.arg(fileName);
            return {};
        }
    }

    void sync()
    {
        m_coreConfig->sync();
        m_greeterConfig->sync();
    }

private:
    enum WhichConfig
    {
        CoreConfig,
        GreeterConfig
    };

    QSharedPointer<KConfig> openConfig(WhichConfig which)
    {
        QString name = QStringLiteral("%1/%2")
            .arg(QStringLiteral(LIGHTDM_CONFIG_DIR))
            .arg(which == CoreConfig ? QStringLiteral("lightdm.conf") : QStringLiteral("lightdm-kde-greeter.conf"))
            ;

        QFile file(name);
        if (!file.exists())
        {
            // If we are creating the config file, ensure it is world-readable: if
            // we don't do that, KConfig will create a file which is only readable
            // by root
            file.open(QIODevice::WriteOnly);
            file.close();
            file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
        }

        return QSharedPointer<KConfig>(new KConfig(file.fileName(), KConfig::SimpleConfig));
    }

    QSharedPointer<KConfig> m_coreConfig;
    QSharedPointer<KConfig> m_greeterConfig;
};

KAuth::ActionReply Helper::save(const QVariantMap &args)
{
    const QString optionsKey = u"options"_s;

    KAuth::ActionReply errorReply = KAuth::ActionReply::HelperErrorReply();
    Configs configs;
    QString errorMessage;

    auto options = args[optionsKey].value<QVariantMap>();

    for (auto i = options.constBegin(); i != options.constEnd(); ++i)
    {
        auto p = ParsedKey::parse(i.key(), errorMessage);
        if (!errorMessage.isEmpty()) {
            errorReply.setErrorDescription(errorMessage);
            return errorReply;
        }

        auto config = configs.getByFilename(p.fileName, errorMessage);
        if (!errorMessage.isEmpty()) {
            errorReply.setErrorDescription(errorMessage);
            return errorReply;
        }

        config->group(p.groupName).writeEntry(p.keyName, i.value());
    }

    // all keys except "options" carry in their value file descriptors of files
    // that need to be copied to the home directory of the greeter user
    for (auto i = args.constBegin() ; i != args.constEnd() ; ++i)
    {
        if (i.key() == optionsKey) continue;

        auto dbusFD = i.value().value<QDBusUnixFileDescriptor>();
        if (!dbusFD.isValid()) {
            errorReply.setErrorDescription(u"Invalid file descriptor for key: %1"_s.arg(i.key()));
            return errorReply;
        }

        auto p = ParsedKey::parse(i.key(), errorMessage);
        if (!errorMessage.isEmpty()) {
            errorReply.setErrorDescription(errorMessage);
            return errorReply;
        }

        auto config = configs.getByFilename(p.fileName, errorMessage);
        if (!errorMessage.isEmpty()) {
            errorReply.setErrorDescription(errorMessage);
            return errorReply;
        }

        QString ldmPath = copyImage(dbusFD.fileDescriptor(), p.groupName, p.keyName);
        if (ldmPath.isEmpty()) {
            errorReply.setErrorDescription(u"Error when copying file for key: %s"_s.arg(i.key()));
            return errorReply;
        }

        config->group(p.groupName).writeEntry(p.keyName, ldmPath);
    }

    configs.sync();

    return KAuth::ActionReply::SuccessReply();
}

/**
 * Copy the image to the home directory of the greeter.
 *
 * @param sourceFD file descriptor
 * @param theme theme name, in which the image will be used
 * @param name filename, must not contain subdirectories, they are ignored
 * @return path to the resulting file, empty string means error
 */
QString Helper::copyImage(int sourceFD, QString theme, QString name)
{
    // in case anyone put some path instead of name
    theme = QFileInfo(theme).fileName();
    if (theme.isEmpty()) {
        qWarning() << "Theme is empty";
        return QString{};
    }

    name = QFileInfo(name).fileName();
    if (name.isEmpty()) {
        qWarning() << "Name is empty";
        return QString{};
    }

    QFile source;
    if (!source.open(sourceFD, QFile::ReadOnly)) {
        qWarning() << "Can't open file from FD:" << sourceFD;
        return QString{};
    }

    // that should be enough
    constexpr qint64 maxImageFileSize = 1024 * 1024 * 50;

    if (source.size() > maxImageFileSize) {
        qWarning() << "Image size is too large:" << source.size() << "max size:" << maxImageFileSize;
        return QString{};
    }

    QFile dest{ QStringLiteral("%1/%2/%3").arg(QStringLiteral(GREETER_IMAGES_DIR)).arg(theme).arg(name) };

    if (dest.exists()) {
        dest.remove();
    } else {
        QDir dir{ QFileInfo{ dest }.absolutePath() };
        if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
            qWarning() << "Can't create target directory: " << dir.absolutePath();
            return QString{};
        }
    }

    if (!dest.open(QFile::WriteOnly)) {
        qWarning() << "Can't open destination file for writing:" << dest.fileName();
        return QString{};
    }

    auto mappedSource = source.map(0, source.size());
    if (!mappedSource) {
        qWarning() << "Can't map source file:" << source.fileName();
        return QString{};
    }

    qint64 written = dest.write((char*)mappedSource, source.size());
    if (written != source.size()) {
        qWarning() << "Not the whole image is copied, copied" << written << "from" << source.fileName();
        return QString{};
    }

    return dest.fileName();
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmlightdm", Helper)

#include "moc_helper.cpp"
