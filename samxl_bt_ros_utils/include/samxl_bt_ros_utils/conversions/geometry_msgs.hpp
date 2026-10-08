#pragma once

/**
 * This header needs to be included to ensure that the conversions declared
 * here are properly linked and can be found by BehaviorTree.CPP at runtime.
 *
 * If this header is not used, BehaviorTree.CPP will issue the following warning for these types:
 * `You (maybe indirectly) called BT::convertFromString() for type [T], but I can't find the template specialization.`
 */

#include <behaviortree_cpp/basic_types.h>

#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <geometry_msgs/msg/wrench.hpp>
#include <geometry_msgs/msg/wrench_stamped.hpp>
#include <samxl_bt_ros_utils/conversions/bt_conversions.hpp>

HEADER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Point)
HEADER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Pose)
HEADER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::PoseStamped)
HEADER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::PoseArray)
HEADER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Vector3)
HEADER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Wrench)
HEADER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::WrenchStamped)
HEADER_TYPE_CONVERT_FROM_STRING(std::shared_ptr<std::deque<geometry_msgs::msg::PoseStamped>>)