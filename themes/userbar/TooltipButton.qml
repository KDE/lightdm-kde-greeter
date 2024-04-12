/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick.Controls 2.15
import QtQuick 2.15
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.ToolButton {
    // if expand == true, a permanent caption will be shown, if false - the
    // caption will shown as a tooltip
    property bool expand: true
    property string caption
    property int approximateFullWidth: visible ? captionSize.width + height + spacing : 0

    Label {
        id: captionSize
        visible: false
        text: caption
    }

    text: expand ? caption : null
    hoverEnabled: true
    ToolTip.delay: params.toolTipDelay
    ToolTip.timeout: params.toolTipTimeout
    ToolTip.visible: !expand && hovered
    ToolTip.text: caption

    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()
}
