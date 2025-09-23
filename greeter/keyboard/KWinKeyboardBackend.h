/***************************************************************************
* Copyright (c) 2025 Anton Golubev <golubevan@altlinux.org>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef KWINKEYBOARDBACKEND_H
#define KWINKEYBOARDBACKEND_H

#include <QObject>
#include <QDBusReply>

#include "KeyboardBackend.h"

class LayoutNames;
class OrgKdeKeyboardLayoutsInterface;

class KWinKeyboardBackend :  public QObject, public KeyboardBackend {

    Q_OBJECT

public:

    KWinKeyboardBackend(KeyboardModelPrivate *kmp);
    virtual ~KWinKeyboardBackend();

    void init() override;
    void disconnect() override;
    void sendChanges() override;
    void dispatchEvents() override;
    void connectEventsDispatcher(KeyboardModel *model) override;

Q_SIGNALS:
    void layoutChanged();
    void layoutListChanged();

private:

    void initInterface();

    static const char *dbusService;
    static const char *dbusPath;

    enum DBusData {
        Layout,
        LayoutsList,
    };

    template<class T>
    void requestDBusData(QDBusPendingReply<T> pendingReply, T &out, void (KWinKeyboardBackend::*notify)());
    template<DBusData>
    void requestDBusData();
    void updateLayoutList();

    QList<LayoutNames> mLayoutsList;

    OrgKdeKeyboardLayoutsInterface *mIface;
    uint mLayout = 0;
};

#endif // KWINKEYBOARDBACKEND_H
