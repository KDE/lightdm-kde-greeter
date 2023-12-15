/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

LightDM-KDE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LightDM-KDE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LightDM-KDE.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "helper.h"

#include <QDebug>
#include <KConfig>
#include <KConfigGroup>

#include <QFile>

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

        config->group(groupName).writeEntry(keyName, i.value());
    }

    coreConfig->sync();
    greeterConfig->sync();

    return KAuth::ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmlightdm", Helper)

#include "moc_helper.cpp"
