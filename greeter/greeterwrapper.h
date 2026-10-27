/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GREETERWRAPPER_H
#define GREETERWRAPPER_H

#include <KSharedConfig>

#ifndef MOCK_INCLUDES
    #include <QLightDM/Greeter>
#else
    #include "mock_qlightdmgreeter.h"
#endif

class Accessibility;

class GreeterWrapper: public QLightDM::Greeter
{
    Q_OBJECT

    Q_PROPERTY(QStringList lastLoggedInUsers READ lastLoggedInUsers CONSTANT)
    Q_PROPERTY(QStringList lastLoggedInSessions READ lastLoggedInSessions CONSTANT)
    Q_PROPERTY(QString guestLoginName READ guestLoginName CONSTANT)
    Q_PROPERTY(bool allowAutologin MEMBER m_allowAutologin)
    Q_PROPERTY(QString autologinSession READ autologinSessionHint)
    Q_PROPERTY(bool hideUsers READ hideUsersHint CONSTANT)
    Q_PROPERTY(bool showManualLogin READ showManualLoginHint CONSTANT)
    Q_PROPERTY(QString platformName READ getPlatformName CONSTANT)
    Q_PROPERTY(Accessibility *accessibility MEMBER m_accessibility)

public:
    explicit GreeterWrapper(QObject *parent = nullptr);

    QStringList lastLoggedInUsers() const;
    QStringList lastLoggedInSessions() const;
    QString guestLoginName() const;
    QString getPlatformName() const;

    QString getAccessibilitySetting(const QString &key) const;
    void setAccessibilitySetting(const QString &key, const QString &value);

Q_SIGNALS:
    void aboutToLogin();

    void fixupUsersList();
    void fixupUsersListQueued();

public Q_SLOTS:
    bool startSessionSync(const QString &session = QString());

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    struct LastUser {
        QString user;
        QString session;
    };

    void readAndValidateLastUsers();
    void saveLastUserAndSession(const QString &user, const QString &session);
    KSharedConfig::Ptr m_config;
    int m_lastUsersCount = 4;
    QList<LastUser> m_lastUsers;
    bool m_allowAutologin = true;
    Accessibility *m_accessibility;
};

#endif // GREETERWRAPPER_H
