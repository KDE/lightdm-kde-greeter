/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022-2026 Anton Golubev <golubevan@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "usersmodel.h"

#include <QLightDM/UsersModel>
#include <QDebug>

#include <config.h>

#define TRANSLATION_DOMAIN "lightdm_kde_greeter"
#include <KLocalizedString>

UsersModel::UsersModel(QObject *parent)
    : ExtraRowProxyModel(parent)
{
    setSourceModel(QSharedPointer<QAbstractItemModel>(new QLightDM::UsersModel(this)));
}

bool UsersModel::addUser(const QString &name, const QString &session)
{
    if (indexForUserName(name) >= 0) {
        return false;
    }

    const bool isGuest = name == QStringLiteral(GUEST_LOGIN_NAME);
    QStandardItem *item = new QStandardItem(isGuest ? i18n("Guest") : name);
    item->setData(name, QLightDM::UsersModel::NameRole);
    item->setData(session, QLightDM::UsersModel::SessionRole);
    if (!isGuest) {
        item->setData(QStringLiteral("/home/%1/.face").arg(name), QLightDM::UsersModel::ImagePathRole);
    }
    extraRowModel()->appendRow(item);
    return true;
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

QString UsersModel::sessionForUserName(const QString &name) const
{
    int row = indexForUserName(name);
    if (row < 0) {
        return QString();
    }
    return data(createIndex(row, 0), QLightDM::UsersModel::UserModelRoles::SessionRole).toString();
}
