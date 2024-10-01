# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

# Makes it easy to reuse code from the greeter. A bit ugly but I don't feel
# like setting up a static library for one class.
set(GREETER_DIR ${CMAKE_SOURCE_DIR}/greeter)

# Add the compiler flags
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GREETER_DIR}
)

# Insert your sources here
set(LIGHTDM_KCM_SRCS
    themesmodel.cpp
    lightdmkcm.cpp
    ${GREETER_DIR}/extrarowproxymodel.cpp
    ${GREETER_DIR}/usersmodel.cpp
    ${GREETER_DIR}/sessionsmodel.cpp
)

set(LIGHTDM_KCM_HDRS
    themesmodel.h
    lightdmkcm.h
    ${GREETER_DIR}/extrarowproxymodel.h
    ${GREETER_DIR}/usersmodel.h
    ${GREETER_DIR}/sessionsmodel.h
)

kcoreaddons_add_plugin(kcm_lightdm INSTALL_NAMESPACE "plasma/kcms/systemsettings" SOURCES ${LIGHTDM_KCM_SRCS} ${LIGHTDM_KCM_HDRS})

# Link against the needed libraries
target_link_libraries(kcm_lightdm
    Qt6::Quick
    KF6::ConfigWidgets
    KF6::I18n
    KF6::Package
    KF6::KCMUtilsQuick
    KF6::AuthCore
    liblightdm-qt-my
)

set(KCMLIGHTDMHELPER_SRCS
    helper.cpp
)

set(KCMLIGHTDMHELPER_HDRS
    helper.h
)

add_executable(kcmlightdmhelper ${KCMLIGHTDMHELPER_SRCS} ${KCMLIGHTDMHELPER_HDRS})
target_link_libraries(kcmlightdmhelper KF6::AuthCore KF6::ConfigCore)
install(TARGETS kcmlightdmhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

# common qml components for the configuration utility
# copied for use from the theme directories, and from the main config directory
install(DIRECTORY package/contents/ui/components/
        DESTINATION ${DATA_INSTALL_DIR}/lightdm-kde-greeter/themes/components/kcm
)

kauth_install_helper_files(kcmlightdmhelper org.kde.kcontrol.kcmlightdm root)
kauth_install_actions(org.kde.kcontrol.kcmlightdm kcm_lightdm.actions)
kcmutils_generate_desktop_file(kcm_lightdm)
kpackage_install_package(package kcm_lightdm kcms)
