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
import QtQuick 2.12
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    width: screenSize.width;
    height: screenSize.height;

    ScreenManager {
        id: screenManager
        delegate: Image {
            // default to keeping aspect ratio
            fillMode: config.readEntry("BackgroundKeepAspectRatio") == false ? Image.Stretch : Image.PreserveAspectCrop;
            //read from config, if there's no entry use plasma theme
            source: config.readEntry("Background") ? config.readEntry("Background"): plasmaTheme.wallpaperPath;
        }
    }

    Item { //recreate active screen at a sibling level which we can anchor in.
        id: activeScreen
        x: screenManager.activeScreen.x
        y: screenManager.activeScreen.y
        width: screenManager.activeScreen.width
        height: screenManager.activeScreen.height
    }


    Connections {
        target: greeter;
        onShowPrompt: {
            greeter.respond(passwordInput.text);
        }

        onAuthenticationComplete: {
            if(greeter.authenticated) {
                loginAnimation.start();
            }
            else {
                feedbackLabel.text = i18n("Sorry, incorrect password please try again.");
                passwordInput.selectAll()
                passwordInput.forceActiveFocus()
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
                text: i18n("Log in as guest");
                onClicked: login()
            }

            Row {
                visible: !useGuestOption.checked
                spacing: 10
                width: childrenRect.width
                height: childrenRect.height
                
                Grid {
                    columns: 2
                    spacing: 15
                    
                    PlasmaCore.IconItem {
                        source: "user"
                        height: usernameInput.height;
                        width: usernameInput.height;
                    }

                    PlasmaComponents.TextField {
                        id: usernameInput;
                        placeholderText: i18n("Username");
                        text: greeter.lastLoggedInUser
                        onAccepted: {
                            passwordInput.focus = true;
                        }
                        width: 160
                        
                        Component.onCompleted: {
                            //if the username field has text, focus the password, else focus the username
                            if (usernameInput.text) {
                                passwordInput.focus = true;
                            } else {
                                usernameInput.focus = true;
                            }
                        }
                        KeyNavigation.tab: passwordInput
                    }

                    PlasmaCore.IconItem {
                        source: "object-locked"
                        height: passwordInput.height;
                        width: passwordInput.height;
                    }

                    PlasmaComponents.TextField {
                        id: passwordInput
                        echoMode: TextInput.Password
                        placeholderText: i18n("Password")
                        onAccepted: {
                            login();
                        }
                        width: 160
                        KeyNavigation.backtab: usernameInput
                        KeyNavigation.tab: loginButton
                    }
                }
                
                ToolButton {
                    id: loginButton
                    anchors.verticalCenter: parent.verticalCenter
                    iconSource: "go-next"
                    onClicked: {
                        login();
                    }
                    KeyNavigation.backtab: passwordInput
                    KeyNavigation.tab: usernameInput
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
}
