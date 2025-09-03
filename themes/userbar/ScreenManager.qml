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
        if (activeScreen) activeScreen.requestActivate()
    }

    property Component delegate

    property var windows: Instantiator {
        id: repeater
        model: screensModel
        delegate: delegateWindow
    }

    function nextActiveScreen() {

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

        var newX = screens[newIndex].x + xRatio * areaTo.width
        var newY = screens[newIndex].y + yRatio * areaTo.height

        activeScreen = screens[newIndex]

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
            property alias crop: crop

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    activeScreen = window
                }
                onWidthChanged: {
                    // the width is supposed to change only at startup
                    if (window != activeScreen) return
                    // put the mouse cursor inside the primary window
                    var newX = delegateWindow.x + mouseArea.width * 0.3
                    var newY = delegateWindow.y + mouseArea.height * 0.3
                    mouseCursor.move(newX, newY)
                }
            }

            Loader {
                id: crop

                property var cropRatio: {

                    let ratioOption = "MaxScreenRatio"
                    let defaultRatio = [ 16, 9 ]

                    let ratioStr = config.readEntry(ratioOption)
                    if (!ratioStr) return defaultRatio

                    let ratio = ratioStr.split(":").map(Number)

                    if (ratio.length != 2 || isNaN(ratio[0]) || isNaN(ratio[1]) || ratio[0] < 1 || ratio[1] < 1 || ratio[0] / ratio[1] < 1) {
                        console.warn("ScreenManager: bad option " + ratioOption + " (" + ratioStr + ")")
                        return defaultRatio
                    }
                    return ratio
                }

                focus: true
                height: parent.height
                width: Math.min(Math.round(parent.height * cropRatio[0] / cropRatio[1]), parent.width)
                x: Math.round((parent.width - width) * 0.5)
                y: parent.y

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
