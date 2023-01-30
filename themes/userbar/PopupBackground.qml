/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

LightDM-KDE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LightDM-KDE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LightDM-KDE.  If not, see <http://www.gnu.org/licenses/>.
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
