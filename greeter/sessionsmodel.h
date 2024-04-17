/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SESSIONSMODEL_H
#define SESSIONSMODEL_H

#include "extrarowproxymodel.h"
#include "config.h"

class SessionsModel: public ExtraRowProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool showLastUsedSession READ showLastUsedSession WRITE setShowLastUsedSession)

public:
    explicit SessionsModel(QObject *parent = nullptr);

#ifdef GREETER_WAYLAND_SESSIONS_FIRST
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        int mappedRow = m_indicesWaylandFirst.at(index.row());
        QModelIndex mappedIndex = index.model()->index(mappedRow, index.column());

        return ExtraRowProxyModel::data(mappedIndex, role);
    }
#endif

    /** Add a row to the sessions model titled "Last Used Session" */
    void setShowLastUsedSession(bool showLastUsedSession);
    bool showLastUsedSession() const;
    Q_INVOKABLE int indexForSessionName(QString name) const;

private:
    void remapIndicesWaylandFirst();

    bool m_showLastUsedSession;
    QList<int> m_indicesWaylandFirst;
};

#endif // SESSIONSMODEL_H
