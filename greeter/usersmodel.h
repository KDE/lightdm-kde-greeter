/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022 Anton Golubev <golubevan@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef USERSMODEL_H
#define USERSMODEL_H

#include "extrarowproxymodel.h"

class UsersModel: public ExtraRowProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool showGuest READ showGuest WRITE setShowGuest)

public:
    explicit UsersModel(QObject *parent = nullptr);

    /** Add a row to the sessions model titled "Last Used Session" */
    void setShowGuest(bool showGuest);
    bool showGuest() const;
    Q_INVOKABLE int indexForUserName(QString name) const;

private:
    bool m_showGuest;
};

#endif // SESSIONSMODEL_H
