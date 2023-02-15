/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

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
#include "lightdmkcm.h"

#include <QQuickItem>

#include <KAboutData>
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#define TRANSLATION_DOMAIN "kcm_lightdm"
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPackage/Package>
#include <KPackage/PackageLoader>
#include <KConfigGroup>
#include <KConfig>

#include "../about.h"
#include "themesmodel.h"
#include "usersmodel.h"

K_PLUGIN_CLASS_WITH_JSON(LightDMKcm, "kcm_lightdm.json")

LightDMKcm::LightDMKcm(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, data, args)
{
    qmlRegisterAnonymousType<ThemesModel>("org.altlinux.lightdm.kcm", 1);
    qmlRegisterAnonymousType<UsersModel>("org.altlinux.lightdm.kcm", 1);
    m_themesModel = new ThemesModel(this);
    m_usersModel = new UsersModel(this);

    KAboutData* aboutData = new KAboutData(
        QStringLiteral("kcm_lightdm"),                // appName
        ki18n("LightDM KDE Config").toString(), // programName
        QStringLiteral("0"),                        // version (set by initAboutData)
        ki18n("Login screen using the LightDM framework").toString(),
        KAboutLicense::GPL);
    initAboutData(aboutData);
    setAboutData(aboutData);

    // Our modules will be checking the Plasmoid attached object when running from Plasma, let it load the module
    // taken from KDE/kscreenlocker source, kcm/kcm.cpp
    const char *uri = "org.kde.plasma.plasmoid";
    qmlRegisterUncreatableType<QObject>(uri, 2, 0, "PlasmoidPlaceholder", QStringLiteral("Do not create objects of type Plasmoid"));
}

void LightDMKcm::load()
{
    // collect everything that is in the configs and send it to qml
    QMap<QString, QString> configs{
        { QStringLiteral("core"),    QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm.conf") },
        { QStringLiteral("greeter"), QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf") }
    };
    QVariantMap settings;
    for (auto configFile = configs.begin(); configFile != configs.end(); ++configFile) {
        KConfig config(configFile.value(), KConfig::SimpleConfig);
        QStringList groups = config.groupList();
        for (auto &groupName : groups) {
            KConfigGroup group = config.group(groupName);
            QStringList entryKeys = group.keyList();
            for (auto &entryKey : entryKeys) {
                QString entryPath = QStringLiteral("%1/%2/%3").arg(configFile.key()).arg(groupName).arg(entryKey);
                settings[entryPath] = group.readEntry(entryKey);
            }
        }
    }

    QMetaObject::invokeMethod(mainUi(), "load", Q_ARG(QVariant, QVariant::fromValue(settings)));
    setNeedsSave(false);
}

void LightDMKcm::save()
{
    QVariant ret;
    QMetaObject::invokeMethod(mainUi(), "save", Q_RETURN_ARG(QVariant, ret));
    QVariantMap args = ret.value<QVariantMap>();

    KAuth::Action saveAction(QStringLiteral("org.kde.kcontrol.kcmlightdm.save"));
    saveAction.setHelperId(QStringLiteral("org.kde.kcontrol.kcmlightdm"));
    saveAction.setArguments(args);
    KAuth::ExecuteJob *job = saveAction.execute();
    if (!job->exec())
    {
        // FIXME: Show a message here
        qWarning() << "save failed:" << job->errorText() << ", " << job->errorString();
    }
    else
    {
        setNeedsSave(false);
    }
}

void LightDMKcm::defaults()
{
    // load empty config
    QMetaObject::invokeMethod(mainUi(), "load", Q_ARG(QVariant, QVariant()));
    setNeedsSave(true);
}

QUrl LightDMKcm::wallpaperConfigSource() const
{
    auto pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Wallpaper"), wallpaperPackageUrl());
    if (pkg.isValid()) {
        return pkg.fileUrl("ui", QStringLiteral("config.qml"));
    } else {
        qWarning() << "Wallpaper package " << wallpaperPackageUrl() << " is failed to load.";
        return QUrl{};
    }
}

#include "lightdmkcm.moc"
