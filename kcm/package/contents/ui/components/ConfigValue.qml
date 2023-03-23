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

import QtQml 2.15

QtObject {

    // types
    property int cfgUnknown: 0
    property int cfgString:  1
    property int cfgInteger: 2

    property string branch
    property string name
    property var type: cfgUnknown
    property var defaultValue

    property var value: defaultValue // --> to widget
    property var listenValue         // <-- from widget
    property var storedValue

    function valueFromString(stringValue) {

        switch (type) {
            case cfgInteger: value = parseInt(stringValue); break;
            case cfgString:  value = stringValue; break;
            default: console.error("Type unknown for ConfigValue: (" + type + ") name: " + name + " branch: " + branch)
        }
    }

    onListenValueChanged: {
        if (!(listenValue || listenValue == "") || branch == "") return
        var parsed = listenValue.toString()
        kcm.updateConfigValue(branch + name, parsed)
        // without this check, you get a binding loop: value -> widget -> listenValue -> value ...
        if (value != listenValue) value = listenValue
    }
}
