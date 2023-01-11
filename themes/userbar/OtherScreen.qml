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
    id: screen
    anchors.fill: parent
    PlasmaCore.FrameSvgItem {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: label.width + margins.left + margins.right
        height: label.height + margins.bottom
        imagePath: "widgets/background"
        enabledBorders: "LeftBorder|BottomBorder|RightBorder"

        TooltipButton {
            id: label
            anchors.horizontalCenter: parent.horizontalCenter
            icon.name: "info"
            caption: i18n("Press %1 to change the primary screen", "F1")
        }
    }
}
