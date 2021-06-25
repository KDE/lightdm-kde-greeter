#We want to expose our new config widgets, but because we use QUiLoader we need a QtPlugin to explain how to load/create them.
#We build a static version of this QtPlugin then the KCM includes this, and uses Q_IMPORT_PLUGIN to reference it.
#alternately we could install this as an additional library, but that seems extra complicated.

include_directories(${Qt5Core_INCLUDE_DIRS})

# Insert your sources here
set(LIGHTDM_CONFIG_WIDGET_SRCS
    selectimagebutton.cpp
    selectimagebuttondesignerfactory.cpp
)

set(LIGHTDM_CONFIG_WIDGET_HDRS
    selectimagebutton.h
    selectimagebuttondesignerfactory.h
)

qt5_wrap_cpp(LIGHTDM_CONFIG_WIDGET_HDRS_MOCS ${LIGHTDM_CONFIG_WIDGET_HDRS})

#not sure if this is a bad hack, CMake made me do it
add_definitions(-fPIC)

add_library(lightdm_config_widgets STATIC
                ${LIGHTDM_CONFIG_WIDGET_SRCS}
                ${LIGHTDM_CONFIG_WIDGET_HDRS}
                ${LIGHTDM_CONFIG_WIDGET_HDRS_MOCS})

# Link against the needed libraries
target_link_libraries(lightdm_config_widgets
  Qt5::Widgets
  Qt5::UiPlugin
  KF5::I18n
  KF5::IconThemes
)

#uncomment this if building a non-static version of this lib.
#install( TARGETS lightdm_config_widgets DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer )
