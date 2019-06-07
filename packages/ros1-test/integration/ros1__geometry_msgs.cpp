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

#include <ros/node_handle.h>

#include <soss/mock/api.hpp>
#include <soss/Instance.hpp>
#include <soss/utilities.hpp>

#include <nav_msgs/GetPlan.h>
#include <geometry_msgs/Pose.h>
#include <yaml-cpp/yaml.h>

#include <catch2/catch.hpp>

#include <random>

using Catch::Matchers::WithinAbs;

geometry_msgs::PoseStamped generate_random_pose(const uint32_t sec = 0)
{
  std::mt19937 rng;
  // Use a fixed seed for deterministic test results
  rng.seed(39);
  std::uniform_real_distribution<double> dist(-100.0, 100.0);

  geometry_msgs::PoseStamped ros1_pose;

  ros1_pose.pose.position.x = dist(rng);
  ros1_pose.pose.position.y = dist(rng);
  ros1_pose.pose.position.z = dist(rng);

  ros1_pose.pose.orientation.w = 1.0;
  ros1_pose.pose.orientation.x = 0.0;
  ros1_pose.pose.orientation.y = 0.0;
  ros1_pose.pose.orientation.z = 0.0;

  ros1_pose.header.frame_id = "map";
  ros1_pose.header.stamp.sec = sec;

  return ros1_pose;
}

soss::Message generate_pose_msg(
    const geometry_msgs::PoseStamped& p)
{
  soss::Message stamp;
  stamp.type = "builtin_interfaces/Time";
  stamp.data["sec"] = soss::Convert<int32_t>::make_soss_field(p.header.stamp.sec);
  stamp.data["nanosec"] = soss::Convert<uint32_t>::make_soss_field(p.header.stamp.nsec);

  soss::Message header;
  header.type = "std_msgs/Header";
  header.data["stamp"] = soss::make_field<soss::Message>(stamp);
  header.data["frame_id"] = soss::Convert<std::string>::make_soss_field("map");

  soss::Message position;
  position.type = "geometry_msgs/Point";
  position.data["x"] = soss::Convert<double>::make_soss_field(p.pose.position.x);
  position.data["y"] = soss::Convert<double>::make_soss_field(p.pose.position.y);
  position.data["z"] = soss::Convert<double>::make_soss_field(p.pose.position.z);

  soss::Message orientation;
  orientation.type = "geometry_msgs/Quaternion";
  orientation.data["x"] = soss::Convert<double>::make_soss_field(p.pose.orientation.x);
  orientation.data["y"] = soss::Convert<double>::make_soss_field(p.pose.orientation.y);
  orientation.data["z"] = soss::Convert<double>::make_soss_field(p.pose.orientation.z);
  orientation.data["w"] = soss::Convert<double>::make_soss_field(p.pose.orientation.w);

  soss::Message pose;
  pose.type = "geometry_msgs/Pose";
  pose.data["position"] = soss::make_field<soss::Message>(position);
  pose.data["orientation"] = soss::make_field<soss::Message>(orientation);

  soss::Message msg;
  msg.type = "geometry_msgs/PoseStamped";
  msg.data["header"] = soss::make_field<soss::Message>(header);
  msg.data["pose"] = soss::make_field<soss::Message>(pose);

  return msg;
}

soss::Message generate_plan_request_msg(
    const geometry_msgs::PoseStamped& start,
    const geometry_msgs::PoseStamped& goal,
    const float tolerance = 1e-3f)
{
  soss::Message msg;
  msg.type = "nav_msgs/GetPlan:request";
  msg.data["goal"] = soss::make_field<soss::Message>(generate_pose_msg(goal));
  msg.data["start"] = soss::make_field<soss::Message>(generate_pose_msg(start));
  msg.data["tolerance"] = soss::Convert<float>::make_soss_field(tolerance);

  return msg;
}

void compare_poses(
    const geometry_msgs::Pose& A,
    const geometry_msgs::Pose& B)
{
  const double tolerance = 1e-8;


  #define COMPARE_POSES_TEST_POSITION_COMPONENT( u ) \
    CHECK_THAT(A.position.u, WithinAbs(B.position.u,  tolerance))

  COMPARE_POSES_TEST_POSITION_COMPONENT(x);
  COMPARE_POSES_TEST_POSITION_COMPONENT(y);
  COMPARE_POSES_TEST_POSITION_COMPONENT(z);


  #define COMPARE_POSES_TEST_ORIENTATION_COMPONENT( u ) \
    CHECK_THAT(A.orientation.u, WithinAbs(B.orientation.u, tolerance))

  COMPARE_POSES_TEST_ORIENTATION_COMPONENT(w);
  COMPARE_POSES_TEST_ORIENTATION_COMPONENT(x);
  COMPARE_POSES_TEST_ORIENTATION_COMPONENT(y);
  COMPARE_POSES_TEST_ORIENTATION_COMPONENT(z);
}

TEST_CASE("Talk between ros1 and the mock middleware", "[ros1]")
{
  using namespace std::chrono_literals;

  const double tolerance = 1e-8;

  YAML::Node config_node = YAML::LoadFile(ROS1__GEOMETRY_MSGS__TEST_CONFIG);

  // We add the build directory that any unfound soss mix packages may have been
  // built in, so that they can be found by the application.
  soss::InstanceHandle handle = soss::run_instance(
        config_node, {ROS1__GENMSG__BUILD_DIR});

  REQUIRE(handle);

//  rclcpp::Node::SharedPtr ros1 = std::make_shared<rclcpp::Node>("ros1_test");
//  rclcpp::executors::SingleThreadedExecutor executor;
  ros::NodeHandle ros1;

  REQUIRE( ros::ok() );

  SECTION("Publish a pose and get it echoed back")
  {
    auto publisher = ros1.advertise<geometry_msgs::Pose>("transmit_pose", 10);

    std::promise<soss::Message> msg_promise;
    std::future<soss::Message> msg_future = msg_promise.get_future();
    std::mutex mock_sub_mutex;
    bool mock_sub_value_received = false;
    auto mock_sub = [&](const soss::Message& msg)
    {
      std::unique_lock<std::mutex> lock(mock_sub_mutex);
      if(mock_sub_value_received)
        return;

      mock_sub_value_received = true;
      msg_promise.set_value(msg);
    };
    REQUIRE(soss::mock::subscribe("transmit_pose", mock_sub));

    geometry_msgs::Pose ros1_pose = generate_random_pose().pose;

    publisher.publish(ros1_pose);

    ros::spinOnce();

    auto start_time = std::chrono::steady_clock::now();
    while(std::chrono::steady_clock::now() - start_time < 30s)
    {
      ros::spinOnce();

      if(msg_future.wait_for(100ms) == std::future_status::ready)
        break;

      publisher.publish(ros1_pose);
    }

    // Wait no longer than a few seconds for the message to arrive. If it's not
    // ready by that time, then something is probably broken with the test, and
    // we should quit instead of waiting for the future and potentially hanging
    // forever.
    REQUIRE(msg_future.wait_for(0s) == std::future_status::ready);
    soss::Message received_msg = msg_future.get();

    CHECK(received_msg.type == "geometry_msgs/Pose");

    soss::Message* position =
        received_msg.data["position"].cast<soss::Message>();
    REQUIRE(position);

    soss::Message* orientation =
        received_msg.data["orientation"].cast<soss::Message>();
    REQUIRE(orientation);

    #define TEST_POSITION_OF( u ) \
    { \
      const double* u = position->data[#u].cast<double>(); \
      REQUIRE(u); \
      CHECK_THAT(*u, WithinAbs(ros1_pose.position.u, tolerance)); \
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
      if(promise_sent)
        return;

      promise_sent = true;
      pose_promise.set_value(msg);
    };

    auto subscriber = ros1.subscribe<geometry_msgs::Pose>(
          "echo_pose", 10, echo_sub);

    // Keep spinning and publishing while we wait for the promise to be
    // delivered. Try to cycle this for no more than a few seconds. If it's not
    // finished by that time, then something is probably broken with the test or
    // with soss, and we should quit instead of waiting for the future and
    // potentially hanging forever.
    start_time = std::chrono::steady_clock::now();
    while(std::chrono::steady_clock::now() - start_time < 30s)
    {
      ros::spinOnce();

      soss::mock::publish_message("echo_pose", received_msg);

      ros::spinOnce();
      if(pose_future.wait_for(100ms) == std::future_status::ready)
        break;
    }

    REQUIRE(pose_future.wait_for(0s) == std::future_status::ready);
    geometry_msgs::Pose received_pose = pose_future.get();

    compare_poses(ros1_pose, received_pose);
  }
  SECTION("Request a plan and get it echoed back")
  {
    // Create a plan
    nav_msgs::GetPlanResponse plan_response;
    plan_response.plan.header.stamp.sec = 266;
    plan_response.plan.header.stamp.nsec = 267;
    plan_response.plan.header.frame_id = "ros1_frame_string";
    for(int i=0 ; i < 5; ++i)
      plan_response.plan.poses.push_back(generate_random_pose(i));

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

      if(service_called)
        return true;

      plan_request = request;
      promised_start.set_value(request.start);
      promised_goal.set_value(request.goal);
      return true;
    };

    const auto ros1_serv = ros1.advertiseService("get_plan", ros1_plan_service);

    ros::spinOnce();

    soss::Message request_msg = generate_plan_request_msg(
          plan_response.plan.poses.front(),
          plan_response.plan.poses.back());

    auto future_response_msg = soss::mock::request(
          "get_plan", request_msg, 100ms);

    auto start_time = std::chrono::steady_clock::now();
    while(std::chrono::steady_clock::now() - start_time < 30s)
    {
      ros::spinOnce();

      if(future_goal.wait_for(100ms) == std::future_status::ready)
        break;
    }

    // Make sure that we got the expected request message
    REQUIRE(future_start.wait_for(0s) == std::future_status::ready);
    REQUIRE(future_goal.wait_for(0s) == std::future_status::ready);
    auto requested_start = future_start.get();
    compare_poses(requested_start.pose, plan_response.plan.poses.front().pose);
    auto requested_goal = future_goal.get();
    compare_poses(requested_goal.pose, plan_response.plan.poses.back().pose);

    start_time = std::chrono::steady_clock::now();
    while(std::chrono::steady_clock::now() - start_time < 30s)
    {
      ros::spinOnce();
      if(future_response_msg.wait_for(100ms) == std::future_status::ready)
        break;
    }
    REQUIRE(future_response_msg.wait_for(0s) == std::future_status::ready);
    const soss::Message response_msg = future_response_msg.get();

    // TODO(MXG): We could copy the request message that gets passed to here and
    // compare it against the original request message that was sent. This would
    // require implementing comparison operators for the soss::Message class.
    std::mutex serve_mutex;
    soss::mock::serve("echo_plan", [&](const soss::Message&)
    {
      std::unique_lock<std::mutex> lock(serve_mutex);
      return response_msg;
    });

    auto client = ros1.serviceClient<nav_msgs::GetPlan>("echo_plan");
    REQUIRE(client.waitForExistence(ros::Duration(10.0)));

    auto request = plan_request;
    nav_msgs::GetPlanResponse response;
    REQUIRE(client.call(request, response));

    REQUIRE(response.plan.poses.size() == plan_response.plan.poses.size());
    for(std::size_t i=0; i < response.plan.poses.size(); ++i)
    {
      compare_poses(response.plan.poses[i].pose,
                    plan_response.plan.poses[i].pose);
    }
  }

  ros1.shutdown();

  // Quit and wait for no more than 5 seconds. We don't want the test to get
  // hung here indefinitely in the case of an error.
  handle.quit().wait_for(5s);

  // Require that it's no longer running. If it is still running, then it is
  // probably stuck, and we should forcefully quit.
  REQUIRE(!handle.running());
  REQUIRE(handle.wait() == 0);
}
