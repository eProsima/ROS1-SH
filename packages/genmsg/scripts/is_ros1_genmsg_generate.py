#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from __future__ import print_function

try:
    from genmsg.msg_loader import load_msg_from_file, load_srv_from_file, MsgContext
    from genmsg.msgs import is_header_type
except ImportError:
    print('Unable to import ROS1 tools. Please source a ROS1 installation first.', end='', file=sys.stderr)
    sys.exit(1)

import argparse
import copy
import em
import os
import sys

if sys.version_info[0] > 2:
    from io import StringIO as BufferIO
else:
    from io import BytesIO as BufferIO


g_msg_context = MsgContext()


def get_type_components(full_type):
    if full_type == 'Header':
        return ('std_msgs', 'Header')

    components = full_type.split('/')
    if len(components) != 2:
        raise ValueError('Unexpected value for message type string: {}'.format(full_type))

    package = components[0]
    end = components[1].find('[')
    if end == -1:
      type = components[1]
    else:
      type = components[1][:end]

    return (package, type)


def parse_message_file(msg_file, msg_name):
    spec = load_msg_from_file(g_msg_context, msg_file, msg_name)
    if is_header_type(spec.full_name):
        # The std_msgs/Header of ROS2 does not contain the `seq` member field
        # that the ROS1 version has. The ROS2 definition should be treated as
        # the canonical one, so we'll remove the `seq` field from this message
        # specification. We make a special case for this because std_msgs/Header
        # usage is so prevalent in downstream message packages.
        seq_index = spec.names.index('seq')
        del spec.names[seq_index]
        del spec.types[seq_index]
        del spec._parsed_fields[seq_index]
    return spec


def parse_service_file(srv_file, srv_name):
    return load_srv_from_file(g_msg_context, srv_file, srv_name)


def generate_file(template, destination, context):

    base_name = template.split('/')[-1]
    base_name_components = base_name.split('.')

    # Remove the last base name component if it's .em
    if base_name_components[-1] == 'em':
        base_name_components.pop(-1)

    package_name, type_name = get_type_components(context['spec'].full_name)
    context['package'] = package_name
    context['type'] = type_name

    # Add the message type name to the source file
    base_name_components[0] = base_name_components[0] + '__' + type_name
    filename = '.'.join(base_name_components)
    output_file_path = '/'.join([destination, filename])

    output_buffer = BufferIO()
    interpreter = em.Interpreter(output=output_buffer, globals=copy.deepcopy(context))
    interpreter.file(open(template, 'r'))

    if not os.path.exists(destination):
        os.makedirs(destination)

    with open(output_file_path, 'w') as file:
        file.write(output_buffer.getvalue())


def generate_files(package, source_dir, header_dir, idl_files, cpp_files, hpp_files, prefix, parse_fnc):

    for idl_file in idl_files:

        name = package + '/' + idl_file.split('/')[-1][:-4]

        context = {
            'spec': parse_fnc(idl_file, name),
            'subdir': prefix,
            'get_type_components': get_type_components
        }

        for cpp_file in cpp_files:
            generate_file(cpp_file, source_dir + '/' + prefix, context)

        for hpp_file in hpp_files:
            generate_file(hpp_file, header_dir + '/' + prefix, context)


def main(cli_args):
    parser = argparse.ArgumentParser(
        description='Generate .cpp and .hpp files for a set of messages and services given the idl files and the EmPy '
                    '(embedded python) templates for the source files.')

    parser.add_argument('--package', required=True, help='Package that the ROS1 idl files belong to')
    parser.add_argument('--source-dir', required=True, help='Output directory for source (.cpp) files')
    parser.add_argument('--header-dir', required=True, help='Output directory for header (.hpp) files')
    parser.add_argument('--msg-idl-files', nargs='*', required=True, help='IDL files for message specifications')
    parser.add_argument('--msg-cpp-files', nargs='*', required=True,
                        help='EmPy templates for .cpp files, each one will be applied to each message idl')
    parser.add_argument('--msg-hpp-files', nargs='*', required=True,
                        help='EmPy templates for .hpp files, each one will be applied to each message idl')
    parser.add_argument('--srv-idl-files', nargs='*', required=True, help='IDL files for service specifications')
    parser.add_argument('--srv-cpp-files', nargs='*', required=True,
                        help='EmPy templates for .cpp files, each one will be applied to each service idl')
    parser.add_argument('--srv-hpp-files', nargs='*', required=True,
                        help='EmPy templates for .hpp files, each one will be applied to each service idl')

    args = parser.parse_args(cli_args[1:])

    generate_files(args.package, args.source_dir, args.header_dir,
                   args.msg_idl_files, args.msg_cpp_files, args.msg_hpp_files,
                   'msg', parse_message_file)

    generate_files(args.package, args.source_dir, args.header_dir,
                   args.srv_idl_files, args.srv_cpp_files, args.srv_hpp_files,
                   'srv', parse_service_file)


if __name__ == '__main__':
    main(sys.argv)
