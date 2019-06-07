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

//==============================================================================
template<typename T, std::size_t N>
void vector_resize(boost::array<T, N>& /*array*/, std::size_t /*size*/)
{
  // Do nothing. Arrays don't need to be resized.
}

//==============================================================================
template<typename T, std::size_t N>
void vector_reserve(boost::array<T, N>& /*array*/, std::size_t /*size*/)
{
  // Do nothing. Arrays don't need to be resized.
}

//==============================================================================
/// \brief Convenience function for converting an array of soss::Messages into
/// an array of middleware-specific messages.
///
/// This is effectively a function, but we wrap it in a struct so that it can be
/// used as a template argument.
template<std::size_t N>
struct convert_array
{
  template<typename FromType, typename ToType,
           typename FromContainer, typename ToContainer>
  static void convert(
      const FromContainer& from,
      ToContainer& to,
      void(*convert)(const FromType& from, ToType& to),
      ToType(* /*initialize*/ )() = []() { return ToType(); })
  {
    // Note: we never need to use *initialize because converting an array will
    // never involved creating a new element.
    vector_resize(to, N);
    for(std::size_t i=0; i < N; ++i)
      (*convert)(from[i], to[i]);
  }
};

//==============================================================================
template<typename ElementType, std::size_t N>
struct Convert<boost::array<ElementType, N>>
    : ContainerConvert<
    ElementType,
    boost::array<typename Convert<ElementType>::native_type, N>,
    boost::array<typename Convert<ElementType>::soss_type, N>,
    convert_array<N>> { };

namespace ros1 {

// These message types are considered built-into ROS1, so there's no IDL
// specification file for automatically generating them from primitives.
// Therefore we will manually write the conversion utilities for these types.
namespace convert__msg__Time {

using Ros1_Msg = ros::Time;
using Ros1_MsgPtr = boost::shared_ptr<ros::Time>;
const std::string g_msg_name = "builtin_interfaces/Time";

// NOTE: We name the Time::nsec field "nanosec" instead of "nsec" so that it's
// compatible with ROS2. We also make the type for "sec" be "int32_t" so that
// it's compatible with ROS2.

//==============================================================================
inline soss::Message initialize()
{
  soss::Message msg;
  msg.type = g_msg_name;
  soss::Convert<uint32_t>::add_field(msg, "nanosec");
  soss::Convert<int32_t>::add_field(msg, "sec");

  return msg;
}

//==============================================================================
inline void convert_to_ros1(const soss::Message& from, ros::Time& to)
{
  auto from_field = from.data.begin();

  soss::Convert<uint32_t>::from_soss_field(from_field++, to.nsec);

  int32_t to_sec;
  soss::Convert<int32_t>::from_soss_field(from_field++, to_sec);
  to.sec = static_cast<uint32_t>(to_sec);
}

//==============================================================================
inline void convert_to_soss(const ros::Time& from, soss::Message& to)
{
  auto to_field = to.data.begin();

  soss::Convert<uint32_t>::to_soss_field(from.nsec, to_field++);
  soss::Convert<int32_t>::to_soss_field(static_cast<int32_t>(from.sec), to_field++);
}

} // namespace convert__msg__Time

namespace convert__msg__Duration {

using Ros1_Msg = ros::Duration;
using Ros1_MsgPtr = boost::shared_ptr<ros::Duration>;
const std::string g_msg_name = "builtin_interfaces/Duration";

// NOTE: We use uint32_t for nanosec so that it's compatible with ROS2.

//==============================================================================
inline soss::Message initialize()
{
  soss::Message msg;
  msg.type = g_msg_name;
  soss::Convert<uint32_t>::add_field(msg, "nanosec");
  soss::Convert<int32_t>::add_field(msg, "sec");
}

//==============================================================================
inline void convert_to_ros1(const soss::Message& from, ros::Duration& to)
{
  auto from_field = from.data.begin();

  uint32_t to_nsec;
  soss::Convert<uint32_t>::from_soss_field(from_field++, to_nsec);
  to.nsec = static_cast<int32_t>(to_nsec);

  soss::Convert<int32_t>::from_soss_field(from_field++, to.sec);
}

// NOTE: We need to do some calculations here since ros::Duration::nsec could
// have a negative value.

//==============================================================================
inline void convert_to_soss(const ros::Duration& from, soss::Message& to)
{
  int32_t sec = from.sec;
  uint32_t nanosec;
  if(from.nsec < 0)
  {
    // We're assuming the size of nsec is always between -1e9 and 1e9
    // TODO(MXG): Is that a safe assumption?
    --sec;
    nanosec = static_cast<uint32_t>(static_cast<int32_t>(1e9) + from.nsec);
  }
  else
  {
    nanosec = static_cast<uint32_t>(from.nsec);
  }

  auto to_field = to.data.begin();
  soss::Convert<uint32_t>::to_soss_field(static_cast<uint32_t>(nanosec), to_field++);
  soss::Convert<int32_t>::to_soss_field(sec, to_field++);
}

} // convert__msg__Duration

} // namespace ros1

template<>
struct Convert<ros::Time>
    : MessageConvert<
     ros::Time,
    &ros1::convert__msg__Time::initialize,
    &ros1::convert__msg__Time::convert_to_ros1,
    &ros1::convert__msg__Time::convert_to_soss
    > { };

template<>
struct Convert<ros::Duration>
    : MessageConvert<
     ros::Duration,
    &ros1::convert__msg__Duration::initialize,
    &ros1::convert__msg__Duration::convert_to_ros1,
    &ros1::convert__msg__Duration::convert_to_soss
    > { };

} // namespace soss


#endif // SOSS__ROS1__UTILITIES_HPP
