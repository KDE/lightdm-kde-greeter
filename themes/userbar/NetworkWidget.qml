/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

import ConnectionEnum 1.0

TooltipButton {
    id: root

    property bool needHint: connectionsModel.primary && (connectionsModel.primary.flags & ConnectionEnum.FLAG_PRIVATE)
    property bool needCaption: !expand || width < implicitWidth
    property string hint: i18n("The connection will be active only on the login screen.")

    caption: connectionsModel.primary.name
    width: Math.min(approximateFullWidth, implicitWidth)
    icon.name: connectionIcon(connectionsModel.primary)
    ToolTip.visible: hovered && (needCaption || needHint)
    ToolTip.text: (needCaption ? caption : "") + (needHint && needCaption ? "\n" : "" ) + (needHint ? hint : "")

    property int approximateFullWidth
    property int gap: screen.padding
    property int iconSize: PlasmaCore.Units.iconSizes.smallMedium

    Connections {
        target: connectionsModel

        function onShowDialog(item, action) {

            confirmAction.data = { "connection": item, "action": action }
            confirmAction.callback = connectionsModel.onActionDialogComplete
            confirmLayout.sourceComponent = labelLayout

            switch (action) {
            case ConnectionEnum.ACTION_NONE: return;
            case ConnectionEnum.ACTION_DISCONNECT:
                confirmAction.text = i18n("Disconnect from %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_ABORT_CONNECTING:
                confirmAction.text = i18n("Abort connecting to %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT:
                confirmAction.text = i18n("Connect to %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT_FREE_WIFI:
                confirmAction.text = i18n("Connect to an insecure access point %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT_WITH_PSK:
                confirmLayout.sourceComponent = pskLayout
                confirmAction.text = i18n("Enter the password to connect to %1", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT_8021X_WIFI:
                confirmLayout.sourceComponent = eapLayout
                confirmAction.text = i18n("Enter login and password to connect to %1", item.name)
            break
            case ConnectionEnum.ACTION_ERROR_8021X_WIFI:
                confirmLayout.sourceComponent = eapLayout
                confirmAction.text = i18n("%1: Connection error, try again.", item.name)
                confirmLayout.item.identityField.text = confirmLayout.lastUser
                confirmLayout.item.focusTo = confirmLayout.item.secretField
            break
            case ConnectionEnum.ACTION_ERROR_RETYPE_PSK:
                confirmLayout.sourceComponent = pskLayout
                confirmAction.text = i18n("%1: Connection error, try again.", item.name)
            break
            case ConnectionEnum.ACTION_UNSUPPORTED:
                confirmAction.text = i18n("The security protocol is not supported for %1", item.name)
            break
            case ConnectionEnum.ACTION_FAILED_TO_CONNECT:
                confirmAction.text = i18n("Failed to connect to %1", item.name)
            break
            case ConnectionEnum.ACTION_ERROR_CANT_FIND_AP:
                confirmAction.text = i18n("Error: Can't find access point %1", item.name)
            break
                default: console.warn("NetworkWidget: error: " + item.actionName(action)); return
            }

            confirmAction.open()
            if (confirmLayout.item.focusTo) confirmLayout.item.focusTo.forceActiveFocus()
        }
    }

    signal popupHide()

    function connectionIcon(item) {
        switch (item.type) {
            case ConnectionEnum.TYPE_NONE: return "connect_no"
            case ConnectionEnum.TYPE_WIRED: return "network-wired";
            case ConnectionEnum.TYPE_WIRELESS: return iconByStrength(item.signalStrength);
            default: return "question";
        }
    }

    function iconByStrength(strength) {
        if (strength > 75) return "network-wireless-signal-excellent"
        if (strength > 50) return "network-wireless-signal-good"
        if (strength > 25) return "network-wireless-signal-ok"
        if (strength > 1) return "network-wireless-signal-weak"
        return "network-wireless-signal-none"
    }

    function itemClicked(item) {
        connectionsModel.onItemClick(item)
    }

    onClicked: {
        screen.startDefaultScreen()
        connectionsModel.requestScanWifi()
        setTabOrder([ switchWireless, switchNetworking, networkList ])
        popup.open()
        popup.forceActiveFocus()
    }

    Dialog {
        id: confirmAction

        parent: activeScreen
        anchors.centerIn: parent
        // shrink the psk dialog vertically so that the input field is at the level of the buttons
        height: contentHeight + topPadding + bottomPadding + confirmFooter.contentHeight + bottomPadding
        // if the width is too small, the buttons also start to shrink and hide the content
        width: Math.max(implicitWidth, layoutType == pskLayout ? confirmFooter.minWidth + layout.focusTo.width : confirmFooter.minWidth + back.border * 2)

        closePolicy: Popup.CloseOnEscape

        property var text: ""
        property var data
        property var callback
        property var layoutType: confirmLayout.sourceComponent
        property var layout: confirmLayout.item

        onAccepted: {
            if (layout.grabData) layout.grabData(data)
            callback(data)
            popup.forceActiveFocus()
        }

        onRejected: popup.forceActiveFocus()

        background: PopupBackground {
            id: back
            border: gap * 2
            fillOpacity: 0.8
        }

        Loader {
            id: confirmLayout
            // to remember username for 802.1x, in case of re-entry
            property string lastUser
        }

        footer: DialogButtonBox {
            id: confirmFooter
            property int minWidth: contentWidth + leftPadding + rightPadding + spacing * 2
            standardButtons: Dialog.Ok | Dialog.Cancel
            buttonLayout: DialogButtonBox.KdeLayout
            alignment: Qt.AlignBottom | Qt.AlignCenter
        }
    }

    // switchable layouts for confirmAction dialog

    Component {
        id: labelLayout

        PlasmaComponents.Label {
            id: label
            width: implicitWidth + gap * 2
            height: implicitHeight + gap * 2
            PlasmaCore.ColorScope.colorGroup: screen.colorGroup
            text: confirmAction.text
        }
    }

    Component {
        id: pskLayout

        Column {
            function grabData(data) {
                data.password = secretField.text
            }

            spacing: gap * 2
            property var focusTo: secretField
            PlasmaComponents.Label {
                id: label
                width: implicitWidth + gap * 2
                height: implicitHeight + gap * 2
                PlasmaCore.ColorScope.colorGroup: screen.colorGroup
                text: confirmAction.text
            }
            PlasmaComponents.TextField {
                id: secretField
                anchors.horizontalCenter: parent.horizontalCenter
                width: 8 * gridUnit
                echoMode: TextInput.Password
                revealPasswordButtonShown: true
                Keys.onReturnPressed: confirmAction.accept()
            }
        }
    }

    Component {
        id: eapLayout

        Column {
            function grabData(data) {
                data.identity = identityField.text
                data.password = secretField.text
                confirmLayout.lastUser = identityField.text
            }

            property var focusTo: identityField
            property alias identityField: identityField
            property alias secretField: secretField

            width: Math.max(label.width, loginRow.width, passwordRow.width)
            spacing: gap * 2

            PlasmaComponents.Label {
                id: label
                width: implicitWidth + gap * 2
                height: implicitHeight + gap * 2
                PlasmaCore.ColorScope.colorGroup: screen.colorGroup
                text: confirmAction.text
            }

            Column {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: gap * 2
                Row {
                    id: loginRow
                    spacing: gap
                    anchors.right: parent.right
                    width: childrenRect.width
                    PlasmaComponents.Label {
                        anchors.verticalCenter: parent.verticalCenter
                        PlasmaCore.ColorScope.colorGroup: screen.colorGroup
                        text: i18n("Login")
                    }
                    PlasmaComponents.TextField {
                        id: identityField
                        width: 8 * gridUnit
                    }
                }
                Row {
                    id: passwordRow
                    spacing: gap
                    anchors.right: parent.right
                    width: childrenRect.width
                    PlasmaComponents.Label {
                        anchors.verticalCenter: parent.verticalCenter
                        PlasmaCore.ColorScope.colorGroup: screen.colorGroup
                        text: i18n("Password")
                    }
                    PlasmaComponents.TextField {
                        id: secretField
                        width: 8 * gridUnit
                        echoMode: TextInput.Password
                        revealPasswordButtonShown: true
                        Keys.onReturnPressed: confirmAction.accept()
                    }
                }
            }
        }
    }

    Popup {
        id: popup
        width: Math.min(gridUnit * 15, screen.width * 0.5)
        height: Math.min(gridUnit * 30, screen.height * 0.7, activeScreen.height - gap * 2)

        x: Math.min(0, parent.parent.width - parent.x - width)
        y: parent.height + gap

        property bool onLine: connectionsModel.networkingEnabled

        closePolicy: confirmAction.visible ? Popup.CloseOnEscape : Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onAboutToHide: root.popupHide()

        background: PopupBackground {
            border: gap * 2
        }

        Row {
            id: interfaceButtons
            anchors.right: parent.right

            TooltipButton {
                id: switchWireless

                visible: popup.onLine
                width: iconSize + gap * 2
                height: width
                expand: false
                caption: connectionsModel.wirelessEnabled ? i18n("Disable wireless") : i18n("Enable wireless");

                PlasmaCore.IconItem {
                    x: gap
                    y: gap
                    width: iconSize
                    height: width
                    colorGroup: screen.colorGroup
                    source: "network-wireless"
                    opacity: connectionsModel.wirelessEnabled ? 1.0 : 0.5
                }
                onClicked: connectionsModel.wirelessEnabled = !connectionsModel.wirelessEnabled
            }

            TooltipButton {
                id: switchNetworking

                width: iconSize + gap * 2
                height: width
                expand: false
                caption: connectionsModel.networkingEnabled ? i18n("Disable networking") : i18n("Enable networking");

                PlasmaCore.IconItem {
                    x: gap
                    y: gap
                    width: iconSize
                    height: width
                    colorGroup: screen.colorGroup
                    source: "system-shutdown"
                    opacity: connectionsModel.networkingEnabled ? 1.0 : 0.5
                }
                onClicked: connectionsModel.networkingEnabled = !connectionsModel.networkingEnabled
            }
        }

        ScrollView {
            visible: popup.onLine
            clip: true
            anchors {
                top: interfaceButtons.bottom
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            ListView {
                id: networkList
                model: connectionsModel

                delegate: Item {
                    id: connectionDelegate
                    width: ListView.view.width
                    height: connectionEntry.height + gap
                    property bool highlighted: mouseArea.containsMouse || (ListView.view.currentIndex == index && ListView.view.activeFocus)

                    Row {
                        id: connectionEntry
                        x: gap / 2
                        y: gap / 2
                        spacing: gap
                        height: childrenRect.height
                        width: parent.width - gap

                        PlasmaCore.IconItem {
                            colorGroup: screen.colorGroup
                            opacity: {
                                if (model == null) return 1.0
                                switch (model.item.state) {
                                    case ConnectionEnum.STATE_ON: return 1.0
                                    case ConnectionEnum.STATE_WAIT: return 0.5
                                    case ConnectionEnum.STATE_OFF: return 0.0
                                }
                                console.warn("NetworkWidget: unknown ConnectionEnum state: " + model.item.state)
                                return 1.0
                            }
                            source: "link"
                        }

                        PlasmaCore.IconItem {
                            id: connectionType
                            colorGroup: screen.colorGroup
                            source: connectionIcon(model.item)
                            animated: false
                        }

                        PlasmaCore.IconItem {
                            colorGroup: screen.colorGroup
                            visible: model != null && model.item.type == ConnectionEnum.TYPE_WIRELESS
                            source: model.item.flags & ConnectionEnum.FLAG_LOCKED ? "object-locked" : "object-unlocked"
                        }

                        PlasmaComponents.Label {
                            id: connectionName
                            PlasmaCore.ColorScope.colorGroup: screen.colorGroup
                            text: model.item.name
                            width: parent.width - x
                            elide: Text.ElideRight
                            anchors.verticalCenter: connectionType.verticalCenter
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        visible: highlighted
                        color: PlasmaCore.ColorScope.highlightColor
                        opacity: mouseArea.pressed ? 0.3 : 0.0
                        radius: gap / 2
                        Behavior on opacity {
                            NumberAnimation { duration: 100 }
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        visible: highlighted
                        color: "transparent"
                        radius: gap / 2
                        border {
                            color: PlasmaCore.ColorScope.highlightColor
                            width: 1
                        }
                    }

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: itemClicked(item)
                        onEntered: connectionDelegate.ListView.view.currentIndex = index
                    }

                    Keys.onSpacePressed: itemClicked(item)

                    property bool needHint: item && (item.flags & ConnectionEnum.FLAG_PRIVATE) && item.state == ConnectionEnum.STATE_ON
                    property bool needCaption: connectionName.width < connectionName.implicitWidth

                    ToolTip.delay: params.toolTipDelay
                    ToolTip.timeout: params.toolTipTimeout
                    ToolTip.visible: mouseArea.containsMouse && (needHint || needCaption)
                    ToolTip.text: (needCaption ? connectionName.text : "") + (needCaption && needHint ? "\n" : "") + (needHint ? hint : "")
                }
            }
        }
    }
}
