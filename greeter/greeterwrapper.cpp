/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
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

#include "greeterwrapper.h"

#include <KConfig>
#include <KConfigGroup>

GreeterWrapper::GreeterWrapper(QObject *parent)
    : QLightDM::Greeter(parent)
{
    connectSync();
    m_config = KSharedConfig::openConfig(QStringLiteral("state-kde"));
}

QString GreeterWrapper::lastLoggedInUser() const
{
    //use suggested user from lightdm.conf if they exist, otherwise load from local config file of last logged in user.
    if (selectGuestHint())
    {
        return QStringLiteral("*guest");
    }

    if (!selectUserHint().isEmpty())
    {
        return selectUserHint();
    }

    return m_config->group("lightdm").readEntry("lastUser");
}

QString GreeterWrapper::guestLoginName() const
{
    return QStringLiteral("*guest");
}

bool GreeterWrapper::startSessionSync(const QString &session)
{
    Q_EMIT aboutToLogin();
    saveLastUser(authenticationUser());
    return QLightDM::Greeter::startSessionSync(session);
}

void GreeterWrapper::saveLastUser(const QString &user)
{
    m_config->group("lightdm").writeEntry("lastUser", user);
    //force a sync as our greeter gets killed
    m_config->sync();
}
