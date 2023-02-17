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
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami

Dialog {
    id: dialog

    property int fillMode
    // required for the background picker plugin to work
    property var wallpaper: ({ configuration: {}})
    property var configDialog: kcm

    anchors.centerIn: Overlay.overlay
    width: root.width - 100 * dpiScale
    height: root.height - 100 * dpiScale
    leftInset: -gap
    rightInset: -gap
    bottomInset: -gap

    function openForPath(path) {
        dialogLoader.source = kcm.wallpaperConfigSource
        dialogLoader.item.cfg_Image = path
        dialogLoader.item.cfg_FillMode = fillMode
        if (!kcm.patchWallpaperPicker(dialogLoader.item)) {
            console.warn("Failed to patch background picker widget");
        }
        open()
    }

    Kirigami.FormLayout {
        id: parentLayout // needed for twinFormLayouts to work in wallpaper plugins
        twinFormLayouts: []
        Layout.fillWidth: true
    }

    Loader {
        id: dialogLoader
        anchors.fill: parent
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: {
        // extract data from plugin fields
        parent.filePath = dialogLoader.item.cfg_Image;
        fillMode = dialogLoader.item.cfg_FillMode
        themeConfig.needsSave = true
    }
}
