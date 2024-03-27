/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "greeterwrapper.h"

#include <QEvent>
#include <QApplication>

#include <KConfig>
#include <KConfigGroup>

GreeterWrapper::GreeterWrapper(QObject *parent)
    : QLightDM::Greeter(parent)
{
    connectSync();
    m_config = KSharedConfig::openConfig(QStringLiteral("state-kde"));
    QCoreApplication::instance()->installEventFilter(this);
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

QString GreeterWrapper::lastLoggedInSession() const
{
    return m_config->group("lightdm").readEntry("lastSession");
}

QString GreeterWrapper::guestLoginName() const
{
    return QStringLiteral("*guest");
}

bool GreeterWrapper::startSessionSync(const QString &session)
{
    Q_EMIT aboutToLogin();
    saveLastUserAndSession(authenticationUser(), session);
    return QLightDM::Greeter::startSessionSync(session);
}

void GreeterWrapper::saveLastUserAndSession(const QString &user, const QString &session)
{
    m_config->group("lightdm").writeEntry("lastUser", user);
    m_config->group("lightdm").writeEntry("lastSession", session);
    //force a sync as our greeter gets killed
    m_config->sync();
}

bool GreeterWrapper::eventFilter(QObject*, QEvent *event)
{
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress || event->type() == QEvent::KeyPress) {
        qDebug("%s: autologin (if it was) canceled due to input event", Q_FUNC_INFO);
        QCoreApplication::instance()->removeEventFilter(this);
        m_allowAutologin = false;
    }
    return false;
}
