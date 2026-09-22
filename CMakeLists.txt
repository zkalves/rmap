# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

cmake_minimum_required(VERSION 3.15)

# Semantic Versioning Determination
include(cmake/Version.cmake)
rmap_determine_version()

project(rmap VERSION ${RMAP_VERSION_SEMVER} LANGUAGES CXX)

rmap_generate_version_header()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Code Coverage Support
option(ENABLE_COVERAGE "Enable code coverage metrics instrumentation" OFF)

if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Enabling code coverage instrumentation across all targets")
        add_compile_options(--coverage -O0 -g)
        add_link_options(--coverage)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-fcondition-coverage" COMPILER_SUPPORTS_CONDITION_COVERAGE)
        if(COMPILER_SUPPORTS_CONDITION_COVERAGE)
            message(STATUS "Enabling condition coverage (-fcondition-coverage)")
            add_compile_options(-fcondition-coverage)
        endif()
    else()
        message(WARNING "Code coverage is only supported on GCC and Clang")
    endif()
endif()


# Testing Support
include(CTest)

# Installation Directories
include(GNUInstallDirs)

# Find Packages
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Protobuf QUIET)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
if(BUILD_TESTING)
    find_package(Qt6 COMPONENTS Test QUIET)
endif()

# Core Library Sources & Headers
set(core_sources
    src/PathUtils.cpp
    src/ProtobufLogCollector.cpp
    src/RegMapTreeItem.cpp
    src/RegMapTreeModel.cpp
    src/CodeGenerator.cpp
    src/UndoCommands.cpp
    src/format/FormatManager.cpp
    src/format/ProtobufHandler.cpp
    src/format/SystemRdlHandler.cpp
    src/format/IpxactHandler.cpp
    src/format/CmsisSvdHandler.cpp
    src/format/JsonHandler.cpp
    src/format/CsvHandler.cpp
    src/ThemeManager.cpp
    src/LanguageManager.cpp
    src/AppSettings.cpp
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
    src/CodeGenerator.hpp
    src/ThemeManager.hpp
    src/LanguageManager.hpp
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

# Fetch Inja Dependency
include(FetchContent)
    set(INJA_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(INJA_USE_EMBEDDED_JSON ON CACHE BOOL "" FORCE)
    set(INJA_INSTALL OFF CACHE BOOL "" FORCE)
    set(INJA_EXPORT OFF CACHE BOOL "" FORCE)

    option(BUILD_BENCHMARK "" OFF)

    if(NOT DEFINED FETCHCONTENT_SOURCE_DIR_INJA AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/inja-src")
        set(FETCHCONTENT_SOURCE_DIR_INJA "${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/inja-src")
    endif()

    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.25")
        FetchContent_Declare(inja
          GIT_REPOSITORY https://github.com/pantor/inja
          GIT_TAG v3.3.0
          SYSTEM
        )
    else()
        FetchContent_Declare(inja
          GIT_REPOSITORY https://github.com/pantor/inja
          GIT_TAG v3.3.0
        )
    endif()
    set(PREV_CMAKE_WARN_DEPRECATED "${CMAKE_WARN_DEPRECATED}")
    set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(inja)
    set(CMAKE_WARN_DEPRECATED "${PREV_CMAKE_WARN_DEPRECATED}" CACHE BOOL "" FORCE)

    # Qt Auto Tools
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOUIC_SEARCH_PATHS src/ui)

    set(gui_sources
        src/RegMapTreeView.cpp
        src/RegMapWindow.cpp
        src/RegMapDelegate.cpp
        src/RegConfigWindow.cpp
        src/RegBitfieldBarWidget.cpp
        src/BlockMemoryMapWidget.cpp
        src/PreferencesWindow.cpp
        src/AboutWindow.cpp
    )

    set(gui_headers
        src/RegMapTreeView.hpp
        src/RegMapWindow.hpp
        src/RegMapDelegate.hpp
        src/RegConfigWindow.hpp
        src/PreferencesWindow.hpp
        src/AboutWindow.hpp
        src/RegBitfieldBarWidget.hpp
        src/BlockMemoryMapWidget.hpp
    )

    set(project_ui src/ui/config.ui src/ui/preferences.ui src/ui/about.ui src/ui/rmap.ui)
    set(project_qrc res/resources.qrc)

    # Core Static Library
    qt_add_library(rmap_core STATIC
        ${core_sources}
        ${gui_sources}
        ${core_headers}
        ${gui_headers}
        ${project_ui}
        ${project_qrc}
    )

    # Protobuf Generation (Robust Target-Based Approach with Proto3 Optional Support)
    set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/proto/rmap.pb.cc")
    set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/proto/rmap.pb.h")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/proto")

    if(TARGET protobuf::protoc)
        set(PROTOC_CMD protobuf::protoc)
    elseif(Protobuf_PROTOC_EXECUTABLE)
        set(PROTOC_CMD ${Protobuf_PROTOC_EXECUTABLE})
    else()
        set(PROTOC_CMD protoc)
    endif()

    add_custom_command(
        OUTPUT "${PROTO_SRC}" "${PROTO_HDR}"
        COMMAND ${PROTOC_CMD}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
                --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
                --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
                --experimental_allow_proto3_optional
                ${CMAKE_CURRENT_SOURCE_DIR}/proto/rmap.proto
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/proto/rmap.proto"
        COMMENT "Running cpp protocol buffer compiler on proto/rmap.proto"
        VERBATIM
    )
    target_sources(rmap_core PRIVATE "${PROTO_SRC}" "${PROTO_HDR}")

    target_include_directories(rmap_core PUBLIC
        src
        ${CMAKE_CURRENT_SOURCE_DIR}/proto
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/proto
        ${CMAKE_CURRENT_BINARY_DIR}/rmap_core_autogen/include
        ${inja_SOURCE_DIR}/single_include/inja
    )

    target_compile_options(rmap_core PRIVATE -Wfatal-errors)

    target_compile_definitions(rmap_core PUBLIC
        HAVE_QT_GUI
        RMAP_INSTALL_DATADIR="${CMAKE_INSTALL_FULL_DATADIR}/rmap"
        RMAP_INSTALL_TEMPLATES_DIR="${CMAKE_INSTALL_FULL_DATADIR}/rmap/templates"
        RMAP_INSTALL_EXAMPLES_DIR="${CMAKE_INSTALL_FULL_DATADIR}/rmap/examples"
        RMAP_INSTALL_DOCDIR="${CMAKE_INSTALL_FULL_DOCDIR}"
        RMAP_INSTALL_TRANSLATIONS_DIR="${CMAKE_INSTALL_FULL_DATADIR}/rmap/translations"
    )

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
    target_compile_definitions(rmap PRIVATE HAVE_QT_GUI)
    target_link_libraries(rmap PRIVATE rmap_core Qt6::Widgets)

# Installation Targets & Assets
install(TARGETS rmap
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION .
)

install(DIRECTORY templates/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/rmap/templates
    PATTERN ".*" EXCLUDE
)

install(DIRECTORY examples/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/rmap/examples
    PATTERN ".*" EXCLUDE
)

# Documentation Generation Target
add_custom_target(docs
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/script/generate_docs.py
        --project-root ${CMAKE_CURRENT_SOURCE_DIR}
        --html-dir ${CMAKE_CURRENT_BINARY_DIR}/docs/html
        --user-pdf ${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_user_manual.pdf
        --dev-pdf ${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_developer_guide.pdf
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating HTML documentation, User Manual PDF, and Developer Guide PDF"
)

# Documentation Installation (HTML Portal, User Manual PDF & Developer Guide PDF)
install(CODE "
    if(NOT EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/docs/html/index.html\"
       OR NOT EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_user_manual.pdf\"
       OR NOT EXISTS \"${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_developer_guide.pdf\")
        message(STATUS \"Generating HTML and PDF documentation for installation...\")
        execute_process(
            COMMAND \"${Python3_EXECUTABLE}\" \"${CMAKE_CURRENT_SOURCE_DIR}/script/generate_docs.py\"
                --project-root \"${CMAKE_CURRENT_SOURCE_DIR}\"
                --html-dir \"${CMAKE_CURRENT_BINARY_DIR}/docs/html\"
                --user-pdf \"${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_user_manual.pdf\"
                --dev-pdf \"${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_developer_guide.pdf\"
            RESULT_VARIABLE _docs_res
        )
        if(NOT _docs_res EQUAL 0)
            message(WARNING \"Documentation generation returned non-zero status: \${_docs_res}\")
        endif()
    endif()
")

install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs/html/"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/html
    PATTERN ".*" EXCLUDE
    PATTERN "*.pyc" EXCLUDE
    PATTERN "__pycache__" EXCLUDE
    PATTERN "pdf" EXCLUDE
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_user_manual.pdf"
    "${CMAKE_CURRENT_BINARY_DIR}/docs/rmap_developer_guide.pdf"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/pdf
    OPTIONAL
)

# Relative symlink from html/pdf to ../pdf so HTML portal links resolve without duplicating files
if(UNIX)
    install(CODE "
        execute_process(
            COMMAND \"${CMAKE_COMMAND}\" -E create_symlink ../pdf \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DOCDIR}/html/pdf\"
        )
    ")
endif()

install(DIRECTORY translations
    DESTINATION ${CMAKE_INSTALL_DATADIR}/rmap
    FILES_MATCHING PATTERN "*.json" PATTERN "*.qm" PATTERN "*.md"
)

install(DIRECTORY themes
    DESTINATION ${CMAKE_INSTALL_DATADIR}/rmap
    FILES_MATCHING PATTERN "*.json" PATTERN "*.md"
)

# FreeDesktop Desktop Entry & Icons
install(FILES res/rmap.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES res/images/app_icon.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps
    RENAME rmap.png
)

foreach(icon_size 16 32 48 64 128 256 512)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/images/app_icon_${icon_size}.png")
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/res/images/app_icon_${icon_size}.png"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/${icon_size}x${icon_size}/apps"
            RENAME rmap.png
        )
    endif()
endforeach()

# Tests Subdirectory
if(BUILD_TESTING AND Qt6_FOUND)
    add_subdirectory(tests)
endif()

# Documentation Automation Targets
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(docs-doxygen
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating C++ API documentation with Doxygen"
        VERBATIM
    )
else()
    add_custom_target(docs-doxygen
        COMMAND ${CMAKE_COMMAND} -E echo "======================================================================"
        COMMAND ${CMAKE_COMMAND} -E echo "ERROR: Doxygen is required to generate C++ API documentation,"
        COMMAND ${CMAKE_COMMAND} -E echo "       but 'doxygen' was not found on PATH."
        COMMAND ${CMAKE_COMMAND} -E echo "======================================================================"
        COMMAND ${CMAKE_COMMAND} -E echo "Please install Doxygen:"
        COMMAND ${CMAKE_COMMAND} -E echo "  - Ubuntu/Debian: sudo apt-get install -y doxygen graphviz"
        COMMAND ${CMAKE_COMMAND} -E echo "  - macOS:         brew install doxygen graphviz"
        COMMAND ${CMAKE_COMMAND} -E echo "  - Fedora/RHEL:   sudo dnf install doxygen graphviz"
        COMMAND ${CMAKE_COMMAND} -E echo "  - Arch Linux:    sudo pacman -S doxygen graphviz"
        COMMAND ${CMAKE_COMMAND} -E false
        COMMENT "Doxygen not found on PATH"
        VERBATIM
    )
endif()

add_custom_target(docs-classes DEPENDS docs-doxygen)

# CPack Packaging Configuration
include(cmake/Packaging.cmake)




