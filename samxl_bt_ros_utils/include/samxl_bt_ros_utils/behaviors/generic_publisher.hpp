#pragma once

#include <behaviortree_ros2/bt_topic_pub_node.hpp>

namespace samxl::behavior
{

/**
 * @brief Generic publisher behaviour to publish a ros message of type MessageT.
 *
 */
template <typename MessageT>
class GenericPublisher : public BT::RosTopicPubNode<MessageT>
{
public:
  static_assert(rosidl_generator_traits::is_message<MessageT>::value, "GenericPublisher only excepts ROS Messages!");

  GenericPublisher(const std::string& instance_name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosTopicPubNode<MessageT>::RosTopicPubNode(instance_name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return BT::RosTopicPubNode<MessageT>::providedBasicPorts({
        BT::InputPort<MessageT>("input", "Message to publish."),
    });
  }

  bool setMessage(MessageT& msg) override
  {
    auto result = this->getInput("input", msg);
    if (!result)
    {
      RCLCPP_ERROR_STREAM(this->node_->get_logger(),
                          this->name() << " could not get input port 'input' value: " << result.error());
      return false;
    }
    return true;
  }
};

}  // namespace samxl::behavior
