/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022 Anton Golubev <golubevan@basealt.ru>

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

#include "usersmodel.h"

#include <QLightDM/UsersModel>
#include <QDebug>

#define TRANSLATION_DOMAIN "greeter"
#include <KLocalizedString>

UsersModel::UsersModel(QObject *parent)
    : ExtraRowProxyModel(parent)
    , m_showGuest(false)
{
    setSourceModel(QSharedPointer<QAbstractItemModel>(new QLightDM::UsersModel(this)));
}

void UsersModel::setShowGuest(bool showGuest)
{
    if (showGuest == m_showGuest)
    {
        return;
    }

    m_showGuest = showGuest;

    if (m_showGuest)
    {
        QStandardItem *guest = new QStandardItem(i18n("Guest"));
        guest->setData(QStringLiteral("*guest"), QLightDM::UsersModel::NameRole);
        extraRowModel()->appendRow(guest);
    }
    else
    {
        extraRowModel()->removeRow(0);
    }
}

bool UsersModel::showGuest() const
{
    return m_showGuest;
}

int UsersModel::indexForUserName(QString name) const
{
    int i = 0;
    while (hasIndex(i, 0)) {
        if (data(index(i), QLightDM::UsersModel::UserModelRoles::NameRole) == name) {
            return i;
        }
        ++i;
    }
    return -1;
}
