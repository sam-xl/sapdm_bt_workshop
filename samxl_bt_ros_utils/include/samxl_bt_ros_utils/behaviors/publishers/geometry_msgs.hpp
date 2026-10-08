#pragma once

#include <geometry_msgs/msg/pose.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/wrench_stamped.hpp>
#include <samxl_bt_ros_utils/behaviors/generic_publisher.hpp>
#include <samxl_bt_ros_utils/conversions/geometry_msgs.hpp>

namespace samxl::behavior
{

template class GenericPublisher<geometry_msgs::msg::WrenchStamped>;
using PublishWrenchStamped = GenericPublisher<geometry_msgs::msg::WrenchStamped>;

template class GenericPublisher<geometry_msgs::msg::Pose>;
using PublishPose = GenericPublisher<geometry_msgs::msg::Pose>;

template class GenericPublisher<geometry_msgs::msg::PoseArray>;
using PublishPoseArray = GenericPublisher<geometry_msgs::msg::PoseArray>;

template class GenericPublisher<geometry_msgs::msg::PoseStamped>;
using PublishPoseStamped = GenericPublisher<geometry_msgs::msg::PoseStamped>;

}  // namespace samxl::behavior
