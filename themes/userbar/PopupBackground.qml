/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    property var border
    property alias fillOpacity: fill.opacity
    // for Plasma style compatibility
    property var margins: { "left": border, "right": border, "top": border, "bottom": border }

    Rectangle {
        id: fill
        anchors.fill: parent
        color: PlasmaCore.ColorScope.backgroundColor
        radius: root.border
        opacity: 0.6
    }

    Rectangle {
        id: edge
        anchors.fill: parent
        color: "transparent"
        border {
            color: PlasmaCore.ColorScope.textColor
            width: 2
        }
        radius: root.border
    }
}
