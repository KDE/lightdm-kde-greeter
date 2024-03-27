/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "sessionsmodel.h"

#include <QLightDM/SessionsModel>

#include <KLocalizedString>

SessionsModel::SessionsModel(QObject *parent)
    : ExtraRowProxyModel(parent)
    , m_showLastUsedSession(false)
{
    setSourceModel(QSharedPointer<QAbstractItemModel>(new QLightDM::SessionsModel(this)));
}

void SessionsModel::setShowLastUsedSession(bool showLastUsedSession)
{
    if (showLastUsedSession == m_showLastUsedSession)
    {
        return;
    }

    m_showLastUsedSession = showLastUsedSession;

    if (m_showLastUsedSession)
    {
        QStandardItem *guest = new QStandardItem(i18n("Previously Used Session"));
        guest->setData(QStringLiteral(""), QLightDM::SessionsModel::KeyRole);
        extraRowModel()->appendRow(guest);
    }
    else
    {
        extraRowModel()->removeRow(0);
    }
}

bool SessionsModel::showLastUsedSession() const
{
    return m_showLastUsedSession;
}

int SessionsModel::indexForSessionName(QString name) const
{
    int i = 0;
    while (hasIndex(i, 0)) {
        if (data(index(i), QLightDM::SessionsModel::SessionModelRoles::IdRole) == name) {
            return i;
        }
        ++i;
    }
    return 0;
}
