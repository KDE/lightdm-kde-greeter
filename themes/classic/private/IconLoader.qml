/*
*   Copyright (C) 2011 by Marco MArtin <mart@kde.org>
*   Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 2.12
        org.kde.plasma.core

Description:
 TODO i need more info here

Properties:
        bool valid:
        Returns if the icon is valid or not.

        string source:
        Returns the dir,in which the icon exists.
**/

import QtQuick 2.12
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root

    property bool valid: false

    property variant source

    onSourceChanged: {
        //is it a qicon?
        if (typeof source != "string") {
            imageLoader.sourceComponent = iconComponent
            valid = true
            return
        } else if (source == "") {
            imageLoader.sourceComponent = null
            valid = false
            return
        }

        svgIcon.imagePath = "toolbar-icons/"+root.source.split("-")[0]
        if (!svgIcon.isValid() || !svgIcon.hasElement(root.source)) {
            svgIcon.imagePath = "icons/"+root.source.split("-")[0]
        }

        if (svgIcon.isValid() && svgIcon.hasElement(root.source)) {
            imageLoader.sourceComponent = svgComponent
        } else if ((root.source.indexOf(".") == -1 && root.source.indexOf(":") == -1)) {
            imageLoader.sourceComponent = iconComponent
        } else {
            imageLoader.sourceComponent = imageComponent
        }
        valid = true
    }

    implicitWidth: units.iconSizes.small
    implicitHeight: units.iconSizes.small

    PlasmaCore.Svg {
        id: svgIcon
    }

    function roundToStandardSize(size)
    {
        if (size >= units.iconSizes.enormous) {
            return units.iconSizes.enormous
        } else if (size >= units.iconSizes.huge) {
            return units.iconSizes.huge
        } else if (size >= units.iconSizes.large) {
            return units.iconSizes.large
        } else if (size >= units.iconSizes.medium) {
            return units.iconSizes.medium
        } else if (size >= units.iconSizes.smallMedium) {
            return units.iconSizes.smallMedium
        } else {
            return units.iconSizes.small
        }
    }

    Loader {
        id: imageLoader
        anchors.fill: parent

        Component {
            id: svgComponent

            PlasmaCore.SvgItem {
                svg: svgIcon
                elementId: root.source
                anchors.fill: parent
                smooth: true
            }
        }

        Component {
            id: iconComponent

            PlasmaCore.IconItem {
                source: root.source
                smooth: true
                anchors.fill: parent
            }
        }

        Component {
            id: imageComponent

            Image {
                source: root.source
                sourceSize.width: width
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.fill: parent
            }
        }
    }
}
