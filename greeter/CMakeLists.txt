# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${LIBXCB_INCLUDE_DIR}
  )

# All the sources and headers
file(GLOB_RECURSE GREETER_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
list(REMOVE_ITEM GREETER_SRCS rootimageapp.cpp)
list(REMOVE_ITEM GREETER_SRCS wifikeeper.cpp)

file(GLOB_RECURSE GREETER_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
list(REMOVE_ITEM GREETER_HDRS rootimageapp.h)
list(REMOVE_ITEM GREETER_HDRS wifikeeper.h)

set_source_files_properties(keyboard/org.kde.KeyboardLayouts.xml PROPERTIES INCLUDE keyboard/layoutnames.h)
qt_add_dbus_interface(GREETER_SRCS "keyboard/org.kde.KeyboardLayouts.xml" keyboard_layout_interface)

# Create the greeter
add_executable(lightdm-kde-greeter ${GREETER_SRCS} ${GREETER_HDRS})
target_compile_definitions(lightdm-kde-greeter PRIVATE LIBEXEC_DIR="${KDE_INSTALL_FULL_LIBEXECDIR}")

# Link against the needed libraries
target_link_libraries(lightdm-kde-greeter
  Qt6::Core
  Qt6::DBus
  Qt6::Qml
  Qt6::Quick
  Qt6::QuickControls2
  Qt6::Widgets
  Qt6::Core5Compat
  KF6::IconThemes
  KF6::ConfigGui
  KF6::I18n
  KF6::NetworkManagerQt
  Plasma::Plasma
  X11::X11
  Qt6::WaylandClientPrivate
  X11::Xrandr
  ${LIBXCB_LIBRARIES}
  ${LIBXKB_LIBRARIES}
  liblightdm-qt-my
  PkgConfig::libei
)

qt6_add_shaders(lightdm-kde-greeter "lightdm-kde-greeter_shaders"
    PREFIX
    "/"
    FILES
    "avatar.frag"
)

install(TARGETS lightdm-kde-greeter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin/)
install(FILES lightdm-kde-greeter-x11.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/xgreeters)
install(FILES lightdm-kde-greeter.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/lightdm/greeters)
configure_file(lightdm-kde-greeter.conf.in lightdm-kde-greeter.conf @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lightdm-kde-greeter.conf DESTINATION ${LIGHTDM_CONFIG_DIR})

# Create the background hack
set(ROOTIMAGE_SRCS rootimageapp.cpp)
set(ROOTIMAGE_HDRS rootimageapp.h)

# Live a little longer than the greeter to keep NetworkManager connected hack
set(WIFIKEEPER_SRCS wifikeeper.cpp)
set(WIFIKEEPER_HDRS wifikeeper.h)

add_executable(lightdm-kde-greeter-rootimage ${ROOTIMAGE_SRCS} ${ROOTIMAGE_HDRS})
target_link_libraries(lightdm-kde-greeter-rootimage Qt6::Widgets ${X11_X11_LIB})
install(TARGETS lightdm-kde-greeter-rootimage DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})

add_executable(lightdm-kde-greeter-wifikeeper ${WIFIKEEPER_SRCS} ${WIFIKEEPER_HDRS})
target_link_libraries(lightdm-kde-greeter-wifikeeper Qt6::Core KF6::NetworkManagerQt)
install(TARGETS lightdm-kde-greeter-wifikeeper DESTINATION ${KDE_INSTALL_FULL_LIBEXECDIR})
configure_file(lightdm-kde-greeter-wifikeeper.service.in lightdm-kde-greeter-wifikeeper.service @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lightdm-kde-greeter-wifikeeper.service DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user)
