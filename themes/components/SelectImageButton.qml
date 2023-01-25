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

Button {
    id: button

    property string dialogTitle
    property string filePath

    icon.name: "viewimage"
    onClicked: {
        inputFilePath.text = filePath
        dialog.open()
    }

    SystemPalette { id: paletteActive; colorGroup: SystemPalette.Active }

    Dialog {
        id: dialog

        anchors.centerIn: Overlay.overlay
        height: implicitHeight
        width: implicitWidth
        leftInset: -gap
        rightInset: -gap
        bottomInset: -gap

        Column {
            width: parent.width
            spacing: gap * 2
            Item {
                id: previewArea

                width: iconWidth * 2 + gap
                height: width * 0.75 + gap

                Rectangle {
                    id: previewBack

                    visible: !noImageIcon.visible
                    anchors.centerIn: preview
                    width: preview.paintedWidth + gap * 2
                    height: preview.paintedHeight + gap * 2
                    radius: gap
                    anchors.margins: -gap
                    border.color: paletteActive.mid
                    border.width: 1
                    color: paletteActive.base
                }

                Image {
                    id: preview

                    anchors.centerIn: parent
                    anchors.fill: parent
                    anchors.margins: gap
                    fillMode: Image.PreserveAspectFit
                    source: inputFilePath.text
                }

                Image {
                    id: noImageIcon

                    visible: preview.status == Image.Null || preview.status == Image.Error
                    anchors.centerIn: parent
                    source: "image://icon/image-missing"
                }
            }

            Row {
                width: parent.width
                spacing: 0
                TextField {
                    id: inputFilePath
                    width: parent.width - selectFile.width
                }
                Button {
                    id: selectFile

                    height: inputFilePath.height
                    width: height
                    icon.name: "document-open-data"
                    onClicked: {
                        fileDialog.folder = inputFilePath.text.replace(/(.*?)[^/]*\..*$/,'$1')
                        fileDialog.open()
                    }
                    StandardDialogs.FileDialog {
                        id: fileDialog

                        onAccepted: {
                            inputFilePath.text = fileDialog.fileUrl.toString();
                        }
                    }
                }
            }
        }

        title: button.dialogTitle
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            filePath = inputFilePath.text
            button.parent.markNeedsSave()
        }
        onRejected: {
            inputFilePath.text = filePath
        }
    }
}
