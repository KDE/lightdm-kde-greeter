/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2023-2024 Anton Golubev <golubevan@altlinux.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmaComponents.ToolButton {
    id: root

    property int currentIndex: keyboard.currentLayout
    onCurrentIndexChanged: keyboard.currentLayout = currentIndex

    icon.name: "globe"

    text: keyboard.layouts[currentIndex].shortName

    function nextLayout() {
        keyboard.currentLayout = (keyboard.currentLayout + 1) % keyboard.layouts.length
    }

    onPressed: nextLayout()
    Keys.onReturnPressed: nextLayout()
    Keys.onEnterPressed: nextLayout()

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
        onClicked: (mouse) => {
            if (mouse.button & Qt.LeftButton) {
                nextLayout()
                root.keyboardLayoutTriggered()
            } else if (mouse.button & Qt.RightButton) {
                menu.popup(root, 0, 0)
            } else {
                menu.dismiss()
            }
        }
    }

    signal keyboardLayoutTriggered()

    PlasmaComponents.Menu {
        id: menu

        Instantiator {
            id: instantiator
            model: keyboard.layouts
            onObjectAdded: menu.insertItem(index, object)
            onObjectRemoved: menu.removeItem(object)
            delegate: PlasmaComponents.MenuItem {
                text: i18nd("xkeyboard-config", modelData.longName)
                onTriggered: {
                    keyboard.currentLayout = model.index
                    root.keyboardLayoutTriggered()
                }
            }
        }
        onAboutToHide: {
            root.keyboardLayoutTriggered()
        }
    }
}
