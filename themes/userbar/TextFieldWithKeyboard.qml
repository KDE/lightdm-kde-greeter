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
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.TextField {
    id: root

    rightPadding: height

    TooltipButton {
        id: virtualKeyboardButton
        anchors {
            right: parent.right
            rightMargin: y
            verticalCenter: parent.verticalCenter
        }
        height: parent.height
        width: height
        icon.name: inputPanel.keyboardEnabled ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
        visible: inputPanel.item && (activeFocus || root.activeFocus)
        caption: i18nc("Button to show/hide virtual keyboard", "Virtual Keyboard")
        expand: false
        activeFocusOnTab: false

        onClicked: {
            inputPanel.switchState()
            root.forceActiveFocus()
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: 100 }
    }
}
