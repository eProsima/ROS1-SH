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
# CMake project to help generating middleware interface extension files for specified ROS types

###############################################################################################
# CMake build rules for the Integration Service ROS 1 pkg MIX generator library
###############################################################################################
cmake_minimum_required(VERSION 3.5.0)

project(is-ros1-mix-generator)

###################################################################################
# Configure options
###################################################################################
option(BUILD_LIBRARY "Compile the ROS 1 SystemHandle" ON)

if(NOT BUILD_LIBRARY)
    return()
endif()

# Get dependencies
find_package(is-core REQUIRED)
find_package(catkin REQUIRED COMPONENTS genmsg)
find_package(is-ros1 REQUIRED)

list(APPEND MIX_ROS_PACKAGES_LIST std_msgs)

if (MIX_ROS_PACKAGES)

    string(REPLACE " " ";" MIX_ROS_PACKAGES_ ${MIX_ROS_PACKAGES})

    foreach(PKG ${MIX_ROS_PACKAGES_})
        find_package(${PKG} REQUIRED)

        list(APPEND MIX_ROS_PACKAGES_LIST ${PKG})
    endforeach()

endif()

if (MIX_ROS1_PACKAGES)

    string(REPLACE " " ";" MIX_ROS1_PACKAGES_ ${MIX_ROS1_PACKAGES})

    foreach(PKG ${MIX_ROS1_PACKAGES_})
        find_package(${PKG} REQUIRED)

        list(APPEND MIX_ROS_PACKAGES_LIST ${PKG})
    endforeach()

endif()

if (BUILD_TESTS OR BUILD_ROS1_TESTS)

    find_package(geometry_msgs REQUIRED)
    find_package(nav_msgs REQUIRED)

    list(APPEND MIX_ROS_PACKAGES_LIST geometry_msgs nav_msgs)

endif()

###################################################################################
# Install the Integration Service is-ros1-genmsg-mix plugin
###################################################################################
if(BUILD_LIBRARY)
    is_mix_install_extension(
        IDL_TYPE
            genmsg
        MIDDLEWARE
            ros1
        MESSAGE
            SOURCES "${CMAKE_CURRENT_LIST_DIR}/resources/convert__msg.cpp.em"
            HEADERS "${CMAKE_CURRENT_LIST_DIR}/resources/convert__msg.hpp.em"
        SERVICE
            SOURCES "${CMAKE_CURRENT_LIST_DIR}/resources/convert__srv.cpp.em"
    )
endif()

include(cmake/is_ros1_genmsg_mix.cmake)

is_ros1_genmsg_mix(
    PACKAGES ${MIX_ROS_PACKAGES_LIST}
    MIDDLEWARES ros1
)

# Install Config.cmake file
install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Config.cmake
    DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
