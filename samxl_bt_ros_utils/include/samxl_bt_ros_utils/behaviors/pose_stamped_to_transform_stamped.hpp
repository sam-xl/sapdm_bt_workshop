#pragma once

#include <behaviortree_cpp/action_node.h>

#include <rclcpp/logging.hpp>
#include <string>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/LinearMath/Transform.h"
#include "tf2/convert.hpp"

namespace samxl
{

/**
 * Converts a PoseStamped into a TransformStamped.
 *
 * 'child_frame_id' always names the frame the pose itself describes,
 * regardless of 'invert':
 *
 *   invert == false:  pose.header.frame_id -> child_frame_id
 *                      (output.header.frame_id = pose.header.frame_id,
 *                       output.child_frame_id  = child_frame_id,
 *                       output.transform        = pose, as-is)
 *
 *   invert == true:   child_frame_id -> pose.header.frame_id
 *                      (parent/child swap, not just a numeric flip -
 *                       output.header.frame_id = child_frame_id,
 *                       output.child_frame_id  = pose.header.frame_id,
 *                       output.transform        = inverse of pose)
 *
 * e.g. a calibration pose with header.frame_id="cam" describing where a
 * eef sits gives cam->eef uninverted, eef->cam inverted.
 *
 */
inline geometry_msgs::msg::TransformStamped poseStampedToTransformStamped(const geometry_msgs::msg::PoseStamped& pose,
                                                                          const std::string& child_frame_id,
                                                                          bool invert)
{
  tf2::Transform tf_pose;
  tf2::fromMsg(pose.pose, tf_pose);

  geometry_msgs::msg::TransformStamped output;
  output.header.stamp = pose.header.stamp;

  if (!invert)
  {
    output.header.frame_id = pose.header.frame_id;
    output.child_frame_id = child_frame_id;
    output.transform = tf2::toMsg(tf_pose);
  }
  else
  {
    output.header.frame_id = child_frame_id;
    output.child_frame_id = pose.header.frame_id;
    output.transform = tf2::toMsg(tf_pose.inverse());
  }

  RCLCPP_DEBUG_STREAM(rclcpp::get_logger("PoseStampedToTransformStamped"),
                      "output_transform: frame_id='"
                          << output.header.frame_id << "', stamp=" << output.header.stamp.sec << "."
                          << output.header.stamp.nanosec << ", position=(" << output.transform.translation.x << ", "
                          << output.transform.translation.y << ", " << output.transform.translation.z << ")"
                          << ", orientation=(" << output.transform.rotation.x << ", " << output.transform.rotation.y
                          << ", " << output.transform.rotation.z << ", " << output.transform.rotation.w << ")");

  return output;
}

}  // namespace samxl

namespace samxl::behavior
{

class PoseStampedToTransformStamped : public BT::SyncActionNode
{
public:
  PoseStampedToTransformStamped(const std::string& name, const BT::NodeConfig& conf)
    : BT::SyncActionNode(name, conf)
  {
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<geometry_msgs::msg::PoseStamped>("input_pose", "Pose to convert into a transform"),
      BT::InputPort<std::string>("child_frame_id", "Name of the frame 'input_pose' describes"),
      BT::InputPort<bool>("invert", false,
                          "If true, swap parent/child and invert the transform (e.g. eef->cam becomes cam->eef)"),
      BT::OutputPort<geometry_msgs::msg::TransformStamped>("output", "Resulting transform"),
    };
  }

  BT::NodeStatus tick() override
  {
    geometry_msgs::msg::PoseStamped input_pose;
    auto result = getInput("input_pose", input_pose);
    if (!result)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("PoseStampedToTransformStamped"),
                          name() << " could not get the value of input port 'input_pose':" << result.error());
      return BT::NodeStatus::FAILURE;
    }

    std::string child_frame_id;
    if (!getInput("child_frame_id", child_frame_id))
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("PoseStampedToTransformStamped"), name() << " could not get the value of "
                                                                                         "input port 'child_frame_id'");
      return BT::NodeStatus::FAILURE;
    }

    bool invert = false;
    if (!getInput("invert", invert))
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("PoseStampedToTransformStamped"), name() << " could not get the value of "
                                                                                         "input port 'invert'");
      return BT::NodeStatus::FAILURE;
    }

    try
    {
      setOutput("output", samxl::poseStampedToTransformStamped(input_pose, child_frame_id, invert));
    }
    catch (const std::exception& e)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("PoseStampedToTransformStamped"), name() << ": " << e.what());
      return BT::NodeStatus::FAILURE;
    }

    return BT::NodeStatus::SUCCESS;
  }
};

}  // namespace samxl::behavior