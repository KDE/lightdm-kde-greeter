/*
 *   Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
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
                configItem: backgroundSelector
                fillMode: cfg_backgroundFill.value
            }
        }
    }
}
