/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021, 2022 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023-2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15

Item {
    id: desktop

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
            property alias crop: crop

            // background
            Image {
                anchors.fill: parent
                // default to keeping aspect ratio
                fillMode: (x => x && x !== "" ? Number(x) : Image.PreserveAspectCrop)(config.readEntry("BackgroundFillMode"))
                //read from config, if there's no entry use plasma theme
                source: {
                    var entry = config.readEntry("Background")
                    return entry != null ? entry : defaultWallpaper
                }
            }

            Item {
                id: crop

                height: parent.height
                width: Math.min(Math.round(parent.height * cropRatio[0] / cropRatio[1]), parent.width)
                x: Math.round((parent.width - width) * 0.5)
                y: parent.y

                property var cropRatio: {

                    let ratioOption = "MaxScreenRatio"
                    let defaultRatio = [ 16, 9 ]

                    let ratioStr = config.readEntry(ratioOption)
                    if (!ratioStr) return defaultRatio

                    let ratio = ratioStr.split(":").map(Number)

                    if (ratio.length != 2 || isNaN(ratio[0]) || isNaN(ratio[1]) || ratio[0] < 1 || ratio[1] < 1 || ratio[0] / ratio[1] < 1) {
                        console.warn("ScreenManager: bad option " + ratioOption + " (" + ratioStr + ")")
                        return defaultRatio
                    }
                    return ratio
                }

                OtherScreen {
                    visible: parent != primary.parent
                }
            }
        }
    }

    PrimaryScreen {
        id: primary
        parent: screenManager.activeScreen && screenManager.activeScreen.loader.item.crop
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
