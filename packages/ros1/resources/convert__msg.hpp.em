// generated from is-ros1/resources/convert__msg.hpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating is/genmsg/ros1/<package>/src/msg/convert__msg__<msg>.hpp files
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
BUILTIN_TYPES = {
    'bool'    : 'bool',
    'byte'    : 'int8_t',
    'char'    : 'char',
    'wchar'   : 'wchar_t',
    'float32' : 'float',
    'float64' : 'double',
    'float128': 'long double',
    'int8'    : 'int8_t',
    'uint8'   : 'uint8_t',
    'int16'   : 'int16_t',
    'uint16'  : 'uint16_t',
    'int32'   : 'int32_t',
    'uint32'  : 'uint32_t',
    'int64'   : 'int64_t',
    'uint64'  : 'uint64_t'
}

cpp_msg_type = '{}::{}'.format(package, type)

msg_type_string = '{}/{}'.format(package, type)

header_guard_parts = [
    'IS__GENMSG__ROS1', package, 'MSG__CONVERT', type + '_HPP']
header_guard_variable = '__'.join([x.upper() for x in header_guard_parts])

namespace_parts = ['convert', package, 'msg', type]
namespace_variable = '__'.join(namespace_parts)

ros1_msg_dependency = '{}/{}.h'.format(package, type)

conversion_dependencies = {}
for field in spec.parsed_fields():
    if field.is_builtin:
        continue

    field_package, field_type = get_type_components(field.type)
    key = 'is/genmsg/ros1/{}/msg/convert__msg__{}.hpp'.format(field_package, field_type)
    if key not in conversion_dependencies:
        conversion_dependencies[key] = set([])
    conversion_dependencies[key].add(field.name)
}@
#ifndef @(header_guard_variable)
#define @(header_guard_variable)

// Include the header for the generic message type
#include <is/core/Message.hpp>

// Include the header for the conversions
#include <is/sh/ros1/utilities.hpp>

// Include the header for the concrete ros1 messagetype
#include <@(ros1_msg_dependency)>

// Include the headers for the message conversion dependencies
@[if conversion_dependencies.keys()]@
@[    for key in sorted(conversion_dependencies.keys())]@
#include <@(key)> // @(', '.join(conversion_dependencies[key]))
@[    end for]@
@[else]@
// <none>
@[end if]@

namespace eprosima {
namespace is {
namespace sh {
namespace ros1 {
namespace @(namespace_variable) {

using Ros1_Msg = @(cpp_msg_type);
using Ros1_MsgPtr = @(cpp_msg_type)Ptr;
const std::string g_msg_name = "@(msg_type_string)";


//==============================================================================
inline const xtypes::StructType type()
{
    xtypes::StructType type(g_msg_name);
@[for field in spec.parsed_fields()]@
@[    if field.base_type in BUILTIN_TYPES.keys()]@
    const xtypes::DynamicType& derived_type_@(field.name) = xtypes::primitive_type<@(BUILTIN_TYPES[field.base_type])>();
@[    else]@
@[        if 'string' in field.base_type]@
    const xtypes::StringType derived_type_@(field.name) = xtypes::StringType();
@[        elif field.base_type in ['duration', 'time']]@
    const xtypes::StructType derived_type_@(field.name) ( // Special "@(field.base_type)" ROS1 built-in type
        is::sh::ros1::convert__msg__Timebase::type("@(field.name)"));
@[        else]@
    const xtypes::StructType derived_type_@(field.name) (
        is::sh::ros1::convert__@(field.base_type[:field.base_type.find('/')])__msg__@(field.base_type[field.base_type.find('/')+1:])::type());
@[        end if]@
@[    end if]@
@[    if field.is_array]@
@[        if field.array_len]@
    type.add_member("@(field.name)", xtypes::SequenceType(std::move(derived_type_@(field.name)), @(field.array_len)));
@[        else]@
    type.add_member("@(field.name)", xtypes::SequenceType(std::move(derived_type_@(field.name))));
@[        end if]@
@[    else]@
    type.add_member("@(field.name)", std::move(derived_type_@(field.name)));
@[    end if]@
@[end for]@
    return type;
}

//==============================================================================
inline void convert_to_ros1(const xtypes::ReadableDynamicDataRef& from, Ros1_Msg& to)
{
@[for field in spec.parsed_fields()]@
    is::utils::Convert<Ros1_Msg::_@(field.name)_type>::from_xtype_field(from["@(field.name)"], to.@(field.name));
@[end for]@

    // Suppress possible unused variable warnings
    (void)from;
    (void)to;
}

//==============================================================================
inline void convert_to_xtype(const Ros1_Msg& from, xtypes::WritableDynamicDataRef to)
{
@[for field in spec.parsed_fields()]@
    is::utils::Convert<Ros1_Msg::_@(field.name)_type>::to_xtype_field(from.@(field.name), to["@(field.name)"]);
@[end for]@

    // Suppress possible unused variable warnings
    (void)from;
    (void)to;
}

} //  namespace @(namespace_variable)
} //  namespace ros1
} //  namespace sh

namespace utils {

template<>
struct Convert<is::sh::ros1::@(namespace_variable)::Ros1_Msg>
    : MessageConvert<
        is::sh::ros1::@(namespace_variable)::Ros1_Msg,
        &is::sh::ros1::@(namespace_variable)::convert_to_ros1,
        &is::sh::ros1::@(namespace_variable)::convert_to_xtype
        > { };

} //  namespace utils

} //  namespace is
} //  namespace eprosima

#endif // @(header_guard_variable)
