/*
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2023-2024 Anton Golubev <golubevan@altlinux.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.workspace.components 2.0 as PW
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasma5support as Plasma5Support

RowLayout {
    id: root

    property int fontSize: Kirigami.Theme.defaultFont.pointSize

    function getOrDefault(source /*object?*/, prop /*string*/, fallback /*T*/) /*-> T*/ {
        return (source !== null && source !== undefined && source.hasOwnProperty(prop))
            ? source[prop] : fallback;
    }

    readonly property var battery: pmSource.data["Battery"]

    // State: Unknown NoCharge Charging Discharging FullyCharged
    // See plasma-workspace/dataengines/powermanagement/powermanagementengine.cpp, batteryStateToString
    readonly property string batteryState: getOrDefault(battery, "State", "Unknown")
    readonly property bool pluggedIn: batteryState !== "Discharging"
    readonly property bool hasBattery: getOrDefault(battery, "Has Battery", false)
    readonly property int percent: getOrDefault(battery, "Percent", 0)

    spacing: Kirigami.Units.smallSpacing
    visible: getOrDefault(battery, "Has Cumulative", false)

    Plasma5Support.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["Battery"]
    }

    PW.BatteryIcon {
        pluggedIn: root.pluggedIn
        hasBattery: root.hasBattery
        percent: root.percent

        Layout.preferredHeight: Math.max(Kirigami.Units.iconSizes.medium, batteryLabel.implicitHeight)
        Layout.preferredWidth: Layout.preferredHeight
        Layout.alignment: Qt.AlignVCenter
    }

    PlasmaComponents3.Label {
        id: batteryLabel
        font.pointSize: root.fontSize
        text: i18nd("lightdm_kde_greeter", "%1%", root.percent)
        Accessible.name: i18nd("lightdm_kde_greeter", "Battery at %1%", root.percent)
        Layout.alignment: Qt.AlignVCenter
    }
}
