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
# is-ros1-genmsg generation code library for the Integration Service ROS1 SystemHandle project

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

project(is-ros1-genmsg)

find_package(is-core REQUIRED)
find_package(catkin QUIET COMPONENTS genmsg)

if(NOT genmsg_FOUND)

    message(FATAL_ERROR
        "Could not find package [genmsg]. Without this, it will not be possible "
        "to generate Integration Service translations for ROS1 message types. "
        "Please source a distribution of ROS1 to build ${PROJECT_NAME}."
  )

endif()

include(GNUInstallDirs)

set(${PROJECT_NAME}_share_dir "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")

install(
    FILES
        "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION
        "${${PROJECT_NAME}_share_dir}"
    COMPONENT
        ${PROJECT_NAME}
    )

install(
    DIRECTORY
        "${CMAKE_CURRENT_LIST_DIR}/cmake"
    DESTINATION
        "${${PROJECT_NAME}_share_dir}"
    COMPONENT
        ${PROJECT_NAME}
    )

install(
    DIRECTORY
        "${CMAKE_CURRENT_LIST_DIR}/scripts"
    DESTINATION
        "${${PROJECT_NAME}_share_dir}"
    COMPONENT
        ${PROJECT_NAME}
    )
