cmake_minimum_required(VERSION 3.5.0)

project(soss-ros1-test)

# Get soss dependencies
find_package(soss-core REQUIRED)
find_package(soss-mock REQUIRED)
find_package(soss-genmsg REQUIRED)
find_package(soss-ros1 REQUIRED)

# Get message dependencies
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)

soss_genmsg_mix(
  PACKAGES geometry_msgs nav_msgs
  MIDDLEWARES ros1
)

add_executable(soss-ros1-test
  main.cpp
  integration/ros1__geometry_msgs.cpp
)

set(thirdparty_dir "${CMAKE_CURRENT_LIST_DIR}/../../thirdparty")

list(APPEND CMAKE_MODULE_PATH "${thirdparty_dir}/catch2/cmake")

include(CTest)
include(Catch)
catch_discover_tests(soss-ros1-test)

target_link_libraries(soss-ros1-test
  PRIVATE
    soss::mock
    soss::ros1
)

target_include_directories(soss-ros1-test
  PRIVATE
    "${thirdparty_dir}/catch2/include"
    ${geometry_msgs_INCLUDE_DIRS}
    ${nav_msgs_INCLUDE_DIRS}
)

target_compile_definitions(soss-ros1-test
  PRIVATE
    "ROS1__GEOMETRY_MSGS__TEST_CONFIG=\"${CMAKE_CURRENT_LIST_DIR}/resources/ros1__geometry_msgs.yaml\""
    "ROS1__GENMSG__BUILD_DIR=\"${CMAKE_BINARY_DIR}/soss/genmsg/ros1/lib\""
)
