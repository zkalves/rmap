# This file is part of rmap.
#
# rmap is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rmap is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rmap.  If not, see <https://www.gnu.org/licenses/>.
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
    test_ProtobufLogCollector
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
    test_RegBitfieldBarWidget
    test_BlockMemoryMapWidget
    test_RegMapTreeView
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
