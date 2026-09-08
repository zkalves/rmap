# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

# Backend Tests
set(BACKEND_TESTS
    test_PathUtils
    test_RegMapTreeItem
    test_RegMapTreeModel
    test_Serialization
    test_CodeGenerator
    test_Formats
    test_ThemeManager
    test_LanguageManager
    test_Version
)

foreach(test_name ${BACKEND_TESTS})
    qt_add_executable(${test_name} backend/${test_name}.cpp)
    target_compile_options(${test_name} PRIVATE -Wfatal-errors)
    target_link_libraries(${test_name} PRIVATE rmap_core Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        LABELS "unit;backend"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    set_property(TARGET ${test_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endforeach()

# Frontend Tests
set(FRONTEND_TESTS
    test_RegMapWindow
    test_RegConfigWindow
    test_PreferencesWindow
    test_AboutWindow
    test_Delegates
)

foreach(test_name ${FRONTEND_TESTS})
    qt_add_executable(${test_name} frontend/${test_name}.cpp)
    target_compile_options(${test_name} PRIVATE -Wfatal-errors)
    target_link_libraries(${test_name} PRIVATE rmap_core Qt6::Test)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES
        LABELS "unit;frontend"
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
    set_property(TARGET ${test_name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endforeach()

# Automated Python Test Suites (Coverage Engine & Semantic Versioning)
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
    add_test(NAME test_Coverage COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_coverage.py)
    set_tests_properties(test_Coverage PROPERTIES
        LABELS "unit;backend;coverage"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )

    add_test(NAME test_SemVer COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_semver.py)
    set_tests_properties(test_SemVer PROPERTIES
        LABELS "unit;backend;version"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    )
endif()
