/*
 * Copyright (C) 2018 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/


#ifndef SOSS__ROS1__UTILITIES_HPP
#define SOSS__ROS1__UTILITIES_HPP

#include <soss/utilities.hpp>

#include <ros/time.h>
#include <ros/duration.h>

#include <boost/array.hpp>

namespace soss {
namespace ros1 {
// These message types are considered built-into ROS1, so there's no .msg
// specification file for automatically generating them from primitives.
// Therefore, we will manually write the conversion utilities for these types.

// NOTE: We name the Time::nsec field "nanosec" instead of "nsec" so that it's
// compatible with ROS2. We also make the type for "sec" be "int32_t" so that
// it's compatible with ROS2.


namespace convert__msg__Timebase {

//==============================================================================
inline const xtypes::StructType type(const std::string& name)
{
  xtypes::StructType type(name);
  type.add_member("sec", xtypes::primitive_type<int32_t>());
  type.add_member("nanosec", xtypes::primitive_type<uint32_t>());
  return type;
}
} // namespace convert__msg__Timebase

namespace convert__msg__Time {

//==============================================================================
inline void convert_to_ros1(const xtypes::ReadableDynamicDataRef& from, ros::Time& to)
{
  int32_t to_sec;
  uint32_t to_nsec;
  soss::Convert<int32_t>::from_xtype_field(from["sec"], to_sec);
  soss::Convert<uint32_t>::from_xtype_field(from["nanosec"], to_nsec);
  to.nsec = to_nsec;
  to.sec = static_cast<uint32_t>(to_sec);
}

//==============================================================================
inline void convert_to_xtype(const ros::Time& from, xtypes::WritableDynamicDataRef to)
{
  soss::Convert<int32_t>::to_xtype_field(from.sec, to["sec"]);
  soss::Convert<uint32_t>::to_xtype_field(static_cast<uint32_t>(from.nsec), to["nanosec"]);
}

} // namespace convert__msg__Time

namespace convert__msg__Duration {

//==============================================================================
inline void convert_to_ros1(const xtypes::ReadableDynamicDataRef& from, ros::Duration& to)
{
  int32_t to_sec;
  uint32_t to_nsec;
  soss:Convert<int32_t>::from_xtype_field(from["sec"], to_sec);
  soss::Convert<uint32_t>::from_xtype_field(from["nanosec"], to_nsec);
  to.sec = to_sec;
  to.nsec = static_cast<int32_t>(to_nsec);
}

//==============================================================================
inline void convert_to_xtype(const ros::Duration&from, xtypes::WritableDynamicDataRef to)
{
  int32_t sec = from.sec;
  uint32_t nanosec;
  if(from.nsec < 0)
  {
    // We're assuming the size of nsec is always between -1e9 and 1e9
    --sec;
    nanosec = static_cast<uint32_t>(static_cast<int32_t>(1e9) + from.nsec);
  }
  else
  {
    nanosec = static_cast<uint32_t>(from.nsec);
  }
  soss::Convert<int32_t>::to_xtype_field(sec, to["sec"]);
  soss::Convert<uint32_t>::to_xtype_field(nanosec, to["nanosec"]);
}

} // namespace convert__msg__Duration

} // namespace ros1

template<>
struct Convert<ros::Time>
    : MessageConvert<
     ros::Time,
    &ros1::convert__msg__Time::convert_to_ros1,
    &ros1::convert__msg__Time::convert_to_xtype
    > { };

template<>
struct Convert<ros::Duration>
    : MessageConvert<
     ros::Duration,
    &ros1::convert__msg__Duration::convert_to_ros1,
    &ros1::convert__msg__Duration::convert_to_xtype
    > { };

} // namespace soss


#endif // SOSS__ROS1__UTILITIES_HPP
