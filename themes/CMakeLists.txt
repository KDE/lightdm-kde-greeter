# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

install(DIRECTORY userbar
        DESTINATION ${DATA_INSTALL_DIR}/lightdm-kde-greeter/themes
        PATTERN Messages.sh EXCLUDE
)
install(DIRECTORY components
        DESTINATION ${DATA_INSTALL_DIR}/lightdm-kde-greeter/themes
)

# custom targets just for displaying sources in IDE
add_custom_target(userbar_theme_sources
        SOURCES
        userbar/ListButton.qml
        userbar/main.qml
        userbar/Messages.sh
        userbar/preview.png
        userbar/ScreenManager.qml
        userbar/PrimaryScreen.qml
        userbar/OtherScreen.qml
        userbar/theme.desktop
        userbar/VisibleScreenEnum.qml
)
