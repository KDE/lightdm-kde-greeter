/*
 *   Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.20 as Kirigami

Dialog {
    id: dialog

    property int fillMode
    // required for the background picker plugin to work
    property var wallpaper: ({ configuration: {}})
    property var configDialog: kcm
    property var configItem

    parent: configArea

    x: 2 * gridUnit
    y: 2 * gridUnit
    width: parent.width - 4 * gridUnit
    height: parent.height - 4 * gridUnit
    leftInset: -gap
    rightInset: -gap
    bottomInset: -gap

    // cut out from there the choice of background color or fill method
    // See: KDE/plasma-workspace, wallpapers/image/imagepackage/contents/ui/config.qml
    function patchWallpaperPicker(item) {

        var formLayout
        for (var p in item.children) {
            var propertyName = item.children[p].toString()
            if (propertyName.startsWith("FormLayout_QMLTYPE_")) {
                formLayout = item.children[p]
                break
            }
        }
        if (!formLayout) return false

        var radioButton, rowLayout
        for (var p in formLayout.children) {
            var propertyName = formLayout.children[p].toString()
            if (propertyName.startsWith("RadioButton_QMLTYPE_")) {
                radioButton = formLayout.children[p]
            } else if (propertyName.startsWith("QQuickRowLayout")) {
                rowLayout = formLayout.children[p]
            }
        }
        if (!radioButton || !rowLayout) return false

        radioButton.visible = false
        rowLayout.visible = false

        return true
    }

    function openForPath(path) {
        const props = { "configDialog": kcm, "cfg_Image": path, "cfg_FillMode": fillMode }
        dialogLoader.setSource(kcm.wallpaperConfigSource, props)
        if (!patchWallpaperPicker(dialogLoader.item)) {
            console.warn("Failed to patch background picker widget");
        }
        open()
    }

    Kirigami.FormLayout {
        id: parentLayout // needed for twinFormLayouts to work in wallpaper plugins
        twinFormLayouts: []
        Layout.fillWidth: true
    }

    Loader {
        id: dialogLoader
        anchors.fill: parent
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: {
        // extract data from plugin fields
        configItem.configPath.value = dialogLoader.item.cfg_Image
        configItem.configFill.value = dialogLoader.item.cfg_FillMode
    }
}
