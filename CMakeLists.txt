# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

cmake_minimum_required(VERSION 3.16.0)
project(greeter-app)

# Versions
set(LIGHTDM_KDE_VERSION_MAJOR 0)
set(LIGHTDM_KDE_VERSION_MINOR 4)
set(LIGHTDM_KDE_VERSION_PATCH 23)
set(LIGHTDM_KDE_VERSION ${LIGHTDM_KDE_VERSION_MAJOR}.${LIGHTDM_KDE_VERSION_MINOR}.${LIGHTDM_KDE_VERSION_PATCH})

# directory where images will be saved so that the greeter has access to it,
# since it runs without privileges
if(NOT DEFINED GREETER_IMAGES_DIR)
    message(FATAL_ERROR "GREETER_IMAGES_DIR must be provided!")
endif()

set(KF5_MIN_VERSION 5.97)
# TODO cleanup code so that this isn't needed
set(KDE_COMPILERSETTINGS_LEVEL "5.84")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${ECM_MODULE_PATH})
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

set(LIGHTDM_CONFIG_DIR "/etc/lightdm"
    CACHE PATH "Directory where LightDM configuration files are kept")
set(GREETER_DEFAULT_WALLPAPER "${CMAKE_INSTALL_PREFIX}/share/design/current/backgrounds/xdm.png"
    CACHE PATH "Path to image file")

find_package(X11 REQUIRED)
find_package(XCB REQUIRED)
find_package(XKB REQUIRED)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core Gui Widgets DBus X11Extras Qml Quick QuickWidgets UiPlugin UiTools QuickControls2)

set(QLIGHTDM_MIN_VERSION 1.4.0)
find_package(QLightDM REQUIRED)

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Declarative
    IconThemes
    KCMUtils
    Plasma
    Package
    Config
    ConfigWidgets
    I18n
    Auth
    NetworkManagerQt
)

find_package(Gettext REQUIRED)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000 KF_NO_DEPRECATED)
add_compile_options(-Werror=pedantic)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

add_subdirectory(greeter)
add_subdirectory(kcm)
add_subdirectory(themes)

find_package(KF5I18n CONFIG REQUIRED)
ki18n_install(po)

# Display feature summary
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
