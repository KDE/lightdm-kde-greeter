include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QLIGHTDM_INCLUDE_DIR}
  ${LIBXCB_INCLUDE_DIR}
  )

# All the sources and headers
file(GLOB_RECURSE GREETER_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
list(REMOVE_ITEM GREETER_SRCS rootimageapp.cpp)
list(REMOVE_ITEM GREETER_SRCS wifikeeper.cpp)

file(GLOB_RECURSE GREETER_HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
list(REMOVE_ITEM GREETER_HDRS rootimageapp.h)
list(REMOVE_ITEM GREETER_HDRS wifikeeper.h)

# Create the greeter
add_executable(lightdm-kde-greeter ${GREETER_SRCS} ${GREETER_HDRS})
target_compile_definitions(lightdm-kde-greeter PRIVATE LIBEXEC_DIR="${LIBEXEC_INSTALL_DIR}")

# Link against the needed libraries
target_link_libraries(lightdm-kde-greeter
  Qt5::Core
  Qt5::DBus
  Qt5::Qml
  Qt5::Quick
  Qt5::QuickControls2
  ${QLIGHTDM_LIBRARIES}
  KF5::Declarative
  KF5::IconThemes
  KF5::Plasma
  KF5::ConfigGui
  KF5::I18n
  KF5::NetworkManagerQt
  X11::X11
  X11::Xrandr
  ${LIBXCB_LIBRARIES}
  ${LIBXKB_LIBRARIES}
)
# PkgConfig::UPOWER

install(TARGETS lightdm-kde-greeter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin/)
install(FILES lightdm-kde-greeter.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/xgreeters)

# Create the background hack
set(ROOTIMAGE_SRCS rootimageapp.cpp)
set(ROOTIMAGE_HDRS rootimageapp.h)

# Live a little longer than the greeter to keep NetworkManager connected hack
set(WIFIKEEPER_SRCS wifikeeper.cpp)
set(WIFIKEEPER_HDRS wifikeeper.h)

add_executable(lightdm-kde-greeter-rootimage ${ROOTIMAGE_SRCS} ${ROOTIMAGE_HDRS})
target_link_libraries(lightdm-kde-greeter-rootimage Qt5::Widgets Qt5::X11Extras ${X11_X11_LIB})
install(TARGETS lightdm-kde-greeter-rootimage DESTINATION ${LIBEXEC_INSTALL_DIR})

add_executable(lightdm-kde-greeter-wifikeeper ${WIFIKEEPER_SRCS} ${WIFIKEEPER_HDRS})
target_link_libraries(lightdm-kde-greeter-wifikeeper Qt5::Core KF5::NetworkManagerQt)
install(TARGETS lightdm-kde-greeter-wifikeeper DESTINATION ${LIBEXEC_INSTALL_DIR})
configure_file(lightdm-kde-greeter-wifikeeper.service.in lightdm-kde-greeter-wifikeeper.service @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lightdm-kde-greeter-wifikeeper.service DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/systemd/user)
