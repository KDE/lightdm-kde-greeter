project(greeter-app)

# Versions
set(LIGHTDM_KDE_VERSION_MAJOR 0)
set(LIGHTDM_KDE_VERSION_MINOR 3)
set(LIGHTDM_KDE_VERSION_PATCH 1)
set(LIGHTDM_KDE_VERSION ${LIGHTDM_KDE_VERSION_MAJOR}.${LIGHTDM_KDE_VERSION_MINOR}.${LIGHTDM_KDE_VERSION_PATCH})

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})

set(LIGHTDM_CONFIG_DIR "/etc/lightdm"
    CACHE PATH "Directory where LightDM configuration files are kept")

find_package(X11 REQUIRED)

find_package(Qt5 COMPONENTS Core Gui Widgets DBus X11Extras Qml Quick UiPlugin UiTools REQUIRED)

set(QLIGHTDM_MIN_VERSION 1.4.0)
find_package(QLightDM REQUIRED)

find_package(KF5Declarative REQUIRED)
find_package(KF5IconThemes REQUIRED)
find_package(KF5Plasma REQUIRED)
find_package(KF5Config REQUIRED)
find_package(KF5ConfigWidgets REQUIRED)
find_package(KF5I18n REQUIRED)
find_package(KF5Auth REQUIRED)

find_package(Gettext REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060000 KF_NO_DEPRECATED)

add_subdirectory(greeter)
add_subdirectory(kcm)
add_subdirectory(themes)
