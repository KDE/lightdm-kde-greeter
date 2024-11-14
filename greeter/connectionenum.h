/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
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
        TYPE_WIRELESS,
        TYPE_VPN
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
        ACTION_PROVIDE_SECRET,
        ACTION_UNSUPPORTED,
        ACTION_FAILED_TO_CONNECT,
        ACTION_FAILED_TO_CONNECT_WITH_REASON,
        ACTION_ERROR_CANT_FIND_DEVICE,
        ACTION_ERROR_CANT_FIND_AP,
        ACTION_ERROR_CAN_CREATE_ONLY_WIRELESS,
    };
    Q_ENUM_NS(Action)
}

#endif // CONNECTIONENUM_H
