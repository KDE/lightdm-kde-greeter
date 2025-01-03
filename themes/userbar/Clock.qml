/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023-2024 Anton Golubev <golubevan@altlinux.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasma5support as Plasma5Support

ColumnLayout {
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software

    property real ratio: 0.5

    PlasmaComponents3.Label {
        text: Qt.formatTime(timeSource.data["Local"]["DateTime"])
        style: softwareRendering ? Text.Outline : Text.Normal
        styleColor: softwareRendering ? Kirigami.Theme.backgroundColor : "transparent" //no outline, doesn't matter
        font.pointSize: 48 * ratio
        Layout.alignment: Qt.AlignHCenter
    }
    PlasmaComponents3.Label {
        text: Qt.formatDate(timeSource.data["Local"]["DateTime"], Qt.DefaultLocaleLongDate)
        style: softwareRendering ? Text.Outline : Text.Normal
        styleColor: softwareRendering ? Kirigami.Theme.backgroundColor : "transparent" //no outline, doesn't matter
        font.pointSize: 24 * ratio
        Layout.alignment: Qt.AlignHCenter
    }
    Plasma5Support.DataSource {
        id: timeSource
        engine: "time"
        connectedSources: ["Local"]
        interval: 1000
    }
}
