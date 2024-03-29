# Copyright 2019 Open Source Robotics Foundation, Inc.
# Copyright (C) 2020 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#  library and integration-service executable CMake project

##################################################################################
# CMake build rules for the Integration Service ROS 1 SystemHandle library
##################################################################################
cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

project(is-ros1 VERSION "3.1.0" LANGUAGES CXX)

###################################################################################
# Configure options
###################################################################################
option(BUILD_LIBRARY "Compile the ROS 1 SystemHandle" ON)

###################################################################################
# Load external CMake Modules.
###################################################################################
if(BUILD_LIBRARY)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SANITIZERS_ROOT}/cmake)

    find_package(Sanitizers QUIET)

    if(SANITIZE_ADDRESS)
        message(STATUS "Preloading AddressSanitizer library could be done using \"${ASan_WRAPPER}\" to run your program.")
    endif()
endif()

###################################################################################
# External dependencies for the Integration Service ROS 1 SystemHandle library
###################################################################################
if(BUILD_LIBRARY)
    find_package(is-core REQUIRED)
    find_package(catkin REQUIRED COMPONENTS roscpp)
endif()

###################################################################################
# Configure the Integration Service ROS 1 SystemHandle library
###################################################################################
if(BUILD_LIBRARY)
    add_library(${PROJECT_NAME}
        SHARED
            src/Factory.cpp
            src/SystemHandle.cpp
            src/MetaPublisher.cpp
        )

    if (Sanitizers_FOUND)
        add_sanitizers(${PROJECT_NAME})
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION
            ${PROJECT_VERSION}
        SOVERSION
            ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        CXX_STANDARD
            17
        CXX_STANDARD_REQUIRED
            YES
        )

    target_compile_options(${PROJECT_NAME}
        PRIVATE
            $<$<CXX_COMPILER_ID:GNU>:-pedantic>
            $<$<CXX_COMPILER_ID:GNU>:-fstrict-aliasing>
            $<$<CXX_COMPILER_ID:GNU>:-Wall>
            $<$<CXX_COMPILER_ID:GNU>:-Wextra>
            $<$<CXX_COMPILER_ID:GNU>:-Wcast-align>
            $<$<CXX_COMPILER_ID:MSVC>:/W4>
            $<$<CXX_COMPILER_ID:MSVC>:/wd4700>
            $<$<CXX_COMPILER_ID:MSVC>:/wd4996>
            $<$<CXX_COMPILER_ID:MSVC>:/wd4820>
            $<$<CXX_COMPILER_ID:MSVC>:/wd4255>
            $<$<CXX_COMPILER_ID:MSVC>:/wd4668>
        )

    include(GNUInstallDirs)
    message(STATUS "Configuring [${PROJECT_NAME}]...")

    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            is::core
            ${catkin_LIBRARIES}
        )

    target_include_directories(${PROJECT_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            ${catkin_INCLUDE_DIRS}
        )
endif()

###################################################################################
# Install the Integration Service ROS 1 SystemHandle library
###################################################################################
if(BUILD_LIBRARY)
    is_install_middleware_plugin(
        MIDDLEWARE
            ros1
        TARGET
            ${PROJECT_NAME}
        DEPENDENCIES
            catkin
    )

    install(
        DIRECTORY
            ${CMAKE_CURRENT_LIST_DIR}/include/
        DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT
            ${PROJECT_NAME}
    )
endif()

###################################################################################
# Integration Service ROS 1 SystemHandle tests
###################################################################################
if(BUILD_LIBRARY)
    if(BUILD_TESTS OR BUILD_ROS1_TESTS)
        add_subdirectory(test)
    endif()
endif()

###################################################################################
# Integration Service ROS 1 SystemHandle API Reference
###################################################################################
if(BUILD_API_REFERENCE)
    if(NOT BUILD_LIBRARY)
        find_file(IS_DOXYGEN_CONFIG_FILE doxygen-config.in PATHS ${CMAKE_INSTALL_PREFIX}/../is-core)
    endif()
    find_package(Doxygen REQUIRED)
    # Create doxygen directories
    add_custom_target(doc-dirs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/doxygen
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/doxygen/html
        COMMENT "Creating documentation directories" VERBATIM)

    set(IS_ROS1_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/is/sh/ros1")
    set(IS_ROS1_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
    file(GLOB_RECURSE HPP_FILES
        "${IS_ROS1_INCLUDE_DIR}/*.h*"
        "${IS_ROS1_SOURCE_DIR}/*.h*")

    # Doxygen related variables
    set(DOXYGEN_INPUT_DIR "${IS_ROS1_INCLUDE_DIR} ${IS_ROS1_SOURCE_DIR}")
    set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DIR}/doxygen")
    set(DOXYGEN_INDEX_FILE "${PROJECT_BINARY_DIR}/doxygen/xml/index.xml")
    set(DOXYFILE_IN "${IS_DOXYGEN_CONFIG_FILE}")
    set(DOXYFILE_OUT ${PROJECT_BINARY_DIR}/doxygen-config)
    set(DOXYGEN_TAGFILES "../is-core/is_core.tag=../is-core/doxygen/html")
    set(DOXYGEN_HTML_DIR "${PROJECT_BINARY_DIR}/doxygen/html")

    # Configure doxygen
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    # Doxygen command
    add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        DEPENDS ${HPP_FILES}
        MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        COMMENT "Generating doxygen documentation")

    # Generate API reference
    add_custom_target(doxygen-${PROJECT_NAME} ALL
        DEPENDS ${DOXYGEN_INDEX_FILE}
        COMMENT "Generated API documentation with doxygen" VERBATIM)
    add_dependencies(doxygen-${PROJECT_NAME} doc-dirs)

    # Install doxygen generated XML files
    install(DIRECTORY ${PROJECT_BINARY_DIR}/doxygen/xml
        DESTINATION doxygen
        COMPONENT doxygen-${PROJECT_NAME})
    set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "ROS1 System Handle doxygen")
    set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION
        "eProsima ROS1 System Handle doxygen documentation")
    set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} doxygen-${PROJECT_NAME})
endif()