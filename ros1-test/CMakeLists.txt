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
# is-ros2 test SystemHandle library and integration-service executable CMake project

###############################################################################################
# CMake build rules for the Integration Service ROS 1 SystemHandle integration test library
###############################################################################################

cmake_minimum_required(VERSION 3.5.0)

project(is-ros1-test)

# Get Integration Service dependencies
find_package(is-core REQUIRED)
find_package(is-mock REQUIRED)
find_package(is-ros1-genmsg REQUIRED)
find_package(is-ros1 REQUIRED)
find_package(xtypes REQUIRED)
find_package(is-ros1-mix-generator REQUIRED)

# Get message dependencies
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

macro(compile_test)
    # Parse arguments
    if("${ARGV0}" STREQUAL "NAME")
        set(uniValueArgs NAME)
    else()
        set(TEST_NAME "${ARGV0}")
    endif()
    set(multiValueArgs SOURCE)
    cmake_parse_arguments(TEST "" "${uniValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_link_libraries(${TEST_NAME}
        PUBLIC
            is::mock
            is::ros1
            ${geometry_msgs_LIBRARIES}
            ${nav_msgs_LIBRARIES}
        PRIVATE
            $<IF:$<BOOL:${IS_GTEST_EXTERNAL_PROJECT}>,libgtest,gtest>
        )

    target_include_directories(${TEST_NAME}
        PRIVATE
            ${geometry_msgs_INCLUDE_DIRS}
            ${nav_msgs_INCLUDE_DIRS}
        )

    target_compile_options(${TEST_NAME}
        INTERFACE
            $<$<CXX_COMPILER_ID:GNU>:-Wall>
            $<$<CXX_COMPILER_ID:GNU>:-Wextra>
        )

    set_target_properties(${TEST_NAME}
        PROPERTIES
            CXX_STANDARD 17
        )

    add_gtest(${TEST_NAME} SOURCES ${TEST_SOURCE})
endmacro()

include(CTest)
include(${IS_GTEST_CMAKE_MODULE_DIR}/gtest.cmake)
enable_testing()

compile_test(${PROJECT_NAME}_geometry_msgs SOURCE integration/ros1__geometry_msgs.cpp)

set_property(
    TARGET ${PROJECT_NAME}_geometry_msgs
    APPEND PROPERTY COMPILE_DEFINITIONS PRIVATE
        "ROS1__GEOMETRY_MSGS__TEST_CONFIG=\"${CMAKE_CURRENT_LIST_DIR}/resources/ros1__geometry_msgs.yaml\""
        "ROS1__GENMSG__BUILD_DIR=\"${CMAKE_BINARY_DIR}/is/genmsg/ros1/lib\""
    )
