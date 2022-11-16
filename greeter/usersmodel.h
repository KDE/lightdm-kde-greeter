/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

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
