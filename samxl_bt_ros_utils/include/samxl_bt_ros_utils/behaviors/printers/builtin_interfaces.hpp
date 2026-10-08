#pragma once

#include <samxl_bt_ros_utils/behaviors/print_message.hpp>
#include <samxl_bt_ros_utils/conversions/builtin_interfaces.hpp>

#include "builtin_interfaces/msg/duration.hpp"
#include "builtin_interfaces/msg/time.hpp"

namespace samxl::behavior
{

template class PrintMessage<builtin_interfaces::msg::Time>;
using PrintTime = PrintMessage<builtin_interfaces::msg::Time>;

template class PrintMessage<builtin_interfaces::msg::Duration>;
using PrintDuration = PrintMessage<builtin_interfaces::msg::Duration>;

}  // namespace samxl::behavior
