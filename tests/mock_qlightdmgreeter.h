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

public:
    explicit Greeter(QObject *parent = nullptr) : QObject(parent) {}

    QString authenticationUser() const { return m_authenticationUser; }
    bool hasGuestAccountHint() const { return m_hasGuestAccount; }
    bool autologinGuestHint() const { return m_autologinGuest; }

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

    void setAuthenticationUser(const QString &user) { m_authenticationUser = user; }
    void setHasGuestAccountHint(bool enabled) { m_hasGuestAccount = enabled; }
    void setAutologinGuestHint(bool enabled) { m_autologinGuest = enabled; }

private:
    QString m_authenticationUser;
    bool m_hasGuestAccount = false;
    bool m_autologinGuest = false;
};

} // namespace QLightDM

#endif // MOCK_QLIGHTDMGREETER_H
