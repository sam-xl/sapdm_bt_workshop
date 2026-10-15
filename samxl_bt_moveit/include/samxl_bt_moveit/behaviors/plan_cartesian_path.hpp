#pragma once

#include <behaviortree_ros2/bt_service_node.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <moveit_msgs/msg/constraints.hpp>
#include <moveit_msgs/msg/joint_constraint.hpp>
#include <moveit_msgs/msg/orientation_constraint.hpp>
#include <moveit_msgs/msg/position_constraint.hpp>
#include <shape_msgs/msg/solid_primitive.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <moveit_msgs/srv/get_cartesian_path.hpp>
#include <moveit_msgs/srv/get_motion_plan.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

// Necessary so we can load pose arrays from yaml
#include <samxl_bt_ros_utils/conversions/geometry_msgs.hpp>

namespace samxl::behavior
{
class PlanCartesianNode : public BT::RosServiceNode<moveit_msgs::srv::GetCartesianPath>
{
public:
  explicit PlanCartesianNode(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<moveit_msgs::srv::GetCartesianPath>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({
        BT::InputPort<std::string>("group_name"),
        BT::InputPort<std::string>("link_name"),
        BT::InputPort<geometry_msgs::msg::PoseArray>("waypoints"),
        BT::InputPort<double>("max_step"),
        BT::InputPort<double>("jump_threshold"),
        BT::InputPort<double>("max_velocity_scaling_factor"),
        BT::InputPort<double>("max_acceleration_scaling_factor"),
        BT::OutputPort<trajectory_msgs::msg::JointTrajectory>("trajectory"),
    });
  }

  bool setRequest(Request::SharedPtr& request) override
  {
    // use input ports to set GetCartesianPath Request
    getInput("group_name", request->group_name);
    getInput("link_name", request->link_name);
    getInput("max_step", request->max_step);
    getInput("jump_threshold", request->jump_threshold);
    // getInput("max_velocity_scaling_factor", request->max_velocity_scaling_factor);
    // getInput("max_acceleration_scaling_factor", request->max_acceleration_scaling_factor);

    geometry_msgs::msg::PoseArray waypoints;
    auto waypoints_result = getInput("waypoints", waypoints);
    if (!waypoints_result || waypoints.poses.empty())
    {
      RCLCPP_ERROR_STREAM(logger(), "No valid waypoints: " << (waypoints_result ? "empty pose list" : waypoints_result.error()));
      return false;
    }
    request->header = waypoints.header;
    request->waypoints = waypoints.poses;
    request->start_state.is_diff = true;  // plan from the robot's current state
    request->avoid_collisions = true;

    std::cout << "Loaded " << request->waypoints.size() << " waypoints" << std::endl;

    // must return true if we are ready to send the request
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    int success = response->error_code.val;
    if (success != 1)
    {
      RCLCPP_INFO(logger(), "GetCartesianPath failed.");
      return BT::NodeStatus::FAILURE;
    }
    if (response->fraction < 0.99)
    {
      RCLCPP_ERROR(logger(), "Cartesian path only %.1f%% achievable; refusing to execute partial path.",
                   response->fraction * 100.0);
      return BT::NodeStatus::FAILURE;
    }
    else
    {
      setOutput("trajectory", response->solution.joint_trajectory);
      return BT::NodeStatus::SUCCESS;
    }
  }

  virtual BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

class PlanJointStateNode : public BT::RosServiceNode<moveit_msgs::srv::GetMotionPlan>
{
public:
  explicit PlanJointStateNode(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<moveit_msgs::srv::GetMotionPlan>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({ BT::InputPort<std::string>("group_name"),
                                BT::InputPort<sensor_msgs::msg::JointState>("joint_state"),
                                BT::OutputPort<trajectory_msgs::msg::JointTrajectory>("trajectory") });
  }

  bool setRequest(Request::SharedPtr& request) override
  {
    std::string group_name;
    sensor_msgs::msg::JointState joint_state;
    if (!getInput("group_name", group_name) || !getInput("joint_state", joint_state) || joint_state.name.empty() ||
        joint_state.name.size() != joint_state.position.size())
    {
      RCLCPP_ERROR(logger(), "A planning group and a joint state with matching names and positions are required.");
      return false;
    }

    auto& motion_request = request->motion_plan_request;
    motion_request.group_name = group_name;
    moveit_msgs::msg::Constraints goal;
    goal.joint_constraints.reserve(joint_state.name.size());
    for (size_t i = 0; i < joint_state.name.size(); ++i)
    {
      moveit_msgs::msg::JointConstraint constraint;
      constraint.joint_name = joint_state.name[i];
      constraint.position = joint_state.position[i];
      constraint.tolerance_above = 0.001;
      constraint.tolerance_below = 0.001;
      constraint.weight = 1.0;
      goal.joint_constraints.push_back(std::move(constraint));
    }
    motion_request.goal_constraints.push_back(std::move(goal));
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->motion_plan_response.error_code.val != 1)
    {
      RCLCPP_INFO(logger(), "GetMotionPlan failed.");
      return BT::NodeStatus::FAILURE;
    }
    setOutput("trajectory", response->motion_plan_response.trajectory.joint_trajectory);
    return BT::NodeStatus::SUCCESS;
  }

  BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};

// Plans a free-space (non-straight-line) motion of a link to a target pose
class PlanPoseNode : public BT::RosServiceNode<moveit_msgs::srv::GetMotionPlan>
{
public:
  explicit PlanPoseNode(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosServiceNode<moveit_msgs::srv::GetMotionPlan>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return providedBasicPorts({ BT::InputPort<std::string>("group_name"),
                                BT::InputPort<std::string>("link_name"),
                                BT::InputPort<geometry_msgs::msg::PoseStamped>("pose"),
                                BT::InputPort<double>("position_tolerance", 0.001, "metres"),
                                BT::InputPort<double>("orientation_tolerance", 0.01, "radians"),
                                BT::InputPort<double>("max_velocity_scaling_factor", 0.5, ""),
                                BT::InputPort<double>("max_acceleration_scaling_factor", 0.5, ""),
                                BT::OutputPort<trajectory_msgs::msg::JointTrajectory>("trajectory") });
  }

  bool setRequest(Request::SharedPtr& request) override
  {
    std::string group_name, link_name;
    geometry_msgs::msg::PoseStamped pose;
    double pos_tol = 0.001, ori_tol = 0.01, vel = 0.5, acc = 0.5;
    if (!getInput("group_name", group_name) || !getInput("link_name", link_name) || !getInput("pose", pose))
    {
      RCLCPP_ERROR(logger(), "group_name, link_name and pose are required.");
      return false;
    }
    getInput("position_tolerance", pos_tol);
    getInput("orientation_tolerance", ori_tol);
    getInput("max_velocity_scaling_factor", vel);
    getInput("max_acceleration_scaling_factor", acc);

    auto& req = request->motion_plan_request;
    req.group_name = group_name;
    req.start_state.is_diff = true;
    req.max_velocity_scaling_factor = vel;
    req.max_acceleration_scaling_factor = acc;
    req.num_planning_attempts = 5;
    req.allowed_planning_time = 5.0;

    moveit_msgs::msg::PositionConstraint position;
    position.header = pose.header;
    position.link_name = link_name;
    shape_msgs::msg::SolidPrimitive sphere;
    sphere.type = shape_msgs::msg::SolidPrimitive::SPHERE;
    sphere.dimensions = { pos_tol };
    position.constraint_region.primitives.push_back(sphere);
    position.constraint_region.primitive_poses.push_back(pose.pose);
    position.weight = 1.0;

    moveit_msgs::msg::OrientationConstraint orientation;
    orientation.header = pose.header;
    orientation.link_name = link_name;
    orientation.orientation = pose.pose.orientation;
    orientation.absolute_x_axis_tolerance = ori_tol;
    orientation.absolute_y_axis_tolerance = ori_tol;
    orientation.absolute_z_axis_tolerance = ori_tol;
    orientation.weight = 1.0;

    moveit_msgs::msg::Constraints goal;
    goal.position_constraints.push_back(std::move(position));
    goal.orientation_constraints.push_back(std::move(orientation));
    req.goal_constraints.push_back(std::move(goal));
    return true;
  }

  BT::NodeStatus onResponseReceived(const Response::SharedPtr& response) override
  {
    if (response->motion_plan_response.error_code.val != 1)
    {
      RCLCPP_ERROR(logger(), "Pose planning failed with error code %d.", response->motion_plan_response.error_code.val);
      return BT::NodeStatus::FAILURE;
    }
    setOutput("trajectory", response->motion_plan_response.trajectory.joint_trajectory);
    return BT::NodeStatus::SUCCESS;
  }

  BT::NodeStatus onFailure(BT::ServiceNodeErrorCode error) override
  {
    RCLCPP_ERROR_STREAM(logger(), "Error: " << name() << " [" << service_name_ << "]: " << BT::toStr(error));
    return BT::NodeStatus::FAILURE;
  }
};
}  // namespace samxl::behavior
