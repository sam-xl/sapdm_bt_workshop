#include <samxl_bt_ros_utils/conversions/bt_conversions.hpp>
#include <samxl_bt_ros_utils/conversions/yaml_conversions.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

namespace samxl::yaml
{

template <>
void msg_from_yaml(const YAML::Node& node, sensor_msgs::msg::JointState& output)
{
  for (const auto& name : node["name"])
  {
    output.name.push_back(name.as<std::string>());
  }
  for (const auto& position : node["position"])
  {
    output.position.push_back(position.as<double>());
  }
  for (const auto& velocity : node["velocity"])
  {
    output.velocity.push_back(velocity.as<double>());
  }
  for (const auto& effort : node["effort"])
  {
    output.effort.push_back(effort.as<double>());
  }
  msg_from_yaml(node["header"], output.header);
}

}  // namespace samxl::yaml

REGISTER_TYPE_CONVERT_FROM_STRING(sensor_msgs::msg::JointState)
