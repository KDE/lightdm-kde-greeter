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
#include "sessionsmodel.h"
#include "usersmodel.h"

K_PLUGIN_CLASS_WITH_JSON(LightDMKcm, "kcm_lightdm.json")

LightDMKcm::LightDMKcm(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, data, args)
{
    qmlRegisterAnonymousType<ThemesModel>("org.altlinux.lightdm.kcm", 1);
    qmlRegisterAnonymousType<UsersModel>("org.altlinux.lightdm.kcm", 1);
    qmlRegisterAnonymousType<SessionsModel>("org.altlinux.lightdm.kcm", 1);
    m_themesModel = new ThemesModel(this);
    m_usersModel = new UsersModel(this);
    m_sessionsModel = new SessionsModel(this);

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

// functions to calculate the most appropriate aspect ratio for background image
// See: KDE/plasma-workspace, PackageFinder::findPreferredImageInPackage

static float distance(const QSize &size, const QSize &desired)
{
    const float desiredAspectRatio = (desired.height() > 0) ? desired.width() / static_cast<float>(desired.height()) : 0;
    const float candidateAspectRatio = (size.height() > 0) ? size.width() / static_cast<float>(size.height()) : std::numeric_limits<float>::max();

    float delta = size.width() - desired.width();
    delta = delta >= 0.0 ? delta : -delta * 2; // Penalize for scaling up

    return std::abs(candidateAspectRatio - desiredAspectRatio) * 25000 + delta;
}

static QSize res_size(const QString &str)
{
    const int index = str.indexOf(QLatin1Char('x'));

    if (index != -1) {
        return QSize(QStringView(str).left(index).toInt(), QStringView(str).mid(index + 1).toInt());
    }

    return QSize();
}

static void find_preferred_image_in_package(KPackage::Package &package, const QSize &targetSize)
{
    if (!package.isValid()) {
        return;
    }

    QSize tSize = targetSize;

    if (tSize.isEmpty()) {
        tSize = QSize(1920, 1080);
    }

    // find preferred size
    auto findBestMatch = [&package, &tSize](const QByteArray &folder) {
        QString preferred;
        const QStringList images = package.entryList(folder);

        if (images.empty()) {
            return preferred;
        }

        float best = std::numeric_limits<float>::max();

        for (const QString &entry : images) {
            QSize candidate = res_size(QFileInfo(entry).baseName());

            if (candidate.isEmpty()) {
                continue;
            }

            const float dist = distance(candidate, tSize);

            if (preferred.isEmpty() || dist < best) {
                preferred = entry;
                best = dist;
            }
        }

        return preferred;
    };

    const QString preferred = findBestMatch(QByteArrayLiteral("images"));
    const QString preferredDark = findBestMatch(QByteArrayLiteral("images_dark"));

    package.removeDefinition("preferred");
    package.addFileDefinition("preferred", QStringLiteral("images/") + preferred, i18n("Recommended wallpaper file"));

    if (!preferredDark.isEmpty()) {
        package.removeDefinition("preferredDark");
        package.addFileDefinition("preferredDark",
                                  QStringLiteral("images_dark%1").arg(QDir::separator()) + preferredDark,
                                  QStringLiteral("Recommended dark wallpaper file"));
    }
}


QString LightDMKcm::preferredImage(QString path)
{
    if (path.isEmpty()) return QStringLiteral("");
    // probably it's just the path to the image file
    if (!QFileInfo(path).isDir()) return path;

    auto package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Wallpaper/Images"));
    package.setPath(path);
    if (!package.isValid()) return QStringLiteral("");

    find_preferred_image_in_package(package, QSize());
    return package.filePath("preferred");
}

#include "lightdmkcm.moc"
