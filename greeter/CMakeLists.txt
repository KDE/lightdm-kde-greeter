include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QLIGHTDM_INCLUDE_DIR}
)

# Insert your sources here
set(GREETER_SRCS
    extrarowproxymodel.cpp
    faceimageprovider.cpp
    greeterwindow.cpp
    main.cpp
    configwrapper.cpp
    sessionsmodel.cpp
    usersmodel.cpp
    screensmodel.cpp
    greeterwrapper.cpp
)

set(GREETER_HDRS
    extrarowproxymodel.h
    faceimageprovider.h
    greeterwindow.h
    configwrapper.h
    sessionsmodel.h
    usersmodel.h
    screensmodel.h
    greeterwrapper.h
    ../about.h
)

qt5_wrap_cpp(GREETER_HDRS_MOCS ${GREETER_HDRS})

qt5_add_dbus_interface(GREETER_SRCS
    org.freedesktop.UPower.xml
    upower
)

# Create the greeter
add_executable(lightdm-kde-greeter ${GREETER_SRCS} ${GREETER_HDRS} ${GREETER_HDRS_MOCS})

# Link against the needed libraries
target_link_libraries(lightdm-kde-greeter
  Qt5::Core
  Qt5::DBus
  Qt5::Qml
  Qt5::Quick
  ${QLIGHTDM_LIBRARIES}
  KF5::Declarative
  KF5::IconThemes
  KF5::Plasma
  KF5::ConfigGui
  KF5::I18n
)
# PkgConfig::UPOWER

install(TARGETS lightdm-kde-greeter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/sbin/)
install(FILES lightdm-kde-greeter.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/xgreeters)

# Create the background hack
set(ROOTIMAGE_SRCS
    rootimageapp.cpp
)

set(ROOTIMAGE_HDRS
    rootimageapp.h
    )

qt5_wrap_cpp(ROOTIMAGE_HDRS_MOCS ${ROOTIMAGE_HDRS})

add_executable(lightdm-kde-greeter-rootimage ${ROOTIMAGE_SRCS} ${ROOTIMAGE_HDRS} ${ROOTIMAGE_HDRS_MOCS})
target_link_libraries(lightdm-kde-greeter-rootimage Qt5::Widgets Qt5::X11Extras ${X11_X11_LIB})
install(TARGETS lightdm-kde-greeter-rootimage DESTINATION ${LIBEXEC_INSTALL_DIR})
