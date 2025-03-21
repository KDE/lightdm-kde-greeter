/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef GREETERWRAPPER_H
#define GREETERWRAPPER_H

#include <KSharedConfig>

#include <QLightDM/Greeter>

class GreeterWrapper: public QLightDM::Greeter
{
    Q_OBJECT

    Q_PROPERTY(QString lastLoggedInUser READ lastLoggedInUser CONSTANT)
    Q_PROPERTY(QString lastLoggedInSession READ lastLoggedInSession CONSTANT)
    Q_PROPERTY(QString guestLoginName READ guestLoginName CONSTANT)
    Q_PROPERTY(bool allowAutologin MEMBER m_allowAutologin)
    Q_PROPERTY(QString autologinSession READ autologinSessionHint)
    Q_PROPERTY(bool hideUsers READ hideUsersHint CONSTANT)
    Q_PROPERTY(bool showManualLogin READ showManualLoginHint CONSTANT)

public:
    explicit GreeterWrapper(QObject *parent = nullptr);

    QString lastLoggedInUser() const;
    QString lastLoggedInSession() const;
    QString guestLoginName() const;

Q_SIGNALS:
    void aboutToLogin();

    void fixupUsersList();
    void fixupUsersListQueued();

public Q_SLOTS:
    bool startSessionSync(const QString &session = QString());

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void saveLastUserAndSession(const QString &user, const QString &session);
    KSharedConfig::Ptr m_config;
    bool m_allowAutologin = true;
};

#endif // GREETERWRAPPER_H
