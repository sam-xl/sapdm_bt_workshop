#include <geometry_msgs/msg/point.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/quaternion.hpp>
#include <geometry_msgs/msg/vector3.hpp>
#include <geometry_msgs/msg/wrench.hpp>
#include <geometry_msgs/msg/wrench_stamped.hpp>
#include <samxl_bt_ros_utils/conversions/bt_conversions.hpp>
#include <samxl_bt_ros_utils/conversions/yaml_conversions.hpp>

namespace samxl::yaml
{

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::Quaternion& output)
{
  output.x = node["x"].as<double>();
  output.y = node["y"].as<double>();
  output.z = node["z"].as<double>();
  output.w = node["w"].as<double>();
}

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::Point& output)
{
  output.x = node["x"].as<double>();
  output.y = node["y"].as<double>();
  output.z = node["z"].as<double>();
}

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::Pose& output)
{
  msg_from_yaml(node["orientation"], output.orientation);
  msg_from_yaml(node["position"], output.position);
}

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::PoseStamped& output)
{
  msg_from_yaml(node["header"], output.header);
  msg_from_yaml(node["pose"], output.pose);
}

template <>
void msg_from_yaml(const YAML::Node& node, std::shared_ptr<std::deque<geometry_msgs::msg::PoseStamped>>& output)
{
  if (!output)
  {
    output = std::make_shared<std::deque<geometry_msgs::msg::PoseStamped>>();
  }
  else
  {
    output->clear();
  }
  for (const auto& pose : node)
  {
    output->push_back(msg_from_yaml<geometry_msgs::msg::PoseStamped>(pose));
  }
}

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::PoseArray& output)
{
  for (const auto& pose : node["poses"])
  {
    output.poses.push_back(msg_from_yaml<geometry_msgs::msg::Pose>(pose));
  }
  msg_from_yaml(node["header"], output.header);
}

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::Vector3& output)
{
  output.x = node["x"].as<double>();
  output.y = node["y"].as<double>();
  output.z = node["z"].as<double>();
}

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::Wrench& output)
{
  msg_from_yaml(node["force"], output.force);
  msg_from_yaml(node["torque"], output.torque);
}

template <>
void msg_from_yaml(const YAML::Node& node, geometry_msgs::msg::WrenchStamped& output)
{
  msg_from_yaml(node["header"], output.header);
  msg_from_yaml(node["wrench"], output.wrench);
}

}  // namespace samxl::yaml

REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Quaternion)
REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Point)
REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Pose)
REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::PoseStamped)
REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::PoseArray)
REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Vector3)
REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::Wrench)
REGISTER_TYPE_CONVERT_FROM_STRING(geometry_msgs::msg::WrenchStamped)
REGISTER_TYPE_CONVERT_FROM_STRING(std::shared_ptr<std::deque<geometry_msgs::msg::PoseStamped>>)
