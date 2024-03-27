/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SESSIONSMODEL_H
#define SESSIONSMODEL_H

#include "extrarowproxymodel.h"

class SessionsModel: public ExtraRowProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool showLastUsedSession READ showLastUsedSession WRITE setShowLastUsedSession)

public:
    explicit SessionsModel(QObject *parent = nullptr);

    /** Add a row to the sessions model titled "Last Used Session" */
    void setShowLastUsedSession(bool showLastUsedSession);
    bool showLastUsedSession() const;
    Q_INVOKABLE int indexForSessionName(QString name) const;

private:
    bool m_showLastUsedSession;
};

#endif // SESSIONSMODEL_H
