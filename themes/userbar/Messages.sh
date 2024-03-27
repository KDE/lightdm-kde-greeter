# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc -a ! -name theme.rc -o -name \*.ui -o -name \*.kcfg` >> rc.cpp
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/lightdm_theme_userbar.pot
rm -f rc.cpp
