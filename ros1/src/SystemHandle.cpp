/*
 * Copyright (C) 2019 Open Source Robotics Foundation
 * Copyright (C) 2020 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "SystemHandle.hpp"
#include "MetaPublisher.hpp"

#include <is/sh/ros1/Factory.hpp>

#include <is/core/runtime/MiddlewareInterfaceExtension.hpp>
#include <is/core/runtime/Search.hpp>

#include <ros/this_node.h>

namespace eprosima {
namespace is {
namespace sh {
namespace ros1 {

//==============================================================================
void SystemHandle::print_missing_mix_file(
        const std::string& msg_or_srv,
        const std::string& type,
        const std::vector<std::string>& checked_paths)
{
    _logger << utils::Logger::Level::ERROR
            << "Could not find .mix file for " << msg_or_srv << " type: '"
            << type << "'.\n -- Make sure that you have generated "
            << "the 'is-ros1' extension for that message type by calling "
            << "'is_ros1_genmsg_mix(PACKAGES <package> MIDDLEWARES ros1)' "
            << "in your build system!" << std::endl;

    _logger << utils::Logger::Level::DEBUG
            << " -- Checked locations (these files did not exist or could not be accessed):\n";

    for (const std::string& checked_path : checked_paths)
    {
        _logger << "\t- " << checked_path << "\n";
    }
    _logger << std::endl;
}

//==============================================================================
SystemHandle::SystemHandle()
    : _logger("is::sh::ROS1")
{
}

//==============================================================================
bool SystemHandle::configure(
        const core::RequiredTypes& types,
        const YAML::Node& configuration,
        TypeRegistry& type_registry)
{
    int argc = 1;
    char* argv[1];
    char sh_name[8] = "is_ros1";
    argv[0] = sh_name;
    bool success = true;

    const YAML::Node yaml_node_name = configuration["node_name"];
    const std::string node_name = yaml_node_name
            ? yaml_node_name.as<std::string>()
            : std::string("is_ros1_node_") + std::to_string(rand());

    // Init ROS 1 context and create SystemHandle node.
    ros::init(argc, argv, std::move(node_name));

    _node = std::make_unique<ros::NodeHandle>();

    if (_node)
    {
        _logger << utils::Logger::Level::INFO
                << "Created node '" << node_name << "'" << std::endl;
    }
    else
    {
        _logger << utils::Logger::Level::ERROR
                << "Failed to create node '" << node_name << "'" << std::endl;

        return false;
    }

    auto register_type = [&](const std::string& type_name) -> bool
            {
                xtypes::DynamicType::Ptr type = Factory::instance().create_type(type_name);
                if (type.get() == nullptr)
                {
                    _logger << utils::Logger::Level::ERROR
                            << "Failed to register the required DynamicType '"
                            << type_name << "'" << std::endl;

                    return false;
                }
                else
                {
                    _logger << utils::Logger::Level::DEBUG
                            << "Registered the required DynamicType '"
                            << type_name << "'" << std::endl;

                    type_registry.emplace(type_name, std::move(type));
                    return true;
                }
            };

    // Add topic types to the TypeRegistry map, if present in the TypeFactory.
    core::Search search("ros1");
    for (const std::string& type : types.messages)
    {
        std::vector<std::string> checked_paths;
        const std::string msg_mix_path =
                search.find_message_mix(type, &checked_paths);

        if (msg_mix_path.empty())
        {
            print_missing_mix_file("message", type, checked_paths);
            success = false;
            continue;
        }

        if (!core::Mix::from_file(msg_mix_path).load())
        {
            _logger << utils::Logger::Level::ERROR
                    << "Failed to load extension for message type '"
                    << type << "' using mix file: " << msg_mix_path << std::endl;

            success = false;
            continue;
        }
        else
        {
            _logger << utils::Logger::Level::DEBUG
                    << "Loaded middleware interface extension for message type '"
                    << type << "' using mix file: " << msg_mix_path << std::endl;

            success &= register_type(type);
        }
    }

    // Add service types to the TypeRegistry map, if present in the TypeFactory.
    for (const std::string& type : types.services)
    {
        const std::string library_name = type.substr(0, type.find(":"));
        std::vector<std::string> checked_paths;

        const std::string srv_mix_path =
                search.find_service_mix(library_name, &checked_paths);

        if (srv_mix_path.empty())
        {
            print_missing_mix_file("service", library_name, checked_paths);
            success = false;
            continue;
        }

        if (!core::Mix::from_file(srv_mix_path).load())
        {
            _logger << utils::Logger::Level::ERROR
                    << "Failed to load extension for service type '"
                    << type << "' using mix file: " << srv_mix_path << std::endl;

            success = false;
            continue;
        }
        else
        {
            _logger << utils::Logger::Level::DEBUG
                    << "Loaded middleware interface extension for service type '"
                    << type << "' using mix file: " << srv_mix_path << std::endl;

            success &= register_type(type);
        }
    }

    return success;
}

//==============================================================================
bool SystemHandle::okay() const
{
    if (_node)
    {
        return ros::ok();
    }

    return false;
}

//==============================================================================
bool SystemHandle::spin_once()
{
    ros::Rate rate(10);
    ros::spinOnce();
    rate.sleep();

    return ros::ok();
}

//==============================================================================
SystemHandle::~SystemHandle()
{
    _subscriptions.clear();
    _client_proxies.clear();

    _node->shutdown();
    _node.reset();

    ros::shutdown();
}

//==============================================================================
bool SystemHandle::subscribe(
        const std::string& topic_name,
        const xtypes::DynamicType& message_type,
        SubscriptionCallback* callback,
        const YAML::Node& configuration)
{
    int queue_size = configuration["queue_size"].as<int>(default_queue_size);
    // TODO(MXG): Parse configuration so users can change transport hints
    auto subscription = Factory::instance().create_subscription(
        message_type, *_node, topic_name,
        callback, queue_size, ros::TransportHints());

    if (!subscription)
    {
        _logger << utils::Logger::Level::ERROR
                << "Failed to create subscription for topic '" << topic_name
                << "' with type '" << message_type.name() << "' on node '"
                << ros::this_node::getName() << "'"
                << ". The requested subscription has not been registered within the "
                << "subscription factory!" << std::endl;
        return false;
    }
    else
    {
        _subscriptions.emplace_back(std::move(subscription));

        _logger << utils::Logger::Level::INFO
                << "Created subscription for topic '" << topic_name << "' with type '"
                << message_type.name() << "' on node '" << ros::this_node::getName()
                << "'" << std::endl;

        return true;
    }
}

//==============================================================================
bool SystemHandle::is_internal_message(
        void* /*filter_handle*/)
{
    // Always return false, since this should be handled by the ROS 1 SubscriptionCallback
    // signature with the MessageEvent metadata about the received message instance.
    return false;
}

//==============================================================================
std::shared_ptr<TopicPublisher> SystemHandle::advertise(
        const std::string& topic_name,
        const xtypes::DynamicType& message_type,
        const YAML::Node& configuration)
{
    std::shared_ptr<TopicPublisher> publisher;

    int queue_size = configuration["queue_size"].as<int>(default_queue_size);
    bool latch_behavior = configuration["latch"].as<bool>(default_latch_behavior);

    if (topic_name.find('{') != std::string::npos)
    {
        // If the topic name contains a curly brace, we must assume that it needs
        // runtime substitutions.
        publisher = make_meta_publisher(
            message_type, *_node, topic_name,
            queue_size, latch_behavior,
            configuration);
    }
    else
    {
        publisher = Factory::instance().create_publisher(
            message_type, *_node, topic_name,
            queue_size, latch_behavior);
    }

    if (nullptr != publisher)
    {
        _logger << utils::Logger::Level::INFO
                << "Created publisher for topic '" << topic_name << "' with type '"
                << message_type.name() << "' on node '" << ros::this_node::getName()
                << "'" << std::endl;
    }
    else
    {
        _logger << utils::Logger::Level::ERROR
                << "Failed to create publisher for topic '" << topic_name
                << "' with type '" << message_type.name() << "' on node '"
                << ros::this_node::getName()
                << "'. The requested publisher has not been registered "
                << "within the publisher factory!" << std::endl;
    }

    return publisher;
}

//==============================================================================
bool SystemHandle::create_client_proxy(
        const std::string& service_name,
        const xtypes::DynamicType& service_type,
        RequestCallback* callback,
        const YAML::Node& /*configuration*/)
{
    auto client_proxy = Factory::instance().create_client_proxy(
        service_type.name(), *_node, service_name, callback);

    if (!client_proxy)
    {
        _logger << utils::Logger::Level::ERROR
                << "Failed to create service client for service '" << service_name
                << "' with type '" << service_type.name() << "' on node '"
                << ros::this_node::getName()
                << "'. The requested service client has not been registered "
                << "within the service client factory!" << std::endl;

        return false;
    }
    else
    {
        _logger << utils::Logger::Level::INFO
                << "Created service client for service '" << service_name
                << "' with type '" << service_type.name() << "' on node '"
                << ros::this_node::getName() << "'" << std::endl;

        _client_proxies.emplace_back(std::move(client_proxy));
        return true;
    }
}

//==============================================================================
std::shared_ptr<ServiceProvider> SystemHandle::create_service_proxy(
        const std::string& service_name,
        const xtypes::DynamicType& service_type,
        const YAML::Node& /*configuration*/)
{
    auto server_proxy = Factory::instance().create_server_proxy(
        service_type.name(), *_node, service_name);

    if (!server_proxy)
    {
        _logger << utils::Logger::Level::ERROR
                << "Failed to create service server for service '" << service_name
                << "' with type '" << service_type.name() << "' on node '"
                << ros::this_node::getName()
                << "'. The requested service server has not been registered "
                << "within the service server factory!" << std::endl;
    }
    else
    {
        _logger << utils::Logger::Level::INFO
                << "Created service server for service '" << service_name
                << "' with type '" << service_type.name() << "' on node '"
                << ros::this_node::getName() << "'" << std::endl;
    }

    return server_proxy;
}

} //  namespace ros1
} //  namespace sh
} //  namespace is
} //  namespace eprosima

//==============================================================================
IS_REGISTER_SYSTEM("ros1", eprosima::is::sh::ros1::SystemHandle)
