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
    remapIndicesWaylandFirst();

    connect(this, &ExtraRowProxyModel::rowsInserted, this, &SessionsModel::remapIndicesWaylandFirst);
    connect(this, &ExtraRowProxyModel::rowsRemoved, this, &SessionsModel::remapIndicesWaylandFirst);
    connect(this, &ExtraRowProxyModel::dataChanged, this, &SessionsModel::remapIndicesWaylandFirst);
}

void SessionsModel::remapIndicesWaylandFirst()
{
    int lastWayland = 0;
    m_indicesWaylandFirst.clear();

    for (int i = 0; i < ExtraRowProxyModel::rowCount(); ++i) {
        QModelIndex cell = ExtraRowProxyModel::index(i, 0);
        auto sessionType = ExtraRowProxyModel::data(cell, QLightDM::SessionsModel::TypeRole).toString();
        if (sessionType == QStringLiteral("wayland")) {
            m_indicesWaylandFirst.insert(lastWayland++, i);
        } else {
            m_indicesWaylandFirst.push_back(i);
        }
    }
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
