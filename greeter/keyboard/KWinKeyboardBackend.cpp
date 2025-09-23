/***************************************************************************
* Copyright (c) 2025 Anton Golubev <golubevan@altlinux.org>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "KWinKeyboardBackend.h"

#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QLoggingCategory>

#include "KeyboardLayout.h"
#include "KeyboardModel.h"
#include "KeyboardModel_p.h"

#include "keyboard_layout_interface.h"

static const QLoggingCategory lc("KWinKeyboardBackend");
using namespace Qt::StringLiterals;

const char *KWinKeyboardBackend::dbusService = "org.kde.keyboard";
const char *KWinKeyboardBackend::dbusPath = "/Layouts";

KWinKeyboardBackend::~KWinKeyboardBackend()
{
}

KWinKeyboardBackend::KWinKeyboardBackend(KeyboardModelPrivate *kmp) : KeyboardBackend(kmp)
{
}

template<>
inline void KWinKeyboardBackend::requestDBusData<KWinKeyboardBackend::Layout>()
{
    if (mIface)
        requestDBusData(mIface->getLayout(), mLayout, &KWinKeyboardBackend::layoutChanged);
}

template<>
inline void KWinKeyboardBackend::requestDBusData<KWinKeyboardBackend::LayoutsList>()
{
    if (mIface)
        requestDBusData(mIface->getLayoutsList(), mLayoutsList, &KWinKeyboardBackend::layoutListChanged);
}

template<class T>
void KWinKeyboardBackend::requestDBusData(QDBusPendingReply<T> pendingReply, T &out, void (KWinKeyboardBackend::*notify)())
{
    connect(new QDBusPendingCallWatcher(pendingReply, this), &QDBusPendingCallWatcher::finished, this, [this, &out, notify](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<T> reply = *watcher;
        if (reply.isError()) {
            qCWarning(lc,) << reply.error().message();
        }
        out = reply.value();
        Q_EMIT(this->*notify)();

        watcher->deleteLater();
    });
}

void KWinKeyboardBackend::init()
{
    LayoutNames::registerMetaType();

    auto watcher = new QDBusServiceWatcher(QLatin1String(dbusService), QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForRegistration, this);
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &KWinKeyboardBackend::initInterface);

    // attempt to connect in case service already exists
    initInterface();
}

void KWinKeyboardBackend::initInterface()
{
    disconnect();

    mIface = new OrgKdeKeyboardLayoutsInterface(QLatin1String(dbusService), QLatin1String(dbusPath), QDBusConnection::sessionBus(), this);
    if (!mIface->isValid()) {
        qCWarning(lc,) << "failed to create DBus interface:" << OrgKdeKeyboardLayoutsInterface::staticInterfaceName() << "service:" << dbusService << "path:" << dbusPath;
        delete mIface;
        mIface = nullptr;
        return;
    }

    connect(mIface, &OrgKdeKeyboardLayoutsInterface::layoutChanged, this, [this](uint index) {
        mLayout = index;
        Q_EMIT layoutChanged();
    });

    connect(mIface, &OrgKdeKeyboardLayoutsInterface::layoutListChanged, this, [this]() {
        requestDBusData<LayoutsList>();
        requestDBusData<Layout>();
    });

    Q_EMIT mIface->OrgKdeKeyboardLayoutsInterface::layoutListChanged();
}

void KWinKeyboardBackend::disconnect()
{
    if (mIface) {
        delete mIface;
        mIface = nullptr;
    }
}

void KWinKeyboardBackend::sendChanges()
{
    if (!mIface) return;

    mLayout = d->layout_id;

    auto pendingReply = mIface->setLayout(mLayout);
    connect(new QDBusPendingCallWatcher(pendingReply, this), &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
        decltype(pendingReply) reply = *watcher;
        if (reply.isError()) {
            qCWarning(lc,) << reply.error().message();
        } else if (!reply.value()) {
            qCWarning(lc,) << "false in result of setLayout on interface" << OrgKdeKeyboardLayoutsInterface::staticInterfaceName();
        }

        watcher->deleteLater();
    });
}

void KWinKeyboardBackend::dispatchEvents()
{
    if (mLayout != (uint)d->layout_id) {
        d->layout_id = mLayout;
    }

    if (mLayoutsList.size() != d->layouts.size()) {
        updateLayoutList();
    } else {
        for (uint i = 0; i < mLayoutsList.size(); ++i) {
            const auto &x = mLayoutsList[i].shortName;
            const auto &y = static_cast<KeyboardLayout*>(d->layouts[i])->shortName();
            if (x != y) {
                updateLayoutList();
                break;
            }
        }
    }
}

void KWinKeyboardBackend::connectEventsDispatcher(KeyboardModel *model)
{
    QObject::connect(this, SIGNAL(layoutChanged()), model, SLOT(dispatchEvents()));
    QObject::connect(this, SIGNAL(layoutListChanged()), model, SLOT(dispatchEvents()));
}

void KWinKeyboardBackend::updateLayoutList()
{
    d->layouts.clear();
    for (auto &thing : mLayoutsList) {
        d->layouts.emplace_back(new KeyboardLayout{ thing.shortName, thing.longName });
    }
}
