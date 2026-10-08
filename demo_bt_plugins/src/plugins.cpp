#include <behaviortree_ros2/plugins.hpp>
#include <demo_bt_plugins/behaviors/conveyor_belt_control.hpp>

BT_REGISTER_ROS_NODES(factory, params)
{
  factory.registerNodeType<samxl::behavior::ConveyorBeltControl>("ConveyorBeltControl", params);
}
