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
import "../components/kcm" as Shared

Item {
    id: config

    property var domain: "lightdm_theme_userbar"

    height: childrenRect.height

    // config values
    property var cfg_backgroundPath: Shared.ConfigValue {
        name: "BackgroundPreview"
        type: cfgString
        defaultValue: "file:///usr/share/design/current/backgrounds/xdm.png"
        listenValue: backgroundSelector.filePath
    }
    property var cfg_backgroundFill: Shared.ConfigValue {
        name: "BackgroundFillMode"
        type: cfgInteger
        defaultValue: Image.PreserveAspectCrop
        listenValue: backgroundSelector.imageDialog.fillMode
    }

    Column {
        width: parent.width
        spacing: gap

        Label {
            text: i18nd(domain, "Background image:")
        }

        Shared.SelectImageButton {
            id: backgroundSelector

            configPath: cfg_backgroundPath
            configFill: cfg_backgroundFill

            anchors.horizontalCenter: parent.horizontalCenter
            imageDialog: Shared.WallpapersDialog {
                fillMode: cfg_backgroundFill.value
            }
        }
    }
}
