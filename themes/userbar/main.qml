/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021, 2022 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15

Item {
    id: desktop

    width: screenSize.width
    height: screenSize.height
    property bool showDebugInfo: false
    property var previousFocusItem

    QtObject {
        id: params
        property int toolTipDelay: 1000
        property int toolTipTimeout: 5000
    }

    ScreenManager {
        id: screenManager

        delegate: Item {

            // background
            Image {
                anchors.fill: parent
                // default to keeping aspect ratio
                fillMode: (x => x !== "" ? Number(x) : Image.PreserveAspectCrop)(config.readEntry("BackgroundFillMode"))
                //read from config, if there's no entry use plasma theme
                source: {
                    var entry = config.readEntry("Background")
                    return entry != null ? entry : plasmaTheme.wallpaperPath
                }
            }

            OtherScreen {
                visible: primary.parent && thisScreen != screenManager.activeScreen
            }
        }
    }

    PrimaryScreen {
        id: primary
        parent: screenManager.activeScreen
        anchors.fill: parent
        Keys.onPressed: (event) => {
            if (event.key == Qt.Key_F1 || event.key == Qt.Key_P && (event.modifiers & Qt.MetaModifier)) {
                screenManager.nextActiveScreen()
            }
            if (event.key == Qt.Key_F10) {
                showDebugInfo = !showDebugInfo
            }
        }
    }
}
