/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022 Anton Golubev <golubevan@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "usersmodel.h"

#include <QLightDM/UsersModel>
#include <QDebug>

#define TRANSLATION_DOMAIN "lightdm_kde_greeter"
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
