/*
    SPDX-FileCopyrightText: 2023 Anton Golubev <golubevan@altlinux.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
import QtGraphicalEffects 1.15

DropShadow {
    source: loginButton
    anchors.fill: source
    visible: source.visible
    opacity: source.opacity
    radius: 10
    samples: 20
    spread: 0.0
    color : "black" // shadows should always be black
}
