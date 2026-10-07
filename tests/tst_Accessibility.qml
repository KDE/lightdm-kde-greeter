/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import QtTest

TestCase {
    id: testCase
    name: "AccessibilityButtonTests"

    property var screen: Item {
        id: mockScreen
        property string lastTestMessage
        function putDissolvingMessage(text, type) {
            console.log("Mock Screen Message:", text, "Type:", type);
            lastTestMessage = text
        }
    }

    function cleanup() {
        // Reset state between tests to prevent leakage
        mockAccessibility.actuallySetScreenReaderActive(false, false)
    }

    function test_button_initial_state() {

        let c = Qt.createComponent("../themes/userbar/AccessibilityButton.qml")
        verify(c.status === Component.Ready)

        let button = c.createObject(testCase, { backend: mockAccessibility })
        verify(button)

        verify(mockAccessibility.screenReaderActive === false)

        button.clicked()

        let popup = findChild(button, "menu")
        verify(popup)
        verify(popup.visible === true)

        let menuItem = findChild(button, "screenReaderItem")
        verify(menuItem)

        let checkbox = findChild(button, "screenReaderCheckBox")
        verify(checkbox)

        // State should be synced on open
        verify(checkbox.checkState === Qt.Unchecked)

        button.destroy()
    }

    function test_toggle_screen_reader_on() {

        let c = Qt.createComponent("../themes/userbar/AccessibilityButton.qml")
        verify(c.status === Component.Ready)

        let button = c.createObject(testCase, { backend: mockAccessibility })
        verify(button)

        button.clicked()

        let popup = findChild(button, "menu")
        verify(popup)
        verify(popup.visible === true)

        let checkbox = findChild(button, "screenReaderCheckBox")
        verify(checkbox)

        // Initial state should be unchecked
        verify(checkbox.checkState === Qt.Unchecked)

        checkbox.clicked()

        // The backend property should not update immediately
        // The property will be updated when a signal from systemd is received
        verify(mockAccessibility.screenReaderActive === false)
        verify(checkbox.checkState === Qt.PartiallyChecked)

        // "signal" here
        mockAccessibility.actuallySetScreenReaderActive(true)

        verify(mockAccessibility.screenReaderActive === true)
        verify(checkbox.checkState === Qt.Checked)

        button.destroy()
    }

    function test_toggle_screen_reader_off() {

        let c = Qt.createComponent("../themes/userbar/AccessibilityButton.qml")
        verify(c.status === Component.Ready)

        let button = c.createObject(testCase, { backend: mockAccessibility, })
        verify(button)

        button.clicked()

        let popup = findChild(button, "menu")
        verify(popup)
        verify(popup.visible === true)

        let checkbox = findChild(button, "screenReaderCheckBox")
        verify(checkbox)

        mockAccessibility.actuallySetScreenReaderActive(true)

        // Checkbox should reflect the initial backend state (true)
        verify(checkbox.checkState === Qt.Checked)

        checkbox.clicked()

        // Backend should not update yet
        verify(mockAccessibility.screenReaderActive === true)
        verify(checkbox.checkState === Qt.PartiallyChecked)

        // "signal" here
        mockAccessibility.actuallySetScreenReaderActive(false)

        verify(mockAccessibility.screenReaderActive === false)
        verify(checkbox.checkState === Qt.Unchecked)

        button.destroy()
    }

    function test_backend_error_signal() {

        let c = Qt.createComponent("../themes/userbar/AccessibilityButton.qml")
        verify(c.status === Component.Ready)

        let button = c.createObject(testCase, { backend: mockAccessibility, })
        verify(button)

        mockAccessibility.screenReaderBackendError("Test Error")

        // the component received the signal and displayed it to the user
        verify(mockScreen.lastTestMessage === "Test Error")

        button.destroy()
    }
}
