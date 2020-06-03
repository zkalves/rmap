cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(rmap LANGUAGES CXX)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)
find_package(Protobuf REQUIRED)

set(project_sources
           src/ObjectFactory.cpp
           src/RegMapTreeItem.cpp
           src/RegMapTreeModel.cpp
           src/RegMapTreeView.cpp
           src/RegMapWindow.cpp
           src/RegMapDelegate.cpp
           src/RegConfigWindow.cpp
           src/rmap.cpp)

set(project_headers
           src/Serializable.hpp
           src/SerializationContext.hpp
           src/ObjectFactory.hpp
           src/RegMapTreeItem.hpp
           src/RegMapTreeModel.hpp
           src/RegMapTreeView.hpp
           src/RegMapWindow.hpp
           src/RegMapDelegate.hpp
           src/RegConfigWindow.hpp
           src/rmap.hpp)

set(project_ui ui/config.ui ui/rmap.ui)
set(project_qrc ui/resources.qrc)

set(PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/protobuf/rmap.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

include_directories(protobuf)
include_directories( "${CMAKE_CURRENT_BINARY_DIR}" )
add_executable(rmap 
    ${project_sources}
    ${project_headers}
    ${project_ui}
    ${project_qrc}
    ${PROTO_SRCS}
    ${PROTO_HDRS})

target_include_directories(rmap PRIVATE src)
target_link_libraries(rmap 
    PUBLIC
    Qt5::Core
    Qt5::Gui 
    Qt5::Widgets
    ${Protobuf_LIBRARIES})
