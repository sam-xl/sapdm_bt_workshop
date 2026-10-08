#include <behaviortree_ros2/plugins.hpp>
#include <samxl_bt_ros_utils/behaviors/apply_transform_to_pose.hpp>
#include <samxl_bt_ros_utils/behaviors/date_string.hpp>
#include <samxl_bt_ros_utils/behaviors/load_text_file.hpp>
#include <samxl_bt_ros_utils/behaviors/marker_publisher.hpp>
#include <samxl_bt_ros_utils/behaviors/move.hpp>
#include <samxl_bt_ros_utils/behaviors/pose_stamped_to_transform_stamped.hpp>
#include <samxl_bt_ros_utils/behaviors/printers/builtin_interfaces.hpp>
#include <samxl_bt_ros_utils/behaviors/printers/geometry_msgs.hpp>
#include <samxl_bt_ros_utils/behaviors/printers/sensor_msgs.hpp>
#include <samxl_bt_ros_utils/behaviors/printers/std_msgs.hpp>
#include <samxl_bt_ros_utils/behaviors/publishers/geometry_msgs.hpp>
#include <samxl_bt_ros_utils/behaviors/set_bool.hpp>
#include <samxl_bt_ros_utils/behaviors/subscribers/std_msgs.hpp>
#include <samxl_bt_ros_utils/behaviors/timer_decorator.hpp>
#include <samxl_bt_ros_utils/behaviors/transform_tf.hpp>
#include <samxl_bt_ros_utils/behaviors/trigger_service.hpp>
#include <samxl_bt_ros_utils/behaviors/unstamper.hpp>
#include <samxl_bt_ros_utils/behaviors/write_text_file.hpp>

BT_REGISTER_ROS_NODES(factory, params)
{
  factory.registerNodeType<samxl::behavior::ApplyTransformToPose>("ApplyTransformToPose");
  factory.registerNodeType<samxl::behavior::DateString>("DateString");
  factory.registerNodeType<samxl::behavior::ExecuteFJTrajectory>("ExecuteFJT", params);
  factory.registerNodeType<samxl::behavior::TriggerService>("Trigger", params);
  factory.registerNodeType<samxl::behavior::LoadTextFile>("LoadTextFile");
  factory.registerNodeType<samxl::behavior::WriteTextFile>("WriteTextFile");
  factory.registerNodeType<samxl::behavior::PoseStampedToTransformStamped>("PoseStampedToTransformStamped");
  factory.registerNodeType<samxl::behavior::SetBool>("SetBool", params);

  // Publishers - geometry_msgs
  factory.registerNodeType<samxl::behavior::PublishWrenchStamped>("PublishWrenchStamped", params);
  factory.registerNodeType<samxl::behavior::PublishPose>("PublishPose", params);
  factory.registerNodeType<samxl::behavior::PublishPoseArray>("PublishPoseArray", params);
  factory.registerNodeType<samxl::behavior::PublishPoseStamped>("PublishPoseStamped", params);

  // Subscribers - std_msgs
  factory.registerNodeType<samxl::behavior::SubscribeBool>("SubscribeBool", params);
  factory.registerNodeType<samxl::behavior::SubscribeString>("SubscribeString", params);

  factory.registerNodeType<samxl::behavior::PublishMeshMarker>("PublishMeshMarker", params);
  // TF Transform
  factory.registerNodeType<samxl::behavior::TransformPoseStamped>("TransformPoseStamped", params);

  // Unstampers
  factory.registerNodeType<samxl::behavior::UnstampPose>("UnstampPose");

  // Decorators
  factory.registerNodeType<samxl::behavior::TimerDecorator>("TimerDecorator");

  // Printers - builtin_interfaces
  factory.registerNodeType<samxl::behavior::PrintDuration>("PrintDuration");
  factory.registerNodeType<samxl::behavior::PrintTime>("PrintTime");

  // Printers - geometry_msgs
  factory.registerNodeType<samxl::behavior::PrintPoint>("PrintPoint");
  factory.registerNodeType<samxl::behavior::PrintPose>("PrintPose");
  factory.registerNodeType<samxl::behavior::PrintPoseArray>("PrintPoseArray");
  factory.registerNodeType<samxl::behavior::PrintPoseStamped>("PrintPoseStamped");
  factory.registerNodeType<samxl::behavior::PrintQuaternion>("PrintQuaternion");
  factory.registerNodeType<samxl::behavior::PrintVector3>("PrintVector3");
  factory.registerNodeType<samxl::behavior::PrintWrench>("PrintWrench");
  factory.registerNodeType<samxl::behavior::PrintWrenchStamped>("PrintWrenchStamped");

  // Printers - sensor_msgs
  factory.registerNodeType<samxl::behavior::PrintJointState>("PrintJointState");

  // Printers - std_msgs
  factory.registerNodeType<samxl::behavior::PrintHeader>("PrintHeader");
}
