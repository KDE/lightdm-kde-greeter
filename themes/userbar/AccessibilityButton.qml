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
                checked: backend.screenReaderActive

                focus: true
                highlighted: focus

                onTriggered: {
                    backend.screenReaderActive = checked
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
