# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

function(rmap_determine_version)
    # 1. Read base version from root VERSION file
    set(VERSION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/VERSION")
    if(EXISTS "${VERSION_FILE}")
        file(READ "${VERSION_FILE}" RAW_VERSION)
        string(STRIP "${RAW_VERSION}" BASE_VERSION)
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${VERSION_FILE}")
    else()
        set(BASE_VERSION "0.2.0")
    endif()

    # Parse base version
    if(BASE_VERSION MATCHES "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(.*)")
        set(MAJOR "${CMAKE_MATCH_1}")
        set(MINOR "${CMAKE_MATCH_2}")
        set(PATCH "${CMAKE_MATCH_3}")
        set(EXTRA "${CMAKE_MATCH_4}")
    else()
        set(MAJOR "0")
        set(MINOR "2")
        set(PATCH "0")
        set(EXTRA "")
    endif()

    # 2. Check Git metadata
    find_package(Git QUIET)
    set(GIT_FOUND_REPO FALSE)
    set(GIT_HASH "unknown")
    set(GIT_BRANCH "unknown")
    set(DIRTY_FLAG "")
    set(COMMITS_AHEAD "0")
    set(GIT_TAG_EXACT FALSE)

    if(GIT_FOUND)
        execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse --is-inside-work-tree
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            OUTPUT_VARIABLE IS_GIT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if("${IS_GIT}" STREQUAL "true")
            set(GIT_FOUND_REPO TRUE)

            # Track git state changes for CMake reconfigure
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git/HEAD")
                set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/HEAD")
            endif()
            if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git/index")
                set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/index")
            endif()

            # Short commit hash
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" rev-parse --short=8 HEAD
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE GIT_HASH
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )

            # Branch name
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE GIT_BRANCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )

            # Dirty check
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" status --porcelain
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE GIT_STATUS
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(NOT "${GIT_STATUS}" STREQUAL "")
                set(DIRTY_FLAG "-dirty")
            endif()

            # Exact tag check
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" describe --tags --match "v[0-9]*" --exact-match
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                OUTPUT_VARIABLE EXACT_TAG
                RESULT_VARIABLE EXACT_TAG_RES
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(EXACT_TAG_RES EQUAL 0 AND EXACT_TAG MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)$")
                set(MAJOR "${CMAKE_MATCH_1}")
                set(MINOR "${CMAKE_MATCH_2}")
                set(PATCH "${CMAKE_MATCH_3}")
                set(GIT_TAG_EXACT TRUE)
            else()
                # Try general tag description
                execute_process(
                    COMMAND "${GIT_EXECUTABLE}" describe --tags --match "v[0-9]*"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                    OUTPUT_VARIABLE TAG_DESCRIBE
                    RESULT_VARIABLE TAG_DESCRIBE_RES
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET
                )
                if(TAG_DESCRIBE_RES EQUAL 0 AND TAG_DESCRIBE MATCHES "^v?([0-9]+)\\.([0-9]+)\\.([0-9]+)-([0-9]+)-g([0-9a-fA-F]+)$")
                    set(MAJOR "${CMAKE_MATCH_1}")
                    set(MINOR "${CMAKE_MATCH_2}")
                    set(PATCH "${CMAKE_MATCH_3}")
                    set(COMMITS_AHEAD "${CMAKE_MATCH_4}")
                endif()
            endif()
        endif()
    endif()

    # 3. Construct SemVer 2.0.0 string
    set(PRERELEASE "")
    set(BUILD_METADATA "")

    if(GIT_TAG_EXACT)
        if(NOT "${DIRTY_FLAG}" STREQUAL "")
            set(BUILD_METADATA "dirty")
        endif()
    elseif(GIT_FOUND_REPO)
        if(NOT "${COMMITS_AHEAD}" STREQUAL "0")
            set(PRERELEASE "dev.${COMMITS_AHEAD}")
        else()
            set(PRERELEASE "dev")
        endif()
        set(BUILD_METADATA "${GIT_HASH}${DIRTY_FLAG}")
    else()
        if(NOT "${EXTRA}" STREQUAL "")
            set(PRERELEASE "${EXTRA}")
        endif()
    endif()

    # Format version string
    set(SEMVER_FULL "${MAJOR}.${MINOR}.${PATCH}")
    if(NOT "${PRERELEASE}" STREQUAL "")
        string(APPEND SEMVER_FULL "-${PRERELEASE}")
    endif()
    if(NOT "${BUILD_METADATA}" STREQUAL "")
        string(APPEND SEMVER_FULL "+${BUILD_METADATA}")
    endif()

    string(TIMESTAMP BUILD_DATE "%Y-%m-%d" UTC)

    # Export variables to caller
    set(RMAP_VERSION_MAJOR "${MAJOR}" PARENT_SCOPE)
    set(RMAP_VERSION_MINOR "${MINOR}" PARENT_SCOPE)
    set(RMAP_VERSION_PATCH "${PATCH}" PARENT_SCOPE)
    set(RMAP_VERSION_SEMVER "${MAJOR}.${MINOR}.${PATCH}" PARENT_SCOPE)
    set(RMAP_VERSION_PRERELEASE "${PRERELEASE}" PARENT_SCOPE)
    set(RMAP_VERSION_BUILD "${BUILD_METADATA}" PARENT_SCOPE)
    set(RMAP_VERSION_STRING "${SEMVER_FULL}" PARENT_SCOPE)
    set(RMAP_GIT_HASH "${GIT_HASH}" PARENT_SCOPE)
    set(RMAP_GIT_BRANCH "${GIT_BRANCH}" PARENT_SCOPE)
    set(RMAP_BUILD_DATE "${BUILD_DATE}" PARENT_SCOPE)
endfunction()

function(rmap_generate_version_header)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/RmapVersion.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/RmapVersion.hpp"
        @ONLY
    )
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/src/RmapVersion.hpp.in"
        "${CMAKE_CURRENT_BINARY_DIR}/include/RmapVersion.hpp"
        @ONLY
    )
    message(STATUS "rmap Version: ${RMAP_VERSION_STRING} (Git: ${RMAP_GIT_BRANCH}@${RMAP_GIT_HASH})")
endfunction()
