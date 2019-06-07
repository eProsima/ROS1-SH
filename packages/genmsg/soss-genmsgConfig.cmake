# - Config file for the soss-genmsg utility for generating soss translations
#   from ROS1/ROS2 message specifications

cmake_minimum_required(VERSION 3.5.1 FATAL_ERROR)

if(soss-genmsg_CONFIG_INCLUDED)
  return()
endif()
set(soss-genmsg_CONFIG_INCLUDED TRUE)

set(SOSS_GENMSG_GENERATE_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/scripts/soss_genmsg_generate.py")
set(SOSS_GENMSG_FIND_PACKAGE_INFO_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/scripts/soss_genmsg_find_package_info.py")

include("${CMAKE_CURRENT_LIST_DIR}/cmake/soss_genmsg_mix.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/soss_genmsg_install_extension.cmake")
