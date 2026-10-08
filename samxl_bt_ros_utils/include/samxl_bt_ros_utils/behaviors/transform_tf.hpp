#pragma once

#include <behaviortree_ros2/bt_action_node.hpp>
#include <rclcpp/logging.hpp>
#include <string>
#include <tf2_ros/buffer_interface.hpp>

// Necessart for Time YAML conversion
#include <samxl_bt_ros_utils/conversions/builtin_interfaces.hpp>

#include "builtin_interfaces/msg/duration.hpp"
#include "builtin_interfaces/msg/time.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/convert.hpp"
#include "tf2_msgs/action/lookup_transform.hpp"
#include "tf2_msgs/msg/tf2_error.hpp"

// Transformation types
#include <samxl_bt_ros_utils/conversions/geometry_msgs.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "geometry_msgs/msg/pose_stamped.hpp"

namespace samxl::behavior
{

inline const std::string tf2_error_name(const tf2_msgs::msg::TF2Error::_error_type error)
{
  switch (error)
  {
    case tf2_msgs::msg::TF2Error::NO_ERROR:
      return "NO ERROR";
    case tf2_msgs::msg::TF2Error::LOOKUP_ERROR:
      return "LOOKUP ERROR";
    case tf2_msgs::msg::TF2Error::CONNECTIVITY_ERROR:
      return "CONNECTIVITY ERROR";
    case tf2_msgs::msg::TF2Error::EXTRAPOLATION_ERROR:
      return "EXTRAPOLATION ERROR";
    case tf2_msgs::msg::TF2Error::INVALID_ARGUMENT_ERROR:
      return "INVALID ARGUMENT ERROR";
    case tf2_msgs::msg::TF2Error::TIMEOUT_ERROR:
      return "TIMEOUT ERROR";
    case tf2_msgs::msg::TF2Error::TRANSFORM_ERROR:
      return "TRANSFORM ERROR";
    default:
      return "UNKNOWN ERROR";
  }
}

/** Base class to interact with a tf2_ros buffer_server */
class GetTransformClientBase : public BT::RosActionNode<tf2_msgs::action::LookupTransform>
{
public:
  explicit GetTransformClientBase(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosActionNode<tf2_msgs::action::LookupTransform>(name, conf, params)
  {
  }

  static BT::PortsList providedBasicPorts(BT::PortsList addition)
  {
    BT::PortsList ports = {
      BT::InputPort<std::string>("target_frame", "The frame to which data should be transformed"),
      // TODO(Jasper): Make type input nicer (currently, YAML)
      BT::InputPort<builtin_interfaces::msg::Time>("target_time", builtin_interfaces::msg::Time(),
                                                   "The time to which the data should be transformed. (0 will get "
                                                   "the latest)"),
      BT::InputPort<std::string>("fixed_frame", "",
                                 "The frame in which to treat the transform as constant in time. Optional: if "
                                 "omitted 'target_frame' is used instead"),
      BT::InputPort<builtin_interfaces::msg::Duration>("timeout", builtin_interfaces::msg::Duration(),
                                                       "Time to wait on transform"),
    };
    ports.insert(addition.begin(), addition.end());
    return BT::RosActionNode<ActionType>::providedBasicPorts(ports);
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({});
  }

  virtual bool setSourceFrame(Goal& goal) = 0;

  bool setGoal(Goal& goal) override final
  {
    if (!getInput("target_frame", goal.target_frame))
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " could not get the value of input port 'target_frame'");
      return false;
    }

    if (!getInput("target_time", goal.target_time))
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " could not get the value of input port 'target_time'");
      return false;
    }

    if (!getInput("fixed_frame", goal.fixed_frame))
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " could not get the value of input port 'fixed_frame'");
      return false;
    }

    if (goal.fixed_frame.empty())
    {
      RCLCPP_WARN_STREAM(logger(), name() << " has default empty 'fixed_frame' value, defaulting to 'target_frame'");
      goal.fixed_frame = goal.target_frame;
    }

    if (!getInput("timeout", goal.timeout))
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " could not get the value of input port 'timeout'");
      return false;
    }

    if (!setSourceFrame(goal))
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " failed to source frame");
      return false;
    }

    return true;
  }

  virtual BT::NodeStatus onTransformReceived(const geometry_msgs::msg::TransformStamped& transform) = 0;

  BT::NodeStatus onResultReceived(const WrappedResult& wr) override final
  {
    const auto result = wr.result;
    if (result->error.error != tf2_msgs::msg::TF2Error::NO_ERROR)
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " failed to retrieve transform [" << tf2_error_name(result->error.error)
                                           << "]: " << result->error.error_string);
      return BT::NodeStatus::FAILURE;
    }

    return onTransformReceived(result->transform);
  }

  virtual BT::NodeStatus onFailure(BT::ActionNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error " << name() << " [" << action_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

// NOTE: Possibility for future: GetTransformClient to retrieve the transform

template <typename MessageT>
class TransformStamped : public GetTransformClientBase
{
public:
  explicit TransformStamped(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : GetTransformClientBase(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({
        BT::InputPort<MessageT>("input", "Message to be transformed"),
        BT::OutputPort<MessageT>("output", "Transformed message"),
    });
  }

  bool setSourceFrame(Goal& goal) override
  {
    if (!getInput("input", this->input_))
    {
      RCLCPP_ERROR_STREAM(logger(), name() << " could not get the value of input port 'input'");
      return false;
    }

    goal.source_frame = tf2::getFrameId(input_);
    goal.source_time = tf2_ros::toMsg(tf2::getTimestamp(input_));

    return true;
  }

  BT::NodeStatus onTransformReceived(const geometry_msgs::msg::TransformStamped& transform) override
  {
    MessageT output;
    tf2::doTransform(input_, output, transform);

    setOutput("output", output);
    return BT::NodeStatus::SUCCESS;
  }

private:
  MessageT input_;
};

template class TransformStamped<geometry_msgs::msg::PoseStamped>;
using TransformPoseStamped = TransformStamped<geometry_msgs::msg::PoseStamped>;

}  // namespace samxl::behavior
