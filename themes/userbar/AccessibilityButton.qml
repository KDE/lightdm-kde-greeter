/*
This file is part of LightDM-KDE.
import org.kde.plasma.components 3.0 as PlasmaComponents

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import QtQuick.Controls
import org.kde.plasma.components 3.0 as PC

TooltipButton {
    id: root

    required property var backend

    onClicked: {
        menu.open()
    }

    signal popupEnded()

    PC.Popup {
        id: menu
        objectName: "menu"

        y: -height + parent.height
        width: contentWidth + leftPadding + rightPadding

        focus: true

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Column {
            PC.MenuItem {
                id: screenReaderItem
                objectName: "screenReaderItem"

                text: i18nc("Button to enable/disable screen reader", "Screen Reader")

                checkable: true

                focus: true
                highlighted: focus

                indicator: PC.CheckBox {
                    id: checkbox
                    objectName: "screenReaderCheckBox"

                    x: parent.leftPadding
                    anchors.verticalCenter: parent.verticalCenter

                    tristate: true

                    onClicked: screenReaderItem.triggered()
                }

                onTriggered: {
                    checkbox.checkState = Qt.PartiallyChecked
                    backend.screenReaderActive = !backend.screenReaderActive
                }

                Connections {
                    target: backend
                    function onScreenReaderActiveChanged() {
                        checkbox.checkState = backend.screenReaderActive ? Qt.Checked : Qt.Unchecked
                    }
                    function onScreenReaderBackendError(errorMessage) {
                        screen.putDissolvingMessage(errorMessage, 1)
                    }
                }

                Shortcut {
                    sequences: [ "Meta+S", "Meta+Alt+S" ]
                    onActivated: {
                        backend.screenReaderActive = !backend.screenReaderActive
                    }
                }
            }
        }

        onAboutToHide: {
            root.popupEnded()
        }
    }
}
