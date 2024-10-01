/*
This file is part of LightDM-KDE.

Copyright 2012 Aurélien Gâteau <agateau@kde.org>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.ToolButton {
    id: root

    property int currentIndex: 0
    property string dataRole: "key"
    property var model
    property bool deactivatedViaKeyboard: false

    function keyEvent(event) {
        if (event.key == Qt.Key_Escape || event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
            deactivatedViaKeyboard = true
        } else if (event.key == Qt.Key_Left) {
            if (root.KeyNavigation.left) {
                deactivatedViaKeyboard = true
                menu.dismiss()
            }
        } else if (event.key == Qt.Key_Right) {
            if (root.KeyNavigation.right) {
                deactivatedViaKeyboard = true
                menu.dismiss()
            }
        }
    }

    signal popupEnded()

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
            menu.currentIndex = root.currentIndex
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
                }
                Keys.onShortcutOverride: root.keyEvent(event)
            }
        }

        onAboutToHide: {
            root.popupEnded()
        }
    }

    Keys.onPressed: if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return) {
        toggle()
        // the method above does not generate event
        toggled()
    }
}
