/*
This file is part of LightDM-KDE.

Copyright 2012 Aurélien Gâteau <agateau@kde.org>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

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

        onAboutToHide: {
            root.itemTriggered()
        }
    }
}
