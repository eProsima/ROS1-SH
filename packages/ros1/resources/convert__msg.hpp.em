// generated from soss/cpp/ros1/resource/soss__ros1__message.hpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating soss/genmsg/ros1/<package>/src/msg/convert__msg__<msg>.hpp files
@#
@# Context:
@#  - spec (genmsg.MsgSpec)
@#    Parsed specification of the .msg file
@#  - subfolder (string)
@#    The subfolder / subnamespace of the message
@#    Either 'msg' or 'srv'
@#  - get_type_components(field) (function)
@#    Produces (package, type) of field
@#  - package (string)
@#    The name of the package
@#  - type (string)
@#    The name of the message type (not including the package name prefix)
@#######################################################################

@{
cpp_msg_type = '{}::{}'.format(package, type)

msg_type_string = '{}/{}'.format(package, type)

header_guard_parts = [
    'SOSS__GENMSG__ROS1', package, 'MSG__CONVERT', type + '_HPP']
header_guard_variable = '__'.join([x.upper() for x in header_guard_parts])

namespace_parts = ['convert', package, 'msg', type]
namespace_variable = '__'.join(namespace_parts)

ros2_msg_dependency = '{}/{}.h'.format(package, type)

conversion_dependencies = {}
for field in spec.parsed_fields():
    if field.is_builtin:
        continue

    field_package, field_type = get_type_components(field.type)
    key = 'soss/genmsg/ros1/{}/msg/convert__msg__{}.hpp'.format(field_package, field_type)
    if key not in conversion_dependencies:
        conversion_dependencies[key] = set([])
    conversion_dependencies[key].add(field.name)

alphabetical_fields = sorted(spec.parsed_fields(), key=lambda x: x.name)
}@
#ifndef @(header_guard_variable)
#define @(header_guard_variable)

// Include the header for the generic soss message type
#include <soss/ros1/utilities.hpp>

// Include the header for the concrete ros1 messagetype
#include <@(ros2_msg_dependency)>

// Include the headers for the soss message conversion dependencies
@[if conversion_dependencies.keys()]@
@[    for key in sorted(conversion_dependencies.keys())]@
#include <@(key)> // @(', '.join(conversion_dependencies[key]))
@[    end for]@
@[else]@
// <none>
@[end if]@

namespace soss {
namespace ros1 {
namespace @(namespace_variable) {

using Ros1_Msg = @(cpp_msg_type);
using Ros1_MsgPtr = @(cpp_msg_type)Ptr;
const std::string g_msg_name = "@(msg_type_string)";

//==============================================================================
inline soss::Message initialize()
{
  soss::Message msg;
  msg.type = g_msg_name;
@[for field in alphabetical_fields]@
  soss::Convert<Ros1_Msg::_@(field.name)_type>::add_field(msg, "@(field.name)");
@[end for]@

  return msg;
}

//==============================================================================
inline void convert_to_ros1(const soss::Message& from, Ros1_Msg& to)
{
  auto from_field = from.data.begin();
@[for field in alphabetical_fields]@
  soss::Convert<Ros1_Msg::_@(field.name)_type>::from_soss_field(from_field++, to.@(field.name));
@[end for]@

  // Suppress possible unused variable warnings
  (void)from;
  (void)to;
  (void)from_field;
}

//==============================================================================
inline void convert_to_soss(const Ros1_Msg& from, soss::Message& to)
{
  auto to_field = to.data.begin();
@[for field in alphabetical_fields]@
  soss::Convert<Ros1_Msg::_@(field.name)_type>::to_soss_field(from.@(field.name), to_field++);
@[end for]@

  // Suppress possible unused variable warnings
  (void)from;
  (void)to;
  (void)to_field;
}

} // namespace @(namespace_variable)
} // namespace ros1

template<>
struct Convert<ros1::@(namespace_variable)::Ros1_Msg>
    : MessageConvert<
      ros1::@(namespace_variable)::Ros1_Msg,
     &ros1::@(namespace_variable)::initialize,
     &ros1::@(namespace_variable)::convert_to_ros1,
     &ros1::@(namespace_variable)::convert_to_soss
     > { };

} // namespace soss

#endif // @(header_guard_variable)
