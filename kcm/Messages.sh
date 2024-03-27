#! /usr/bin/env bash

# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/kcm_lightdm.pot
rm -f rc.cpp
