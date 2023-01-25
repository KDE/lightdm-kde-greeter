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
import QtQuick.Window 2.15 as Windows
import QtQuick.Dialogs 1.3
import org.kde.kquickcontrolsaddons 2.0 as Addons

Item {
    id: root
    property real dpi96: 3.7820092576037516
    property real dpiScale: Windows.Screen.pixelDensity / dpi96
    property int iconWidth: 160 * dpiScale
    property int gap: 6 * dpiScale
    property bool settingsLoaded: false

    SystemPalette { id: paletteActive;   colorGroup: SystemPalette.Active }
    SystemPalette { id: paletteInactive; colorGroup: SystemPalette.Inactive }

    function readEntry(settings, key, defval) {
        if (!settings) return defval
        if (settings[key] == undefined) return defval
        return settings[key]
    }

    function save() {
        var settings = {}
        settings["core/SeatDefaults/autologin-user"] = autoLogin.checked ? usersCombo.currentValue : ""
        settings["core/SeatDefaults/allow-guest"] = guestLogin.checked
        settings["greeter/greeter/theme-name"] = themesList.currentItem.properties.id
        themeConfig.save(settings)
        return settings
    }

    function load(settings) {
        guestLogin.checked = readEntry(settings, "core/SeatDefaults/allow-guest", "false") != "false"
        var autoUser = readEntry(settings, "core/SeatDefaults/autologin-user", "")
        if (autoUser != "") {
            var index = usersCombo.indexOfValue(autoUser)
            if (index < 0) {
                autoLogin.checked = false
            } else {
                autoLogin.checked = true
                usersCombo.currentIndex = index
            }
        }
        var themeName = readEntry(settings, "greeter/greeter/theme-name", "userbar")
        var themeIndex = kcm.themesModel.indexForId(themeName)
        themesList.currentIndex = themeIndex

        themeConfig.load(settings)
        settingsLoaded = true
    }

    function markNeedsSave() {
        kcm.needsSave = true
    }

    Row {
        anchors.fill: parent
        anchors.margins: gap
        spacing: gap

        Component {
            id: themeIcon
            Item {
                width: childrenRect.width
                height: childrenRect.height
                anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                property bool isCurrent: ListView.isCurrentItem
                property var properties: model
                Column {
                    spacing: gap
                    Rectangle {
                        id: preview
                        radius: gap / 2
                        width: iconWidth
                        height: iconWidth * 0.75
                        border.color: paletteActive.mid
                        border.width: 1
                        color: isCurrent ? paletteActive.highlight : mouseArea.containsMouse ? paletteInactive.highlight : paletteActive.base

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                if (themesList.currentIndex != index) {
                                    if (themeConfig.needsSave) {
                                        confirmSwitch.open()
                                    } else {
                                        themesList.switchToIndex(index)
                                    }
                                }
                            }

                            MessageDialog {
                                id: confirmSwitch
                                text: i18n("Theme changes will be lost")
                                icon: StandardIcon.Warning
                                standardButtons: Dialog.Ok | Dialog.Cancel
                                onAccepted: {
                                    themesList.switchToIndex(index)
                                }
                            }
                        }

                        Addons.QPixmapItem {
                            pixmap: model.preview
                            smooth: true
                            anchors.fill: parent
                            anchors.margins: gap
                        }
                    }

                    Label {
                        width: preview.width
                        horizontalAlignment: Text.AlignHCenter
                        text: model.name
                    }
                }
            }
        }

        ScrollView {
            Component.onCompleted: background.visible = true
            clip: true
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: iconWidth + gap * 10

            ListView {
                id: themesList
                visible: settingsLoaded

                model: kcm.themesModel
                delegate: themeIcon
                spacing: gap * 3
                topMargin: gap * 3
                bottomMargin: gap * 3
                function switchToIndex(index) {
                    themesList.currentIndex = index
                    root.markNeedsSave()
                    themeConfig.needsSave = false
                }
            }
        }

        Item {
            height: parent.height
            width: parent.width - x
            GroupBox {
                id: themeConfigBox

                padding: gap * 2
                title: i18n("Theme")
                width: parent.width
                anchors.top: parent.top
                anchors.bottom: generalSettings.top
                anchors.bottomMargin: gap

                Column {
                    anchors.fill: parent
                    spacing: gap
                    clip: true

                    Grid {
                        width: parent.width
                        columns: 2
                        rowSpacing: gap * 2
                        columnSpacing: gap * 2

                        Label { font.bold: true; text: i18n("Name:") }
                        Label { text: themesList.currentItem.properties.name }
                        Label { font.bold: true; text: i18n("Description:") }
                        Label {
                            wrapMode: Text.WordWrap
                            text: themesList.currentItem.properties.description
                            width: parent.width - x
                        }
                    }

                    MenuSeparator {
                        width: parent.width
                        topPadding: gap
                        bottomPadding: gap
                    }

                    Loader {
                        id: themeConfig
                        visible: settingsLoaded

                        property var cachedSettings
                        property bool needsSave: false

                        function load(settings) {
                            cachedSettings = settings
                            item.load(settings)
                        }

                        function save(settings) {
                            item.save(settings)
                            // convert values to strings for consistency with load function
                            // copy because the original VariantMap object goes to c++ for saving
                            cachedSettings = {}
                            for (var p in settings) {
                                cachedSettings[p] = String(settings[p])
                            }
                            needsSave = false
                        }

                        function markNeedsSave() {
                            needsSave = true
                            root.markNeedsSave()
                        }

                        onItemChanged: {
                            item.load(cachedSettings)
                        }

                        width: parent.width
                        height: parent.height - y
                        source: themesList.currentItem.properties.path + "/config.qml"
                    }
                }
            }

            GroupBox {
                id: generalSettings

                title: i18n("General")
                anchors.bottom: parent.bottom
                width: parent.width
                padding: gap * 2
                clip: true

                Column {
                    spacing: gap
                    height: childrenRect.height
                    CheckBox {
                        id: guestLogin; text: i18n("Allow guest login")
                        onCheckedChanged: {
                            root.markNeedsSave()
                            kcm.usersModel.showGuest = checked
                        }
                    }

                    CheckBox {
                        id: autoLogin
                        text: i18n("Automatically log in:")
                        onCheckedChanged: root.markNeedsSave()
                    }

                    Row {
                        spacing: gap
                        enabled: autoLogin.checked
                        Label {
                            anchors.verticalCenter: parent.verticalCenter
                            text: i18n("As user:")
                        }
                        ComboBox {
                            id: usersCombo
                            anchors.verticalCenter: parent.verticalCenter
                            model: kcm.usersModel
                            valueRole: 'name'
                            textRole: 'display'
                            onActivated: root.markNeedsSave()

                            Component.onCompleted: {
                                guestLogin.onCheckedChanged.connect(() => {
                                    if (currentIndex >= count) {
                                        currentIndex = count - 1
                                    }
                                })
                            }
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        themesList.positionViewAtIndex(themesList.currentIndex, ListView.Contains)
    }
}
