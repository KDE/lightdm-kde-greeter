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

        ListElement { key: "plasma"; display: "Plasma (X11)" }
        ListElement { key: "plasma-wayland"; display: "Plasma (Wayland)" }
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
            for (let i = 0; i < usersModel.count; ++i) {
                if (usersModel.get(i).name === user) {
                    return i
                }
            }
            return -1
        }

        function sessionForUserName(name) {
            let index = indexForUserName(name)
            if (index < 0) return ""
            return usersModel.get(index).session
        }

        ListElement {
            session: "session0"
            name: "username0"
            display: "display0"
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

    function findUsersList(primary) {

        let focusScope = nthChildrenStartsWith(primary.children, "QQuickFocusScope")
        if (!focusScope) return

        let centerPanel = nthChildrenStartsWith(focusScope.children, "QQuickColumn_QML_")
        if (!centerPanel) return

        return nthChildrenStartsWith(centerPanel.children, "QQuickListView_QML_")
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

    function createPrimary() {
        var c = Qt.createComponent("../themes/userbar/PrimaryScreen.qml")
        verify(c.status === Component.Ready)

        let primary = c.createObject(testCase, { width: 1920, height: 1080 })
        verify(primary)
        return primary
    }

    function test_hide_messages() {

        testSetup.resetGreeter("", "")
        greeter.setSelectUserHint("")
        greeter.setSelectGuestHint(false)

        var c = Qt.createComponent("../themes/userbar/PrimaryScreen.qml")
        verify(c.status === Component.Ready)

        let primary = c.createObject(testCase, { width: 1920, height: 1080 })
        verify(primary)
        verify(primary.userReadTime == 48)

        let messages = nthChildrenStartsWith(primary.data, "QQmlListModel")
        verify(messages)

        let msgList = findMessagesView(primary)
        verify(msgList)

        let usersList = findUsersList(primary)
        verify(usersList)

        usersList.enabled = true
        verify(usersList.activeFocus)

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

        greeter.setAuthenticated(false)
        primary.putMessage("1234") // 192 ms read time
        primary.putMessage("1234") // same messages are not displayed
        primary.putMessage("1234") // same messages are not displayed
        greeter.authenticationComplete()

        verify(messages.rowCount() == 1)
        wait(500)
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

        greeter.setAuthenticated(true)
        primary.putMessage("1234") // 192 ms read time
        primary.putMessage("4321") // 192 ms read time
        greeter.authenticationComplete()
        wait(300)

        verify(messages.rowCount() == 2)

        wait(200)

        verify(messages.rowCount() == 0)

        // if close all the messages, the session starts immediately

        greeter.setAuthenticated(true)
        primary.putMessage("1234") // 192 ms read time
        primary.putMessage("4321") // 192 ms read time
        greeter.authenticationComplete()
        wait(150)

        verify(messages.rowCount() == 2)

        closeAllTheMessages(msgList)

        verify(messages.rowCount() == 0)

        // focus should not move from usersList when closing message
        verify(usersList.activeFocus)
    }

    function test_preferred_user_select_user() {
        testSetup.resetGreeter("last_logged", "last_session")
        greeter.setSelectUserHint("domainuser")
        greeter.setSelectGuestHint(true)

        let primary = createPrimary()

        verify(primary.preferredUser() === "domainuser")
    }

    function test_preferred_user_guest_hint() {
        testSetup.resetGreeter("last_logged", "last_session")
        greeter.setSelectUserHint("")
        greeter.setSelectGuestHint(true)

        let primary = createPrimary()

        verify(primary.preferredUser() === greeter.guestLoginName)
    }

    function test_preferred_user_last_logged_in() {
        testSetup.resetGreeter("last_logged", "last_session")
        greeter.setSelectUserHint("")
        greeter.setSelectGuestHint(false)

        let primary = createPrimary()

        verify(primary.preferredUser() === "last_logged")
    }

    function test_preferred_user_empty() {
        testSetup.resetGreeter("", "")
        greeter.setSelectUserHint("")
        greeter.setSelectGuestHint(false)

        let primary = createPrimary()

        verify(primary.preferredUser() === "")
    }

    function test_session_for_user_from_model() {
        // username0 is in the users model with session "session0", but the
        // history remembers a different session for it
        testSetup.resetGreeter("username0", "plasma-wayland")

        let primary = createPrimary()

        verify(primary.sessionForUser("username0") === "session0")
    }

    function test_session_for_user_from_history() {
        // domainuser is not in the users model, only in the history
        testSetup.resetGreeter("domainuser", "plasma-wayland")

        let primary = createPrimary()

        verify(primary.sessionForUser("domainuser") === "plasma-wayland")
    }

    function test_session_for_user_empty() {
        testSetup.resetGreeter("", "")

        let primary = createPrimary()

        verify(primary.sessionForUser("unknownuser") === "")
    }

    function test_login_as_other_user_prefills() {
        testSetup.resetGreeter("domainuser", "plasma-wayland")
        greeter.setSelectUserHint("")
        greeter.setSelectGuestHint(false)

        let primary = createPrimary()

        primary.loginAsOtherUser()

        // find sessionButton in the tree
        function findSessionButton(item, depth = 0) {
            if (depth > 5) return null
            let name = item.toString()
            if (name.startsWith("ListButton")) {
                return item
            }
            for (let i in item.children) {
                let found = findSessionButton(item.children[i], depth + 1)
                if (found) return found
            }
            return null
        }
        let sessionBtn = findSessionButton(primary)
        verify(sessionBtn)
        // the session remembered in the history is selected
        verify(sessionBtn.currentData() === "plasma-wayland")
    }

    function test_users_list_current_index() {
        // add more users to the model
        usersModel.append({ session: "s1", name: "user1", display: "User 1" })
        usersModel.append({ session: "s2", name: "user2", display: "User 2" })

        testSetup.resetGreeter("user2", "s2")
        greeter.setSelectUserHint("")
        greeter.setSelectGuestHint(false)

        let primary = createPrimary()

        let usersList = findUsersList(primary)
        verify(usersList)
        verify(usersList.currentIndex === 2)

        // no preferred user: index falls back to 0
        testSetup.resetGreeter("", "")
        wait(100)
        verify(usersList.currentIndex === 0)
    }
}
