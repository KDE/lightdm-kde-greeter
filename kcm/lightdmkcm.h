/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef LIGHTDMKCM_H
#define LIGHTDMKCM_H

#include <KQuickAddons/ManagedConfigModule>

class ThemesModel;
class UsersModel;
class SessionsModel;

class LightDMKcm: public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(ThemesModel *themesModel READ themesModel CONSTANT)
    Q_PROPERTY(UsersModel *usersModel READ usersModel CONSTANT)
    Q_PROPERTY(SessionsModel *sessionsModel READ sessionsModel CONSTANT)
    Q_PROPERTY(QUrl wallpaperConfigSource READ wallpaperConfigSource CONSTANT)
    Q_PROPERTY(QString currentWallpaper READ wallpaperPackageUrl CONSTANT)

    ThemesModel *themesModel() const { return m_themesModel; }
    UsersModel *usersModel() const { return m_usersModel; }
    SessionsModel *sessionsModel() const { return m_sessionsModel; }
    QUrl wallpaperConfigSource() const;
    const QString wallpaperPackageUrl() const { return QStringLiteral("org.kde.image"); }

public:
    explicit LightDMKcm(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

    Q_INVOKABLE QString preferredImage(QString dir);
    Q_INVOKABLE void updateConfigValue(QString url, QVariant value);
    Q_INVOKABLE void storeDefaultValue(QString url, QVariant value);
    Q_INVOKABLE QVariant getConfigValue(QString url);

Q_SIGNALS:
    void errorAction(QString message);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    using ConfigMap = QMap<QString, QString>;

    bool hasSomethingNew(ConfigMap &newCfg, ConfigMap &oldCfg);

    ThemesModel *m_themesModel;
    UsersModel *m_usersModel;
    SessionsModel *m_sessionsModel;

    ConfigMap m_storedConfig;
    ConfigMap m_updatedConfig;
};

#endif // LIGHTDMKCM_H
