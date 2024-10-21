/*
This file is part of LightDM-KDE.

Copyright (C) 2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.TextField {

    rightPadding: height + screen.padding

    property bool revealPasswordButtonShown: false

    PlasmaComponents.ToolButton {
        id: eyeButton
        anchors {
            right: parent.right
            rightMargin: y
            verticalCenter: parent.verticalCenter
        }
        height: parent.height
        width: height
        icon.name: parent.echoMode == TextInput.Normal ? "password-show-off" : "password-show-on"
        visible: parent.revealPasswordButtonShown
        activeFocusOnTab: false

        onClicked: {
            parent.echoMode = parent.echoMode == TextInput.Normal ? TextInput.Password : TextInput.Normal
            parent.forceActiveFocus()
        }
    }
}
