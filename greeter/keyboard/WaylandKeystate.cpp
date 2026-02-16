/***************************************************************************
* Copyright (c) 2026 Anton Golubev <golubevan@altlinux.org>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "WaylandKeystate.h"
#include <QtWaylandClient/private/qwayland-wayland.h>
#include <QGuiApplication>
#include <QLoggingCategory>

static const QLoggingCategory lc("WaylandKeystate");

class WaylandRegistryListener;

class WaylandKeyboardListener : public QtWayland::wl_keyboard
{
public:
    WaylandKeyboardListener(struct ::wl_keyboard *keyboard, WaylandKeystate *keystate)
        : QtWayland::wl_keyboard(keyboard)
        , m_keystate(keystate)
    {}

private:
    void keyboard_modifiers(uint32_t /*serial*/, uint32_t /*mods_depressed*/, uint32_t /*mods_latched*/, uint32_t mods_locked, uint32_t /*group*/) override
    {
        // only interested in these bits
        WaylandKeystate::XkbModifierMask needed{ WaylandKeystate::Lock | WaylandKeystate::Num };

        m_keystate->setState(mods_locked & needed);
    }

    WaylandKeystate *m_keystate;
};

class WaylandSeatListener : public QtWayland::wl_seat
{
public:
    WaylandSeatListener(struct ::wl_seat *seat, WaylandKeystate *keystate)
        : QtWayland::wl_seat(seat)
        , m_keystate(keystate)
    {}

private:
    void seat_capabilities(uint32_t capabilities) override
    {
        if (capabilities & wl_seat::capability::capability_keyboard) {
            m_keyListener.reset(new WaylandKeyboardListener(get_keyboard(), m_keystate));
        } else {
            qCWarning(lc,) << "Seat does not have a keyboard in capabilities, won't be able to listen";
        }
    }
    std::unique_ptr<WaylandKeyboardListener> m_keyListener;
    WaylandKeystate *m_keystate;
};

class WaylandRegistryListener : public QtWayland::wl_registry
{
public:
    WaylandRegistryListener(struct ::wl_registry *registry, WaylandKeystate *keystate)
        : QtWayland::wl_registry(registry)
        , m_keystate(keystate)
    {}

private:
    void registry_global(uint32_t name, const QString &interface, uint32_t version) override
    {
        if (!m_seat && interface == QString::fromLatin1(QtWayland::wl_seat::interface()->name)) {
            auto seat = (struct ::wl_seat*)bind(name, QtWayland::wl_seat::interface(), version);
            if (!seat) {
                qCWarning(lc,) << "The wl_seat event has arrived, but the binding fails";
                return;
            }
            m_seat.reset(new WaylandSeatListener(seat, m_keystate));
        }
    }
    std::unique_ptr<WaylandSeatListener> m_seat;
    WaylandKeystate *m_keystate;
};

WaylandKeystate::WaylandKeystate(QObject *parent)
    : QObject(parent)
    , m_state(0)
{
    auto interface = qApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!interface) {
        qCWarning(lc,) << "Can't get wayland native interface, won't be able to listen";
        return;
    }

    struct ::wl_display *display = interface->display();
    if (!display) {
        qCWarning(lc,) << "Can't get display, won't be able to listen";
        return;
    }

    struct ::wl_registry *registry = wl_display_get_registry(display);
    if (!registry) {
        qCWarning(lc,) << "Can't get registry, won't be able to listen";
        return;
    }

    m_registryListener.reset(new WaylandRegistryListener(registry, this));
}

WaylandKeystate::~WaylandKeystate()
{
}

void WaylandKeystate::setState(XkbModifierMask state)
{
    if (m_state != state) {
        m_state = state;
        Q_EMIT updated();
    }
}

