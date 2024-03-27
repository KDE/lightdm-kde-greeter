/***************************************************************************
* Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef XCBKEYBOARDBACKEND_H
#define XCBKEYBOARDBACKEND_H

#include <QtCore/QString>

#include "KeyboardBackend.h"

#define explicit explicit_is_keyword_in_cpp
#include <xcb/xkb.h>
#undef explicit

class QSocketNotifier;

class XcbKeyboardBackend : public KeyboardBackend {
public:
    XcbKeyboardBackend(KeyboardModelPrivate *kmp);
    virtual ~XcbKeyboardBackend();

    void init() override;
    void disconnect() override;
    void sendChanges() override;
    void dispatchEvents() override;

    void connectEventsDispatcher(KeyboardModel *model) override;

    static QList<QString> parseShortNames(QString text);

private:
    // Initializers
    void connectToDisplay();
    void initLedMap();
    void initLayouts();
    void initState();

    // Helpers
    QString atomName(xcb_atom_t atom) const;
    QString atomName(xcb_get_atom_name_cookie_t cookie) const;

    uint8_t getIndicatorMask(uint8_t id) const;

    // Connection
    xcb_connection_t *m_conn { nullptr };

    // Socket listener
    QSocketNotifier *m_socket { nullptr };
};

#endif // XCBKEYBOARDBACKEND_H
