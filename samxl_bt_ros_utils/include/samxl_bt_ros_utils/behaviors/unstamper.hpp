#pragma once

#include <behaviortree_cpp/action_node.h>

#include <string>

// Necessary for YAML conversions
#include <samxl_bt_ros_utils/conversions/geometry_msgs.hpp>

#include "builtin_interfaces/msg/time.hpp"
#include "geometry_msgs/msg/pose.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"

namespace samxl::behavior
{

// NOTE(Jasper): Not completely happy as this is mostly duplicating boiler plate.

class UnstampPose : public BT::SyncActionNode
{
protected:
  using Stamped = geometry_msgs::msg::PoseStamped;
  using Unstamped = geometry_msgs::msg::Pose;

public:
  explicit UnstampPose(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<Stamped>("input", "The stamped pose message"),
      BT::OutputPort<Unstamped>("output", "The unstamped pose message"),
      BT::OutputPort<std::string>("frame_id", "The original frame_id of the message"),
      BT::OutputPort<builtin_interfaces::msg::Time>("stamp", "The original timestamp of the message"),
    };
  }

  BT::NodeStatus tick() override
  {
    Stamped input;
    if (!getInput("input", input))
    {
      std::cerr << name() << " could not get the value of input port 'input'";
      return BT::NodeStatus::FAILURE;
    }

    setOutput("output", input.pose);
    setOutput("frame_id", input.header.frame_id);
    setOutput("stamp", input.header.stamp);

    return BT::NodeStatus::SUCCESS;
  }
};

}  // namespace samxl::behavior
