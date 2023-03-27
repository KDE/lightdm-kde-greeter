/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021, 2022 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

LightDM-KDE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LightDM-KDE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LightDM-KDE.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.15

Item {
    id: desktop

    width: screenSize.width
    height: screenSize.height

    ScreenManager {
        id: screenManager

        delegate: Item {

            // background
            Image {
                anchors.fill: parent
                // default to keeping aspect ratio
                fillMode: (x => x !== "" ? Number(x) : Image.PreserveAspectCrop)(config.readEntry("BackgroundFillMode"))
                //read from config, if there's no entry use plasma theme
                source: config.readEntry("Background") ? config.readEntry("Background") : plasmaTheme.wallpaperPath
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
        }
    }
}
