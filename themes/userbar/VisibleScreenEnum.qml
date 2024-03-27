/*
This file is part of LightDM-KDE.

Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.15

Item {
    id: visibleScreenEnum

    enum VisibleScreen {
        DefaultScreen,
        LoginScreen,
        WaitScreen,
        PromptScreen
    }
}
