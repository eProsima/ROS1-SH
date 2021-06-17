// generated from is-ros1/resources/convert__msg.cpp.em
// generated code does not contain a copyright notice

@#######################################################################
@# EmPy template for generating is/genmsg/ros1/<package>/src/msg/convert__msg__<msg>.cpp files
@#
@# Context:
@#  - spec (genmsg.MsgSpec)
@#    Parsed specification of the .msg file
@#  - subfolder (string)
@#    The subfolder / subnamespace of the message
@#    Either 'msg' or 'srv'
@#  - package (string)
@#    The name of the package
@#  - type (string)
@#    The name of the message type (not including the package name prefix)
@#######################################################################

@{
namespace_parts = ['convert', package, 'msg', type]
namespace_variable = '__'.join(namespace_parts)

conversion_dependency = 'is/genmsg/ros1/{}/msg/convert__msg__{}.hpp'.format(
    package, type)
}@

// Include the API header for this message type
#include <@(conversion_dependency)>

// Include the Factory header so we can add this message type to the Factory
#include <is/sh/ros1/Factory.hpp>

// Include the NodeHandle API so we can subscribe and advertise
#include <ros/node_handle.h>
#include <ros/this_node.h>

// TODO(jamoralp): Add utils::Logger traces here
namespace eprosima {
namespace is {
namespace sh {
namespace ros1 {
namespace @(namespace_variable) {

//==============================================================================
namespace {
TypeToFactoryRegistrar register_type(g_msg_name, &type);
} //  anonymous namespace

//==============================================================================
class Subscription final
{
public:

    Subscription(
            ros::NodeHandle& node,
            const std::string& topic_name,
            const xtypes::DynamicType& message_type,
            TopicSubscriberSystem::SubscriptionCallback* callback,
            uint32_t queue_size,
            const ros::TransportHints& transport_hints)
        : _topic(topic_name)
        , _callback(callback)
        , _message_type(message_type)
    {

        _subscription = node.subscribe(
            topic_name, queue_size, &Subscription::subscription_callback, this,
            transport_hints);
    }

private:

    void subscription_callback(
            const ros::MessageEvent<Ros1_Msg const>& msg_event)
    {
        if (ros::this_node::getName() == msg_event.getPublisherName())
        {
            // This is a local publication from within Integration Service. Return
            return;
        }

        xtypes::DynamicData data(_message_type);
        convert_to_xtype(*(msg_event.getMessage().get()), data);
        (*_callback)(data, nullptr);
    }

    const std::string _topic;

    TopicSubscriberSystem::SubscriptionCallback* _callback;

    const xtypes::DynamicType& _message_type;

    ros::Subscriber _subscription;
};

//==============================================================================
std::shared_ptr<void> subscribe(
        ros::NodeHandle& node,
        const std::string& topic_name,
        const xtypes::DynamicType& message_type,
        TopicSubscriberSystem::SubscriptionCallback* callback,
        const uint32_t queue_size,
        const ros::TransportHints& transport_hints)
{
    return std::make_shared<Subscription>(
        node, topic_name, message_type, callback, queue_size, transport_hints);
}

//==============================================================================
namespace {
SubscriptionToFactoryRegistrar register_subscriber(g_msg_name, &subscribe);
} //  anonymous namespace

//==============================================================================
class Publisher final : public virtual is::TopicPublisher
{
public:

    Publisher(
            ros::NodeHandle& node,
            const std::string& topic_name,
            uint32_t queue_size,
            bool latch)
    {
        _publisher = node.advertise<Ros1_Msg>(topic_name, queue_size, latch);
    }

    bool publish(
            const xtypes::DynamicData& message) override
    {
        Ros1_Msg ros1_msg;
        convert_to_ros1(message, ros1_msg);

        _publisher.publish(ros1_msg);
        return true;
    }

private:

    ros::Publisher _publisher;
};

//==============================================================================
std::shared_ptr<is::TopicPublisher> make_publisher(
        ros::NodeHandle& node,
        const std::string& topic_name,
        const uint32_t queue_size,
        const bool latch)
{
    return std::make_shared<Publisher>(node, topic_name, queue_size, latch);
}

namespace {
PublisherToFactoryRegistrar register_publisher(g_msg_name, &make_publisher);
} //  anonymous namespace

} //  namespace @(namespace_variable)
} //  namespace ros1
} //  namespace sh
} //  namespace is
} //  namespace eprosima
