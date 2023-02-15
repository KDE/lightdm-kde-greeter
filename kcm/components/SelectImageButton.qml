/*
 *   Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3 as StandardDialogs

Rectangle {
    id: button

    property string filePath
    property bool hovered: mouseArea.containsMouse || deleteImageButton.hovered || openFileButton.hovered
    // maybe it's just a file selection dialog, or maybe a background selection dialog from Plasma
    property var imageDialog: selectFileDialog

    SystemPalette { id: paletteActive; colorGroup: SystemPalette.Active }

    width: 160 * dpiScale
    height: width * 0.75
    border.color: paletteActive.mid
    border.width: 1
    radius: gap
    color: paletteActive.base

    Image {
        id: preview

        anchors.centerIn: parent
        anchors.fill: parent
        anchors.margins: gap + 1
        fillMode: Image.PreserveAspectFit
        source: button.filePath
    }

    Column {
        spacing: gap
        visible: preview.status == Image.Null || preview.status == Image.Error
        anchors.centerIn: parent

        Image {
            id: noImageIcon
            anchors.horizontalCenter: parent.horizontalCenter
            source: "image://icon/image-missing"
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("No image")
            width: Math.min(implicitWidth, preview.width)
            elide: Text.ElideRight
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    Column {
        id: buttons
        visible: button.hovered

        anchors {
            top: parent.top
            right: parent.right
            margins: gap
        }

        Button {
            id: deleteImageButton
            icon.name: "delete"
            onClicked: {
                filePath = ""
                themeConfig.needsSave = true
            }
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: i18n("Clear Image")
        }

        Button {
            id: openFileButton
            icon.name: "fileopen"
            onClicked: imageDialog.openForPath(filePath)
            ToolTip.visible: hovered
            ToolTip.delay: 500
            ToolTip.text: i18n("Select image")
        }
    }

    StandardDialogs.FileDialog {
        id: selectFileDialog

        function openForPath(path) {
            folder = path.replace(/(.*?)[^/]*$/,'$1')
            open()
        }

        onAccepted: {
            filePath = imageDialog.fileUrl.toString();
            themeConfig.needsSave = true
        }
    }
}
