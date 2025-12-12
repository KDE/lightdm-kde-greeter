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

import org.kde.plasma.private.battery

RowLayout {
    id: root

    property int fontSize: Kirigami.Theme.defaultFont.pointSize

    spacing: Kirigami.Units.smallSpacing
    visible: batteryControl.hasInternalBatteries

    BatteryControlModel {
        id: batteryControl
    }

    PW.BatteryIcon {
        pluggedIn: batteryControl.pluggedIn
        hasBattery: batteryControl.hasCumulative
        percent: batteryControl.percent

        Layout.preferredHeight: Math.max(Kirigami.Units.iconSizes.medium, batteryLabel.implicitHeight)
        Layout.preferredWidth: Layout.preferredHeight
        Layout.alignment: Qt.AlignVCenter
    }

    PlasmaComponents3.Label {
        id: batteryLabel
        font.pointSize: root.fontSize
        text: i18nd("lightdm_kde_greeter", "%1%", batteryControl.percent)
        Accessible.name: i18nd("lightdm_kde_greeter", "Battery at %1%", batteryControl.percent)
        Layout.alignment: Qt.AlignVCenter
    }
}
