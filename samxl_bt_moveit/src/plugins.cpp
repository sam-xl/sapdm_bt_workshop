#include <behaviortree_ros2/plugins.hpp>
#include <samxl_bt_moveit/behaviors/plan_cartesian_path.hpp>

BT_REGISTER_ROS_NODES(factory, params)
{
  factory.registerNodeType<samxl::behavior::PlanCartesianNode>("PlanCartesian", params);
  factory.registerNodeType<samxl::behavior::PlanJointStateNode>("PlanJointState", params);
  factory.registerNodeType<samxl::behavior::PlanPoseNode>("PlanPose", params);
}
