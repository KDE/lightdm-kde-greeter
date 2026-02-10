# SPDX-FileCopyrightText: None
# SPDX-License-Identifier: CC0-1.0

add_executable(testQml testQml.cpp ${CMAKE_SOURCE_DIR}/greeter/connectionenum.h)
add_test(NAME testQml COMMAND testQml -platform offscreen -input ${CMAKE_SOURCE_DIR}/tests)

target_link_libraries(testQml
    PRIVATE
    Qt6::QuickTest
    Qt6::Qml
)

target_include_directories(testQml
    PRIVATE
    ${CMAKE_SOURCE_DIR}/greeter
)
