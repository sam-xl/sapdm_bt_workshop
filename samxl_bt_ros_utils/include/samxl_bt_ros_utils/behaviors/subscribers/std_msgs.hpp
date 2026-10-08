#pragma once

#include <std_msgs/msg/bool.hpp>
#include <std_msgs/msg/string.hpp>
#include <samxl_bt_ros_utils/behaviors/generic_subscriber.hpp>

namespace samxl::behavior
{

template <>
struct SubscriberOutput<std_msgs::msg::Bool>
{
  using type = bool;
  static type get(const std_msgs::msg::Bool& msg)
  {
    return msg.data;
  }
};

template <>
struct SubscriberOutput<std_msgs::msg::String>
{
  using type = std::string;
  static type get(const std_msgs::msg::String& msg)
  {
    return msg.data;
  }
};

template class GenericSubscriber<std_msgs::msg::Bool>;
using SubscribeBool = GenericSubscriber<std_msgs::msg::Bool>;

template class GenericSubscriber<std_msgs::msg::String>;
using SubscribeString = GenericSubscriber<std_msgs::msg::String>;

}  // namespace samxl::behavior
