/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021, 2022 Aleksei Nikiforov <darktemplar@basealt.ru>

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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

Item {
    id: screen
    width: screenSize.width;
    height: screenSize.height;

    VisibleScreenEnum {
        id: visibleScreenEnum
    }

    property int visibleScreen: VisibleScreenEnum.VisibleScreen.LoginScreen

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
        height: inputPanel.item ? inputPanel.item.y : wholeScreen.height
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

    Connections {
        target: greeter;

        onShowPrompt: {
            if (type == 0) {
                echoOnLabel.text = text;
                echoOnInput.text = "";
                visibleScreen = VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen;
                echoOnInput.forceActiveFocus();
            } else {
                echoOffLabel.text = text;
                echoOffInput.text = "";
                visibleScreen = VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen;
                echoOffInput.forceActiveFocus();
            }
        }

        onShowMessage: {
            if (type == 0) {
                infoMsgLabel.text = text;
                visibleScreen = VisibleScreenEnum.VisibleScreen.InfoMsgScreen;
            } else {
                errorMsgLabel.text = text;
                visibleScreen = VisibleScreenEnum.VisibleScreen.ErrorMsgScreen;
            }
        }

        onAuthenticationComplete: {
            if(greeter.authenticated) {
                visibleScreen = VisibleScreenEnum.VisibleScreen.SuccessScreen;
                loginAnimation.start();
            } else {
                feedbackLabel.text = i18n("Sorry, incorrect password. Please try again.");
                feedbackLabel.showFeedback();
                visibleScreen = VisibleScreenEnum.VisibleScreen.LoginScreen;
            }
        }
    }

    function doSessionSync() {
       var session = sessionButton.dataForIndex(sessionButton.currentIndex);
       if (session == "") {
           session = "default";
       }
       var startresult = greeter.startSessionSync(session);
        if (!startresult) {
            visibleScreen = VisibleScreenEnum.VisibleScreen.LoginScreen;
            startSessionFailureAnimation.start();
        }
    }

    ParallelAnimation {
        id: loginAnimation
        NumberAnimation { target: welcomeLabel; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: feedbackLabel; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: usersList; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: loginButtonItem; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: sessionButton; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: powerBar; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: virtualKeyboardBar; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        onFinished: doSessionSync()
    }

    ParallelAnimation {
        id: startSessionFailureAnimation
        NumberAnimation { target: welcomeLabel; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: feedbackLabel; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: usersList; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: loginButtonItem; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: sessionButton; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: powerBar; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: virtualKeyboardBar; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
    }

    Component.onCompleted: {
        setTabOrder([usersList, loginButtonItem, sessionButton, suspendButton, hibernateButton, restartButton, shutdownButton]);
        usersList.forceActiveFocus();
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

    PlasmaComponents.Label {
        visible: false
        id: welcomeLabel
        anchors.horizontalCenter: activeScreen.horizontalCenter
        anchors.top: activeScreen.top
        anchors.topMargin: 5
        font.pointSize: 14
        text: i18n("Welcome to %1", greeter.hostname);
    }

    FeedbackLabel {
        id: feedbackLabel
        anchors.horizontalCenter: activeScreen.horizontalCenter
        anchors.top: welcomeLabel.bottom
        anchors.topMargin: 5
        font.pointSize: 14
    }

    property int userItemWidth: 120
    property int userItemHeight: 80
    property int userFaceSize: 64

    property int padding: 6

    Component {
        id: userDelegate

        Item {
            id: wrapper

            property bool isCurrent: ListView.isCurrentItem

            /* Expose current item info to the outer world. I can't find
             * another way to access this from outside the list. */
            property string username: model.name
            property string usersession: model.session

            width: userItemWidth
            height: userItemHeight

            opacity: isCurrent ? 1.0 : 0.618

            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                }
            }

            PlasmaCore.FrameSvgItem {
                id: frameFocus
                anchors {
                    fill: frame
                    leftMargin: -margins.left
                    topMargin: -margins.top
                    bottomMargin: -margins.bottom
                    rightMargin: -margins.right
                }
                imagePath: "widgets/button"
                prefix: "hover"
                visible: wrapper.isCurrent
                opacity: wrapper.activeFocus ? 1 : 0
                Behavior on opacity {
                    NumberAnimation { duration: 100 }
                }
            }

            PlasmaCore.FrameSvgItem {
                id: frame

                anchors {
                    top: face.top
                    bottom: loginText.bottom
                    left: loginText.left
                    right: loginText.right
                    topMargin: -padding
                }

                imagePath: "widgets/lineedit"
                prefix: "base"
                enabledBorders: "NoBorder"
            }

            PlasmaCore.FrameSvgItem {
                id: frameHover
                anchors.fill: frame
                imagePath: "widgets/lineedit"
                prefix: "hover"
                opacity: (mouseArea.containsMouse && !(wrapper.isCurrent && wrapper.activeFocus)) ? 1 : 0
                Behavior on opacity {
                    NumberAnimation { duration: 100 }
                }
            }

            Face {
                id: face
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

                width: parent.width - padding * 2
                text: display
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    wrapper.ListView.view.currentIndex = index;
                    wrapper.ListView.view.forceActiveFocus();
                }
            }
        }
    }

    function startLogin() {
        visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
        var username = usersList.currentItem.username;
        if (username == greeter.guestLoginName) {
            greeter.authenticateAsGuest();
        } else {
            greeter.authenticate(username);
        }
    }

    function startLoginWithoutUsername() {
        visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
        greeter.authenticate();
    }

    function indexForUserName(name) {
        var index;
        for (index = 0; index < usersList.count; ++index) {
            if (usersList.contentItem.children[index].username == name) {
                return index;
            }
        }
        return 0;
    }

    ListView {
        id: usersList
        anchors {
            horizontalCenter: activeScreen.horizontalCenter
            bottom: loginButtonItem.top
            bottomMargin: 24
        }
        width: activeScreen.width
        height: userItemHeight
        currentIndex: indexForUserName(greeter.lastLoggedInUser)
        model: usersModel
        visible: visibleScreen == VisibleScreenEnum.VisibleScreen.LoginScreen
        enabled: visible

        cacheBuffer: count * 80

        delegate: userDelegate

        orientation: ListView.Horizontal

        highlightRangeMode: ListView.StrictlyEnforceRange
        preferredHighlightBegin: width / 2 - userItemWidth / 2
        preferredHighlightEnd: width / 2 + userItemWidth / 2
    }

    Column {
        id: loginButtonItem
        anchors {
            horizontalCenter: activeScreen.horizontalCenter
            bottom: activeScreen.verticalCenter
        }
        spacing: 15

        PlasmaCore.FrameSvgItem {
            id: dialog;
            imagePath: "widgets/background"

            visible: (visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen) || (visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen) || (visibleScreen == VisibleScreenEnum.VisibleScreen.InfoMsgScreen) || (visibleScreen == VisibleScreenEnum.VisibleScreen.ErrorMsgScreen)

            width: childrenRect.width + 55;
            height: childrenRect.height + 55;

            Row {
                spacing: 10
                anchors.centerIn: parent
                height: childrenRect.height

                PlasmaComponents.Label {
                    id: echoOnLabel
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen
                    enabled: visible

                    Behavior on opacity {
                        NumberAnimation { duration: 100 }
                    }
                }

                PlasmaComponents.TextField {
                    id: echoOnInput
                    width: 200
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen
                    enabled: visible

                    onAccepted: {
                        visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                        greeter.respond(echoOnInput.text);
                    }

                    PlasmaComponents.ToolButton {
                        id: echoOnInputButton
                        anchors {
                            right: parent.right
                            rightMargin: y
                            verticalCenter: parent.verticalCenter
                        }
                        width: implicitWidth
                        height: width

                        iconSource: "go-jump-locationbar"
                        onClicked: {
                            visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                            greeter.respond(echoOnInput.text);
                        }
                    }

                    Behavior on opacity {
                        NumberAnimation { duration: 100 }
                    }
                }

                PlasmaComponents.Label {
                    id: echoOffLabel
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen
                    enabled: visible

                    Behavior on opacity {
                        NumberAnimation { duration: 100 }
                    }
                }

                PlasmaComponents.TextField {
                    id: echoOffInput
                    width: 200
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen
                    enabled: visible

                    echoMode: TextInput.Password
                    onAccepted: {
                        visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                        greeter.respond(echoOffInput.text);
                    }

                    PlasmaComponents.ToolButton {
                        id: echoOffInputButton
                        anchors {
                            right: parent.right
                            rightMargin: y
                            verticalCenter: parent.verticalCenter
                        }
                        width: implicitWidth
                        height: width

                        iconSource: "go-jump-locationbar"
                        onClicked: {
                            visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                            greeter.respond(echoOffInput.text);
                        }
                    }

                    Behavior on opacity {
                        NumberAnimation { duration: 100 }
                    }
                }

                PlasmaComponents.Label {
                    id: infoMsgLabel
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.InfoMsgScreen
                    enabled: visible
                }

                PlasmaComponents.Label {
                    id: errorMsgLabel
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.ErrorMsgScreen
                    enabled: visible
                }
            }
        }

        PlasmaComponents.Button {
            id: loginButton
            anchors.horizontalCenter: parent.horizontalCenter
            width: userFaceSize + 2 * padding
            visible: visibleScreen == VisibleScreenEnum.VisibleScreen.LoginScreen
            enabled: visible

            iconSource: "go-jump-locationbar"
            text: i18n("Log in")
            onClicked: startLogin();

            Behavior on opacity {
                NumberAnimation { duration: 100 }
            }
        }
    }

    ListButton {
        id: sessionButton
        anchors {
            top: loginButtonItem.bottom
            topMargin: 24
            bottom: powerBar.top
            horizontalCenter: activeScreen.horizontalCenter
        }

        model: sessionsModel
        dataRole: "key"
        currentIndex: {
            var index = indexForData(usersList.currentItem.usersession)
            if (index >= 0) {
                return index;
            }
            index = indexForData(greeter.defaultSession)
            if (index >= 0) {
                return index;
            }
            return 0;
        }
    }

    PlasmaCore.FrameSvgItem {
        id: virtualKeyboardBar
        anchors.bottom: wholeScreen.bottom
        anchors.left: wholeScreen.left
        width: childrenRect.width + margins.right
        height: childrenRect.height + margins.top
        imagePath: "widgets/background"

        enabledBorders: "RightBorder|TopBorder"

        Row {
            spacing: 5
            x: parent.margins.left
            y: parent.margins.top

            PlasmaComponents3.ToolButton {
                text: i18ndc("kcm_lightdm", "Button to show/hide virtual keyboard", "Virtual Keyboard")
                icon.name: inputPanel.keyboardEnabled ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
                onClicked: inputPanel.switchState()
                visible: inputPanel.item
            }
        }
    }

    // Bottom "Power" bar
    PlasmaCore.FrameSvgItem {
        id: powerBar
        anchors.bottom: wholeScreen.bottom
        anchors.right: wholeScreen.right
        width: childrenRect.width + margins.left
        height: childrenRect.height + margins.top
        imagePath: "widgets/background"

        enabledBorders: "LeftBorder|TopBorder"

        Row {
            spacing: 5
            x: parent.margins.left
            y: parent.margins.top

            PlasmaComponents3.ToolButton {
                id: loginAsOtherButton
                text: i18n("Log in as another user")
                icon.name: "go-jump-locationbar"
                visible: visibleScreen == VisibleScreenEnum.VisibleScreen.LoginScreen
                enabled: visible
                onClicked: startLoginWithoutUsername();
            }

            PlasmaComponents3.ToolButton {
                id: suspendButton
                text: i18n("Suspend")
                icon.name: "system-suspend"
                enabled: power.canSuspend;
                onClicked: power.suspend();
            }

            PlasmaComponents3.ToolButton {
                id: hibernateButton
                text: i18n("Hibernate")
                icon.name: "system-suspend-hibernate"
                //Hibernate is a special case, lots of distros disable it, so if it's not enabled don't show it
                visible: power.canHibernate
                onClicked: power.hibernate();
            }

            PlasmaComponents3.ToolButton {
                id: restartButton
                text: i18n("Restart")
                icon.name: "system-reboot"
                enabled: power.canRestart
                onClicked: power.restart();
            }

            PlasmaComponents3.ToolButton {
                id: shutdownButton
                text: i18n("Shutdown")
                icon.name: "system-shutdown"
                enabled: power.canShutdown
                onClicked: power.shutdown();
            }
        }
    }
}
