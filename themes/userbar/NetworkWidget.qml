/*
This file is part of LightDM-KDE.

Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.kirigami as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents
import "../components" as Shared

import ConnectionEnum 1.0

TooltipButton {
    id: root

    property bool needHint: connectionsModel.primary && (connectionsModel.primary.flags & ConnectionEnum.FLAG_PRIVATE)
    property bool needCaption: !expand || width < implicitWidth
    property bool deactivatedViaKeyboard: false
    property string hint: i18n("The connection will be active only on the login screen.")
    property var colorSet: screen.Kirigami.Theme.colorSet

    caption: connectionsModel.primary.name
    width: Math.min(approximateFullWidth, implicitWidth)
    icon.name: connectionIcon(connectionsModel.primary)
    ToolTip.visible: hovered && (needCaption || needHint)
    ToolTip.text: (needCaption ? caption : "") + (needHint && needCaption ? "\n" : "" ) + (needHint ? hint : "")

    property int approximateFullWidth
    property int gap: screen.padding
    property int iconSize: Kirigami.Units.iconSizes.smallMedium

    function keyEvent(event) {
        if (event.key === Qt.Key_Escape) {
            deactivatedViaKeyboard = true
        }
    }

    Connections {
        target: connectionsModel

        function onShowDialog(item, action) {

            confirmAction.data = { "connection": item, "action": action }
            confirmAction.callback = connectionsModel.onActionDialogComplete
            confirmLayout.sourceComponent = labelLayout

            switch (action) {
            case ConnectionEnum.ACTION_NONE: return;
            case ConnectionEnum.ACTION_DISCONNECT:
                if (!connectionsModel.allowNetworkControl) return
                confirmAction.text = i18n("Disconnect from %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_ABORT_CONNECTING:
                confirmAction.text = i18n("Abort connecting to %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT:
                if (!connectionsModel.allowNetworkControl) return
                confirmAction.text = i18n("Connect to %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT_FREE_WIFI:
                if (!connectionsModel.allowModifyOwnSettings) return
                confirmAction.text = i18n("Connect to an insecure access point %1 ?", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT_WITH_PSK:
                if (!connectionsModel.allowModifyOwnSettings) return
                confirmLayout.sourceComponent = pskLayout
                confirmAction.text = i18n("Enter the password to connect to %1", item.name)
            break
            case ConnectionEnum.ACTION_CONNECT_8021X_WIFI:
                if (!connectionsModel.allowModifyOwnSettings) return
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
            if (confirmLayout.item.focusTo) {
                confirmLayout.item.focusTo.forceActiveFocus()
                confirmFooter.linkArrowNavigation(confirmLayout.item)
            }
            // focus on the buttons then
            else confirmFooter.forceActiveFocus()
        }
    }

    signal popupHide()

    function connectionIcon(item) {
        switch (item.type) {
            case ConnectionEnum.TYPE_NONE: return "connect_no"
            case ConnectionEnum.TYPE_WIRED: return "network-wired";
            case ConnectionEnum.TYPE_WIRELESS: return iconByStrength(item.signalStrength);
            case ConnectionEnum.TYPE_VPN: return "network-vpn";
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
        connectionsModel.requestScanWifi()
        setTabOrder([ switchWireless, switchNetworking, networkList ])
        popup.open()
        interfaceButtons.forceActiveFocus()
    }

    Dialog {
        id: confirmAction

        parent: activeScreen
        anchors.centerIn: parent
        // shrink the psk dialog vertically so that the input field is at the level of the buttons
        height: contentHeight + topPadding + confirmFooter.height
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
            Kirigami.Theme.colorSet: root.colorSet
        }

        contentItem: Loader {
            id: confirmLayout
            // to remember username for 802.1x, in case of re-entry
            property string lastUser
            Kirigami.Theme.colorSet: root.colorSet
        }

        footer: DialogButtonBox {
            id: confirmFooter
            property int minWidth: contentWidth + leftPadding + rightPadding + spacing * 2
            standardButtons: Dialog.Ok | Dialog.Cancel
            buttonLayout: DialogButtonBox.KdeLayout
            alignment: Qt.AlignBottom | Qt.AlignCenter
            bottomPadding: confirmAction.bottomPadding
            topPadding: confirmAction.topPadding

            Component.onCompleted: {

                // buttons navigation with arrows

                if (contentChildren.length <= 0) return

                var first = contentChildren[0]
                var last = contentChildren[contentChildren.length - 1]

                // initial button highlight
                confirmFooter.KeyNavigation.right = first
                confirmFooter.KeyNavigation.left = last

                // loop
                first.KeyNavigation.left = last
                last.KeyNavigation.right = first

                // horizontal links
                for (var i in contentChildren) {
                    if (i > 0) {
                        contentChildren[i - 1].KeyNavigation.right = contentChildren[i]
                        contentChildren[i].KeyNavigation.left = contentChildren[i - 1]
                    }
                }
            }

            // button activation on enter
            Keys.onReturnPressed: pushButton()
            Keys.onEnterPressed: pushButton()
            function pushButton() {
                for (var i in contentChildren) {
                    var button = contentChildren[i]
                    if (button.visualFocus) {
                        button.clicked()
                        return
                    }
                }
            }

            function linkArrowNavigation(item) {
                if (contentChildren.length <= 0) return

                item.bottomItem.KeyNavigation.down = contentChildren[0]

                for (var i in contentChildren) {
                    contentChildren[i].KeyNavigation.up = item.bottomItem
                }
            }
        }
    }

    // switchable layouts for confirmAction dialog

    Component {
        id: labelLayout

        PlasmaComponents.Label {
            id: label
            width: implicitWidth + gap * 2
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
            property var bottomItem: secretField

            PlasmaComponents.Label {
                id: label
                width: implicitWidth + gap * 2
                text: confirmAction.text
            }
            Shared.TextField {
                id: secretField
                anchors.horizontalCenter: parent.horizontalCenter
                width: 9 * gridUnit
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

            property var bottomItem: secretField
            property var focusTo: identityField
            property alias identityField: identityField
            property alias secretField: secretField

            width: Math.max(label.width, loginRow.width, passwordRow.width)
            spacing: gap * 2

            PlasmaComponents.Label {
                id: label
                width: implicitWidth + gap * 2
                height: implicitHeight + gap * 2
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
                        text: i18n("Login")
                    }
                    PlasmaComponents.TextField {
                        id: identityField
                        width: 9 * gridUnit
                        KeyNavigation.down: secretField
                    }
                }
                Row {
                    id: passwordRow
                    spacing: gap
                    anchors.right: parent.right
                    width: childrenRect.width
                    PlasmaComponents.Label {
                        anchors.verticalCenter: parent.verticalCenter
                        text: i18n("Password")
                    }
                    Shared.TextField {
                        id: secretField
                        width: 9 * gridUnit
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
            Kirigami.Theme.colorSet: root.colorSet
            border: gap * 2
        }

        Row {
            id: interfaceButtons
            anchors.right: parent.right

            KeyNavigation.down: switchNetworking
            Keys.onShortcutOverride: root.keyEvent(event)
            Kirigami.Theme.colorSet: root.colorSet

            TooltipButton {
                id: switchWireless

                KeyNavigation.up: interfaceButtons
                KeyNavigation.down: networkList
                Keys.onShortcutOverride: root.keyEvent(event)

                visible: popup.onLine
                width: iconSize + gap * 2
                height: width
                expand: false
                caption: {
                    (connectionsModel.wirelessEnabled ? i18n("Disable wireless") : i18n("Enable wireless"))
                    + (connectionsModel.allowSwitchWifi ? "" : " (" + i18n("Action prohibited") + ")" )
                }

                Kirigami.Icon {
                    id: switchWirelessIcon
                    x: gap
                    y: gap
                    width: iconSize
                    height: width
                    source: "network-wireless"
                    opacity: connectionsModel.wirelessEnabled ? 1.0 : 0.5
                }

                Kirigami.Icon {
                    x: Math.round(parent.width * 0.5)
                    y: Math.round(parent.height * 0.5)
                    width: parent.width - x
                    height: parent.height - y
                    source: "object-locked"
                    opacity: switchWirelessIcon.opacity
                    visible: !connectionsModel.allowSwitchWifi
                }

                onClicked: if (connectionsModel.hasManagedWifiDevices()) {
                    connectionsModel.wirelessEnabled = !connectionsModel.wirelessEnabled
                }
            }

            TooltipButton {
                id: switchNetworking

                KeyNavigation.up: interfaceButtons
                KeyNavigation.left: switchWireless
                KeyNavigation.down: networkList
                Keys.onShortcutOverride: root.keyEvent(event)

                width: iconSize + gap * 2
                height: width
                expand: false
                caption: {
                    (connectionsModel.networkingEnabled ? i18n("Disable networking") : i18n("Enable networking"))
                    + (connectionsModel.allowSwitchNetworking ? "" : " (" + i18n("Action prohibited") + ")" )
                }

                Kirigami.Icon {
                    id: switchNetworkingIcon
                    x: gap
                    y: gap
                    width: iconSize
                    height: width
                    source: "system-shutdown"
                    opacity: connectionsModel.networkingEnabled ? 1.0 : 0.5
                }

                Kirigami.Icon {
                    x: Math.round(parent.width * 0.5)
                    y: Math.round(parent.height * 0.5)
                    width: parent.width - x
                    height: parent.height - y
                    source: "object-locked"
                    opacity: switchNetworkingIcon.opacity
                    visible: !connectionsModel.allowSwitchNetworking
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

            Kirigami.Theme.colorSet: root.colorSet

            ListView {
                id: networkList
                model: connectionsModel

                delegate: Item {
                    id: connectionDelegate
                    width: ListView.view.width
                    height: connectionEntry.height + gap
                    property bool highlighted: mouseArea.containsMouse || (ListView.view.currentIndex == index && ListView.view.activeFocus)

                    Keys.onShortcutOverride: root.keyEvent(event)

                    Row {
                        id: connectionEntry
                        x: gap / 2
                        y: gap / 2
                        spacing: gap
                        height: childrenRect.height
                        width: parent.width - gap

                        Kirigami.Icon {
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
                            height: iconSize
                            width: height
                            source: "link"
                        }

                        Kirigami.Icon {
                            id: connectionType
                            source: connectionIcon(model.item)
                            animated: false
                            height: iconSize
                            width: height
                        }

                        Kirigami.Icon {
                            visible: model != null && model.item.type == ConnectionEnum.TYPE_WIRELESS
                            source: model.item.flags & ConnectionEnum.FLAG_LOCKED ? "object-locked" : "object-unlocked"
                            height: iconSize
                            width: height
                        }

                        PlasmaComponents.Label {
                            id: connectionName
                            text: model.item.name
                            width: parent.width - x
                            elide: Text.ElideRight
                            anchors.verticalCenter: connectionType.verticalCenter
                        }
                    }

                    Rectangle {
                        anchors.fill: parent
                        visible: highlighted
                        color: Kirigami.Theme.highlightColor
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
                            color: Kirigami.Theme.highlightColor
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
                    Keys.onEnterPressed: itemClicked(item)
                    Keys.onReturnPressed: itemClicked(item)

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
