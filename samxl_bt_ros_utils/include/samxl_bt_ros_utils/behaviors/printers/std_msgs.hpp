#pragma once

#include <samxl_bt_ros_utils/behaviors/print_message.hpp>
#include <samxl_bt_ros_utils/conversions/std_msgs.hpp>

#include "std_msgs/msg/header.hpp"

namespace samxl::behavior
{

template class PrintMessage<std_msgs::msg::Header>;
using PrintHeader = PrintMessage<std_msgs::msg::Header>;

}  // namespace samxl::behavior
