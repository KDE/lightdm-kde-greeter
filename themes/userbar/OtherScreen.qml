/*
This file is part of LightDM-KDE.

Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import org.kde.ksvg as KSvg

Item {
    id: screen
    anchors.fill: parent
    KSvg.FrameSvgItem {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: label.width + margins.left + margins.right
        height: label.height + margins.bottom
        imagePath: "widgets/background"
        enabledBorders: "LeftBorder|BottomBorder|RightBorder"

        TooltipButton {
            id: label
            anchors.horizontalCenter: parent.horizontalCenter
            icon.name: "info"
            caption: i18n("Press %1 to change the primary screen", "F1")
        }
    }
}
