#pragma once

#include <behaviortree_ros2/bt_service_node.hpp>

#include "conveyorbelt_msgs/srv/conveyor_belt_control.hpp"

namespace samxl::behavior
{
class ConveyorBeltControl : public BT::RosServiceNode<conveyorbelt_msgs::srv::ConveyorBeltControl>
{
public:
  ConveyorBeltControl(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<conveyorbelt_msgs::srv::ConveyorBeltControl>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({ BT::InputPort<double>("power") });
  }

  bool setRequest(Request::SharedPtr& request) override
  {
    getInput("power", request->power);
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->success)
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      RCLCPP_ERROR_STREAM(logger(), service_name_ << " service failed");
      return BT::NodeStatus::FAILURE;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};
}  // namespace samxl::behavior
