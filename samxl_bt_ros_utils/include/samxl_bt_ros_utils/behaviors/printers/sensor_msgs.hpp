#pragma once

#include <samxl_bt_ros_utils/behaviors/print_message.hpp>
#include <samxl_bt_ros_utils/conversions/sensor_msgs.hpp>

#include "sensor_msgs/msg/joint_state.hpp"

namespace samxl::behavior
{

template class PrintMessage<sensor_msgs::msg::JointState>;
using PrintJointState = PrintMessage<sensor_msgs::msg::JointState>;

}  // namespace samxl::behavior
