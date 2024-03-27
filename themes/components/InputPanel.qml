/*
This file is part of LightDM-KDE.

Copyright (C) 2022 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
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
