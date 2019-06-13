cmake_minimum_required(VERSION 3.5.0)

project(soss-ros1)

find_package(catkin QUIET)

if(NOT catkin_FOUND)

  message("Could not find package [catkin] -- we will skip configuring [soss-ros1]")
  return()

endif()

find_package(soss-core REQUIRED)
find_package(soss-genmsg REQUIRED)

if(NOT CMAKE_CXX_STANDARD)
  # TODO(MXG): Remove this block and use target_compile_features(~)
  # instead when we no longer need to support Ubuntu 16.04.
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

message(STATUS "Configuring [soss-ros1]")

include(GNUInstallDirs)

find_package(catkin REQUIRED COMPONENTS roscpp)

add_library(soss-ros1 SHARED
  src/Factory.cpp
  src/SystemHandle.cpp
  src/MetaPublisher.cpp
)

target_link_libraries(soss-ros1
  PUBLIC
    soss::core
    ${catkin_LIBRARIES}
)

target_include_directories(soss-ros1
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    ${catkin_INCLUDE_DIRS}
)

###############################
# Install soss-ros1
soss_install_middleware_plugin(
  MIDDLEWARE ros1
  TARGET soss-ros1
)

install(
  DIRECTORY   ${CMAKE_CURRENT_LIST_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT soss-ros1
)


###############################
# Install soss-genmsg-ros1-mix
soss_genmsg_install_extension(
  MIDDLEWARE ros1
  MESSAGE
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/resources/convert__msg.cpp.em"
    HEADERS "${CMAKE_CURRENT_LIST_DIR}/resources/convert__msg.hpp.em"
  SERVICE
    SOURCES "${CMAKE_CURRENT_LIST_DIR}/resources/convert__srv.cpp.em"
)
