cmake_minimum_required(VERSION 3.5.0)

project(soss-genmsg)

find_package(soss-core REQUIRED)
find_package(catkin QUIET COMPONENTS genmsg)

if(NOT genmsg_FOUND)

  message(FATAL_ERROR
    "Could not find package [genmsg]. Without this, it will not be possible "
    "to generate soss translations for ROS1 message types. "
    "Please source a distribution of ROS1 to build soss-genmsg."
  )

endif()

include(GNUInstallDirs)

set(soss_genmsg_share_dir "${CMAKE_INSTALL_DATADIR}/soss-genmsg")

install(
  FILES "${CMAKE_CURRENT_LIST_DIR}/soss-genmsgConfig.cmake"
  DESTINATION "${soss_genmsg_share_dir}"
  COMPONENT soss-genmsg
)

install(
  DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/cmake"
  DESTINATION "${soss_genmsg_share_dir}"
  COMPONENT soss-genmsg
)

install(
  DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/scripts"
  DESTINATION "${soss_genmsg_share_dir}"
  COMPONENT soss-genmsg
)
