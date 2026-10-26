/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022-2026 Anton Golubev <golubevan@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef USERSMODEL_H
#define USERSMODEL_H

#include "extrarowproxymodel.h"

class UsersModel: public ExtraRowProxyModel
{
    Q_OBJECT

public:
    explicit UsersModel(QObject *parent = nullptr);

    // Add a user row to the extra rows if not already present. Returns true if added.
    bool addUser(const QString &name, const QString &session);
    Q_INVOKABLE int indexForUserName(QString name) const;
    Q_INVOKABLE QString sessionForUserName(const QString &name) const;
};

#endif // USERSMODEL_H
