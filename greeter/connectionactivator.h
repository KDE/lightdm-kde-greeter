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
