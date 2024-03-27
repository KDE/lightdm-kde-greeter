/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
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

#include "config.h"
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

    // Our modules will be checking the Plasmoid attached object when running from Plasma, let it load the module
    // taken from KDE/kscreenlocker source, kcm/kcm.cpp
    const char *uri = "org.kde.plasma.plasmoid";
    qmlRegisterUncreatableType<QObject>(uri, 2, 0, "PlasmoidPlaceholder", QStringLiteral("Do not create objects of type Plasmoid"));
}

void LightDMKcm::load()
{
    // collect everything that is in the configs
    QMap<QString, QStringList> configs{
        { QStringLiteral("core"),    { QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm.conf"), QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm.conf.d/autologin.conf") }},
        { QStringLiteral("greeter"), { QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf") }}
    };
    m_storedConfig.clear();
    m_updatedConfig.clear();
    for (auto configFileList = configs.begin(); configFileList != configs.end(); ++configFileList) {
        KConfig config(configFileList.value()[0], KConfig::CascadeConfig);
        config.addConfigSources(configFileList.value());
        QStringList groups = config.groupList();
        for (auto &groupName : groups) {
            KConfigGroup group = config.group(groupName);
            QStringList entryKeys = group.keyList();
            for (auto &entryKey : entryKeys) {
                QString entryPath = QStringLiteral("%1/%2/%3").arg(configFileList.key()).arg(groupName).arg(entryKey);
                m_storedConfig[entryPath] = group.readEntry(entryKey);
            }
        }
    }
    QMetaObject::invokeMethod(mainUi(), "load");
    setNeedsSave(false);
}

static QString deniedIfEmpty(QString errorString)
{
    return errorString.length() == 0 ? i18n("Access denied") : errorString;
}

void LightDMKcm::save()
{
    QVariantMap args;
    for (auto i = m_storedConfig.begin(); i != m_storedConfig.end(); ++i) {
        args[i.key()] = i.value();
    }
    // overwrite the changed values, if any
    for (auto i = m_updatedConfig.begin(); i != m_updatedConfig.end(); ++i) {
        args[i.key()] = i.value();
    }

    // any entry ending with 'Preview' is treated as an image
    // it can be either a file or a kpackage
    // need to identify a specific file and copy it to the home directory of
    // the greeter

    // "<path>/<id>Preview" -> "<path>/copy_<id>"
    for (auto entry = args.begin(); entry != args.end(); ++entry) {
        if(!entry.key().endsWith(QStringLiteral("Preview"))) continue;
        QString key = entry.key();
        int sep = key.lastIndexOf(QLatin1Char('/'));
        QString path = key.left(sep);
        QString id = key.mid(sep + 1).chopped(7);
        QString copyEntry = QStringLiteral("%1/copy_%2").arg(path).arg(id);
        args[copyEntry] = preferredImage(entry.value().toString());
    }

    KAuth::Action saveAction(QStringLiteral("org.kde.kcontrol.kcmlightdm.save"));
    saveAction.setHelperId(QStringLiteral("org.kde.kcontrol.kcmlightdm"));
    saveAction.setArguments(args);
    KAuth::ExecuteJob *job = saveAction.execute();
    if (!job->exec())
    {
        qWarning() << "Save failed:" << job->errorText() << ", " << job->errorString();
        Q_EMIT errorAction(QStringLiteral("%1\n%2").arg(i18n("Save settings failed:")).arg(deniedIfEmpty(job->errorString())));
        setNeedsSave(true);
    }
    else
    {
        for (auto i = m_updatedConfig.begin(); i != m_updatedConfig.end(); ++i) {
            m_storedConfig[i.key()] = i.value();
        }
        QMetaObject::invokeMethod(mainUi(), "updateConfigValues");
    }
}

void LightDMKcm::defaults()
{
    QMetaObject::invokeMethod(mainUi(), "defaults");
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
                                  i18n("Recommended dark wallpaper file"));
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

bool LightDMKcm::hasSomethingNew(ConfigMap &newCfg, ConfigMap &oldCfg)
{
    for (auto i = newCfg.begin(); i != newCfg.end(); ++i) {
        if (!oldCfg.contains(i.key()) || oldCfg[i.key()] != i.value()) {
            return true;
        }
    }
    return false;
}

void LightDMKcm::updateConfigValue(QString url, QVariant value)
{
    m_updatedConfig[url] = value.toString();
    bool needSave = hasSomethingNew(m_updatedConfig, m_storedConfig);
    setNeedsSave(needSave);
}

void LightDMKcm::storeDefaultValue(QString url, QVariant value)
{
    if (!m_storedConfig.contains(url)) {
        m_storedConfig[url] = value.toString();
    }
}

QVariant LightDMKcm::getConfigValue(QString url)
{
    if (m_storedConfig.contains(url)) {
        return m_storedConfig[url];
    } else {
        return {};
    }
}

#include "lightdmkcm.moc"
