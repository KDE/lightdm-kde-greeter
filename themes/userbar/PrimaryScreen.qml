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
import QtQuick.Controls 2.15
import QtQuick.Window 2.15 // for Screen.pixelDensity
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaCore.ColorScope {
    id: screen

    colorGroup: PlasmaCore.Theme.ComplementaryColorGroup

    property var screens: VisibleScreenEnum.VisibleScreen
    property int visibleScreen: screens.DefaultScreen

    property real gridUnit: PlasmaCore.Units.gridUnit
    property int padding: gridUnit / 3
    property int userFaceSize: 7 * gridUnit
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    PlasmaComponents.Label {
        id: debugInfo
        visible: showDebugInfo
        anchors {
            bottom: wholeScreen.bottom
            right: wholeScreen.right
            margins: screen.padding
        }
        text: "PPI: " + (Screen.pixelDensity * 25.4).toFixed(2) + " PPM: " + Screen.pixelDensity.toFixed(3)
        + "\nDevice pixel ratio: " + (Screen.devicePixelRatio).toFixed(3)
        + "\n" + (softwareRendering ? "Software rendering" : "Hardware rendering")
        + "\n" + "gridUnit: " + gridUnit + " padding: " + screen.padding
    }

    Shadow { source: debugInfo }

    Component.onCompleted: {
        startDefaultScreen()
    }

    Connections {
        target: greeter

        function onShowPrompt(text, type) {
            if (type == 0) { // enter something that is not secret, such as a username
                inputBoxLabel.text = text
                inputBox.clear()
                inputBox.echoMode = TextInput.Normal
                inputBox.revealPasswordButtonShown = false
            } else { // enter secret word
                inputBoxLabel.text = text
                inputBox.clear()
                inputBox.echoMode = TextInput.Password
                inputBox.revealPasswordButtonShown = true
            }
            if (visibleScreen != screens.LoginScreen) {
                startPromptScreen()
            } else {
                inputDialog.visibleOnLoginScreen = true
            }
        }

        function onShowMessage(text, type) {
            putMessage(text, type)
        }

        function onAuthenticationComplete() {
            if (greeter.authenticated) {
                doSessionSync()
            } else if (visibleScreen != screens.DefaultScreen) {
                if (messages.count == 0) {
                    putMessage(i18n("Login failed"), 1)
                }
                startDefaultScreen()
            }
            dissolveMessages()
        }

        function onAutologinTimerExpired() {
            if (!greeter.allowAutologin) return
            var startresult = greeter.authenticateAutologin()
            if (!startresult) {
                startDefaultScreen()
            }
        }
    }

    function putMessage(text, type) {
        messages.append({ text: text, type: type })
        msgList.positionViewAtEnd()
    }

    function clearMessages() {
        messages.clear()
        msgList.dissolve.complete()
    }

    function dissolveMessages() {
        msgList.dissolve.restart()
    }

    function doSessionSync() {
        var session = greeter.allowAutologin ? greeter.autologinSession : sessionButton.currentData()

        var startresult = greeter.startSessionSync(session)
        if (!startresult) {
            startDefaultScreen()
        }
    }

    function setTabOrder(lst) {
        var idx
        var lastIdx = lst.length - 1
        for (idx = 0; idx <= lastIdx; ++idx) {
            var item = lst[idx]
            item.KeyNavigation.backtab = lst[idx > 0 ? idx - 1 : lastIdx]
            item.KeyNavigation.tab = lst[idx < lastIdx ? idx + 1 : 0]
        }
    }

    // global state switching

    function startLoginScreen() {
        clearMessages()
        visibleScreen = screens.LoginScreen
        setTabOrder([ inputBox, keyboardLayoutButton, sessionButton ])
        inputBox.forceActiveFocus()
        var username = usersList.currentItem.username
        inputBox.clear()
        if (username == greeter.guestLoginName) {
            greeter.authenticateAsGuest()
        } else {
            greeter.authenticate(username)
        }
    }

    function startPromptScreen() {
        visibleScreen = screens.PromptScreen
        setTabOrder([ inputBox, keyboardLayoutButton, sessionButton ])
        inputBox.forceActiveFocus()
    }

    function startDefaultScreen() {
        visibleScreen = screens.DefaultScreen
        // don't show password prompt unless prompted by PAM
        inputDialog.visibleOnLoginScreen = false
        setTabOrder([ usersList, keyboardLayoutButton, sessionButton, connectionsButton, loginAsOtherButton, virtualKeyboardButton, suspendButton, hibernateButton, restartButton, shutdownButton ])
        usersList.forceActiveFocus()
    }

    function loginAsOtherUser() {
        clearMessages()
        greeter.authenticate()
        inputBox.overrideText = greeter.lastLoggedInUser
        startPromptScreen()
        sessionButton.setCurrentSession(greeter.lastLoggedInSession)
    }

    function finishDialog() {
        switch (visibleScreen) {
        case screens.WaitScreen:
            return
        case screens.PromptScreen:
        case screens.LoginScreen:
            clearMessages()
            visibleScreen = screens.WaitScreen
            greeter.respond(inputBox.text)
            break
        default:
            startDefaultScreen()
        }
    }

    function cancelInput() {
        clearMessages()
        startDefaultScreen()
        greeter.cancelAuthentication()
        sessionButton.updateCurrentSession()
    }

    Item {
        id: wholeScreen
        width: screen.width
        height: screen.height
    }

    Item {
        id: activeScreen
        x: wholeScreen.x
        y: wholeScreen.y + menuBar.height
        width: wholeScreen.width
        height: Math.min(inputPanel.item.y - menuBar.height, wholeScreen.height - menuBar.height)
    }

    ListModel {
        id: messages
    }

    FocusScope {
        id: centerPanelFocus

        anchors.fill: activeScreen

        Column {
            id: centerPanel
            spacing: screen.padding
            anchors.horizontalCenter: parent.horizontalCenter

            // never hide the input dialog with the virtual keyboard
            property bool fitsAsWhole: height < parent.height
            states: [
                State {
                    name: "Centered"
                    when: centerPanel.fitsAsWhole
                    AnchorChanges {
                        target: centerPanel
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.bottom: undefined
                    }
                },
                State {
                    name: "AlignedToBottom"
                    when: !centerPanel.fitsAsWhole
                    AnchorChanges {
                        target: centerPanel
                        anchors.verticalCenter: undefined
                        anchors.bottom: parent.bottom
                    }
                }
            ]

            ListView {
                id: usersList

                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                // so as not to animate if the dimensions change when the screen changes
                property bool animateDelegate: false

                visible: visibleScreen == screens.DefaultScreen || visibleScreen == screens.LoginScreen
                enabled: visible
                interactive: visibleScreen == screens.DefaultScreen

                // dim the list of users when it loses focus
                property bool itemHovered: false
                opacity: centerPanelFocus.activeFocus || itemHovered || visibleScreen != screens.DefaultScreen ? 1.0 : 0.7

                spacing: padding
                width: activeScreen.width
                height: currentItem.height
                model: usersModel
                currentIndex: model.indexForUserName(greeter.lastLoggedInUser)
                cacheBuffer: count * 80
                delegate: UserDelegate {

                    property var usersession: model.session
                    property var username: model.name

                    avatarPath: "image://face/" + model.name

                    opacity: isCurrent ? 1.0 : visibleScreen == screens.DefaultScreen ? 0.618 : 0.0
                    name: model.display

                    width: userFaceSize + screen.padding * 2
                    animate: usersList.animateDelegate

                    //if we only have one delegate, we don't need to clip the text as it won't be overlapping with anything
                    constrainText: usersList.model.rowCount() != 1

                    isCurrent: ListView.isCurrentItem

                    onClicked: {
                        if (!usersList.interactive) return
                        if (usersList.currentIndex == index) {
                            startLoginScreen()
                            return
                        }
                        usersList.animateDelegate = true
                        usersList.currentIndex = index
                        usersList.forceActiveFocus()
                        usersList.animateDelegate = false
                    }
                    Keys.onReturnPressed: startLoginScreen()
                }

                orientation: ListView.Horizontal
                highlightRangeMode: ListView.StrictlyEnforceRange
                preferredHighlightBegin: width / 2 - currentItem.width / 2
                preferredHighlightEnd: width / 2 + currentItem.width / 2

                Component.onCompleted: {
                    sessionButton.onCurrentIndexChanged.connect(() => {
                        if (visibleScreen != screens.DefaultScreen) return;
                        currentItem.usersession = sessionButton.currentData()
                    })
                }
            }

            Item {
                id: loginItem

                anchors.horizontalCenter: parent.horizontalCenter

                width: inputDialog.width
                height: inputDialog.height

                Item {

                    anchors.centerIn: parent
                    width: inputDialog.width
                    height: inputDialog.height
                    visible: inputDialog.visible

                    Rectangle {
                        anchors.fill: parent
                        color: PlasmaCore.ColorScope.backgroundColor
                        radius: screen.padding * 2
                        opacity: 0.6
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        border {
                            color: PlasmaCore.ColorScope.textColor
                            width: 2
                        }
                        radius: screen.padding * 2
                    }
                }

                Item {
                    id: inputDialog

                    visible: (visibleScreen == screens.LoginScreen && visibleOnLoginScreen) || (visibleScreen == screens.PromptScreen)
                    enabled: visible

                    property bool visibleOnLoginScreen: false

                    width: rows.width + rows.height * 1.5
                    height: rows.height * 2.5

                    Row {
                        id: rows

                        anchors.centerIn: parent

                        spacing: screen.padding

                        PlasmaComponents.ToolButton {
                            id: cancelButton
                            icon.name: "undo"
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: cancelInput()
                        }

                        PlasmaComponents.Label {
                            id: inputBoxLabel
                            height: inputBox.height
                            anchors.verticalCenter: parent.verticalCenter
                            Behavior on opacity {
                                NumberAnimation { duration: 100 }
                            }
                        }

                        PlasmaComponents.TextField {
                            id: inputBox

                            // if set overrideText, the first clear will set this text
                            // need for 'log in as another user'
                            property string overrideText
                            function clear() {
                                if (overrideText.length != 0) {
                                    text = overrideText
                                    overrideText = ""
                                } else {
                                    text = ""
                                }
                            }
                            width: 8 * gridUnit
                            onAccepted: finishDialog()
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        PlasmaComponents.ToolButton {
                            id: enterButton
                            anchors.verticalCenter: parent.verticalCenter
                            width: implicitWidth
                            height: width

                            icon.name: "go-next"
                            onClicked: finishDialog()
                        }
                    }
                }

                ActionButton {
                    id: loginButton
                    anchors.centerIn: parent
                    iconSource: "go-next"
                    text: i18n("Log in")
                    onClicked: startLoginScreen()
                    visible: visibleScreen == screens.DefaultScreen
                    enabled: visible
                }
            }
        }

        ListView {
            id: msgList

            PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.NormalColorGroup
            PlasmaCore.ColorScope.inherit: false

            anchors.horizontalCenter: parent.horizontalCenter
            y: (centerPanel.y - height) / 2
            width: contentItem.childrenRect.width
            height: Math.min(centerPanel.y, contentItem.childrenRect.height)
            clip: true

            model: messages
            delegate: PlasmaCore.FrameSvgItem {
                imagePath: "widgets/lineedit"
                prefix: "base"
                width: msgBody.width
                height: msgBody.height
                Row {
                    id: msgBody
                    anchors.centerIn: parent
                    spacing: screen.padding
                    padding: spacing * 2
                    PlasmaCore.IconItem {
                        width: units.iconSizes.medium
                        height: width
                        source: model.type == 0 ? "dialog-information" : "dialog-error"
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Label {
                        id: label
                        anchors.verticalCenter: parent.verticalCenter
                        text: model.text.trim()
                    }
                }
            }

            property var dissolve: SequentialAnimation {
                running: false
                PauseAnimation { duration: 5000 }
                NumberAnimation { target: msgList; property: "opacity"; from: 1.0; to: 0.0; duration: 1000 }
                onFinished: {
                    messages.clear()
                    msgList.opacity = 1.0
                }
            }
        }
    }

    Shadow { source: menuBar }

    Row {
        id: menuBar

        anchors.top: wholeScreen.top
        anchors.right: wholeScreen.right
        anchors.margins: screen.padding
        width: childrenRect.width
        height: childrenRect.height
        spacing: screen.padding
        visible: visibleScreen != screens.WaitScreen
        enabled: visible

        property int expandedWidth: {
            var approximateFullWidth = 0
            for(var i in children) {
                var child = children[i]
                if (child.approximateFullWidth == null) {
                    console.warn("menuBar: child " + child + " does not have the required property (approximateFullWidth)")
                    continue
                }
                approximateFullWidth += child.approximateFullWidth
            }
            approximateFullWidth += children.length * menuBar.spacing
            return approximateFullWidth
        }

        // whether to show button captions
        property bool expand: expandedWidth < activeScreen.width * 0.9

        KeyboardButton {
            id: keyboardLayoutButton
            property int approximateFullWidth: height * 2
            onKeyboardLayoutTriggered: {
                centerPanelFocus.forceActiveFocus()
            }
        }

        ListButton {
            id: sessionButton
            model: sessionsModel
            dataRole: "key"
            icon.name: "computer"
            // probably won't get wider
            property int approximateFullWidth: height * 6

            onItemTriggered: {
                centerPanelFocus.forceActiveFocus()
            }

            function updateCurrentSession() {
                setCurrentSession(usersList.currentItem.usersession)
            }

            function setCurrentSession(session) {
                var i =  indexForData(session)
                i = i || indexForData(greeter.defaultSession)
                i = i || 0
                currentIndex = i
            }

            Component.onCompleted: {
                updateCurrentSession()
                usersList.onCurrentIndexChanged.connect(updateCurrentSession)
            }
        }

        NetworkWidget {
            id: connectionsButton
            expand: menuBar.expand
            approximateFullWidth: height * 6
            onPopupHide: centerPanelFocus.forceActiveFocus()
        }

        TooltipButton {
            id: loginAsOtherButton
            caption: i18n("Log in as another user")
            expand: menuBar.expand
            icon.name: "auto-type"
            onClicked: loginAsOtherUser()
        }

        TooltipButton {
            id: virtualKeyboardButton
            caption: i18nc("Button to show/hide virtual keyboard", "Virtual Keyboard")
            expand: menuBar.expand
            icon.name: inputPanel.keyboardEnabled ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
            onClicked: {
                inputPanel.switchState()
                if (desktop.previousFocusItem) desktop.previousFocusItem.forceActiveFocus()
            }
        }

        TooltipButton {
            id: suspendButton
            caption: i18n("Suspend")
            expand: menuBar.expand
            icon.name: "system-suspend"
            enabled: power.canSuspend
            onClicked: power.suspend()
        }

        TooltipButton {
            id: hibernateButton
            caption: i18n("Hibernate")
            expand: menuBar.expand
            icon.name: "system-suspend-hibernate"
            //Hibernate is a special case, lots of distros disable it, so if it's not enabled don't show it
            visible: power.canHibernate
            onClicked: power.hibernate()
        }

        TooltipButton {
            id: restartButton
            caption: i18n("Restart")
            expand: menuBar.expand
            icon.name: "system-reboot"
            enabled: power.canRestart
            onClicked: power.restart()
        }

        TooltipButton {
            id: shutdownButton
            caption: i18n("Shutdown")
            expand: menuBar.expand
            icon.name: "system-shutdown"
            enabled: power.canShutdown
            onClicked: power.shutdown()
        }
    }

    Clock {
        id: clock
        ratio: 0.5
        anchors.bottom: wholeScreen.bottom
        anchors.left: wholeScreen.left
        anchors.margins: screen.padding * 2
    }
    Shadow { source: clock }

    Loader {
        id: inputPanel

        property bool keyboardEnabled: item && item.keyboardEnabled

        function switchState() {
            if (item) {
                item.switchState()
            }
        }

        Component.onCompleted: {
            inputPanel.source = "../components/InputPanel.qml"
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: wholeScreen
        visible: visibleScreen == screens.WaitScreen
        Behavior on visible {
            SequentialAnimation {
                PauseAnimation { duration: busyIndicator.visible ? 0 : 5000 }
                NumberAnimation { duration: 0 }
            }
        }
    }

    Keys.onEscapePressed: cancelInput()
}
