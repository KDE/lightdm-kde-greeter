/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef MOCK_QLIGHTDMGREETER_H
#define MOCK_QLIGHTDMGREETER_H

#include <QObject>
#include <QString>

namespace QLightDM
{

class Greeter : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool authenticated READ isAuthenticated NOTIFY authenticationComplete)
    Q_PROPERTY(QString selectUser READ selectUserHint NOTIFY selectUserChanged)
    Q_PROPERTY(bool selectGuestHint READ selectGuestHint NOTIFY selectGuestHintChanged)

public:
    explicit Greeter(QObject *parent = nullptr) : QObject(parent) {}

    bool isAuthenticated() const { return m_authenticated; }
    QString authenticationUser() const { return m_authenticationUser; }
    bool hasGuestAccountHint() const { return m_hasGuestAccount; }
    bool autologinGuestHint() const { return m_autologinGuest; }
    QString selectUserHint() const { return m_selectUser; }
    bool selectGuestHint() const { return m_selectGuest; }

    QString autologinSessionHint() const { return {}; }
    bool hideUsersHint() const { return false; }
    bool showManualLoginHint() const { return true; }

public Q_SLOTS:
    bool connectSync() { return true; }
    bool startSessionSync(const QString &session = QString())
    {
        Q_UNUSED(session);
        return true;
    }

    void setAuthenticated(bool authenticated) { m_authenticated = authenticated; }
    void setAuthenticationUser(const QString &user) { m_authenticationUser = user; }
    void setHasGuestAccountHint(bool enabled) { m_hasGuestAccount = enabled; }
    void setAutologinGuestHint(bool enabled) { m_autologinGuest = enabled; }
    void setSelectUserHint(const QString &user) {
        if (m_selectUser != user) {
            m_selectUser = user;
            Q_EMIT selectUserChanged();
        }
    }
    void setSelectGuestHint(bool enabled) {
        if (m_selectGuest != enabled) {
            m_selectGuest = enabled;
            Q_EMIT selectGuestHintChanged();
        }
    }

    void authenticate(const QString &username = QString())
    {
        Q_UNUSED(username);
    }
    void authenticateAsGuest() {}
    void authenticateAutologin() {}
    void respond(const QString &response)
    {
        Q_UNUSED(response);
    }
    void cancelAuthentication() {}

Q_SIGNALS:
    void showMessage(QString text, int type);
    void showPrompt(QString text, int type);
    void authenticationComplete();
    void autologinTimerExpired();
    void selectUserChanged();
    void selectGuestHintChanged();

private:
    bool m_authenticated = false;
    QString m_authenticationUser;
    bool m_hasGuestAccount = false;
    bool m_autologinGuest = false;
    QString m_selectUser;
    bool m_selectGuest = false;
};

} // namespace QLightDM

#endif // MOCK_QLIGHTDMGREETER_H
