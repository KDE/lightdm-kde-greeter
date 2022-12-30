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

    Component {
        id: delegateItem

        Item {

            property var delegateWindow: Window {

                x: geometry.x
                y: geometry.y
                width: geometry.width
                height: geometry.height
                visible: true

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        delegateWindow.requestActivate()
                        screenManager.activeScreen = parent
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
