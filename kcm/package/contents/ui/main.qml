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
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15 as Windows
import org.kde.kquickcontrolsaddons 2.0 as Addons
import org.kde.plasma.core 2.0 as PlasmaCore
import "components" as Shared

Item {
    id: root
    property real gridUnit: PlasmaCore.Units.gridUnit
    property int iconWidth: 7 * gridUnit
    property int gap: gridUnit / 3
    property bool settingsLoaded: false
    property var themeSettings: []
    property var themeBranch

    SystemPalette { id: paletteActive;   colorGroup: SystemPalette.Active }
    SystemPalette { id: paletteInactive; colorGroup: SystemPalette.Inactive }

    // config values
    property var cfg_autoLogin: Shared.ConfigValue {
        branch: "core/Seat:seat0/"
        name: "autologin-user"
        type: cfgString
        defaultValue: ""
        listenValue: autoLogin.checked ? usersCombo.currentValue : ""
    }
    property var cfg_autoSession: Shared.ConfigValue {
        branch: "core/Seat:seat0/"
        name: "autologin-session"
        type: cfgString
        defaultValue: ""
        listenValue: autoLogin.checked ? sessionsCombo.currentValue : ""
    }
    property var cfg_themeName: Shared.ConfigValue {
        branch: "greeter/greeter/"
        name: "theme-name"
        type: cfgString
        defaultValue: "userbar"
        listenValue: themesList.currentItem.properties.id
    }

    function setupConfigValues(item, branch, setDefaults) {
        let props = []
        for (var p in item) {
            if (p.startsWith("cfg_") && !p.endsWith("Changed")) {
                if (branch) item[p].branch = branch
                props.push(p)
                if (setDefaults) {
                    item[p].value = item[p].defaultValue
                    continue
                }
                let configValue = kcm.getConfigValue(item[p].branch + item[p].name)
                if (configValue) {
                    item[p].valueFromString(configValue)
                } else {
                    item[p].value = item[p].defaultValue
                    // to keep track of future changes
                    kcm.storeDefaultValue(item[p].branch + item[p].name, item[p].defaultValue)
                }
                item[p].storedValue = item[p].value
            }
        }
        return props
    }

    function load() {
        setupConfigValues(root)
        var themeIndex = kcm.themesModel.indexForId(cfg_themeName.value)
        themesList.switchToIndex(themeIndex)
        settingsLoaded = true
    }

    function defaults() {
        // perhaps we don't want to set a default theme, we want to load the
        // default settings for the selected theme
        let currentThemeName = cfg_themeName.value
        setupConfigValues(root, null, true)
        cfg_themeName.value = currentThemeName
        themeSettings = setupConfigValues(themeConfig.item, themeBranch, true)
    }

    // reread the main settings and settings of the active theme
    function updateConfigValues() {
        setupConfigValues(root)
        themeSettings = setupConfigValues(themeConfig.item, themeBranch)
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
                                    if (themeConfig.needsSave()) {
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
                                    setupConfigValues(themeConfig.item, themeBranch)
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
            Component.onCompleted: {
                background.visible = true
            }
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
                    themeBranch = "greeter/" + themeConfig.item.domain + "/"
                    themeSettings = setupConfigValues(themeConfig.item, themeBranch)
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
                        verticalItemAlignment: Grid.AlignVCenter

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

                    ScrollView {
                        id: themeScrollView

                        height: parent.height - y
                        width: parent.width
                        clip: true

                        Flickable {
                            contentHeight: themeConfig.item.height
                            boundsBehavior: Flickable.StopAtBounds

                            Loader {
                                id: themeConfig

                                visible: settingsLoaded
                                width: themeScrollView.width

                                function needsSave() {
                                    for (let prop of themeSettings) {
                                        if (item[prop].listenValue != item[prop].storedValue) {
                                            return true
                                        }
                                    }
                                    return false;
                                }

                                source: themesList.currentItem.properties.path + "/config.qml"
                            }
                        }
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
                    width: parent.width

                    CheckBox {
                        id: autoLogin
                        text: i18n("Automatically log in:")
                        checked: cfg_autoLogin.value != ""
                    }

                    Flow {
                        width: parent.width
                        spacing: gap
                        enabled: autoLogin.checked
                        Row {
                            spacing: gap
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: i18n("As user:")
                            }
                            ComboBox {
                                id: usersCombo
                                currentIndex: kcm.usersModel.indexForUserName(cfg_autoLogin.value)
                                anchors.verticalCenter: parent.verticalCenter
                                model: kcm.usersModel
                                valueRole: 'name'
                                textRole: 'display'
                            }
                        }
                        Row {
                            spacing: gap
                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                text: i18n("Using session:")
                            }
                            ComboBox {
                                id: sessionsCombo
                                currentIndex: kcm.sessionsModel.indexForSessionName(cfg_autoSession.value)
                                anchors.verticalCenter: parent.verticalCenter
                                model: kcm.sessionsModel
                                valueRole: 'key'
                                textRole: 'display'
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
