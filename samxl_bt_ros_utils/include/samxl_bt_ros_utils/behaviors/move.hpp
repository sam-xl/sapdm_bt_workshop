#pragma once

#include <behaviortree_ros2/bt_action_node.hpp>
#include <control_msgs/action/follow_joint_trajectory.hpp>

namespace samxl::behavior
{
class ExecuteFJTrajectory : public BT::RosActionNode<control_msgs::action::FollowJointTrajectory>
{
public:
  ExecuteFJTrajectory(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosActionNode<control_msgs::action::FollowJointTrajectory>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({
        BT::InputPort<trajectory_msgs::msg::JointTrajectory>("trajectory"),
        BT::InputPort<std::string>("joint_name_prefix", "", "Prefix to add to joint names before sending to robot"),
        BT::InputPort<bool>("clear_positions", false,
                            "Whether or not to clear/empty the positions field in the joint_trajectory"),
        BT::InputPort<bool>("clear_velocity", false,
                            "Whether or not to clear/empty the velocities field in the joint_trajectory"),
        BT::InputPort<bool>("clear_effort", true, "Whether or not to clear/empty the effort in the joint_trajectory"),
    });
  }

  bool setGoal(Goal& goal) override
  {
    // get "trajectory" from the Input port
    auto result = getInput("trajectory", goal.trajectory);
    if (!result)
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " could not get input port 'trajectory' value: " << result.error());
      return false;
    }
    const std::string joint_name_prefix = getInput<std::string>("joint_name_prefix").value_or("");
    if (!joint_name_prefix.empty())
    {
      for (auto& joint_name : goal.trajectory.joint_names)
      {
        joint_name = joint_name_prefix + joint_name;
      }
    }
    for (auto& point : goal.trajectory.points)
    {
      if (getInput<bool>("clear_positions").value_or(false))
        point.positions.clear();
      if (getInput<bool>("clear_velocities").value_or(false))
        point.velocities.clear();
      if (getInput<bool>("clear_effort").value_or(false))
        point.effort.clear();
    }
    // return true, if we were able to set the goal correctly.
    return true;
  }

  BT::NodeStatus onResultReceived(const WrappedResult& wr) override
  {
    const int success = wr.result->error_code;
    if (success != 0)
    {
      RCLCPP_INFO(logger(), "FollowJointTrajectory failed: %s", wr.result->error_string.c_str());
      return BT::NodeStatus::FAILURE;
    }
    else
    {
      return BT::NodeStatus::SUCCESS;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ActionNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error " << name() << " [" << action_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};
}  // namespace samxl::behavior
