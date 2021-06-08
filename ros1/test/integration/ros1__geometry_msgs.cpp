/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <ros/node_handle.h>

#include <is/sh/mock/api.hpp>
#include <is/core/Instance.hpp>
#include <is/utils/Convert.hpp>

#include <nav_msgs/GetPlan.h>
#include <geometry_msgs/Pose.h>
#include <yaml-cpp/yaml.h>

#include <gtest/gtest.h>

#include <random>
#include <limits>

namespace is = eprosima::is;
namespace xtypes = eprosima::xtypes;

geometry_msgs::PoseStamped generate_random_pose()
{
    std::mt19937 rng;
    // Use a fixed seed for deterministic test results
    rng.seed(39);
    std::uniform_real_distribution<double> dist(-100.0, 100.0);
    std::uniform_int_distribution<uint32_t>t_dist(0, std::numeric_limits<int32_t>::max());

    geometry_msgs::PoseStamped ros1_pose;

    ros1_pose.pose.position.x = dist(rng);
    ros1_pose.pose.position.y = dist(rng);
    ros1_pose.pose.position.z = dist(rng);

    ros1_pose.pose.orientation.w = 1.0;
    ros1_pose.pose.orientation.x = 0.0;
    ros1_pose.pose.orientation.y = 0.0;
    ros1_pose.pose.orientation.z = 0.0;

    ros1_pose.header.frame_id = "map";
    ros1_pose.header.stamp.sec = t_dist(rng);
    ros1_pose.header.stamp.nsec = t_dist(rng);

    return ros1_pose;
}

void transform_pose_msg(
        const geometry_msgs::PoseStamped& p,
        xtypes::WritableDynamicDataRef to)
{
    to["header"]["stamp"]["sec"] = static_cast<int32_t>(p.header.stamp.sec);
    to["header"]["stamp"]["nanosec"] = p.header.stamp.nsec;
    to["header"]["frame_id"] = "map";
    to["pose"]["position"]["x"] = p.pose.position.x;
    to["pose"]["position"]["y"] = p.pose.position.y;
    to["pose"]["position"]["z"] = p.pose.position.z;
    to["pose"]["orientation"]["x"] = p.pose.orientation.x;
    to["pose"]["orientation"]["y"] = p.pose.orientation.y;
    to["pose"]["orientation"]["z"] = p.pose.orientation.z;
    to["pose"]["orientation"]["w"] = p.pose.orientation.w;
}

xtypes::DynamicData generate_plan_request_msg(
        const xtypes::DynamicType& request_type,
        const geometry_msgs::PoseStamped& start,
        const geometry_msgs::PoseStamped& goal,
        const float tolerance = 1e-3f)
{
    xtypes::DynamicData msg(request_type);
    transform_pose_msg(goal, msg["goal"]);
    transform_pose_msg(start, msg["start"]);
    msg["tolerance"] = tolerance;

    return msg;
}

void compare_poses(
        const geometry_msgs::Pose& A,
        const geometry_msgs::Pose& B)
{
    const double tolerance = 1e-8;


  #define COMPARE_POSES_TEST_POSITION_COMPONENT( u ) \
    ASSERT_NEAR(A.position.u, B.position.u, tolerance)

    COMPARE_POSES_TEST_POSITION_COMPONENT(x);
    COMPARE_POSES_TEST_POSITION_COMPONENT(y);
    COMPARE_POSES_TEST_POSITION_COMPONENT(z);


  #define COMPARE_POSES_TEST_ORIENTATION_COMPONENT( u ) \
    ASSERT_NEAR(A.orientation.u, B.orientation.u, tolerance)

    COMPARE_POSES_TEST_ORIENTATION_COMPONENT(w);
    COMPARE_POSES_TEST_ORIENTATION_COMPONENT(x);
    COMPARE_POSES_TEST_ORIENTATION_COMPONENT(y);
    COMPARE_POSES_TEST_ORIENTATION_COMPONENT(z);
}

TEST(ROS1, Publish_subscribe_between_ros1_and_mock)
{
    using namespace std::chrono_literals;

    const double tolerance = 1e-8;

    YAML::Node config_node = YAML::LoadFile(ROS1__GEOMETRY_MSGS__TEST_CONFIG);

    // We add the build directory that any unfound mix packages may have been
    // built in, so that they can be found by the application.
    is::core::InstanceHandle handle = is::run_instance(
        config_node, {ROS1__GENMSG__BUILD_DIR});
    ASSERT_TRUE(handle);

    ros::NodeHandle ros1;
    ASSERT_TRUE(ros::ok());

    auto publisher = ros1.advertise<geometry_msgs::Pose>("transmit_pose", 10);
    ASSERT_TRUE(publisher);

    std::promise<xtypes::DynamicData> msg_promise;
    std::future<xtypes::DynamicData> msg_future = msg_promise.get_future();
    std::mutex mock_sub_mutex;
    bool mock_sub_value_received = false;
    auto mock_sub = [&](const xtypes::DynamicData& msg)
            {
                std::unique_lock<std::mutex> lock(mock_sub_mutex);
                if (mock_sub_value_received)
                {
                    return;
                }

                mock_sub_value_received = true;
                msg_promise.set_value(msg);
            };
    ASSERT_TRUE(is::sh::mock::subscribe("transmit_pose", mock_sub));

    geometry_msgs::Pose ros1_pose = generate_random_pose().pose;

    publisher.publish(ros1_pose);

    ros::spinOnce();

    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time < 30s)
    {
        ros::spinOnce();

        if (msg_future.wait_for(100ms) == std::future_status::ready)
        {
            break;
        }

        publisher.publish(ros1_pose);
    }

    // Wait no longer than a few seconds for the message to arrive. If it's not
    // ready by that time, then something is probably broken with the test, and
    // we should quit instead of waiting for the future and potentially hanging
    // forever.
    ASSERT_EQ(msg_future.wait_for(0s), std::future_status::ready);
    xtypes::DynamicData received_msg = msg_future.get();

    EXPECT_EQ(received_msg.type().name(), "geometry_msgs/Pose");

    xtypes::ReadableDynamicDataRef position = received_msg["position"];
    xtypes::ReadableDynamicDataRef orientation = received_msg["orientation"];

    #define TEST_POSITION_OF( u ) \
    { \
        const double u = position[#u]; \
        ASSERT_NEAR(u, ros1_pose.position.u, tolerance); \
    }

    TEST_POSITION_OF(x);
    TEST_POSITION_OF(y);
    TEST_POSITION_OF(z);

    bool promise_sent = false;
    std::promise<geometry_msgs::Pose> pose_promise;
    auto pose_future = pose_promise.get_future();
    std::mutex echo_mutex;

    boost::function<void(const geometry_msgs::Pose&)> echo_sub =
            [&](const geometry_msgs::Pose& msg)
            {
                std::unique_lock<std::mutex> lock(echo_mutex);

                // promises will throw an exception if set_value(~) is called more than
                // once, so we'll guard against that.
                if (promise_sent)
                {
                    return;
                }

                promise_sent = true;
                pose_promise.set_value(msg);
            };

    auto subscriber = ros1.subscribe<geometry_msgs::Pose>(
        "echo_pose", 10, echo_sub);
    ASSERT_TRUE(subscriber);

    // Keep spinning and publishing while we wait for the promise to be
    // delivered. Try to cycle this for no more than a few seconds. If it's not
    // finished by that time, then something is probably broken with the test or
    // with Integration Service, and we should quit instead of waiting for the future and
    // potentially hanging forever.
    start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time < 30s)
    {
        ros::spinOnce();

        is::sh::mock::publish_message("echo_pose", received_msg);

        ros::spinOnce();
        if (pose_future.wait_for(100ms) == std::future_status::ready)
        {
            break;
        }
    }

    ASSERT_EQ(pose_future.wait_for(0s), std::future_status::ready);
    geometry_msgs::Pose received_pose = pose_future.get();

    compare_poses(ros1_pose, received_pose);

    // Shutdown ROS 1
    ros1.shutdown();

    // Quit and wait for no more than a minute. We don't want the test to get
    // hung here indefinitely in the case of an error.
    handle.quit().wait_for(5s);

    // Require that it's no longer running. If it is still running, then it is
    // probably stuck, and we should forcefully quit.
    ASSERT_TRUE(!handle.running());
    ASSERT_TRUE(handle.wait() == 0);
}

TEST(ROS1, Request_reply_between_ros1_and_mock)
{
    using namespace std::chrono_literals;

    const double tolerance = 1e-8;

    YAML::Node config_node = YAML::LoadFile(ROS1__GEOMETRY_MSGS__TEST_CONFIG);

    // We add the build directory that any unfound mix packages may have been
    // built in, so that they can be found by the application.
    is::core::InstanceHandle handle = is::run_instance(
        config_node, {ROS1__GENMSG__BUILD_DIR});
    ASSERT_TRUE(handle);

    ros::NodeHandle ros1;
    ASSERT_TRUE(ros::ok());

    // Get request type from ros1 middleware
    const is::TypeRegistry& ros1_types = *handle.type_registry("ros1");
    const xtypes::DynamicType& request_type = *ros1_types.at("nav_msgs/GetPlan:request");
    // Create a plan
    nav_msgs::GetPlanResponse plan_response;
    plan_response.plan.header.stamp.sec = 266;
    plan_response.plan.header.stamp.nsec = 267;
    plan_response.plan.header.frame_id = "ros1_frame_string";

    for (int i = 0; i < 5; ++i)
    {
        plan_response.plan.poses.push_back(generate_random_pose());
    }

    std::promise<geometry_msgs::PoseStamped> promised_start;
    auto future_start = promised_start.get_future();
    std::promise<geometry_msgs::PoseStamped> promised_goal;
    auto future_goal = promised_goal.get_future();

    nav_msgs::GetPlan::Request plan_request;

    using ServiceSignature = boost::function<bool(
                        nav_msgs::GetPlanRequest&, nav_msgs::GetPlanResponse&)>;

    bool service_called = false;
    std::mutex service_mutex;
    ServiceSignature ros1_plan_service = [&](
        nav_msgs::GetPlan::Request& request,
        nav_msgs::GetPlan::Response& response)
            {
                std::unique_lock<std::mutex> locK(service_mutex);
                response = plan_response;

                if (service_called)
                {
                    return true;
                }

                plan_request = request;
                promised_start.set_value(request.start);
                promised_goal.set_value(request.goal);
                return true;
            };

    const auto ros1_serv = ros1.advertiseService("get_plan", ros1_plan_service);

    ros::spinOnce();

    xtypes::DynamicData request_msg = generate_plan_request_msg(
        request_type,
        plan_response.plan.poses.front(),
        plan_response.plan.poses.back());

    auto future_response_msg = is::sh::mock::request(
        "get_plan", request_msg, 100ms);

    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time < 30s)
    {
        ros::spinOnce();

        if (future_goal.wait_for(100ms) == std::future_status::ready)
        {
            break;
        }
    }

    // Make sure that we got the expected request message
    ASSERT_EQ(future_start.wait_for(0s), std::future_status::ready);
    ASSERT_EQ(future_goal.wait_for(0s), std::future_status::ready);

    auto requested_start = future_start.get();
    compare_poses(requested_start.pose, plan_response.plan.poses.front().pose);

    auto requested_goal = future_goal.get();
    compare_poses(requested_goal.pose, plan_response.plan.poses.back().pose);

    start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time < 30s)
    {
        ros::spinOnce();
        if (future_response_msg.wait_for(100ms) == std::future_status::ready)
        {
            break;
        }
    }

    ASSERT_EQ(future_response_msg.wait_for(0s), std::future_status::ready);
    const xtypes::DynamicData response_msg = future_response_msg.get();

    // TODO(MXG): We could copy the request message that gets passed to here and
    // compare it against the original request message that was sent. This would
    // require implementing comparison operators for the xtypes::DynamicData class.
    std::mutex serve_mutex;
    is::sh::mock::serve("echo_plan", [&](const xtypes::DynamicData&)
            {
                std::unique_lock<std::mutex> lock(serve_mutex);
                return response_msg;
            });

    auto client = ros1.serviceClient<nav_msgs::GetPlan>("echo_plan");
    ASSERT_TRUE(client.waitForExistence(ros::Duration(10.0)));

    auto request = plan_request;
    nav_msgs::GetPlanResponse response;
    ASSERT_TRUE(client.call(request, response));

    ASSERT_EQ(response.plan.poses.size(), plan_response.plan.poses.size());
    for (std::size_t i = 0; i < response.plan.poses.size(); ++i)
    {
        compare_poses(response.plan.poses[i].pose,
                plan_response.plan.poses[i].pose);
        ASSERT_EQ(response.plan.poses[i].header.frame_id, plan_response.plan.poses[i].header.frame_id);
    }

    ros1.shutdown();

    // Quit and wait for no more than 5 seconds. We don't want the test to get
    // hung here indefinitely in the case of an error.
    handle.quit().wait_for(5s);

    // Require that it's no longer running. If it is still running, then it is
    // probably stuck, and we should forcefully quit.
    ASSERT_TRUE(!handle.running());
    ASSERT_EQ(handle.wait(), 0);
}

int main(
        int argc,
        char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
