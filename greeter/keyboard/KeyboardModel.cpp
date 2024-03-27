/***************************************************************************
* Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "KeyboardModel.h"
#include "KeyboardModel_p.h"
#include "XcbKeyboardBackend.h"

/**********************************************/
/* KeyboardModel                              */
/**********************************************/

KeyboardModel::KeyboardModel(QObject *parent)
    : QObject(parent)
    , d(new KeyboardModelPrivate)
{
    m_backend = new XcbKeyboardBackend(d);
    m_backend->init();
    m_backend->connectEventsDispatcher(this);
}

KeyboardModel::~KeyboardModel() {
    m_backend->disconnect();
    delete m_backend;

    for (QObject *layout: d->layouts) {
        delete layout;
    }
    delete d;
}


bool KeyboardModel::numLockState() const {
    return d->numlock.enabled;
}

void KeyboardModel::setNumLockState(bool state) {
    if (d->numlock.enabled != state) {
        d->numlock.enabled = state;
        m_backend->sendChanges();

        emit numLockStateChanged();
    }
}

bool KeyboardModel::capsLockState() const {
    return d->capslock.enabled;
}

void KeyboardModel::setCapsLockState(bool state) {
    if (d->capslock.enabled != state) {
        d->capslock.enabled = state;
        m_backend->sendChanges();

        emit capsLockStateChanged();
    }
}

QList<QObject*> KeyboardModel::layouts() const {
    return d->layouts;
}

int KeyboardModel::currentLayout() const {
    return d->layout_id;
}

void KeyboardModel::setCurrentLayout(int id) {
    if (d->layout_id != id) {
        d->layout_id = id;
        m_backend->sendChanges();

        emit currentLayoutChanged();
    }
}

bool KeyboardModel::enabled() const {
    return d->enabled;
}

void KeyboardModel::dispatchEvents() {
    // Save old states
    bool num_old = d->numlock.enabled, caps_old = d->capslock.enabled;
    int layout_old = d->layout_id;
    QList<QObject*> layouts_old = d->layouts;

    // Process events
    m_backend->dispatchEvents();

    // Send updates
    if (caps_old != d->capslock.enabled)
        emit capsLockStateChanged();

    if (num_old != d->numlock.enabled)
        emit numLockStateChanged();

    if (layout_old != d->layout_id)
        emit currentLayoutChanged();

    if (layouts_old != d->layouts)
        emit layoutsChanged();
}

#include "moc_KeyboardModel.cpp"
