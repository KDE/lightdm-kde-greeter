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

#ifndef CONNECTIONENUM_H
#define CONNECTIONENUM_H

#include <QMetaEnum>

namespace ConnectionEnum
{
    Q_NAMESPACE

    enum Type {
        TYPE_UNDEFINED,
        TYPE_NONE,
        TYPE_WIRED,
        TYPE_WIRELESS
    };
    Q_ENUM_NS(Type)

    enum State {
        STATE_OFF,
        STATE_WAIT,
        STATE_ON
    };
    Q_ENUM_NS(State)

    enum FlagBits {
        FLAG_PRIVATE = 0x01,
        FLAG_LOCKED  = 0x02
    };
    Q_ENUM_NS(FlagBits)

    enum Action {
        ACTION_NONE,
        ACTION_DISCONNECT,
        ACTION_ABORT_CONNECTING,
        ACTION_CONNECT,
        ACTION_CONNECT_FREE_WIFI,
        ACTION_CONNECT_WITH_PSK,
        ACTION_CONNECT_8021X_WIFI,
        ACTION_ERROR_8021X_WIFI,
        ACTION_ERROR_RETYPE_PSK,
        ACTION_UNSUPPORTED,
        ACTION_FAILED_TO_CONNECT,
        ACTION_ERROR_CANT_FIND_DEVICE,
        ACTION_ERROR_CANT_FIND_AP,
        ACTION_ERROR_CAN_CREATE_ONLY_WIRELESS,
    };
    Q_ENUM_NS(Action)
}

#endif // CONNECTIONENUM_H
