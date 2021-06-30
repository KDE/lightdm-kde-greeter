# Makes it easy to reuse code from the greeter. A bit ugly but I don't feel
# like setting up a static library for one class.
set(GREETER_DIR ${CMAKE_SOURCE_DIR}/greeter)

# Add the compiler flags
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QLIGHTDM_INCLUDE_DIR}
  ${GREETER_DIR}
)

add_subdirectory(configwidgets)

# Insert your sources here
set(LIGHTDM_KCM_SRCS
    themeconfig.cpp
    themesmodel.cpp
    coreconfig.cpp
    lightdmkcm.cpp
    configoptions.cpp
    ${GREETER_DIR}/extrarowproxymodel.cpp
    ${GREETER_DIR}/usersmodel.cpp
)

set(LIGHTDM_KCM_HDRS
    themeconfig.h
    themesmodel.h
    coreconfig.h
    lightdmkcm.h
    configoptions.h
    ${GREETER_DIR}/extrarowproxymodel.h
    ${GREETER_DIR}/usersmodel.h
)

set(LIGHTDM_KCM_UI
    themeconfig.ui
    coreconfig.ui)

qt5_wrap_ui(LIGHTDM_KCM_UI_SRCS ${LIGHTDM_KCM_UI})

add_library(kcm_lightdm MODULE
                ${LIGHTDM_KCM_SRCS}
                ${LIGHTDM_KCM_HDRS}
                ${LIGHTDM_KCM_UI_SRCS})


# Link against the needed libraries
target_link_libraries(kcm_lightdm
  ${QLIGHTDM_LIBRARIES}
  Qt5::UiTools
  KF5::ConfigCore
  KF5::ConfigGui
  KF5::ConfigWidgets
  lightdm_config_widgets
)

kcoreaddons_desktop_to_json(kcm_lightdm "kcm_lightdm.desktop")

install(TARGETS kcm_lightdm
          DESTINATION ${CMAKE_INSTALL_PLUGINDIR})

install(FILES kcm_lightdm.desktop
         DESTINATION ${CMAKE_INSTALL_KSERVICES5DIR}
)

set(KCMLIGHTDMHELPER_SRCS
    helper.cpp
)

set(KCMLIGHTDMHELPER_HDRS
    helper.h
)

add_executable(kcmlightdmhelper ${KCMLIGHTDMHELPER_SRCS} ${KCMLIGHTDMHELPER_HDRS})
target_link_libraries(kcmlightdmhelper KF5::Auth KF5::ConfigCore)
install(TARGETS kcmlightdmhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

kauth_install_helper_files(kcmlightdmhelper org.kde.kcontrol.kcmlightdm root)

kauth_install_actions(org.kde.kcontrol.kcmlightdm kcm_lightdm.actions)
