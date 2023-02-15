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

    // the default i18n domain here is "kcm_lightdm"
    // each theme has its own language file, common for main layout and setup utility
    property string domain: "lightdm_theme_classic"

    height: childrenRect.height

    function save(settings) {
        var branch = "greeter/" + domain + "/"

        settings[branch + "Background"] = backgroundSelector.filePath
        settings[branch + "BackgroundFillMode"] = backgroundSelector.imageDialog.fillMode
        settings[branch + "Logo"] = welcomeImageSelector.filePath
        settings[branch + "GreetMessage"] = welcomeText.text
    }

    function load(settings) {
        var branch = "greeter/" + domain + "/"

        backgroundSelector.filePath = root.readEntry(settings, branch + "Background", "file:///usr/share/design/current/backgrounds/xdm.png")
        backgroundSelector.imageDialog.fillMode = Number(root.readEntry(settings, branch + "BackgroundFillMode", Image.PreserveAspectCrop))
        welcomeImageSelector.filePath = root.readEntry(settings, branch + "Logo", "")
        welcomeText.text = root.readEntry(settings, branch + "GreetMessage", i18nd(domain, "Welcome to %1", "%hostname%"))
    }

    Column {
        width: parent.width
        spacing: gap

        Label {
            text: i18nd(domain, "Background image:")
        }
        Shared.SelectImageButton {
            id: backgroundSelector
            anchors.horizontalCenter: parent.horizontalCenter
            imageDialog: Shared.WallpapersDialog {}
        }

        Label {
            text: i18nd(domain, "Welcome image:")
        }
        Shared.SelectImageButton {
            id: welcomeImageSelector
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            spacing: gap
            Label {
                height: welcomeText.height
                text: i18nd(domain, "Welcome text:")
            }
            TextField {
                id: welcomeText
                width: themeConfig.width - x - gap * 3
                onTextEdited: themeConfig.needsSave = true
            }
        }
    }
}
