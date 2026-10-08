#include <samxl_bt_ros_utils/conversions/bt_conversions.hpp>
#include <samxl_bt_ros_utils/conversions/yaml_conversions.hpp>
#include <std_msgs/msg/header.hpp>

namespace samxl::yaml
{

template <>
void msg_from_yaml(const YAML::Node& node, std_msgs::msg::Header& output)
{
  output.frame_id = node["frame_id"].as<std::string>();
  if (node["stamp"])
  {
    msg_from_yaml(node["stamp"], output.stamp);
  }
  else
  {
    output.stamp.sec = 0;
    output.stamp.nanosec = 0;
  }
}

}  // namespace samxl::yaml

REGISTER_TYPE_CONVERT_FROM_STRING(std_msgs::msg::Header)
