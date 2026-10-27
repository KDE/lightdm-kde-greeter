# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

add_executable(testQml testQml.cpp ${CMAKE_SOURCE_DIR}/greeter/connectionenum.h mock_accessibilitybackend.h)
add_test(NAME testQml COMMAND testQml -platform offscreen -input ${CMAKE_SOURCE_DIR}/tests)

target_link_libraries(testQml
    PRIVATE
    Qt6::QuickTest
    Qt6::Qml
)

add_compile_definitions(MOCK_INCLUDES)

add_executable(testCpp
    testCpp.cpp
    tst_Accessibility.cpp
    mock_qdbus.h
    mock_qlightdmgreeter.h
    ${CMAKE_SOURCE_DIR}/greeter/accessibility.cpp
    ${CMAKE_SOURCE_DIR}/greeter/greeterwrapper.cpp
)

target_link_libraries(testCpp
    PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    KF6::ConfigGui
    KF6::I18n
)

target_include_directories(testCpp
    PRIVATE
    ${CMAKE_SOURCE_DIR}/greeter
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

add_test(NAME testCpp COMMAND testCpp)

target_include_directories(testQml
    PRIVATE
    ${CMAKE_SOURCE_DIR}/greeter
)
