#pragma once

/**
 * This header needs to be included to ensure that the conversions declared
 * here are properly linked and can be found by BehaviorTree.CPP at runtime.
 *
 * If this header is not used, BehaviorTree.CPP will issue the following warning for these types:
 * `You (maybe indirectly) called BT::convertFromString() for type [T], but I can't find the template specialization.`
 */

#include <behaviortree_cpp/basic_types.h>

#include <samxl_bt_ros_utils/conversions/bt_conversions.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

HEADER_TYPE_CONVERT_FROM_STRING(sensor_msgs::msg::JointState)
