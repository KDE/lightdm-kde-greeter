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
import QtQuick.Controls 2.15
import QtQuick.Window 2.15 // for Screen.pixelDensity
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

Item {
    id: screen

    property var screens: VisibleScreenEnum.VisibleScreen
    property int visibleScreen: screens.DefaultScreen

    property real dpi96: 3.7820092576037516
    property real dpiScale: Screen.pixelDensity / dpi96
    property int padding: 6 * dpiScale
    property int userFaceSize: 100 * dpiScale

    property var msgQueue: {
        consumeOnEvent: false
        items: []
    }

    Component.onCompleted: {
        startDefaultScreen()
    }

    onParentChanged: {
        menuBar.expand = true
    }

    Connections {
        target: greeter

        function onShowPrompt(text, type) {
            msgQueue.items.push({ func: putPrompt, text: text, type: type })
            if (msgQueue.consumeOnEvent) processQueue()
        }

        function onShowMessage(text, type) {
            msgQueue.items.push({ func: putMessage, text: text, type: type })
            if (msgQueue.consumeOnEvent) processQueue()
        }

        function onAuthenticationComplete() {
            if(!greeter.authenticated) {
                startMsgScreen(screens.ErrorMsgScreen, i18n("Sorry, incorrect password. Please try again."))
            } else {
                doSessionSync()
            }
        }
    }

    function processQueue() {
        msgQueue.consumeOnEvent = false
        var next = msgQueue.items.shift()
        if (next) next.func(next.text, next.type)
        return next
    }

    function putPrompt(text, type) {
        if (type == 0) { // enter something that is not secret, such as a username
            inputBoxLabel.text = text
            inputBox.text = ""
            inputBox.echoMode = TextInput.Normal
        } else { // enter secret word
            inputBoxLabel.text = text
            inputBox.text = ""
            inputBox.echoMode = TextInput.Password
        }
        if (visibleScreen != screens.LoginScreen) startPromptScreen()
    }

    function putMessage(text, type) {
        startMsgScreen(type == 0 ? screens.InfoMsgScreen : screens.ErrorMsgScreen, text)
    }

    function doSessionSync() {
        var session = sessionButton.currentData()
        if (session == "") {
            session = "default"
        }

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
        visibleScreen = screens.LoginScreen
        setTabOrder([ inputBox, sessionButton, keyboardLayoutButton ])
        inputBox.forceActiveFocus()
        var username = usersList.currentItem.username
        msgQueue.consumeOnEvent = true
        inputBox.text = ""
        if (username == greeter.guestLoginName) {
            greeter.authenticateAsGuest()
        } else {
            greeter.authenticate(username)
        }
    }

    function startPromptScreen() {
        visibleScreen = screens.PromptScreen
        setTabOrder([ inputBox, sessionButton, keyboardLayoutButton ])
        inputBox.forceActiveFocus()
    }

    function startDefaultScreen() {
        msgQueue.items = []
        visibleScreen = screens.DefaultScreen
        setTabOrder([ usersList, sessionButton, keyboardLayoutButton, loginAsOtherButton, suspendButton, hibernateButton, restartButton, shutdownButton ])
        usersList.forceActiveFocus()
    }

    function startMsgScreen(msgScreen, text) {
        infoMsgLabel.text = text.trim()
        setTabOrder([ msgOkButton, msgCancelButton ])
        visibleScreen = msgScreen
        msgBox.forceActiveFocus()
    }

    function loginAsOtherUser() {
        msgQueue.consumeOnEvent = true
        greeter.authenticate()
        startPromptScreen()
    }

    function finishDialog() {
        switch (visibleScreen) {
            case screens.WaitScreen:
            return
            case screens.InfoMsgScreen:
            case screens.ErrorMsgScreen:
            if (processQueue()) return
            startDefaultScreen()
            break
            case screens.PromptScreen:
            if (processQueue()) break
            // deliberate fallthrough
            case screens.LoginScreen:
            visibleScreen = screens.WaitScreen
            msgQueue.consumeOnEvent = true
            greeter.respond(inputBox.text)
            break
            default:
            startDefaultScreen()
        }
    }

    Item {
        id: wholeScreen
        width: screen.width
        height: screen.height
    }

    Item {
        id: activeScreen
        x: wholeScreen.x
        y: wholeScreen.y
        width: wholeScreen.width
        height: Math.min(inputPanel.item.y, wholeScreen.height - menuBar.height)
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

                visible: visibleScreen == screens.DefaultScreen || visibleScreen == screens.LoginScreen
                enabled: visible
                interactive: visibleScreen == screens.DefaultScreen

                // dim the list of users when it loses focus
                property bool itemHovered: false
                opacity: activeFocus || itemHovered || visibleScreen != screens.DefaultScreen ? 1.0 : 0.7

                spacing: padding
                width: activeScreen.width
                height: contentItem.childrenRect.height
                model: usersModel
                currentIndex: model.indexForUserName(greeter.lastLoggedInUser)
                cacheBuffer: count * 80
                delegate: userDelegate
                orientation: ListView.Horizontal
                highlightRangeMode: ListView.StrictlyEnforceRange
                preferredHighlightBegin: width / 2 - currentItem.width / 2
                preferredHighlightEnd: width / 2 + currentItem.width / 2

                Component.onCompleted: {
                    sessionButton.onCurrentIndexChanged.connect(() => {
                        currentItem.usersession = sessionButton.currentData()
                    })
                }
            }

            Item {
                id: loginButtonItem

                anchors.horizontalCenter: parent.horizontalCenter

                width: msgBox.visible ? msgBox.width : inputDialog.width
                height: msgBox.visible ? msgBox.height : inputDialog.height

                PlasmaCore.FrameSvgItem {
                    id: inputDialog
                    imagePath: "widgets/background"

                    visible: (visibleScreen == screens.LoginScreen) || (visibleScreen == screens.PromptScreen)
                    enabled: visible

                    width: rows.width + rows.height * 1.5
                    height: rows.height * 2.5

                    Keys.onEscapePressed: startDefaultScreen()

                    Row {
                        id: rows

                        anchors.centerIn: parent

                        spacing: screen.padding

                        PlasmaComponents.ToolButton {
                            id: cancelButton
                            icon.name: "undo"
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: startDefaultScreen()
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
                            width: 150 * dpiScale
                            onAccepted: finishDialog()

                            anchors.verticalCenter: parent.verticalCenter

                            PlasmaComponents.ToolButton {
                                id: enterButton
                                anchors {
                                    right: parent.right
                                    rightMargin: y
                                    verticalCenter: parent.verticalCenter
                                }
                                width: implicitWidth
                                height: width

                                icon.name: "go-jump-locationbar"
                                onClicked: finishDialog()
                            }

                            Behavior on opacity {
                                NumberAnimation { duration: 100 }
                            }
                        }

                        TooltipButton {
                            id: virtualKeyboardButton
                            icon.name: inputPanel.keyboardEnabled ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
                            anchors.verticalCenter: parent.verticalCenter
                            visible: inputPanel.item
                            caption: i18ndc("kcm_lightdm", "Button to show/hide virtual keyboard", "Virtual Keyboard")
                            expand: false

                            onClicked: {
                                inputPanel.switchState()
                                inputBox.forceActiveFocus()
                            }
                        }
                    }
                }

                PlasmaCore.FrameSvgItem {
                    id: msgBox
                    imagePath: "widgets/background"

                    width: msgColumns.childrenRect.width + padding * 6
                    height: msgColumns.childrenRect.height + padding * 6

                    visible: (visibleScreen == screens.InfoMsgScreen) || (visibleScreen == screens.ErrorMsgScreen)
                    enabled: visible

                    Keys.onReturnPressed: finishDialog()
                    Keys.onEscapePressed: startDefaultScreen()

                    Column {
                        id: msgColumns

                        height: childrenRect.height
                        enabled: visible
                        anchors.centerIn: parent
                        spacing: screen.padding

                        Row {
                            spacing: screen.padding

                            // a separate element for each icon, otherwise the icon is switched in the opened window
                            PlasmaCore.IconItem {
                                id: infoIcon
                                visible: visibleScreen == screens.InfoMsgScreen
                                source: "dialog-information"
                                anchors.verticalCenter: parent.verticalCenter
                                height: msgOkButton.height
                                width: height
                            }

                            PlasmaCore.IconItem {
                                id: errorIcon
                                visible: visibleScreen == screens.ErrorMsgScreen
                                source: "dialog-error"
                                anchors.verticalCenter: parent.verticalCenter
                                height: msgOkButton.height
                                width: height
                            }

                            Item {
                                id: infoMsgLabelPadding

                                width: infoMsgLabelFormatted.width + screen.padding * 3
                                height: infoMsgLabelFormatted.height + screen.padding * 2

                                // raw message string, possibly too wide, having line breaks
                                PlasmaComponents.Label {
                                    id: infoMsgLabel
                                    visible: false
                                }

                                PlasmaComponents.Label {
                                    id: infoMsgLabelFormatted

                                    anchors {
                                        verticalCenter: parent.verticalCenter
                                        centerIn: parent
                                    }
                                    property int maxWidth: screen.width - infoIcon.width - screen.padding * 10
                                    width: Math.min(maxWidth, infoMsgLabel.width)
                                    wrapMode: Text.WordWrap
                                    // replace newlines with spaces, if had to narrow the text
                                    text: infoMsgLabel.width > maxWidth ? infoMsgLabel.text.replace(/(\r\n|\n|\r)/gm, " ") : infoMsgLabel.text
                                }
                            }
                        }

                        Row {
                            spacing: screen.padding * 2
                            anchors.horizontalCenter: parent.horizontalCenter
                            PlasmaComponents.ToolButton {
                                id: msgCancelButton
                                icon.name: "dialog-cancel"
                                onClicked: startDefaultScreen()
                            }
                            PlasmaComponents.ToolButton {
                                id: msgOkButton
                                icon.name: "dialog-ok"
                                onClicked: finishDialog()
                            }
                        }
                    }
                }

                PlasmaComponents.Button {
                    id: loginButton

                    anchors.centerIn: parent
                    visible: visibleScreen == screens.DefaultScreen
                    enabled: visible

                    icon.name: "go-jump-locationbar"
                    text: i18n("Log in")
                    onClicked: startLoginScreen()

                    Behavior on opacity {
                        NumberAnimation { duration: 100 }
                    }
                }
            }
        }
    }

    Component {
        id: userDelegate

        Item {
            id: wrapper

            property bool isCurrent: ListView.isCurrentItem

            // Expose current item info to the outer world. I can't find
            // another way to access this from outside the list.
            property string username: model.name
            property string usersession: model.session

            width: frame.width
            height: frame.height

            opacity: isCurrent ? 1.0 : visibleScreen == screens.DefaultScreen ? 0.618 : 0.0

            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }

            PlasmaCore.FrameSvgItem {
                id: frame

                width: userFaceSize + screen.padding * 2
                height: childrenRect.height + screen.padding * 2

                imagePath: "widgets/lineedit"
                prefix: "base"
                enabledBorders: "NoBorder"

                Image {
                    id: face
                    width: userFaceSize
                    height: userFaceSize
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: screen.padding
                    sourceSize.width: userFaceSize
                    sourceSize.height: userFaceSize
                    source: "image://face/" + name
                }

                PlasmaComponents.Label {
                    id: loginText

                    anchors.top: face.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: userFaceSize

                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    text: display
                }
            }

            PlasmaCore.FrameSvgItem {
                id: frameHover
                anchors.fill: frame
                imagePath: "widgets/lineedit"
                prefix: "hover"
                opacity: mouseArea.containsMouse ? 1 : 0
                Behavior on opacity {
                    NumberAnimation { duration: 100 }
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: frame
                anchors.margins: -padding * 0.5
                hoverEnabled: true
                onEntered: usersList.itemHovered = true
                onExited: usersList.itemHovered = false
                onClicked: {
                    if (!usersList.interactive) return
                    usersList.currentIndex = index
                    usersList.forceActiveFocus()
                }
            }

            Keys.onReturnPressed: startLoginScreen()

            ListView.onAdd: {
                if (visibleScreen == screens.DefaultScreen && username == greeter.lastLoggedInUser) {
                    usersList.currentIndex = usersList.model.indexForUserName(username)
                }
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: menuBar
        anchors.bottom: wholeScreen.bottom
        anchors.right: wholeScreen.right
        width: childrenRect.width + margins.left
        height: childrenRect.height + margins.top
        imagePath: "widgets/background"
        enabledBorders: "LeftBorder|TopBorder"

        visible: visibleScreen != screens.WaitScreen
        enabled: visible

        // whether to show button captions
        property bool expand: true
        onXChanged: {
            expand = expand && x + keyboardLayoutButton.x > activeScreen.width * 0.25
        }

        Row {
            spacing: padding
            x: parent.margins.left
            y: parent.margins.top

            ListButton {
                id: sessionButton
                model: sessionsModel
                dataRole: "key"
                icon.name: "computer"

                onItemTriggered: {
                    centerPanelFocus.forceActiveFocus()
                }

                function updateCurrentSession() {
                    var i =  indexForData(usersList.currentItem.usersession)
                    i = i || indexForData(greeter.defaultSession)
                    i = i || 0
                    currentIndex = i
                }

                Component.onCompleted: {
                    updateCurrentSession()
                    usersList.onCurrentIndexChanged.connect(updateCurrentSession)
                }
            }

            KeyboardButton {
                id: keyboardLayoutButton
                onKeyboardLayoutTriggered: {
                    centerPanelFocus.forceActiveFocus()
                }
            }

            TooltipButton {
                id: loginAsOtherButton
                caption: i18n("Log in as another user")
                expand: menuBar.expand
                icon.name: "auto-type"
                onClicked: loginAsOtherUser()
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
    }

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
        anchors.centerIn: parent
        visible: visibleScreen == screens.WaitScreen
    }
}
