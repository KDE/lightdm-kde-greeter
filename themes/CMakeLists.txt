install(DIRECTORY classic
        DESTINATION ${DATA_INSTALL_DIR}/lightdm-kde-greeter/themes
        PATTERN Messages.sh EXCLUDE
)
install(DIRECTORY userbar
        DESTINATION ${DATA_INSTALL_DIR}/lightdm-kde-greeter/themes
        PATTERN Messages.sh EXCLUDE
)
install(DIRECTORY components
        DESTINATION ${DATA_INSTALL_DIR}/lightdm-kde-greeter/themes
)

# custom targets just for displaying sources in IDE
add_custom_target(classic_theme_sources
        SOURCES
        classic/config.ui
        classic/main.qml
        classic/main.xml
        classic/Messages.sh
        classic/preview.png
        classic/private/ButtonShadow.qml
        classic/private/IconLoader.qml
        classic/private/TextFieldFocus.qml
        classic/ScreenManager.qml
        classic/TextField.qml
        classic/theme.desktop
        classic/ToolButton.qml
        classic/VisibleScreenEnum.qml
)

add_custom_target(userbar_theme_sources
        SOURCES
        userbar/config.ui
        userbar/ListButton.qml
        userbar/main.qml
        userbar/main.xml
        userbar/Messages.sh
        userbar/preview.png
        userbar/ScreenManager.qml
        userbar/PrimaryScreen.qml
        userbar/OtherScreen.qml
        userbar/theme.desktop
        userbar/VisibleScreenEnum.qml
)
