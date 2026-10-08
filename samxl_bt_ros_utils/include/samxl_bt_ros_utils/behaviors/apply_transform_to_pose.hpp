#pragma once

#include <behaviortree_cpp/action_node.h>

#include <rclcpp/logging.hpp>
#include <stdexcept>
#include <string>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/LinearMath/Transform.h"
#include "tf2/convert.hpp"

namespace samxl
{

/**
 * Applies 'transform' to 'input_pose', following tf2 convention: a
 * TransformStamped with header.frame_id=A, child_frame_id=B converts a pose
 * FROM B INTO A. So 'input_pose.header.frame_id' must equal
 * 'transform.child_frame_id', strict match.
 * Throws std::invalid_argument otherwise.
 *
 * The output frame comes directly from 'transform.header.frame_id'.
 *
 */
inline geometry_msgs::msg::PoseStamped applyTransform(const geometry_msgs::msg::PoseStamped& input_pose,
                                                      const geometry_msgs::msg::TransformStamped& transform)
{
  if (input_pose.header.frame_id != transform.child_frame_id)
  {
    throw std::invalid_argument("applyTransform: 'input_pose' is in frame '" + input_pose.header.frame_id +
                                "', but 'transform' expects child_frame_id '" + transform.child_frame_id +
                                "'. These must match exactly - no automatic direction guessing.");
  }

  tf2::Transform tf_transform, tf_input;
  tf2::fromMsg(transform.transform, tf_transform);
  tf2::fromMsg(input_pose.pose, tf_input);

  const tf2::Transform tf_output = tf_transform * tf_input;

  geometry_msgs::msg::PoseStamped output;
  output.header.frame_id = transform.header.frame_id;
  output.header.stamp = input_pose.header.stamp;
  tf2::toMsg(tf_output, output.pose);

  RCLCPP_DEBUG_STREAM(rclcpp::get_logger("ApplyTransformToPose"),
                      "output_pose: frame_id='"
                          << output.header.frame_id << "', stamp=" << output.header.stamp.sec << "."
                          << output.header.stamp.nanosec << ", position=(" << output.pose.position.x << ", "
                          << output.pose.position.y << ", " << output.pose.position.z << ")" << ", orientation=("
                          << output.pose.orientation.x << ", " << output.pose.orientation.y << ", "
                          << output.pose.orientation.z << ", " << output.pose.orientation.w << ")");

  return output;
}

}  // namespace samxl

namespace samxl::behavior
{

class ApplyTransformToPose : public BT::SyncActionNode
{
public:
  ApplyTransformToPose(const std::string& name, const BT::NodeConfig& conf)
    : BT::SyncActionNode(name, conf)
  {
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<geometry_msgs::msg::PoseStamped>("input_pose", "Pose to be transformed"),
      BT::InputPort<geometry_msgs::msg::TransformStamped>("transform",
                                                          "Transform to apply. 'input_pose' must be in frame "
                                                          "'transform.child_frame_id'"),
      BT::OutputPort<geometry_msgs::msg::PoseStamped>("output_pose", "Resulting transformed pose"),
    };
  }

  BT::NodeStatus tick() override
  {
    geometry_msgs::msg::PoseStamped input_pose;
    auto result = getInput("input_pose", input_pose);
    if (!result)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("ApplyTransformToPose"),
                          name() << " could not get the value of input port 'input_pose':" << result.error());
      return BT::NodeStatus::FAILURE;
    }

    geometry_msgs::msg::TransformStamped transform;
    if (!getInput("transform", transform))
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("ApplyTransformToPose"), name() << " could not get the value of input "
                                                                                "port 'transform'");
      return BT::NodeStatus::FAILURE;
    }

    try
    {
      setOutput("output_pose", samxl::applyTransform(input_pose, transform));
    }
    catch (const std::exception& e)
    {
      RCLCPP_ERROR_STREAM(rclcpp::get_logger("ApplyTransformToPose"), name() << ": " << e.what());
      return BT::NodeStatus::FAILURE;
    }

    return BT::NodeStatus::SUCCESS;
  }
};

}  // namespace samxl::behavior