/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include <soss/ros1/Factory.hpp>

#include <soss/Mix.hpp>
#include <soss/Search.hpp>

namespace soss {
namespace ros1 {

//==============================================================================
SystemHandle::SystemHandle()
{
  // Do nothing
}

//==============================================================================
// TODO(MXG): Consider refactoring this function so that we only need one
// implementation between the ros1 and ros2 plugins
void print_missing_mix_file(
    const std::string& msg_or_srv,
    const std::string& type,
    const std::vector<std::string>& /*checked_paths*/)
{
  std::string error = "soss-ros1 could not find .mix file for " + msg_or_srv
      + " type: " + type +
      "\n -- Make sure that you have generated the soss-ros1 extension for "
      "that message type by calling "
      "soss_rosidl_mix(PACKAGES <package> MIDDLEWARES ros1) "
      "in your build system!\n";

//  // TODO(MXG): Introduce a way for users to request a "debug", especially from
//  // the command line. When debug mode is active, this should be printed out.
//  error += " -- Checked locations (these files did not exist or could not be accessed):\n";
//  for(const auto& p : checked_paths)
//    error += "     - " + p + "\n";

  std::cerr << error;
}

//==============================================================================
bool SystemHandle::configure(
    const RequiredTypes& types,
    const YAML::Node& configuration)
{
  int argc = 1;
  char* argv[1];
  char buffer[5] = "soss";
  argv[0] = buffer;

  const YAML::Node yaml_node_name = configuration["node_name"];
  std::string node_name = yaml_node_name?
        yaml_node_name.as<std::string>() : std::string("soss_ros1");
  ros::init(argc, argv, std::move(node_name));

  _node = std::make_unique<ros::NodeHandle>();

  bool success = true;

  soss::Search search("ros1");
  for(const std::string& type : types.messages)
  {
    std::vector<std::string> checked_paths;
    const std::string msg_mix_path =
        search.find_message_mix(type, &checked_paths);

    if(msg_mix_path.empty())
    {
      print_missing_mix_file("message", type, checked_paths);
      success = false;
      continue;
    }

    if(!Mix::from_file(msg_mix_path).load())
    {
      std::cerr << "soss-ros1 failed to load extension for message type ["
                << type << "] using mix file: " << msg_mix_path << std::endl;
      success = false;
    }
  }

  for(const std::string& type : types.services)
  {
    std::vector<std::string> checked_paths;
    const std::string srv_mix_path =
        search.find_service_mix(type, &checked_paths);

    if(srv_mix_path.empty())
    {
      print_missing_mix_file("service", type, checked_paths);
      success = false;
      continue;
    }

    if(!Mix::from_file(srv_mix_path).load())
    {
      std::cerr << "soss-ros1 failed to load extension for service type ["
                << type << "] using mix file: " << srv_mix_path << std::endl;
      success = false;
    }
  }

  return success;
}

//==============================================================================
bool SystemHandle::okay() const
{
  if(_node)
    return ros::ok();

  return false;
}

//==============================================================================
bool SystemHandle::spin_once()
{
  ros::spinOnce();
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
    const std::string& message_type,
    SubscriptionCallback callback,
    const YAML::Node& configuration)
{
  const YAML::Node& queue_config = configuration["queue_size"];
  int queue_size = queue_config ? queue_config.as<int>() : default_queue_size;
  // TODO(MXG): Parse configuration so users can change transport hints
  auto subscription = Factory::instance().create_subscription(
        message_type, *_node, topic_name,
        callback, queue_size, ros::TransportHints());

  if(!subscription)
    return false;

  _subscriptions.emplace_back(std::move(subscription));
  return true;
}

//==============================================================================
std::shared_ptr<TopicPublisher> SystemHandle::advertise(
    const std::string& topic_name,
    const std::string& message_type,
    const YAML::Node& configuration)
{
  const YAML::Node& queue_config = configuration["queue_size"];
  const YAML::Node& latch_config = configuration["latch"];

  int queue_size = queue_config ? queue_config.as<int>() : default_queue_size;
  bool latch_behavior = latch_config ? latch_config.as<bool>() : default_latch_behavior;

  if(topic_name.find('{') != std::string::npos)
  {
    // If the topic name contains a curly brace, we must assume that it needs
    // runtime substitutions.
    return make_meta_publisher(
          message_type, *_node, topic_name,
          queue_size, latch_behavior,
          configuration);
  }

  return Factory::instance().create_publisher(
        message_type, *_node, topic_name,
        queue_size, latch_behavior);
}

//==============================================================================
bool SystemHandle::create_client_proxy(
    const std::string& service_name,
    const std::string& service_type,
    RequestCallback callback,
    const YAML::Node& /*configuration*/)
{
  auto client_proxy = Factory::instance().create_client_proxy(
        service_type, *_node, service_name, callback);

  if(!client_proxy)
    return false;

  _client_proxies.emplace_back(std::move(client_proxy));
  return true;
}

//==============================================================================
std::shared_ptr<ServiceProvider> SystemHandle::create_service_proxy(
    const std::string& service_name,
    const std::string& service_type,
    const YAML::Node& /*configuration*/)
{
  return Factory::instance().create_server_proxy(
        service_type, *_node, service_name);
}

} // namespace ros1
} // namespace soss

//==============================================================================
SOSS_REGISTER_SYSTEM("ros1", soss::ros1::SystemHandle)
