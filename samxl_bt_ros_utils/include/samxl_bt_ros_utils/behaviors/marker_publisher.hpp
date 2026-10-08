#pragma once

#include <behaviortree_ros2/bt_topic_pub_node.hpp>
#include <resource_retriever/retriever.hpp>
#include <samxl_bt_ros_utils/conversions/geometry_msgs.hpp>
#include <visualization_msgs/msg/marker.hpp>

namespace samxl::behavior
{

class PublishMeshMarker : public BT::RosTopicPubNode<visualization_msgs::msg::Marker>
{
public:
  PublishMeshMarker(const std::string& name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosTopicPubNode<visualization_msgs::msg::Marker>(name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    // the msg takes also text and colors inputs for text markers and for setting colors. the first version of this
    // publisher will only cover important inputs for mesh display additional checks can be added later for when the
    // use-case arises for these scenarios
    return providedBasicPorts({
        BT::InputPort<geometry_msgs::msg::PoseStamped>("pose", "the pose to publish the object in"),
        BT::InputPort<std::string>("ns", "",
                                   "Namespace to place this object in... used in conjunction with id to create a "
                                   "unique name for the object"),
        BT::InputPort<uint>("id", 0,
                            "object ID useful in conjunction with the namespace for manipulating and deleting the "
                            "object later. Defaults to 0."),
        BT::InputPort<int>("action", "0",
                           "0 add/modify an object, 1 (deprecated), 2 deletes an object, 3 deletes all objects"),
        BT::InputPort<geometry_msgs::msg::Vector3>("scale",
                                                   geometry_msgs::build<geometry_msgs::msg::Vector3>().x(1).y(1).z(1),
                                                   "scale of the object. Defaults to 1,1,1"),
        BT::InputPort<std::string>("mesh_file_path",
                                   "mesh file to publish. Must be prefixed with package://, or file://"),
        BT::InputPort<bool>("mesh_use_embedded_materials", true, "only used for mesh markers"),
    });
  }

  bool setMessage(visualization_msgs::msg::Marker& msg) override
  {
    std::string mesh_file_path;
    resource_retriever::Retriever r;
    resource_retriever::MemoryResource resource;
    auto filename_result = getInput<std::string>("mesh_file_path", mesh_file_path);
    // Check before using
    if (!filename_result)
    {
      RCLCPP_ERROR_STREAM(node_->get_logger(),
                          "[" << name() << "] Missing input [mesh_file_path]: " << filename_result.error());
      return false;
    }

    try
    {
      resource = r.get(mesh_file_path);
    }
    catch (resource_retriever::Exception& e)
    {
      RCLCPP_ERROR_STREAM(node_->get_logger(), "[" << name() << "] Failed to get file: " << e.what());
      return false;
    }

    auto pose = getInput<geometry_msgs::msg::PoseStamped>("pose");
    if (!pose)
    {
      RCLCPP_ERROR_STREAM(node_->get_logger(), "Failed to get 'pose' key in PublishMeshMarker: " << pose.error());
      return false;
    }

    auto pose_stamped = pose.value();
    if (pose_stamped.header.frame_id.empty())
    {
      RCLCPP_ERROR_STREAM(node_->get_logger(), name() << " frame_id empty!");
      return false;
    }

    msg.header = pose_stamped.header;
    if (msg.header.stamp.sec == 0 && msg.header.stamp.nanosec == 0)
    {
      msg.header.stamp = node_->now();
    }
    msg.pose = pose_stamped.pose;
    msg.type = visualization_msgs::msg::Marker::MESH_RESOURCE;
    msg.mesh_resource = mesh_file_path;
    getInput("action", msg.action);
    getInput("ns", msg.ns);
    getInput("id", msg.id);
    getInput("scale", msg.scale);
    getInput("mesh_use_embedded_materials", msg.mesh_use_embedded_materials);

    return true;
  }
};
}  // namespace samxl::behavior
