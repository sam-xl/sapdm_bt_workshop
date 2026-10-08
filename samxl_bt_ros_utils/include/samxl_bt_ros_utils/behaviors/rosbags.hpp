#pragma once

#include <behaviortree_ros2/bt_service_node.hpp>
#include <sstream>

#include "rosbag2_interfaces/srv/is_discovery_running.hpp"
#include "rosbag2_interfaces/srv/is_paused.hpp"
#include "rosbag2_interfaces/srv/pause.hpp"
#include "rosbag2_interfaces/srv/record.hpp"
#include "rosbag2_interfaces/srv/resume.hpp"
#include "rosbag2_interfaces/srv/start_discovery.hpp"
#include "rosbag2_interfaces/srv/stop.hpp"
#include "rosbag2_interfaces/srv/stop_discovery.hpp"
#include "rosbag2_interfaces/srv/toggle_paused.hpp"
namespace samxl::behavior
{

class RosbagRecord : public BT::RosServiceNode<rosbag2_interfaces::srv::Record>
{
public:
  explicit RosbagRecord(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<rosbag2_interfaces::srv::Record>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({
        BT::InputPort<std::string>("uri", "", "Optional path where the new folder for recording will be created."),
        BT::InputPort<bool>("expand_date_time_specifiers", true,
                            "Whether to expand specifiers such as %y to the current year. See documentation for C++ "
                            "std::put_time() for details on available specifiers."),
    });
  }

  bool setRequest(Request::SharedPtr& request) override
  {
    std::string raw_uri;
    const auto success = this->getInput("uri", raw_uri);
    if (!success.has_value())
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " could not get input port 'uri' value");
      return false;
    }
    const bool expand_date_time_specifiers = this->getInput<bool>("expand_date_time_specifiers").value_or(false);
    if (!expand_date_time_specifiers)
    {
      // Don't expand date time specifiers
      request->uri = raw_uri;
      return true;
    }

    // Get current date/time for expanding specifiers
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    // Expand date/time specification
    std::stringstream ss;
    ss << std::put_time(&tm, raw_uri.c_str());
    request->uri = ss.str();

    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->return_code == 0)
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      RCLCPP_ERROR_STREAM(logger(),
                          "Could not start recording rosbag on '" << service_name_ << "': " << response->error_string);
      return BT::NodeStatus::FAILURE;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

class RosbagIsPaused : public BT::RosServiceNode<rosbag2_interfaces::srv::IsPaused>
{
public:
  explicit RosbagIsPaused(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<rosbag2_interfaces::srv::IsPaused>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({});
  }

  bool setRequest(Request::SharedPtr&) override
  {
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->paused)
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      return BT::NodeStatus::FAILURE;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

class RosbagIsDiscoveryRunning : public BT::RosServiceNode<rosbag2_interfaces::srv::IsDiscoveryRunning>
{
public:
  explicit RosbagIsDiscoveryRunning(const std::string& name, const BT::NodeConfig& conf,
                                    const BT::RosNodeParams& params)
    : BT::RosServiceNode<rosbag2_interfaces::srv::IsDiscoveryRunning>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({});
  }

  bool setRequest(Request::SharedPtr&) override
  {
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->running)
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      return BT::NodeStatus::FAILURE;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

class RosbagStartDiscovery : public BT::RosServiceNode<rosbag2_interfaces::srv::StartDiscovery>
{
public:
  explicit RosbagStartDiscovery(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<rosbag2_interfaces::srv::StartDiscovery>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({});
  }

  bool setRequest(Request::SharedPtr&) override
  {
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->return_code == 0)
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      RCLCPP_ERROR_STREAM(logger(), "RosbagStartDiscovery failed: " << response->error_string);
      return BT::NodeStatus::FAILURE;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

class RosbagStopDiscovery : public BT::RosServiceNode<rosbag2_interfaces::srv::StopDiscovery>
{
public:
  explicit RosbagStopDiscovery(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<rosbag2_interfaces::srv::StopDiscovery>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({});
  }

  bool setRequest(Request::SharedPtr&) override
  {
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->return_code == 0)
    {
      return BT::NodeStatus::SUCCESS;
    }
    else
    {
      RCLCPP_ERROR_STREAM(logger(), "RosbagStopDiscovery failed: " << response->error_string);
      return BT::NodeStatus::FAILURE;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

/**
 * @brief A RosServiceNode for use with trivial services.
 *
 * A trivial service is a service that has no entries in the request or response parts of the service.
 * In other words the `.srv` file only contains "---".
 * ```
 *
 * @param ServiceT The service type, for example `rosbag2_interfaces/srv/Pause`
 */
template <typename ServiceT>
class TrivialServiceNode : public BT::RosServiceNode<ServiceT>
{
public:
  explicit TrivialServiceNode(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<ServiceT>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return BT::RosServiceNode<ServiceT>::providedBasicPorts({});
  }

  bool setRequest(typename ServiceT::Request::SharedPtr&) override
  {
    return true;
  }

  BT::NodeStatus onResponseReceived(const typename ServiceT::Response::SharedPtr&) override
  {
    // Service returns nothing, so we must assume it worked
    return BT::NodeStatus::SUCCESS;
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(this->logger(),
                        "Error: " << this->name() << " [" << this->service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

using RosbagResume = TrivialServiceNode<rosbag2_interfaces::srv::Resume>;
using RosbagTogglePaused = TrivialServiceNode<rosbag2_interfaces::srv::TogglePaused>;
using RosbagStop = TrivialServiceNode<rosbag2_interfaces::srv::Stop>;
using RosbagPause = TrivialServiceNode<rosbag2_interfaces::srv::Pause>;

}  // namespace samxl::behavior
