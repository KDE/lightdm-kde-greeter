/*
This file is part of LightDM-KDE.

Copyright (C) 2022 Anton Golubev <golubevan@altlinux.org>

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
import QtQuick.VirtualKeyboard 2.1

InputPanel {
    id: inputPanel
    y: yPositionWhenHidden
    x: 0
    width: wholeScreen.width
    active: keyboardEnabled && inputMethodVisible
    desktopPanel: true

    keyboard.shadowInputControl.height: wholeScreen.height - keyboard.height

    property bool inputMethodVisible: Qt.inputMethod.visible
    property bool keyboardEnabled: false
    property real yPositionWhenHidden: wholeScreen.height

    function switchState() {
        keyboardEnabled = !keyboardEnabled
    }

    states: State {
        name: "visible"
        when: inputPanel.active
        PropertyChanges {
            target: inputPanel
            y: inputPanel.yPositionWhenHidden - inputPanel.height
        }
    }

    transitions: Transition {
        id: inputPanelTransition
        from: ""
        to: "visible"
        reversible: true
        enabled: true
        ParallelAnimation {
            NumberAnimation {
                properties: "y"
                duration: 270
                easing.type: Easing.InOutQuad
            }
        }
    }
}
