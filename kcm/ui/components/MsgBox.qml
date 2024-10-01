/*
    SPDX-FileCopyrightText: 2023-2024 Anton Golubev <golubevan@altlinux.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami as Kirigami

Dialog {

    // center manually so that there is an integer value
    x: Math.round((parent.width - width) * 0.5)
    y: Math.round((parent.height - height) * 0.5)

    property int rawContentWidth: messageIcon.width + measureText.width + root.gridUnit + leftPadding + rightPadding

    property int width0: Math.min(rawContentWidth, parent.width * 0.8, root.gridUnit * 25)
    width: Math.max(width0, root.gridUnit * 12)

    property alias buttons: footerItem.standardButtons
    property string message: ""
    property alias icon: messageIcon.source

    property string iconInfo:    "dialog-information"
    property string iconWarning: "dialog-warning"
    property string iconError:   "dialog-error"

    function show(msg, icon) {
        if (!icon || icon === "") icon = iconInfo
        messageIcon.source = icon
        message = msg
        open()
    }

    verticalPadding: root.gridUnit
    horizontalPadding: root.gridUnit
    clip: true

    Label {
        id: measureText
        visible: false
        text: message
    }

    contentItem: Row {

        spacing: root.gridUnit

        Kirigami.Icon {
             id: messageIcon
             width: Kirigami.Units.iconSizes.medium
             height: width
             anchors.verticalCenter: parent.verticalCenter
             source: "image-missing"
        }
        Label {
            id: messageItem
            wrapMode: Text.WordWrap
            width: parent.width - x
            anchors.verticalCenter: parent.verticalCenter
            text: message
        }
    }

    footer: DialogButtonBox {
        id: footerItem
        topPadding: 0
        bottomPadding: root.gridUnit
        standardButtons: { return DialogButtonBox.Ok }
        alignment: Qt.AlignHCenter
    }
}
