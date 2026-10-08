#pragma once

#include <behaviortree_ros2/bt_service_node.hpp>
#include <rclcpp/duration.hpp>
#include <samxl_bt_ros_utils/conversions/builtin_interfaces.hpp>

#include "builtin_interfaces/msg/duration.hpp"
#include "std_srvs/srv/trigger.hpp"

namespace samxl::behavior
{
class TriggerService : public BT::RosServiceNode<std_srvs::srv::Trigger>
{
public:
  explicit TriggerService(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : RosServiceNode<std_srvs::srv::Trigger>(name, conf, withTimeoutPort(name, conf, params))
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts(
        { BT::InputPort<builtin_interfaces::msg::Duration>("timeout", builtin_interfaces::msg::Duration(),
                                                           "Response timeout (e.g. `500ms`, `15s`), zero for the "
                                                           "server's `ros_plugins_timeout`. Must be a literal, "
                                                           "not a blackboard entry") });
  }

  bool setRequest(Request::SharedPtr&) override
  {
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
      RCLCPP_ERROR(logger(), "Trigger service failed: %s", response->message.c_str());
      return BT::NodeStatus::FAILURE;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }

private:
  /**
   * Applies a non-zero `timeout` port to the params forwarded to the base class.
   *
   * The response timeout of `BT::RosServiceNode` is fixed at construction, so the port cannot be read at tick time
   * and only literal values are supported.
   */
  static BT::RosNodeParams withTimeoutPort(const std::string& name, const BT::NodeConfig& conf,
                                           BT::RosNodeParams params)
  {
    const auto port_it = conf.input_ports.find("timeout");
    if (port_it == conf.input_ports.end() || port_it->second.empty())
    {
      return params;
    }
    if (isBlackboardPointer(port_it->second))
    {
      throw BT::RuntimeError("[", name, "] port 'timeout' must be a literal value, not a blackboard entry (",
                             port_it->second, ")");
    }
    const auto timeout = rclcpp::Duration(BT::convertFromString<builtin_interfaces::msg::Duration>(port_it->second))
                             .to_chrono<std::chrono::milliseconds>();
    if (timeout > std::chrono::milliseconds::zero())
    {
      params.server_timeout = timeout;
    }
    return params;
  }
};
}  // namespace samxl::behavior
