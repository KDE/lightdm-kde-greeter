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

#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include "config.h"

using namespace Qt::StringLiterals;

static const char *s_greeterUserName = "_ldm";

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

class Helper::Privileges {

public:
    bool drop(const char *username)
    {
        auto pw = getpwnam(username);
        if (!pw) {
            qWarning() << "Failed to retrieve the user database entry";
            return false;
        }

        if (setegid(pw->pw_gid)) {
            qWarning() << "Failed to set gid:" << strerror(errno);
            return false;
        }

        m_groupListSize = getgroups(sizeof(m_groupList) / sizeof(m_groupList[0]), m_groupList);

        if (m_groupListSize < 0) {
            qWarning() << "Failed to get groups:" << strerror(errno);
            return false;
        }

        if (m_groupListSize > 0) {
            // drop any supplementary group membership
            if (setgroups(0, nullptr)) {
                qWarning() << "Failed to unset groups:" << strerror(errno);
                return false;
            }
        }

        if (seteuid(pw->pw_uid)) {
            qWarning() << "Failed to set uid:" << strerror(errno);
            return false;
        }

        return true;
    }

    bool reset()
    {
        if (setegid(0)) {
            qCritical() << "Failed to reset gid:" << strerror(errno);
            return false;
        }

        if (seteuid(0)) {
            qCritical() << "Failed to reset uid:" << strerror(errno);
            return false;
        }

        if (m_groupListSize > 0) {
            // get back supplementary group membership
            if (setgroups(m_groupListSize, m_groupList)) {
                qWarning() << "Failed to get back groups:" << strerror(errno);
                return false;
            }
        }

        return true;
    }

private:
    int m_groupListSize{};
    gid_t m_groupList[NGROUPS_MAX]{};
};

KAuth::ActionReply Helper::save(const QVariantMap &args)
{
    const QString optionsKey = u"options"_s;

    KAuth::ActionReply errorReply = KAuth::ActionReply::HelperErrorReply();
    Configs configs;
    QString errorMessage;

    auto options = args[optionsKey].value<QVariantMap>();

    auto setReplyError = [&errorReply](const QString &errorMessage) {
        errorReply.setErrorDescription(errorMessage);
        return errorReply;
    };

    for (auto i = options.constBegin(); i != options.constEnd(); ++i)
    {
        auto p = ParsedKey::parse(i.key(), errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        auto config = configs.getByFilename(p.fileName, errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        config->group(p.groupName).writeEntry(p.keyName, i.value());
    }

    // all keys except "options" carry in their value file descriptors of files
    // that need to be copied to the home directory of the greeter user
    Privileges privileges;
    if (!privileges.drop(s_greeterUserName)) {
        return setReplyError(u"Can't drop privileges to greeter user (%1)"_s.arg(s_greeterUserName));
    }

    for (auto i = args.constBegin() ; i != args.constEnd() ; ++i)
    {
        if (i.key() == optionsKey) continue;

        auto dbusFD = i.value().value<QDBusUnixFileDescriptor>();
        if (!dbusFD.isValid()) {
            return setReplyError(u"Invalid file descriptor for key: %1"_s.arg(i.key()));
        }

        auto p = ParsedKey::parse(i.key(), errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        auto config = configs.getByFilename(p.fileName, errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        QString ldmPath = copyImage(dbusFD.fileDescriptor(), p.groupName, p.keyName, errorMessage);
        if (!errorMessage.isEmpty()) return setReplyError(errorMessage);

        config->group(p.groupName).writeEntry(p.keyName, ldmPath);
    }

    if (!privileges.reset()) {
        // If resetting privileges fails then there is a bigger issue at hand
        // and the helper should actually kill itself, because it can no longer
        // fulfill its purpose.
        exit(1);
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
 * @param errorMessage reference to string, to write an error in it
 * @return path to the resulting file
 */
QString Helper::copyImage(int sourceFD, QString theme, QString name, QString &errorMessage)
{
    auto setError = [&errorMessage] (const QString &msg) {
        errorMessage = msg;
        return QString{};
    };
    // in case anyone put some path instead of name
    theme = QFileInfo(theme).fileName();
    if (theme.isEmpty()) return setError(u"Theme is empty"_s);

    name = QFileInfo(name).fileName();
    if (name.isEmpty()) return setError(u"Name is empty"_s);

    QFile source;
    if (!source.open(sourceFD, QFile::ReadOnly)) {
        return setError(u"Can't open file from FD: %1"_s.arg(sourceFD));
    }

    // that should be enough
    constexpr qint64 maxImageFileSize = 1024 * 1024 * 50;

    if (source.size() > maxImageFileSize) {
        return setError(u"Image size is too large: %1 max size %2"_s.arg(source.size()).arg(maxImageFileSize));
    }

    QFile dest{ QStringLiteral("%1/%2/%3").arg(QStringLiteral(GREETER_IMAGES_DIR)).arg(theme).arg(name) };

    if (dest.exists()) {
        dest.remove();
    } else {
        QDir dir{ QFileInfo{ dest }.absolutePath() };
        if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
            return setError(u"Can't create target directory: %1"_s.arg(dir.absolutePath()));
        }
    }

    if (!dest.open(QFile::WriteOnly)) {
        return setError(u"Can't open destination file for writing: %1"_s.arg(dest.fileName()));
    }

    auto mappedSource = source.map(0, source.size());
    if (!mappedSource) {
        return setError(u"Can't map source file: %1"_s.arg(source.fileName()));
    }

    qint64 written = dest.write((char*)mappedSource, source.size());
    if (written != source.size()) {
        return setError(u"Not the whole image is copied, copied %1 from %2"_s.arg(written).arg(source.fileName()));
    }

    return dest.fileName();
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmlightdm", Helper)

#include "moc_helper.cpp"
