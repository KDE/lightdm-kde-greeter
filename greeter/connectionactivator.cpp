/*
This file is part of LightDM-KDE.

Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QDBusInterface>
#include <QDBusPendingReply>

#include "connectionactivator.h"

ConnectionActivator::ConnectionActivator(QObject *parent) : QObject(parent)
{
}

void ConnectionActivator::wifiKeeperDBusCallResult(QDBusPendingCallWatcher *w)
{
    w->deleteLater();
    QDBusPendingReply<QString> reply = *w;
    if (reply.isError()) {
        qWarning("%s: Failed to call WifiKeeper on DBus: %s", __FUNCTION__, qPrintable(reply.error().message()));
    } else {
        QString result = reply.argumentAt<0>();
        if (result.length() == 0) qWarning("%s: WifiKeeper failed to call NetworkManager on DBus.", __FUNCTION__);
        else Q_EMIT wifiKeeperNewConnection(result);
    }
}

bool ConnectionActivator::addAndActivateConnection(NMVariantMapMap connection, QDBusObjectPath device, QDBusObjectPath specificObject, QVariantMap options)
{
    QDBusInterface qiface(QStringLiteral("org.altlinux.LightDMKdeGreeter.WifiKeeper"),
                          QStringLiteral("/"),
                          QStringLiteral("org.altlinux.LightDMKdeGreeter.WifiKeeper.interface"),
                          QDBusConnection::sessionBus());

    if (!qiface.isValid()) return false;
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(connection) << device << specificObject << options;
    QDBusPendingReply<> reply = qiface.asyncCallWithArgumentList(QStringLiteral("addAndActivateConnection"), argumentList);

    auto pendingCallWatcher = new QDBusPendingCallWatcher(reply, this);
    connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, &ConnectionActivator::wifiKeeperDBusCallResult);
    return true;
}
