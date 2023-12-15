/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

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

#ifndef CONNECTIONITEM_H
#define CONNECTIONITEM_H

#include <QMetaEnum>
#include <NetworkManagerQt/AccessPoint>

#include "connectionenum.h"

class ConnectionItem
{
    Q_GADGET

public:

    Q_PROPERTY(QString name MEMBER name CONSTANT)
    Q_PROPERTY(QString path MEMBER path CONSTANT)
    Q_PROPERTY(ConnectionEnum::Type type MEMBER type CONSTANT)
    Q_PROPERTY(bool locked MEMBER locked CONSTANT)
    Q_PROPERTY(int signalStrength MEMBER signalStrength CONSTANT)
    Q_PROPERTY(ConnectionEnum::State state MEMBER state CONSTANT)
    Q_PROPERTY(int wpaFlags MEMBER wpaFlags CONSTANT)

    ConnectionItem &setName(const QString &x) { name = x; return *this; }
    ConnectionItem &setPath(const QString &x) { path = x; return *this; }
    ConnectionItem &setType(ConnectionEnum::Type x) { type = x; return *this; }
    ConnectionItem &setLock(bool x) { locked = x; return *this; }
    ConnectionItem &setSignalStrength(int x) { signalStrength = x; return *this; }
    ConnectionItem &setState(ConnectionEnum::State x) { state = x; return *this; }
    ConnectionItem &setWpaFlags(NetworkManager::AccessPoint::WpaFlags x) { wpaFlags = x; return *this; }

    Q_INVOKABLE QString actionName(ConnectionEnum::Action action)
    {
        return QLatin1String(QMetaEnum::fromType<ConnectionEnum::Action>().key(action));
    }

    operator QString() const {
        return QStringLiteral("%1 %2 %3 %4")
            .arg(name, 30)
            .arg(path, 30)
            .arg(QLatin1String(QMetaEnum::fromType<ConnectionEnum::Type>().key(type)), 18)
            .arg(QLatin1String(QMetaEnum::fromType<ConnectionEnum::State>().key(state)), 25);
    }

    bool operator==(const ConnectionItem &rhs) const {
        return name == rhs.name
            && path == rhs.path
            && type == rhs.type
            && locked == rhs.locked
            && signalStrength == rhs.signalStrength
            && state == rhs.state;
    }

    bool operator!=(const ConnectionItem &rhs) const {
        return !(*this == rhs);
    }

    QString name;
    QString path;
    ConnectionEnum::Type type;
    bool locked;
    int signalStrength;
    ConnectionEnum::State state;
    NetworkManager::AccessPoint::WpaFlags wpaFlags;

    bool mark;
};

Q_DECLARE_METATYPE(ConnectionItem)

#endif // CONNECTIONITEM_H
