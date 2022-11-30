cmake_policy(SET CMP0012 NEW)
project(greeter-app)

# Versions
set(LIGHTDM_KDE_VERSION_MAJOR 0)
set(LIGHTDM_KDE_VERSION_MINOR 3)
set(LIGHTDM_KDE_VERSION_PATCH 1)
set(LIGHTDM_KDE_VERSION ${LIGHTDM_KDE_VERSION_MAJOR}.${LIGHTDM_KDE_VERSION_MINOR}.${LIGHTDM_KDE_VERSION_PATCH})

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

set(LIGHTDM_CONFIG_DIR "/etc/lightdm"
    CACHE PATH "Directory where LightDM configuration files are kept")

find_package(X11 REQUIRED)

find_package(Qt5 CONFIG REQUIRED COMPONENTS Core Gui Widgets DBus X11Extras Qml Quick UiPlugin UiTools)

set(QLIGHTDM_MIN_VERSION 1.4.0)
find_package(QLightDM REQUIRED)

find_package(KF5 REQUIRED COMPONENTS
    Declarative
    IconThemes
    Plasma
    Config
    ConfigWidgets
    I18n
    Auth
)

find_package(Gettext REQUIRED)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000 KF_NO_DEPRECATED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

add_subdirectory(greeter)
add_subdirectory(kcm)
add_subdirectory(themes)

find_package(KF5I18n CONFIG REQUIRED)
ki18n_install(po)
