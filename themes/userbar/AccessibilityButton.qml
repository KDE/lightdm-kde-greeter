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

        y: -height + parent.height
        width: contentWidth + leftPadding + rightPadding

        focus: true

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Column {
            PC.MenuItem {
                id: screenReaderItem

                text: i18nc("Button to enable/disable screen reader", "Screen Reader")

                checkable: true

                focus: true
                highlighted: focus

                indicator: PC.CheckBox {
                    id: checkbox

                    x: parent.leftPadding
                    anchors.verticalCenter: parent.verticalCenter

                    tristate: true

                    onClicked: screenReaderItem.triggered()
                }

                onTriggered: {
                    backend.screenReaderActive = !backend.screenReaderActive
                    checkbox.checkState = Qt.PartiallyChecked
                }

                Connections {
                    target: backend
                    function onScreenReaderActiveChanged() {
                        checkbox.checkState = backend.screenReaderActive ? Qt.Checked : Qt.Unchecked
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
