/***************************************************************************
* Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef KEYBOARDBACKEND_H
#define KEYBOARDBACKEND_H

class KeyboardModel;
class KeyboardModelPrivate;

class KeyboardBackend {
public:
    KeyboardBackend(KeyboardModelPrivate *kmp) : d(kmp) {}

    virtual ~KeyboardBackend() {}

    virtual void init() = 0;
    virtual void disconnect() = 0;
    virtual void sendChanges() = 0;
    virtual void dispatchEvents() = 0;

    virtual void connectEventsDispatcher(KeyboardModel *model) = 0;

protected:
    KeyboardModelPrivate *d;
};

#endif // KEYBOARDBACKEND_H
