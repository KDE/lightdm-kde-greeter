/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import QtTest

TestCase {
    id: testCase
    name: "Tests"

    width: 1920
    height: 1080

    property bool isFallbackSession: false
    property bool showDebugInfo: false

    ListModel {
        id: sessionsModel
    }

    ListModel {
        id: connectionsModel
        signal showDialog
        property var primary: QtObject {
            property string name: "fake_primary_name"
        }
        property bool networkManagerAvailable: false
        property bool networkingEnabled: false
    }
    property string localHostName: "test_host"

    ListModel {
        id: usersModel
        function indexForUserName(user) {
            return 0
        }
    }

    QtObject {
        id: power
        property bool canSuspend: false
        property bool canHibernate: false
        property bool canRestart: false
        property bool canShutdown: false
    }

    QtObject {
        id: greeter

        property string lastLoggedInUser: "last_logged"
        property bool authenticated: false
        property bool allowAutologin: false

        property bool testSessionStarted: false

        signal showPrompt
        signal showMessage
        signal authenticationComplete
        signal autologinTimerExpired
        signal fixupUsersListQueued

        function authenticate() {
        }

        function startSessionSync(session) {
            testSessionStarted = authenticated
            return authenticated
        }
    }

    QtObject {
        id: keyboard
        property int currentLayout: 0
        property list<QtObject> layouts: [
            QtObject {
                property var shortName: "name1"
                property int index: 0
            }
        ]
    }

    QtObject {
        id: params
        property int toolTipDelay: 1000
        property int toolTipTimeout: 5000
    }

    function nthChildrenStartsWith(items, startsWith, counter = 1) {
        for (let i in items) {
            let propertyName = items[i].toString()
            if (propertyName.startsWith(startsWith)) {
                if (counter == 1) {
                    return items[i]
                } else {
                    --counter
                }
            }
        }
    }

    function findMessagesView(primary) {

        let focusScope = nthChildrenStartsWith(primary.children, "QQuickFocusScope")
        if (!focusScope) return

        return nthChildrenStartsWith(focusScope.children, "QQuickListView_QML_")
    }

    function closeAllTheMessages(msgList) {
        let data = msgList.data[0]
        verify(data)
        let i = 1
        let item
        while (item = nthChildrenStartsWith(data.children, "KSvg::FrameSvgItem", i)) {

            let msgBody = item.children[0]
            verify(msgBody)

            let x = nthChildrenStartsWith(msgBody.children, "ToolButton_QMLTYPE")
            verify(x)

            x.click()

            ++i
        }
    }

    function test_hide_messages() {

        var c = Qt.createComponent("../themes/userbar/PrimaryScreen.qml")
        verify(c.status === Component.Ready)

        let primary = c.createObject(testCase, { width: 1920, height: 1080 })
        verify(primary)
        verify(primary.userReadTime == 48)

        let messages = nthChildrenStartsWith(primary.data, "QQmlListModel")
        verify(messages)

        let msgList = findMessagesView(primary)
        verify(msgList)

        let dissolveAnimation = msgList.dissolve
        verify(dissolveAnimation.defaultPause == 5000)

        let pauseAnimation = dissolveAnimation.animations[0]
        verify(pauseAnimation)

        let fadeOutAnimation = dissolveAnimation.animations[1]
        verify(fadeOutAnimation)

        dissolveAnimation.defaultPause = 400
        verify(pauseAnimation.duration == 400)

        fadeOutAnimation.duration = 10

        // Messages are hidden by default timeout after failed authentication

        greeter.authenticated = false
        primary.putMessage("1234") // 192 ms read time
        primary.putMessage("1234") // same messages do not lengthen reading time
        primary.putMessage("1234") // same messages do not lengthen reading time
        greeter.authenticationComplete()

        verify(messages.rowCount() == 3)
        wait(300)
        verify(messages.rowCount() == 3)
        wait(200)
        verify(messages.rowCount() == 0)

        // Messages viewing time is extended depending on the content after
        // failed authentication

        dissolveAnimation.defaultPause = 50
        dissolveAnimation.pause = 50
        verify(pauseAnimation.duration == 50)

        primary.putMessage("1234") // 192 ms read time
        primary.putMessage("4321") // 192 ms read time
        greeter.authenticationComplete()

        wait(250)
        verify(messages.rowCount() == 2)
        wait(250)
        verify(messages.rowCount() == 0)

        // extra time to read before starting the session

        greeter.authenticated = true
        greeter.testSessionStarted = false
        primary.putMessage("1234") // 192 ms read time
        primary.putMessage("4321") // 192 ms read time
        greeter.authenticationComplete()
        wait(300)

        verify(!greeter.testSessionStarted)
        verify(messages.rowCount() == 2)

        wait(200)

        verify(messages.rowCount() == 0)
        verify(greeter.testSessionStarted)

        // if close all the messages, the session starts immediately

        greeter.authenticated = true
        greeter.testSessionStarted = false
        primary.putMessage("1234") // 192 ms read time
        primary.putMessage("4321") // 192 ms read time
        greeter.authenticationComplete()
        wait(150)

        verify(!greeter.testSessionStarted)
        verify(messages.rowCount() == 2)

        closeAllTheMessages(msgList)

        verify(messages.rowCount() == 0)
        verify(greeter.testSessionStarted)
    }
}
