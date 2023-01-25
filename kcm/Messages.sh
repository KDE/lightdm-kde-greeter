#! /usr/bin/env bash
$EXTRACTRC `find . -name "*.ui"` >> rc.cpp || exit 11
$XGETTEXT `find . -name \*.qml -o -name \*.cpp` -o $podir/kcm_lightdm.pot
rm -f rc.cpp
