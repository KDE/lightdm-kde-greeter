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
#ifndef LIGHTDMKCM_H
#define LIGHTDMKCM_H

#include <KQuickAddons/ManagedConfigModule>

class ThemesModel;
class UsersModel;

class LightDMKcm: public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(ThemesModel *themesModel READ themesModel CONSTANT)
    Q_PROPERTY(UsersModel *usersModel READ usersModel CONSTANT)

public:
    explicit LightDMKcm(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

    ThemesModel *themesModel() const { return m_themesModel; }
    UsersModel *usersModel() const { return m_usersModel; }

private:
    ThemesModel *m_themesModel;
    UsersModel *m_usersModel;
};

#endif // LIGHTDMKCM_H
