#pragma once

#include <samxl_bt_ros_utils/behaviors/print_message.hpp>
#include <samxl_bt_ros_utils/conversions/geometry_msgs.hpp>

#include "geometry_msgs/msg/point.hpp"
#include "geometry_msgs/msg/pose.hpp"
#include "geometry_msgs/msg/pose_array.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/quaternion.hpp"
#include "geometry_msgs/msg/vector3.hpp"
#include "geometry_msgs/msg/wrench.hpp"
#include "geometry_msgs/msg/wrench_stamped.hpp"

namespace samxl::behavior
{

template class PrintMessage<geometry_msgs::msg::Point>;
using PrintPoint = PrintMessage<geometry_msgs::msg::Point>;

template class PrintMessage<geometry_msgs::msg::Pose>;
using PrintPose = PrintMessage<geometry_msgs::msg::Pose>;

template class PrintMessage<geometry_msgs::msg::PoseArray>;
using PrintPoseArray = PrintMessage<geometry_msgs::msg::PoseArray>;

template class PrintMessage<geometry_msgs::msg::PoseStamped>;
using PrintPoseStamped = PrintMessage<geometry_msgs::msg::PoseStamped>;

template class PrintMessage<geometry_msgs::msg::Quaternion>;
using PrintQuaternion = PrintMessage<geometry_msgs::msg::Quaternion>;

template class PrintMessage<geometry_msgs::msg::Vector3>;
using PrintVector3 = PrintMessage<geometry_msgs::msg::Vector3>;

template class PrintMessage<geometry_msgs::msg::Wrench>;
using PrintWrench = PrintMessage<geometry_msgs::msg::Wrench>;

template class PrintMessage<geometry_msgs::msg::WrenchStamped>;
using PrintWrenchStamped = PrintMessage<geometry_msgs::msg::WrenchStamped>;

}  // namespace samxl::behavior
