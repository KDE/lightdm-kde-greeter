/*
 *   Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQml 2.15

QtObject {

    // types - to correctly parse a string from the config
    property int cfgUnknown: 0
    property int cfgString:  1
    property int cfgInteger: 2
    property int cfgBoolean: 3

    property string branch
    property string name
    property var type: cfgUnknown
    property var defaultValue

    property var value: defaultValue // --> to widget
    property var listenValue         // <-- from widget
    property var storedValue

    function valueFromString(stringValue) {

        switch (type) {
            case cfgString:  value = stringValue; break;
            case cfgInteger: value = parseInt(stringValue); break;
            case cfgBoolean: value = stringValue === "true"; break;
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
