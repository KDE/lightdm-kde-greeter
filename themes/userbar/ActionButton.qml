/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023-2024 Anton Golubev <golubevan@altlinux.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.kirigami as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3

Item {
    id: root
    property alias text: label.text
    property alias iconSource: icon.source
    property alias containsMouse: mouseArea.containsMouse
    property alias font: label.font
    property alias labelRendering: label.renderType
    property alias circleOpacity: iconCircle.opacity
    property alias circleVisiblity: iconCircle.visible
    property int fontSize: Kirigami.Theme.defaultFont.pointSize + 1
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software
    signal clicked

    activeFocusOnTab: true

    property int iconSize: Kirigami.Units.gridUnit * 3

    implicitWidth: Math.max(iconSize + Kirigami.Units.largeSpacing * 2, label.contentWidth)
    implicitHeight: iconSize + Kirigami.Units.smallSpacing + label.implicitHeight

    Behavior on opacity {
        PropertyAnimation { // OpacityAnimator makes it turn black at random intervals
            duration: Kirigami.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    Rectangle {
        id: iconCircle
        anchors.centerIn: icon
        width: iconSize + Kirigami.Units.smallSpacing
        height: width
        radius: Math.floor(width / 2)
        color: Kirigami.Theme.backgroundColor
        opacity: 0.6
        Behavior on opacity {
            PropertyAnimation { // OpacityAnimator makes it turn black at random intervals
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    Rectangle {
        id: iconCircleBorder
        anchors.centerIn: icon
        width: iconSize + Kirigami.Units.smallSpacing
        height: width
        radius: Math.floor(width / 2)
        color: "transparent"
        border {
            color: containsMouse ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
            width: 2
        }
    }

    Kirigami.Icon {
        id: icon
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        width: iconSize
        height: iconSize

        active: mouseArea.containsMouse || root.activeFocus
    }

    PlasmaComponents3.Label {
        id: label
        font.pointSize: root.fontSize
        anchors {
            top: icon.bottom
            topMargin: (softwareRendering ? 1.5 : 1) * Kirigami.Units.mediumSpacing
            left: parent.left
            right: parent.right
        }
        style: softwareRendering ? Text.Outline : Text.Normal
        styleColor: softwareRendering ? Kirigami.Theme.backgroundColor : "transparent" //no outline, doesn't matter
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        wrapMode: Text.WordWrap
        font.underline: root.activeFocus
    }

    Shadow { source: label }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        onClicked: root.clicked()
        anchors.fill: parent
    }

    Keys.onEnterPressed: clicked()
    Keys.onReturnPressed: clicked()
    Keys.onSpacePressed: clicked()

    Accessible.onPressAction: clicked()
    Accessible.role: Accessible.Button
    Accessible.name: label.text
}
