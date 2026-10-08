#pragma once

#include <behaviortree_ros2/bt_service_node.hpp>

#include "std_srvs/srv/set_bool.hpp"

namespace samxl::behavior
{
class SetBool : public BT::RosServiceNode<std_srvs::srv::SetBool>
{
public:
  SetBool(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<std_srvs::srv::SetBool>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({ BT::InputPort<bool>("value") });
  }

  bool setRequest(Request::SharedPtr& request) override
  {
    getInput("value", request->data);
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
      RCLCPP_ERROR_STREAM(logger(), service_name_ << " service failed: " << response->message.c_str());
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