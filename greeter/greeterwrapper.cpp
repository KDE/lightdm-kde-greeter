/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2024-2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "greeterwrapper.h"

#include "accessibility.h"

#include <QEvent>
#include <QApplication>
#include <QLoggingCategory>

#include <KConfig>
#include <KConfigGroup>

static const QLoggingCategory lc("GreeterWrapper");
using namespace Qt::StringLiterals;

static const QString s_accessibilityGroup = u"accessibility"_s;

GreeterWrapper::GreeterWrapper(QObject *parent)
    : QLightDM::Greeter(parent)
{
    connectSync();
    m_config = KSharedConfig::openConfig(QStringLiteral("state-kde"));
    m_accessibility = new Accessibility(this);
    QCoreApplication::instance()->installEventFilter(this);
    connect(this, &GreeterWrapper::fixupUsersList, this, &GreeterWrapper::fixupUsersListQueued, Qt::ConnectionType::QueuedConnection);
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

    return m_config->group(QStringLiteral("lightdm")).readEntry("lastUser");
}

QString GreeterWrapper::lastLoggedInSession() const
{
    return m_config->group(QStringLiteral("lightdm")).readEntry("lastSession");
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
    m_config->group(QStringLiteral("lightdm")).writeEntry("lastUser", user);
    m_config->group(QStringLiteral("lightdm")).writeEntry("lastSession", session);
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

QString GreeterWrapper::getPlatformName() const
{
    return QGuiApplication::platformName();
}

QString GreeterWrapper::getAccessibilitySetting(const QString &key) const
{
    if (!m_config) {
        qCWarning(lc,) << "attempt to read from an uninitialized config";
        return {};
    }
    return m_config->group(s_accessibilityGroup).readEntry(key);
}

void GreeterWrapper::setAccessibilitySetting(const QString &key, const QString &value)
{
    if (!m_config) {
        qCWarning(lc,) << "attempt to save to an uninitialized config";
        return;
    }
    m_config->group(s_accessibilityGroup).writeEntry(key, value);
}
