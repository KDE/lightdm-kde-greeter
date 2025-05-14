/*
This file is part of LightDM-KDE.

Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15 // for Screen.pixelDensity
import org.kde.ksvg as KSvg
import org.kde.kirigami as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents
import "../components" as Shared

Item {
    id: screen

    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary

    property var screens: VisibleScreenEnum.VisibleScreen
    property int visibleScreen: screens.DefaultScreen

    property real gridUnit: Kirigami.Units.gridUnit
    property int padding: gridUnit / 3
    property int userFaceSize: 7 * gridUnit
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software
    property var pendingPrompts: []
    property int authStep: 0
    property bool hideUsersList: greeter.hideUsers || usersList.model.rowCount() == 0

    readonly property int userReadTime: 48 // ms

    PlasmaComponents.Label {
        id: debugInfo
        visible: showDebugInfo
        anchors {
            bottom: bottomBar.top
            right: wholeScreen.right
            margins: screen.padding
        }
        text: "Physical DPI: " + (Screen.pixelDensity * 25.4).toFixed(2)
        + "\n" + "Logical DPI: " + (Screen.logicalPixelDensity * 25.4).toFixed(2)
        + "\n" + "Device pixel ratio: " + (Screen.devicePixelRatio).toFixed(3)
        + "\n" + (softwareRendering ? "Software rendering" : "Hardware rendering")
        + "\n" + "gridUnit: " + gridUnit + " padding: " + screen.padding
    }

    Shadow { source: debugInfo }

    Component.onCompleted: {
        startDefaultScreen()
    }

    Timer {
        id: lastMessageTimer
        onTriggered: {
            doSessionSync()
        }
    }

    Connections {
        target: greeter

        function onShowPrompt(text, type) {
            authStep++;
            pendingPrompts.push({ "text": text, "type": type })
            if (!inputDialog.visible) consumePrompt()
        }

        function onShowMessage(text, type) {
            putMessage(text, type)
        }

        function onAuthenticationComplete() {
            if (greeter.authenticated) {
                if (messages.count > 0) {
                    var now = Date.now()
                    var hideLastMessage = messages.get(messages.count - 1).hideTime
                    if (hideLastMessage > now) {
                        lastMessageTimer.interval = hideLastMessage - now
                        lastMessageTimer.start()
                        return
                    }
                }
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

        function onFixupUsersListQueued() {
            // This call leads to updating the position of the users list.
            usersList.returnToBounds()
        }
    }

    function consumePrompt() {
        if (pendingPrompts.length == 0) return
        var pr = pendingPrompts.shift()
        if (pr["type"] == 0) { // enter something that is not secret, such as a username
            inputBoxLabel.text = pr["text"]
            inputBox.clear()
            inputBox.echoMode = TextInput.Normal
            inputBox.revealPasswordButtonShown = false
        } else { // enter secret word
            inputBoxLabel.text = pr["text"]
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

    function putMessage(text, type) {
        // provide some time to view the message
        var readStart = Date.now()
        if (messages.count > 0) {
            readStart = Math.max(readStart, messages.get(messages.count - 1).hideTime)
        }
        var hideTime = readStart + text.length * userReadTime

        // but we don’t count if the message is repeated
        for (var i = 0; i < messages.count; ++i) {
            var message = messages.get(i)
            if (text === message.text) {
                hideTime = messages.get(messages.count - 1).hideTime
                break
            }
        }
        messages.append({ text: text, type: type, hideTime: hideTime })
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
        setTabOrder([ inputBox, keyboardLayoutButton, sessionButton, connectionsButton, suspendButton, hibernateButton, restartButton, shutdownButton, loginAsOtherButton ])
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
        setTabOrder([ inputBox, keyboardLayoutButton, sessionButton, connectionsButton, suspendButton, hibernateButton, restartButton, shutdownButton, loginAsOtherButton ])
        inputBox.forceActiveFocus()
    }

    function startDefaultScreen() {
        authStep = 0
        if (hideUsersList) {
            loginAsOtherUser()
            return
        }
        visibleScreen = screens.DefaultScreen
        // don't show password prompt unless prompted by PAM
        inputDialog.visibleOnLoginScreen = false
        setTabOrder([ usersList, keyboardLayoutButton, sessionButton, connectionsButton, suspendButton, hibernateButton, restartButton, shutdownButton, loginAsOtherButton ])
        usersList.forceActiveFocus()
    }

    function loginAsOtherUser() {
        visibleScreen = screens.WaitScreen
        greeter.authenticate()
        inputBox.overrideText = greeter.lastLoggedInUser
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
            consumePrompt()
            break
        default:
            startDefaultScreen()
        }
    }

    function cancelInput() {
        clearMessages()
        pendingPrompts.length = 0
        greeter.cancelAuthentication()
        startDefaultScreen()
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
        height: Math.min(inputPanel.item ? inputPanel.item.y : 0 - menuBar.height, wholeScreen.height - menuBar.height)
    }

    Item {
        id: inputPanelArea

        // The maximum ratio of the keyboard width to the screen height
        // On wider screens, the entire width is not filled
        property var maxRatio: [ 4, 3 ]

        height: wholeScreen.height
        width: Math.min(wholeScreen.width, Math.round(wholeScreen.height * maxRatio[0] / maxRatio[1]))

        x: wholeScreen.x + Math.round((wholeScreen.width - width) * 0.5)
        y: wholeScreen.y
    }

    ListModel {
        id: messages
    }

    FocusScope {
        id: centerPanelFocus

        KeyNavigation.up: keyboardLayoutButton

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

                KeyNavigation.down: loginAsOtherButton

                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                // so as not to animate if the dimensions change when the screen changes
                property bool animateDelegate: false

                property var currentItemSafe: currentItem ? currentItem : {}

                visible: visibleScreen == screens.DefaultScreen || visibleScreen == screens.LoginScreen
                enabled: visible
                interactive: visibleScreen == screens.DefaultScreen

                // dim the list of users when it loses focus
                property bool itemHovered: false
                opacity: usersList.activeFocus || itemHovered || visibleScreen != screens.DefaultScreen ? 1.0 : 0.7

                spacing: padding
                width: activeScreen.width
                height: currentItemSafe.height
                model: usersModel
                currentIndex: Math.max(model.indexForUserName(greeter.lastLoggedInUser), 0)
                cacheBuffer: count * 80
                delegate: UserDelegate {

                    property var usersession: model.session
                    property var username: model.name

                    avatarPath: "image://face/" + model.name

                    opacity: isCurrent ? 1.0 : visibleScreen == screens.DefaultScreen ? 0.618 : 0.0
                    name: model.display

                    width: userFaceSize
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
                    // In Qt 6.8, at the start, the position of user list is calculated too early,
                    // before the positions of the elements inside the list are calculated.
                    // As a result, the list of users is not centered on the chosen element, since
                    // its coordinate is not yet calculated and equal to 0.
                    // The coordinate of the item inside the list is calculated later, in the idle queue.
                    // We want to postpone the update of the position of the user list
                    // after the coordinates of all elements are calculated.
                    // To do this, we use signal with QueuedConnection type.
                    onXChanged: {
                        greeter.fixupUsersList();
                    }
                    Keys.onReturnPressed: startLoginScreen()
                }

                orientation: ListView.Horizontal
                highlightRangeMode: ListView.StrictlyEnforceRange
                preferredHighlightBegin: Math.round((width - userFaceSize) / 2)
                preferredHighlightEnd: preferredHighlightBegin + userFaceSize

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
                        color: Kirigami.Theme.backgroundColor
                        radius: screen.padding * 2
                        opacity: 0.6
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        border {
                            color: Kirigami.Theme.textColor
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
                            visible: authStep > 1 || !hideUsersList
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: cancelInput()
                        }

                        PlasmaComponents.Label {
                            id: inputBoxLabel
                            height: inputBox.height
                            verticalAlignment: Text.AlignVCenter
                            Behavior on opacity {
                                NumberAnimation { duration: 100 }
                            }
                        }

                        Shared.TextField {
                            id: inputBox

                            focus: true

                            Kirigami.Theme.inherit: false

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
                            width: 9 * gridUnit
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

                Shadow { source: keyboardStatus }
                Label {
                    id: keyboardStatus

                    anchors {
                        top: inputDialog.bottom
                        topMargin: screen.padding
                        horizontalCenter: inputDialog.horizontalCenter
                    }

                    visible: inputDialog.visible && !value.length !== 0

                    property string value: {
                        if (keyboard.capsLock && !keyboard.numLock) {
                            return i18n("Caps Lock is on")
                        }
                        if (!keyboard.capsLock && keyboard.numLock) {
                            return i18n("Num Lock is on")
                        }
                        if (keyboard.capsLock && keyboard.numLock) {
                            return i18n("Caps Lock and Num Lock is on")
                        }
                        return ""
                    }

                    text: value
                    font.italic: true
                }

                ActionButton {
                    id: loginButton
                    anchors.centerIn: parent
                    iconSource: "go-next-view"
                    text: i18n("Log in")
                    onClicked: startLoginScreen()
                    visible: visibleScreen == screens.DefaultScreen
                    enabled: visible
                }
            }
        }

        Shadow { source: loginAsOtherButton }
        TooltipButton {
            id: loginAsOtherButton

            KeyNavigation.up: usersList

            anchors {
                horizontalCenter: parent.horizontalCenter
                top: centerPanel.bottom
                topMargin: gridUnit * 2
            }
            caption: i18n("Log in as another user")
            // XXX: show-manual-login is false by default yet, this is undesirable
            visible: !hideUsersList /* && greeter.showManualLogin */ && visibleScreen == screens.DefaultScreen
            expand: true
            icon.name: "auto-type"
            onClicked: { cancelInput(); loginAsOtherUser() }
        }

        ListView {
            id: msgList

            anchors.horizontalCenter: parent.horizontalCenter
            y: (centerPanel.y - height) / 2
            width: Math.max(1, contentItem.childrenRect.width)
            height: Math.min(centerPanel.y, contentItem.childrenRect.height)
            clip: true

            model: messages
            delegate: KSvg.FrameSvgItem {
                imagePath: "widgets/lineedit"
                prefix: "base"
                width: msgBody.width
                height: msgBody.height
                Kirigami.Theme.inherit: false
                Row {
                    id: msgBody
                    anchors.centerIn: parent
                    spacing: screen.padding
                    padding: spacing * 2
                    Kirigami.Icon {
                        width: Kirigami.Units.iconSizes.medium
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

    Shadow { source: hostName }

    PlasmaComponents.Label {
        id: hostName

        anchors {
            top: wholeScreen.top
            left: wholeScreen.left
            margins: screen.padding
            leftMargin: screen.padding * 2
        }
        height: menuBar.height

        property real maxWidth: activeScreen.width * 0.3
        width: Math.min(implicitWidth, maxWidth)

        elide: Text.ElideRight

        ToolTip.delay: params.toolTipDelay
        ToolTip.timeout: params.toolTipTimeout
        ToolTip.visible: hostNameArea.containsMouse && implicitWidth > maxWidth
        ToolTip.text: text

        MouseArea {
            id: hostNameArea
            anchors.fill: parent
            hoverEnabled: true
        }

        text: localHostName
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
        visible: true
        enabled: visible

        KeyNavigation.down: centerPanelFocus

        function widthForOrder(order) {
            var approximateFullWidth = 0
            var maxOrder = 0
            var minWidth = height
            var gap = 0
            for(var i in children) {
                var child = children[i]
                if (child.approximateFullWidth == null) {
                    console.warn("menuBar: child " + child + " does not have the required property (approximateFullWidth)")
                    continue
                }
                if (!child.expandOrder || child.expandOrder > order) {
                    approximateFullWidth += child.approximateFullWidth
                } else {
                    approximateFullWidth += minWidth + gap
                    gap = spacing
                }
                if (child.expandOrder) {
                    maxOrder = Math.max(maxOrder, child.expandOrder)
                }
            }
            return [approximateFullWidth, maxOrder]
        }

        // You can set an expandOrder for an item, and collapse it if its
        // expandOrder does not exceed this value. This will ensure gradual
        // collapse of menu items as free space decreases. Thus, the larger the
        // expandOrder an item has, the later it will collapse.
        property int expandOrder: {
            const allWidth = (activeScreen.width - hostName.width) * 0.9
            if (allWidth === 0) return 0
            var order = 0
            while (true) {
                const [width, maxOrder] = widthForOrder(order)
                if (maxOrder === 0) {
                    console.warn("menuBar: children do not use expand order")
                    return 0
                }
                if (width <= allWidth) return order
                if (order > maxOrder) {
                    console.warn("menuBar: no more children to shrink, but still not enough space")
                    return order
                }
                ++order;
            }
        }

        KeyboardButton {
            id: keyboardLayoutButton
            property int approximateFullWidth: height * 2
            onKeyboardLayoutTriggered: {
                centerPanelFocus.forceActiveFocus()
            }
            ToolTip.delay: params.toolTipDelay
            ToolTip.timeout: params.toolTipTimeout
            ToolTip.visible: hovered
            ToolTip.text: i18n("Keyboard layout")
            KeyNavigation.right: sessionButton
        }

        ListButton {
            id: sessionButton
            model: sessionsModel
            dataRole: "key"
            icon.name: "computer"

            // probably won't get wider
            property int approximateFullWidth: height * 6
            property int expandOrder: 3
            property bool expand: expandOrder > menuBar.expandOrder

            text: expand ? currentText() : ""
            property string extraTooltip: expand || currentText().length === 0 ? "" : " - " + currentText()

            onPopupEnded: {
                if (deactivatedViaKeyboard) {
                    sessionButton.forceActiveFocus(Qt.TabFocus)
                    deactivatedViaKeyboard = false
                } else {
                    centerPanelFocus.forceActiveFocus()
                }
            }

            function updateCurrentSession() {
                setCurrentSession(usersList.currentItemSafe.usersession)
            }

            function setCurrentSession(session) {
                var i =  indexForData(session)
                i = i || indexForData(greeter.defaultSession)
                i = i || 0
                currentIndex = i
            }

            Component.onCompleted: {
                updateCurrentSession()
                usersList.onCurrentItemSafeChanged.connect(updateCurrentSession)
            }
            ToolTip.delay: params.toolTipDelay
            ToolTip.timeout: params.toolTipTimeout
            ToolTip.visible: hovered
            ToolTip.text: i18n("Desktop session") + extraTooltip
            KeyNavigation.left: keyboardLayoutButton
            KeyNavigation.right: connectionsButton
        }

        NetworkWidget {
            id: connectionsButton
            visible: connectionsModel.networkManagerAvailable && generalConfig.readEntry("hide-network-widget") !== "true"

            approximateFullWidth: height * 6
            property int expandOrder: 2
            expand: expandOrder > menuBar.expandOrder

            onPopupHide: {
                if (deactivatedViaKeyboard) {
                    connectionsButton.forceActiveFocus(Qt.TabFocus)
                    deactivatedViaKeyboard = false
                } else {
                    centerPanelFocus.forceActiveFocus()
                }
            }
            KeyNavigation.left: sessionButton
            KeyNavigation.right: suspendButton
        }

        TooltipButton {
            id: suspendButton
            caption: i18n("Suspend")
            property int expandOrder: 1
            expand: expandOrder > menuBar.expandOrder
            icon.name: "system-suspend"
            enabled: power.canSuspend
            onClicked: power.suspend()
            KeyNavigation.left: connectionsButton
            KeyNavigation.right: hibernateButton
        }

        TooltipButton {
            id: hibernateButton
            caption: i18n("Hibernate")
            property int expandOrder: 1
            expand: expandOrder > menuBar.expandOrder
            icon.name: "system-suspend-hibernate"
            //Hibernate is a special case, lots of distros disable it, so if it's not enabled don't show it
            visible: power.canHibernate
            onClicked: power.hibernate()
            KeyNavigation.left: suspendButton
            KeyNavigation.right: restartButton
        }

        TooltipButton {
            id: restartButton
            caption: i18n("Restart")
            property int expandOrder: 1
            expand: expandOrder > menuBar.expandOrder
            icon.name: "system-reboot"
            enabled: power.canRestart
            onClicked: power.restart()
            KeyNavigation.left: hibernateButton
            KeyNavigation.right: shutdownButton
        }

        TooltipButton {
            id: shutdownButton
            caption: i18n("Shutdown")
            property int expandOrder: 1
            expand: expandOrder > menuBar.expandOrder
            icon.name: "system-shutdown"
            enabled: power.canShutdown
            onClicked: power.shutdown()
            KeyNavigation.left: restartButton
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

    Row {
        id: bottomBar
        anchors {
            right: wholeScreen.right
            bottom: wholeScreen.bottom
            margins: screen.padding
        }
        spacing: screen.padding

        TooltipButton {
            id: virtualKeyboardButton
            caption: i18nc("Button to show/hide virtual keyboard", "Virtual Keyboard")
            expand: true
            icon.name: inputPanel.keyboardEnabled ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
            onClicked: {
                inputPanel.switchState()
                if (desktop.previousFocusItem) desktop.previousFocusItem.forceActiveFocus()
            }
        }

        Shared.Battery {
            id: myBattery
        }
    }
    Shadow { source: bottomBar }

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
