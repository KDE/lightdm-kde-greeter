/*
This file is part of LightDM-KDE.

Copyright (C) 2022 Anton Golubev <golubevan@altlinux.org>

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
import QtQuick.Window 2.15

Item {
    id: manager

    property Item activeScreen
    property Component delegate

    Repeater {
        id: repeater
        model: screensModel
        delegate: delegateItem
    }

    Component.onCompleted: {
        activeScreen = manager.children[0].delegateWindow.contentItem
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
            if (screens[i].contentItem == activeScreen) {
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

        activeScreen = screens[newIndex].contentItem
        screens[newIndex].requestActivate()

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

                property var mouseArea: mouseArea

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        delegateWindow.requestActivate()
                        screenManager.activeScreen = parent
                    }
                    onWidthChanged: {
                        // the width is supposed to change only at startup
                        if (parent != activeScreen) return
                        // put the mouse cursor inside the primary window
                        var newX = delegateWindow.x + mouseArea.width * 0.3
                        var newY = delegateWindow.y + mouseArea.height * 0.3
                        mouseCursor.move(newX, newY)
                    }
                }

                Loader {
                    property var thisScreen: parent
                    sourceComponent: manager.delegate
                    anchors.fill: parent
                }
            }
        }
    }
}
