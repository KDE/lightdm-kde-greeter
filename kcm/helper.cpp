/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "helper.h"

#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

#include <QDir>
#include <QFile>
#include <QUrl>

#include "config.h"

enum WhichConfig
{
    CoreConfig,
    GreeterConfig
};

static QSharedPointer<KConfig> openConfig(WhichConfig which)
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

KAuth::ActionReply Helper::save(const QVariantMap &args)
{
    KAuth::ActionReply errorReply = KAuth::ActionReply::HelperErrorReply();
    QSharedPointer<KConfig> coreConfig = openConfig(CoreConfig);
    QSharedPointer<KConfig> greeterConfig = openConfig(GreeterConfig);

    for (auto i = args.constBegin() ; i != args.constEnd() ; ++i)
    {
        QStringList lst = i.key().split(QLatin1Char('/'));
        if (lst.size() != 3)
        {
            errorReply.setErrorDescription(QStringLiteral("Invalid key format: %1").arg(i.key()));
            return errorReply;
        }

        QSharedPointer<KConfig> config;
        QString fileName = lst[0];
        QString groupName = lst[1];
        QString keyName = lst[2];

        if (fileName == QStringLiteral("core"))
        {
            config = coreConfig;
        }
        else if (fileName == QStringLiteral("greeter"))
        {
            config = greeterConfig;
        }
        else
        {
            errorReply.setErrorDescription(QStringLiteral("Unknown config file: %1").arg(fileName));
            return errorReply;
        }

        // keys starting with "copy_" are handled in a special way, in fact,
        // this is an instruction to copy the file to the greeter's home
        // directory, because the greeter will not be able to read the image
        // from the user's home folder

        QLatin1String prefixFrom{ "copy_" };

        if (keyName.startsWith(prefixFrom)) {
            QString sourceFile = QUrl(i.value().toString()).path();
            QString theme = groupName;
            QString name = keyName.mid(5);

            auto entry = QStringLiteral("%1/%2/%3").arg(fileName).arg(groupName).arg(name);

            QString ldmPath = copyImage(sourceFile, theme, name);
            QString cleanKey = keyName.mid(prefixFrom.size());
            config->group(groupName).writeEntry(cleanKey, ldmPath);

            continue;
        }

        config->group(groupName).writeEntry(keyName, i.value());
    }

    coreConfig->sync();
    greeterConfig->sync();

    return KAuth::ActionReply::SuccessReply();
}

/**
 * Copy the image to the home directory of the greeter.
 *
 * @param sourceFile filename with absolute path
 * @param theme theme name, in which the image will be used
 * @param name filename, must not contain subdirectories, they are ignored
 * @return path to the resulting file, empty string means error
 */
QString Helper::copyImage(QString sourceFile, QString theme, QString name)
{
    // in case anyone put some path instead of name
    theme = QFileInfo(theme).fileName();
    name = QFileInfo(name).fileName();
    if (sourceFile.isEmpty() || theme.isEmpty() || name.isEmpty()) return QString{};

    QFile source{ sourceFile };
    QFile dest{ QStringLiteral("%1/%2/%3").arg(QStringLiteral(GREETER_IMAGES_DIR)).arg(theme).arg(name) };

    if (!source.exists()) {
        qWarning() << "The source file does not exist: " << source.fileName();
        return QString{};
    }

    if (dest.exists()) {
        dest.remove();
    } else {
        QDir dir{ QFileInfo{ dest }.absolutePath() };
        if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
            qWarning() << "Can't create target directory: " << dir.absolutePath();
            return QString{};
        }
    }

    if (!source.copy(dest.fileName())) {
        qWarning() << "Can't copy file. Source: " << source.fileName() << " Destination: " << dest.fileName();
        return QString{};
    }
    return dest.fileName();
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmlightdm", Helper)

#include "moc_helper.cpp"
