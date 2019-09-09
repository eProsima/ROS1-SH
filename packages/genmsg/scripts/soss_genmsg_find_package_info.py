#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from __future__ import print_function

import sys

try:
    from rospkg.rospack import ManifestManager
    from rospkg.common import PACKAGE_FILE
    from genmsg.msg_loader import load_msg_from_file, load_srv_from_file, MsgContext
except ImportError:
    print('Unable to import ROS1 tools. Please source a ROS1 installation first.', end='', file=sys.stderr)
    sys.exit(1)

import argparse
import os


g_manifest = ManifestManager(PACKAGE_FILE)
g_manifest._update_location_cache()

g_msg_context = MsgContext()

class PackageInfo:

    def __init__(self, name):
        self.pkg_name = name
        self.msg_files = []
        self.srv_files = []
        self.dependencies = []


def append_package_name(field, output_list):
    if field.is_header:
        output_list.append('std_msgs')
        return

    components = field.type.split('/')
    if len(components) > 1:
        output_list.append(components[0])


def append_msg_dependencies(msg, output_list):
    for field in msg.parsed_fields():
        if field.is_builtin:
            continue

        append_package_name(field, output_list)


def find_package_info(requested_pkg_name):
    info = PackageInfo(requested_pkg_name)

    share_dir = g_manifest.get_path(requested_pkg_name)

    message_dir = '{}/msg'.format(share_dir)
    if os.path.exists(message_dir):
        for relative_msg_file in os.listdir(message_dir):
            if not relative_msg_file.endswith('.msg'):
                continue

            msg_file = '{}/{}'.format(message_dir, relative_msg_file)

            # Strip off the extension to get the local name of the message type
            msg_subname = relative_msg_file[:-4]

            info.msg_files.append(msg_file)
            msg = load_msg_from_file(g_msg_context, msg_file, '{}/{}'.format(requested_pkg_name, msg_subname))
            append_msg_dependencies(msg, info.dependencies)

    service_dir = '{}/srv'.format(share_dir)
    if os.path.exists(service_dir):
        for relative_srv_file in os.listdir(service_dir):
            if not relative_srv_file.endswith('.srv'):
                continue

            srv_file = '{}/{}'.format(service_dir, relative_srv_file)

            # Strip off the extension to get the local name of the service type
            srv_subname = relative_srv_file[:-4]

            info.srv_files.append(srv_file)
            srv = load_srv_from_file(g_msg_context, srv_file, '{}/{}'.format(requested_pkg_name, srv_subname))
            for component in [srv.request, srv.response]:
                append_msg_dependencies(component, info.dependencies)

    return info


def traverse_packages(root_pkg_name):
    package_queue = [root_pkg_name]
    inspected_packages = {}

    while package_queue:
        next_pkg = package_queue.pop()
        if next_pkg in inspected_packages:
            continue

        info = find_package_info(next_pkg)
        inspected_packages[next_pkg] = info
        for dependency in info.dependencies:
            package_queue.append(dependency)

    return inspected_packages


def print_package_info(root_pkg_name, pkg_info_dict):
    dependency_list = set(pkg_info_dict.keys())
    dependency_list.remove(root_pkg_name)
    dependency_list_str = '#'.join(dependency_list)

    message_files = pkg_info_dict[root_pkg_name].msg_files
    message_files_str = '#'.join(message_files)

    service_files = pkg_info_dict[root_pkg_name].srv_files
    service_files_str = '#'.join(service_files)

    file_dependencies = []
    for pkg, info in pkg_info_dict.items():
        file_dependencies.extend(info.msg_files)
        if pkg == root_pkg_name:
            file_dependencies.extend(info.srv_files)

    file_dependencies_str = '#'.join(file_dependencies)

    output_str = ';'.join([dependency_list_str, message_files_str, service_files_str, file_dependencies_str])
    print(output_str)


def main(cli_args):
    parser = argparse.ArgumentParser(
      description='Find dependencies for a message package.\n'
                  'First line of stdout contains a semi-colon separated list of package names.\n'
                  'Second line of stdout contains a semi-colon separated list of message file locations.')
    parser.add_argument('package', help='The packages whose dependencies should be searched for.')
    args = parser.parse_args(cli_args[1:])

    root_package_name = args.package
    print_package_info(root_package_name, traverse_packages(root_package_name))


if __name__ == '__main__':
    main(sys.argv)
