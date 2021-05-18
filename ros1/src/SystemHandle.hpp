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

#ifndef _IS_SH_ROS1__INTERNAL__SYSTEMHANDLE_HPP_
#define _IS_SH_ROS1__INTERNAL__SYSTEMHANDLE_HPP_

#include <is/systemhandle/SystemHandle.hpp>

#include <is/utils/Log.hpp>

#include <ros/node_handle.h>

namespace xtypes = eprosima::xtypes;

namespace eprosima {
namespace is {
namespace sh {
namespace ros1 {

/**
 * @class SystemHandle
 * @brief Implements all the interface defined for the *Integration Service* FullSystem,
 *        for the ROS 1 ecosystem.
 *
 * @note This SystemHandle is currently prepared to support the latest LTS distribution
 *       of ROS 1, that is, Noetic Ninjemys.
 */
class SystemHandle : public virtual FullSystem
{
public:

    /**
     * @brief Construct a new SystemHandle object.
     *
     */
    SystemHandle();

    /**
     * @brief Inherited from SystemHandle.
     */
    bool configure(
            const core::RequiredTypes& types,
            const YAML::Node& configuration,
            TypeRegistry& type_registry) override;

    /**
     * @brief Inherited from SystemHandle.
     */
    bool okay() const override;

    /**
     * @brief Inherited from SystemHandle.
     */
    bool spin_once() override;

    /**
     * @brief Inherited from SystemHandle.
     */
    ~SystemHandle() override;

    /**
     * @brief Inherited from SystemHandle.
     */
    bool subscribe(
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            SubscriptionCallback* callback,
            const YAML::Node& configuration) override;

    /**
     * @brief Inherited from SystemHandle.
     */
    std::shared_ptr<TopicPublisher> advertise(
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            const YAML::Node& configuration) override;

    /**
     * @brief Inherited from SystemHandle.
     */
    bool create_client_proxy(
            const std::string& service_name,
            const xtypes::DynamicType& service_type,
            RequestCallback* callback,
            const YAML::Node& configuration) override;

    /**
     * @brief Inherited from ServiceClientSystem.
     */
    bool create_client_proxy(
            const std::string& service_name,
            const xtypes::DynamicType&,
            const xtypes::DynamicType& reply_type,
            RequestCallback* callback,
            const YAML::Node& configuration) override
    {
        return create_client_proxy(service_name, reply_type, callback, configuration);
    }

    /**
     * @brief Inherited from SystemHandle.
     */
    std::shared_ptr<ServiceProvider> create_service_proxy(
            const std::string& service_name,
            const xtypes::DynamicType& service_type,
            const YAML::Node& configuration) override;

    /**
     * @brief Inherited from ServiceProviderSystem.
     */
    std::shared_ptr<ServiceProvider> create_service_proxy(
            const std::string& service_name,
            const xtypes::DynamicType& request_type,
            const xtypes::DynamicType&,
            const YAML::Node& configuration) override
    {
        return create_service_proxy(service_name, request_type, configuration);
    }

private:

    /**
     * @brief Log an error message, indicating that a *mix* file is missing.
     *
     * @param[in] msg_or_srv Whether the *mix* file corresponds to a message or service.
     *
     * @param[in] type The ROS 1 type whose *mix* file could not be found.
     *
     * @param[in] checked_paths The paths where the *mix* file was searched for.
     */
    void print_missing_mix_file(
            const std::string& msg_or_srv,
            const std::string& type,
            const std::vector<std::string>& checked_paths);

    /**
     * Class members.
     */

    std::unique_ptr<ros::NodeHandle> _node;
    std::vector<std::shared_ptr<void> > _subscriptions;
    std::vector<std::shared_ptr<ServiceClient> > _client_proxies;

    const uint32_t default_queue_size = 10;
    const bool default_latch_behavior = false;

    utils::Logger _logger;
};


} //  namespace ros1
} //  namespace sh
} //  namespace is
} //  namespace eprosima

#endif //  _IS_SH_ROS1__INTERNAL__SYSTEMHANDLE_HPP_
