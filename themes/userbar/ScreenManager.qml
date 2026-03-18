/*
This file is part of LightDM-KDE.

Copyright (C) 2022-2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15

Item {
    id: manager

    property QtObject activeScreen

    onActiveScreenChanged: {
        if (activeScreen) screensModel.setFocusedWindow(activeScreen)
    }

    property Component delegate

    property var windows: Instantiator {
        id: repeater
        model: screensModel
        delegate: delegateWindow
    }

    function nextActiveScreen() {

        if (manager.windows.count < 2) return;

        var screens = []
        for (let i = 0; i < manager.windows.count; ++i) {
            screens.push(manager.windows.objectAt(i))
        }
        // determine the index of the current active screen
        var currentIndex = -1
        for (var i = 0; i < screens.length; ++i) {
            if (screens[i] == activeScreen) {
                currentIndex = i
                break
            }
        }

        if (currentIndex < 0) {
            console.log("WARNING: ScreenManager - unable to find reference to active screen in array of active screens")
            return
        }

        var newIndex = (currentIndex + 1) % screens.length

        var areaFrom = screens[currentIndex].mouseArea
        var areaTo = screens[newIndex].mouseArea

        // move the mouse cursor to proportionally the same position on the new screen
        var xRatio = areaFrom.mouseX / areaFrom.width
        var yRatio = areaFrom.mouseY / areaFrom.height

        // guarantee that the mouse cursor will move to the desired screen
        if (xRatio <= 0) xRatio = 0.5
        if (yRatio <= 0) yRatio = 0.5

        var newX = screens[newIndex].x + xRatio * areaTo.width
        var newY = screens[newIndex].y + yRatio * areaTo.height

        mouseCursor.move(newX, newY)
    }

    Component {
        id: delegateWindow

        Window {
            id: window

            x: geometry.x
            y: geometry.y
            width: geometry.width
            height: geometry.height
            visible: true
            color: "black"

            property var mouseArea: mouseArea
            property var storedFocusItem
            property alias loader: loader

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    activeScreen = window
                }
            }

            Component.onCompleted: {
                if (screensModel.windowIsOnPrimaryScreen(window)) {
                    activeScreen = window
                }
            }

            Loader {
                id: loader

                focus: true
                anchors.fill: parent

                sourceComponent: manager.delegate
            }

            // need to keep track of where the previous focus was, so that you can return the focus when you click somewhere else
            onActiveFocusItemChanged: {
                if (storedFocusItem) desktop.previousFocusItem = storedFocusItem
                storedFocusItem = activeFocusItem
            }
        }
    }
}
