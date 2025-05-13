/*
This file is part of LightDM-KDE.

Copyright (C) 2022 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15

Item {
    id: manager

    property Item activeScreen: {
        let window = manager.children[0].delegateWindow
        return window ? window.crop : null
    }

    property Component delegate

    Repeater {
        id: repeater
        model: screensModel
        delegate: delegateItem
    }

    function nextActiveScreen() {

        var screens = []
        // collect all children that are windows
        for (var i = 0; i < manager.children.length; ++i) {
            var window = manager.children[i].delegateWindow
            if (window) {
                screens.push(window)
            }
        }
        // determine the index of the current active screen
        var currentIndex = -1
        for (var i = 0; i < screens.length; ++i) {
            if (screens[i].crop == activeScreen) {
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

        activeScreen = screens[newIndex].crop
        activeScreen.forceActiveFocus()

        mouseCursor.move(newX, newY)
    }

    Component {
        id: delegateItem

        Item {

            property var delegateWindow: Window {

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
                        delegateWindow.requestActivate()
                        activeScreen = crop
                    }
                    onWidthChanged: {
                        // the width is supposed to change only at startup
                        if (crop != activeScreen) return
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
}
