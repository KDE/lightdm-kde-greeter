# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/QLightDM
)

add_library(liblightdm-qt-my STATIC
    greeter.cpp
    power.cpp
    sessionsmodel.cpp
    usersmodel.cpp
)

set_property(TARGET liblightdm-qt-my PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(liblightdm-qt-my
    Qt6::Core
    Qt6::Gui
    PkgConfig::liblightdm
    PkgConfig::gtk2
)
