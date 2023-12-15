/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021, 2022 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022 Anton Golubev <golubevan@basealt.ru>

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

import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import QtQuick.Controls 2
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

Item {
    id: screen
    width: screenSize.width;
    height: screenSize.height;

    property var screens: VisibleScreenEnum.VisibleScreen
    property int visibleScreen: screens.DefaultScreen

    property real dpi96: 3.7820092576037516
    property real dpiScale: Screen.pixelDensity / dpi96
    property int padding: 6 * dpiScale
    property int userItemWidth: 120 * dpiScale
    property int userItemHeight: 80 * dpiScale
    property int userFaceSize: userItemWidth - padding * 4

    ScreenManager {
        id: screenManager
        delegate: Image {
             // default to keeping aspect ratio
            fillMode: config.readEntry("BackgroundKeepAspectRatio") == false ? Image.Stretch : Image.PreserveAspectCrop;
            //read from config, if there's no entry use plasma theme
            source: config.readEntry("Background") ? config.readEntry("Background"): plasmaTheme.wallpaperPath;
        }
    }

    Item {
        id: wholeScreen
        x: screenManager.activeScreen.x
        y: screenManager.activeScreen.y
        width: screenManager.activeScreen.width
        height: screenManager.activeScreen.height
    }

    Item {
        id: activeScreen
        x: wholeScreen.x
        y: wholeScreen.y
        width: wholeScreen.width
        height: Math.min(inputPanel.item.y, wholeScreen.height - menuBar.height)
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

    Component.onCompleted: {
        startDefaultScreen()
    }


    Connections {
        target: greeter;

        function onShowPrompt(text, type) {
            if (type == 0) { // enter something that is not secret, such as a username
                inputBoxLabel.text = text;
                inputBox.text = "";
                inputBox.echoMode = TextInput.Normal
            } else { // enter secret word
                inputBoxLabel.text = text;
                inputBox.text = "";
                inputBox.echoMode = TextInput.Password
            }
        }

        function onShowMessage(text, type) {
            infoMsgLabel.text = text;
            startMsgScreen(screens.InfoMsgScreen);
        }

        function onAuthenticationComplete() {
            if(!greeter.authenticated) {
                infoMsgLabel.text = i18n("Sorry, incorrect password. Please try again.");
                startMsgScreen(screens.ErrorMsgScreen);
            } else {
                doSessionSync()
            }
        }
    }

    function doSessionSync() {
       var session = sessionButton.currentData()
       if (session == "") {
           session = "default";
       }

       var startresult = greeter.startSessionSync(session);
        if (!startresult) {
            startDefaultScreen()
        }
    }

    function setTabOrder(lst) {
        var idx;
        var lastIdx = lst.length - 1;
        for (idx = 0; idx <= lastIdx; ++idx) {
            var item = lst[idx];
            item.KeyNavigation.backtab = lst[idx > 0 ? idx - 1 : lastIdx];
            item.KeyNavigation.tab = lst[idx < lastIdx ? idx + 1 : 0];
        }
    }

    // global state switching
    function startLoginScreen() {
        visibleScreen = screens.LoginScreen;
        setTabOrder([inputBox, sessionButton, keyboardLayoutButton]);
        inputBox.forceActiveFocus();
        var username = usersList.currentItem.username;
        if (username == greeter.guestLoginName) {
            greeter.authenticateAsGuest();
        } else {
            greeter.authenticate(username);
        }
    }

    function startDefaultScreen() {
        visibleScreen = screens.DefaultScreen;
        setTabOrder([usersList, sessionButton, keyboardLayoutButton, loginAsOtherButton, suspendButton, hibernateButton, restartButton, shutdownButton]);
        usersList.forceActiveFocus()
    }

    function startEnterUsernameScreen() {
        visibleScreen = screens.EnterUserNameScreen;
        setTabOrder([inputBox, sessionButton, keyboardLayoutButton]);
        inputBox.forceActiveFocus()
        greeter.authenticate();
    }

    function startMsgScreen(msgScreen) {
        visibleScreen = msgScreen
        msgBox.forceActiveFocus()
    }

    function finishDialog() {
        switch (visibleScreen) {
            case screens.WaitScreen:
                return
            case screens.LoginScreen:
            case screens.EnterPasswordScreen:
                visibleScreen = screens.WaitScreen;
            break;

            case screens.EnterUserNameScreen:
                visibleScreen = screens.EnterPasswordScreen;
            break;
            default:
                startDefaultScreen()
        }
        greeter.respond(inputBox.text);
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

                    visible: (visibleScreen == screens.LoginScreen) || (visibleScreen == screens.EnterUserNameScreen) || (visibleScreen == screens.EnterPasswordScreen)
                    enabled: visible

                    width: rows.width + rows.height * 1.5
                    height: rows.height * 2.5

                    Keys.onEscapePressed: startDefaultScreen()

                    Row {
                        id: rows

                        anchors.centerIn: parent

                        spacing: screen.padding

                        PlasmaComponents3.ToolButton {
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

                                iconSource: "go-jump-locationbar"
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

                    width: msgRows.childrenRect.width + msgRows.childrenRect.height * 1.5
                    height: msgRows.childrenRect.height * 2.5

                    visible: (visibleScreen == screens.InfoMsgScreen) || (visibleScreen == screens.ErrorMsgScreen)
                    enabled: visible

                    Keys.onReturnPressed: startDefaultScreen()
                    Keys.onEscapePressed: startDefaultScreen()

                    Row {
                        id: msgRows

                        anchors.centerIn: parent
                        height: childrenRect.height
                        spacing: screen.padding
                        enabled: visible

                        PlasmaComponents3.ToolButton {
                            id: msgCancelButton
                            icon.name: "undo"
                            onClicked: startDefaultScreen()
                        }

                        PlasmaComponents.Label {
                            id: infoMsgLabel
                        }

                        PlasmaCore.IconItem {
                            height: msgCancelButton.height
                            width: height
                            source: visibleScreen == screens.ErrorMsgScreen ? "dialog-error" : "dialog-information"
                        }
                    }
                }

                PlasmaComponents.Button {
                    id: loginButton

                    anchors.centerIn: parent
                    visible: visibleScreen == screens.DefaultScreen
                    enabled: visible

                    iconSource: "go-jump-locationbar"
                    text: i18n("Log in")
                    onClicked: startLoginScreen();

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

                width: Math.max(userItemWidth, loginText.width + padding * 2)
                height: Math.max(userItemHeight, childrenRect.height)

                imagePath: "widgets/lineedit"
                prefix: "base"
                enabledBorders: "NoBorder"

                Face {
                    id: face
                    width: userFaceSize
                    height: userFaceSize
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottomMargin: padding * 1.5
                    sourceSize.width: userFaceSize
                    sourceSize.height: userFaceSize
                    source: "image://face/" + name
                }

                PlasmaComponents.Label {
                    id: loginText

                    anchors.top: face.bottom
                    anchors.topMargin: padding
                    anchors.horizontalCenter: parent.horizontalCenter

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
                    if (!usersList.interactive) return;
                    usersList.currentIndex = index;
                    usersList.forceActiveFocus();
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
                onClicked: startEnterUsernameScreen();
            }

            TooltipButton {
                id: suspendButton
                caption: i18n("Suspend")
                expand: menuBar.expand
                icon.name: "system-suspend"
                enabled: power.canSuspend;
                onClicked: power.suspend();
                Component.onCompleted: {
                    // hide labels of menubar buttons if the screen is too narrow
                    if (-menuBar.x - x > screen.width * 0.5) {
                        menuBar.expand = false
                    }
                }
            }

            TooltipButton {
                id: hibernateButton
                caption: i18n("Hibernate")
                expand: menuBar.expand
                icon.name: "system-suspend-hibernate"
                //Hibernate is a special case, lots of distros disable it, so if it's not enabled don't show it
                visible: power.canHibernate
                onClicked: power.hibernate();
            }

            TooltipButton {
                id: restartButton
                caption: i18n("Restart")
                expand: menuBar.expand
                icon.name: "system-reboot"
                enabled: power.canRestart
                onClicked: power.restart();
            }

            TooltipButton {
                id: shutdownButton
                caption: i18n("Shutdown")
                expand: menuBar.expand
                icon.name: "system-shutdown"
                enabled: power.canShutdown
                onClicked: power.shutdown();
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        visible: visibleScreen == screens.WaitScreen
    }
}
