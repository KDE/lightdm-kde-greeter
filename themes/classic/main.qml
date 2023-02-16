/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

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
import QtQuick.Controls 2.15 as QC
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
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
            fillMode: (x => x !== "" ? Number(x) : Image.PreserveAspectCrop)(config.readEntry("BackgroundFillMode"))
            //read from config, if there's no entry use plasma theme
            source: config.readEntry("Background") ? config.readEntry("Background") : plasmaTheme.wallpaperPath;
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
            }
            else {
                feedbackLabel.text = i18n("Sorry, incorrect password please try again.");
                visibleScreen = VisibleScreenEnum.VisibleScreen.LoginScreen;
            }
        }
    }

    function login() {
        if (useGuestOption.checked) {
            greeter.authenticateAsGuest();
        } else {
            greeter.authenticate(usernameInput.text);
        }
    }

    function doSessionSync() {
        var session = optionsMenu.currentSession;
        var startresult = greeter.startSessionSync(session);
        if (!startresult) {
            visibleScreen = VisibleScreenEnum.VisibleScreen.LoginScreen;
            startSessionFailureAnimation.start();
        }
    }

    ParallelAnimation {
        id: loginAnimation
        NumberAnimation { target: dialog; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        NumberAnimation { target: powerDialog; property: "opacity"; to: 0; duration: 400; easing.type: Easing.InOutQuad }
        onFinished: doSessionSync()
    }

    ParallelAnimation {
        id: startSessionFailureAnimation
        NumberAnimation { target: dialog; property: "opacity"; to: 1; duration: 400; easing.type: Easing.InOutQuad }
    }

    PlasmaCore.FrameSvgItem {
        id: dialog;
        imagePath: "widgets/background"
        anchors.centerIn: activeScreen;

        width: childrenRect.width + 55;
        height: childrenRect.height + 55;

        Column {
            spacing: 15
            anchors.centerIn: parent

            Image {
                id: logo
                source: config.readEntry("Logo")
                fillMode: Image.PreserveAspectFit
                height: 100
                anchors.horizontalCenter: parent.horizontalCenter
                smooth: true
            }


            PlasmaComponents.Label {
                anchors.horizontalCenter: parent.horizontalCenter;
                id: feedbackLabel;
                font.pointSize: 9
                text: config.readEntry("GreetMessage").replace("%hostname%", greeter.hostname);
            }

            //if guest checked, replace the normal "user/pass" textboxes with a big login button
            PlasmaComponents.Button {
                visible: useGuestOption.checked
                enabled: visible
                text: i18n("Log in as guest");
                onClicked: login()
            }

            Row {
                visible: !useGuestOption.checked
                enabled: visible
                spacing: 10
                width: childrenRect.width
                height: childrenRect.height

                PlasmaCore.IconItem {
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.LoginScreen
                    enabled: visible
                    source: "user"
                    height: usernameInput.height;
                    width: usernameInput.height;
                }

                PlasmaComponents.TextField {
                    id: usernameInput;
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.LoginScreen
                    enabled: visible
                    placeholderText: i18n("Username");
                    text: greeter.lastLoggedInUser
                    onAccepted: {
                        visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                        login();
                    }
                    width: 160
                    
                    Component.onCompleted: {
                        usernameInput.focus = true;
                    }
                    KeyNavigation.tab: inputButton
                }

                PlasmaComponents.Label {
                    id: echoOnLabel
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen
                    enabled: visible
                }

                PlasmaComponents.TextField {
                    id: echoOnInput
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen
                    enabled: visible
                    onAccepted: {
                        visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                        greeter.respond(echoOnInput.text);
                    }
                    width: 160
                    KeyNavigation.tab: inputButton
                }

                PlasmaComponents.Label {
                    id: echoOffLabel
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen
                    enabled: visible
                }

                PlasmaComponents.TextField {
                    id: echoOffInput
                    visible: visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen
                    enabled: visible
                    echoMode: TextInput.Password
                    onAccepted: {
                        visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                        greeter.respond(echoOffInput.text);
                    }
                    width: 160
                    KeyNavigation.tab: inputButton
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
                
                ToolButton {
                    id: inputButton
                    visible: (visibleScreen == VisibleScreenEnum.VisibleScreen.LoginScreen) || (visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen) || (visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen)
                    enabled: visible
                    anchors.verticalCenter: parent.verticalCenter
                    iconSource: "go-next"
                    onClicked: {
                        if (visibleScreen == VisibleScreenEnum.VisibleScreen.LoginScreen) {
                            visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                            login();
                        }

                        if (visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOnScreen) {
                            visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                            greeter.respond(echoOnInput.text);
                        }

                        if (visibleScreen == VisibleScreenEnum.VisibleScreen.PromptEchoOffScreen) {
                            visibleScreen = VisibleScreenEnum.VisibleScreen.BlankScreen;
                            greeter.respond(echoOffInput.text);
                        }
                    }
                }
            }

            Item {
                height: 10
            }
            
            Row {               
                spacing: 8;
                PlasmaComponents.Button {
                    iconSource: "system-shutdown"
                    onClicked: {
                        if (powerDialog.opacity == 1) {
                            powerDialog.opacity = 0;
                        } else {
                            powerDialog.opacity = 1;
                        }
                    }
                }

                PlasmaComponents.ContextMenu {
                    id: sessionMenu
                    visualParent: sessionMenuOption.action
                }

                Repeater {
                    model: sessionsModel
                    delegate : PlasmaComponents.MenuItem {
                        text: model.display
                        checkable: true
                        checked: model.key === optionsMenu.currentSession
                        onClicked : {
                            optionsMenu.currentSession = model.key;
                        }

                        Component.onCompleted: {
                            parent = sessionMenu
                        }
                    }
                    Component.onCompleted: {
                        model.showLastUsedSession = true
                    }
                }

                PlasmaComponents.ContextMenu {
                    id: optionsMenu
                    visualParent: optionsButton
                    //in LightDM  "" means "last user session". whereas NULL is default.
                    property string currentSession: ""
                    PlasmaComponents.MenuItem {
                        id: useGuestOption
                        text: i18n("Log in as guest")
                        checkable: true
                        enabled: greeter.hasGuestAccount
                    }
                    PlasmaComponents.MenuItem {
                        separator: true
                    }

                    PlasmaComponents.MenuItem {
                        text: i18n("Session")
                        id: sessionMenuOption
                        onClicked: sessionMenu.open()
                    }
                }

                PlasmaComponents.Button {
                    id: optionsButton
                    iconSource: "system-log-out"
                    onClicked: {
                        optionsMenu.open();
                    }
                }

                PlasmaComponents.Button {
                    QC.ToolTip.delay: 1000
                    QC.ToolTip.visible: hovered
                    QC.ToolTip.text: i18nc("Button to show/hide virtual keyboard", "Virtual Keyboard")
                    iconSource: inputPanel.keyboardEnabled ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
                    onClicked: inputPanel.switchState()
                    visible: inputPanel.item
                }
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: powerDialog
        anchors.top: dialog.bottom
        anchors.topMargin: 3
        anchors.horizontalCenter: activeScreen.horizontalCenter
        imagePath: "translucent/dialogs/background"
        opacity: 0
        enabled: powerDialog.opacity != 0;

        Behavior on opacity { PropertyAnimation { duration: 500} }

        width: childrenRect.width + 30;
        height: childrenRect.height + 30;

        Row {
            spacing: 5
            anchors.centerIn: parent

            PlasmaComponents.Button {
                text: i18n("Suspend")
                iconSource: "system-suspend"
                enabled: power.canSuspend;
                onClicked: {power.suspend();}
            }
            
            PlasmaComponents.Button {
                text: i18n("Hibernate")
                iconSource: "system-suspend-hibernate"
                //Hibernate is a special case, lots of distros disable it, so if it's not enabled don't show it
                visible: power.canHibernate;
                onClicked: {power.hibernate();}
            }

            PlasmaComponents.Button {
                text: i18n("Restart")
                iconSource: "system-reboot"
                enabled: power.canRestart;
                onClicked: {power.restart();}
            }
            
            PlasmaComponents.Button {
                text: i18n("Shutdown")
                iconSource: "system-shutdown"
                enabled: power.canShutdown;
                onClicked: {power.shutdown();}
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
}
