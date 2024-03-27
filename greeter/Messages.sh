#! /usr/bin/env bash

# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

$XGETTEXT `find . ../themes/components -name \*.qml -o -name \*.cpp` -o $podir/lightdm_kde_greeter.pot
