#! /usr/bin/env bash
$XGETTEXT `find . ../themes/components -name \*.qml -o -name \*.cpp` -o $podir/lightdm_kde_greeter.pot
