/*
 *   Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs as StandardDialogs
import org.kde.plasma.wallpapers.image as Wallpaper
import org.kde.kirigami as Kirigami

Rectangle {
    id: button

    property bool hovered: mouseArea.containsMouse || deleteImageButton.hovered || openFileButton.hovered
    // maybe it's just a file selection dialog, or maybe a background selection dialog from Plasma
    property var imageDialog: selectFileDialog
    property var configPath
    property var configFill
    property string filePath: configPath.value

    SystemPalette { id: paletteActive; colorGroup: SystemPalette.Active }

    width: 8 * gridUnit
    height: width * 0.75
    border.color: paletteActive.mid
    border.width: 1
    radius: gap
    color: paletteActive.base

    property var mediaProxy: Wallpaper.MediaProxy {
        source: button.filePath
    }

    Image {
        id: preview

        anchors.centerIn: parent
        anchors.fill: parent
        anchors.margins: gap + 1
        fillMode: Image.PreserveAspectFit
        // mediaProxy for a strange reason returns some image for an empty string
        source: button.filePath === "" ? "" : mediaProxy.modelImage
    }

    Column {
        spacing: gap
        visible: preview.status == Image.Null || preview.status == Image.Error
        anchors.centerIn: parent

        Kirigami.Icon {
            id: noImageIcon
            anchors.horizontalCenter: parent.horizontalCenter
            source: "image-missing"
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
                configPath.value = ""
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
            configPath.value = imageDialog.fileUrl.toString();
        }
    }
}
