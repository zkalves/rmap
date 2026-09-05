# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

cmake_minimum_required(VERSION 3.15)

project(rmap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Fetch Dependencies
include(FetchContent)
set(INJA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(INJA_USE_EMBEDDED_JSON ON CACHE BOOL "" FORCE)

option(BUILD_BENCHMARK "" OFF)

FetchContent_Declare(inja
  GIT_REPOSITORY https://github.com/pantor/inja
  GIT_TAG v3.3.0
  SYSTEM
)
FetchContent_MakeAvailable(inja)

# Testing Support
include(CTest)

# Find Packages
find_package(Qt6 COMPONENTS REQUIRED Core Widgets)
find_package(Protobuf REQUIRED)
if(BUILD_TESTING)
    find_package(Qt6 COMPONENTS REQUIRED Test)
endif()

# Qt Auto Tools
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS src/ui)

# Core Library Sources & Headers
set(core_sources
    src/PathUtils.cpp
    src/ProtobufLogCollector.cpp
    src/RegMapTreeItem.cpp
    src/RegMapTreeModel.cpp
    src/RegMapTreeView.cpp
    src/RegMapWindow.cpp
    src/RegMapDelegate.cpp
    src/RegConfigWindow.cpp
    src/RegBitfieldBarWidget.cpp
    src/BlockMemoryMapWidget.cpp
    src/CodeGenerator.cpp
    src/format/FormatManager.cpp
    src/format/ProtobufHandler.cpp
    src/format/SystemRdlHandler.cpp
    src/format/IpxactHandler.cpp
    src/format/CmsisSvdHandler.cpp
    src/format/JsonHandler.cpp
    src/format/CsvHandler.cpp
    src/ThemeManager.cpp
    src/AppSettings.cpp
    src/PreferencesWindow.cpp
)

set(core_headers
    src/PathUtils.hpp
    src/ProtobufLogCollector.hpp
    src/Serializable.hpp
    src/SerializationContext.hpp
    src/ObjectFactory.hpp
    src/UndoCommands.hpp
    src/RegMapTreeItem.hpp
    src/RegMapTreeModel.hpp
    src/RegMapTreeView.hpp
    src/RegMapWindow.hpp
    src/RegMapDelegate.hpp
    src/RegConfigWindow.hpp
    src/PreferencesWindow.hpp
    src/RegBitfieldBarWidget.hpp
    src/BlockMemoryMapWidget.hpp
    src/CodeGenerator.hpp
    src/ThemeManager.hpp
    src/AppSettings.hpp
    src/format/IFormatHandler.hpp
    src/format/FormatManager.hpp
    src/format/ProtobufHandler.hpp
    src/format/SystemRdlHandler.hpp
    src/format/IpxactHandler.hpp
    src/format/CmsisSvdHandler.hpp
    src/format/JsonHandler.hpp
    src/format/CsvHandler.hpp
)

set(project_ui src/ui/config.ui src/ui/preferences.ui src/ui/rmap.ui)
set(project_qrc res/resources.qrc)

# Core Static Library
qt_add_library(rmap_core STATIC
    ${core_sources}
    ${core_headers}
    ${project_ui}
    ${project_qrc}
)

# Protobuf Generation (Modern Target-Based Approach)
set(PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/proto/rmap.proto)
protobuf_generate(TARGET rmap_core PROTOS ${PROTOS})

target_include_directories(rmap_core PUBLIC
    src
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/proto
    ${CMAKE_CURRENT_BINARY_DIR}/rmap_core_autogen/include
    ${inja_SOURCE_DIR}/single_include/inja
)

target_compile_options(rmap_core PRIVATE -Wfatal-errors)

target_link_libraries(rmap_core PUBLIC
    Qt6::Core
    Qt6::Widgets
    protobuf::libprotobuf
    pantor::inja
)

# Target Executable
qt_add_executable(rmap
    src/main.cpp
    src/rmap.hpp
)

target_compile_options(rmap PRIVATE -Wfatal-errors)
target_link_libraries(rmap PRIVATE rmap_core)

# Tests Subdirectory
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()


