/***************************************************************************
* Copyright (c) 2026 Anton Golubev <golubevan@altlinux.org>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include <QObject>

#ifndef WAYLANDKEYSTATE_H
#define WAYLANDKEYSTATE_H


class WaylandRegistryListener;

class WaylandKeystate : public QObject
{
    Q_OBJECT

public:
    WaylandKeystate(QObject *parent);
    ~WaylandKeystate() override;

    // borrowed from KWin::Xkb::Modifiers
    enum XkbModifier {
        NoModifier = 0,
        Shift = 1 << 0,
        Lock = 1 << 1,
        Control = 1 << 2,
        Mod1 = 1 << 3,
        Num = 1 << 4,
        Mod3 = 1 << 5,
        Mod4 = 1 << 6,
        Mod5 = 1 << 7,
    };
    using XkbModifierMask = std::underlying_type_t<XkbModifier>;

    void setState(XkbModifierMask state);
    XkbModifierMask getState() { return m_state; }

Q_SIGNALS:
    void updated();

private:
    std::unique_ptr<WaylandRegistryListener> m_registryListener;
    XkbModifierMask m_state;
};

#endif // WAYLANDKEYSTATE_H
