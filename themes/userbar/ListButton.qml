/*
This file is part of LightDM-KDE.

Copyright 2012 Aurélien Gâteau <agateau@kde.org>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022 Anton Golubev <golubevan@basealt.ru>

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

import QtQuick 2.12
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.ToolButton {
    id: root

    property int currentIndex: 0
    property string dataRole: "key"
    property var model

    signal itemTriggered()

    text: {
        var item = instantiator.objectAt(currentIndex)
        return item ? item.text : ""
    }

    visible: menu.count > 1

    checkable: true
    checked: menu.opened
    onToggled: {
        if (checked) {
            menu.popup(root, 0, 0)
        } else {
            menu.dismiss()
        }
    }

    function currentData() {
        var item = instantiator.objectAt(currentIndex)
        return item ? item.data : ""
    }

    function indexForData(data) {
        for (var index = 0; index < instantiator.count; ++index) {
            if (instantiator.objectAt(index).data == data) {
                return index;
            }
        }
        return null;
    }

    PlasmaComponents.Menu {
        id: menu
        PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.NormalColorGroup
        PlasmaCore.ColorScope.inherit: false

        Instantiator {
            id: instantiator
            model: root.model
            onObjectAdded: menu.insertItem(index, object)
            onObjectRemoved: menu.removeItem(object)
            delegate: PlasmaComponents.MenuItem {
                property string data: model[root.dataRole]
                text: model.display
                onTriggered: {
                    root.currentIndex = model.index
                    root.itemTriggered()
                }
            }
        }
    }
}
