/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CONNECTIONACTIVATOR_H
#define CONNECTIONACTIVATOR_H

#include <QObject>
#include <NetworkManagerQt/GenericTypes>

class QDBusPendingCallWatcher;

class ConnectionActivator: public QObject
{
    Q_OBJECT

public:
    explicit ConnectionActivator(QObject *parent = nullptr);
    bool addAndActivateConnection(NMVariantMapMap connection, QDBusObjectPath device, QDBusObjectPath specificObject, QVariantMap options);

Q_SIGNALS:
    void wifiKeeperNewConnection(QString newPath);

private Q_SLOTS:
    void wifiKeeperDBusCallResult(QDBusPendingCallWatcher *w);
};

#endif /* CONNECTIONACTIVATOR_H */
