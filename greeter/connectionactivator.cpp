/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.ru>

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

#include <QDBusInterface>
#include <QDBusPendingReply>

#include "connectionactivator.h"

ConnectionActivator::ConnectionActivator(QObject *parent) : QObject(parent)
{
}

void ConnectionActivator::wifiKeeperDBusCallResult(QDBusPendingCallWatcher *w)
{
    w->deleteLater();
    QDBusPendingReply<> reply = *w;
    if (reply.isError()) {
        qWarning("%s: Failed to call WifiKeeper on DBus: %s", __FUNCTION__, qPrintable(reply.error().message()));
    } else {
        QString result = reply.argumentAt(0).toString();
        if (result.length() == 0) qWarning("%s: WifiKeeper failed to call NetworkManager on DBus.", __FUNCTION__);
        else emit wifiKeeperNewConnection(result);
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
