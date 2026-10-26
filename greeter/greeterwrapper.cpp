/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2024-2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "greeterwrapper.h"

#include "accessibility.h"

#include <QEvent>
#include <QApplication>
#include <QLoggingCategory>

#include <KConfig>
#include <KConfigGroup>

#include <config.h>

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

    KConfig config(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"));
    m_lastUsersCount = config.group(QStringLiteral("greeter")).readEntry("last-users-count", 4);

    readAndValidateLastUsers();
}

void GreeterWrapper::readAndValidateLastUsers()
{
    m_lastUsers.clear();

    const QString usersData = m_config->group(QStringLiteral("lightdm")).readEntry("lastUser");
    const QString sessionsData = m_config->group(QStringLiteral("lightdm")).readEntry("lastSession");

    if (usersData.isEmpty() && sessionsData.isEmpty()) {
        return;
    }

    const QStringList userList = usersData.split(QLatin1Char(','));
    const QStringList sessionList = sessionsData.split(QLatin1Char(','));

    if (userList.size() != sessionList.size()) {
        qCWarning(lc,) << "Mismatch between lastUser size" << userList.size()
                      << "and lastSession size" << sessionList.size()
                      << ", clearing the remembered users list";
        return;
    }

    for (const QString &name : userList) {
        if (name.isEmpty()) {
            qCWarning(lc,) << "Empty user name found in lastUser, clearing the remembered users list";
            return;
        }
    }

    for (int i = 0; i < userList.size(); ++i) {
        m_lastUsers.append({userList[i], sessionList[i]});
    }

    if (m_lastUsers.size() > m_lastUsersCount) {
        m_lastUsers.resize(m_lastUsersCount);
    }
}

QStringList GreeterWrapper::lastLoggedInUsers() const
{
    QStringList users;
    for (const auto &entry : m_lastUsers) {
        users.append(entry.user);
    }
    return users;
}

QStringList GreeterWrapper::lastLoggedInSessions() const
{
    QStringList sessions;
    for (const auto &entry : m_lastUsers) {
        sessions.append(entry.session);
    }
    return sessions;
}

QString GreeterWrapper::guestLoginName() const
{
    return QStringLiteral("*guest");
}

bool GreeterWrapper::startSessionSync(const QString &session)
{
    Q_EMIT aboutToLogin();
    QString user = authenticationUser();

    // if the user string is empty here, it’s most likely a guest
    // also check that hints are enabled
    if (user.isEmpty() && (hasGuestAccountHint() || autologinGuestHint())) {
        user = guestLoginName();
    }
    saveLastUserAndSession(user, session);
    return QLightDM::Greeter::startSessionSync(session);
}

void GreeterWrapper::saveLastUserAndSession(const QString &user, const QString &session)
{
    if (user.isEmpty()) {
        qCWarning(lc,) << "Attempt to save an empty user name, ignoring";
        return;
    }

    for (int i = 0; i < m_lastUsers.size(); ++i) {
        if (m_lastUsers[i].user == user) {
            m_lastUsers.removeAt(i);
            break;
        }
    }

    m_lastUsers.prepend({user, session});

    if (m_lastUsers.size() > m_lastUsersCount) {
        m_lastUsers.resize(m_lastUsersCount);
    }

    QStringList usersToSave;
    QStringList sessionsToSave;
    for (const auto &entry : m_lastUsers) {
        usersToSave.append(entry.user);
        sessionsToSave.append(entry.session);
    }

    m_config->group(QStringLiteral("lightdm")).writeEntry("lastUser", usersToSave.join(QLatin1Char(',')));
    m_config->group(QStringLiteral("lightdm")).writeEntry("lastSession", sessionsToSave.join(QLatin1Char(',')));

    // force a sync as our greeter gets killed
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
