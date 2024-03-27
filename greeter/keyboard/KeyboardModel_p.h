/***************************************************************************
* Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef KEYBOARDMODEL_P_H
#define KEYBOARDMODEL_P_H

#include <QtCore/QObject>

struct Indicator {
    bool enabled { false };
    uint8_t mask { 0 };
};

class KeyboardModelPrivate {
public:
    // is extension enabled
    bool enabled { true };

    // indicator state
    Indicator numlock, capslock;

    // Layouts
    int layout_id { 0 };
    QList<QObject*> layouts;
};

#endif // KEYBOARDMODEL_P_H
