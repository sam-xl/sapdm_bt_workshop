#pragma once

#include <behaviortree_ros2/bt_topic_sub_node.hpp>

namespace samxl::behavior
{

/**
 * @brief Describes what a GenericSubscriber writes to its output port. By default the whole message is written;
 * specialize to expose a simpler type (e.g. std_msgs/Bool -> bool).
 */
template <typename MessageT>
struct SubscriberOutput
{
  using type = MessageT;
  static type get(const MessageT& msg)
  {
    return msg;
  }
};

/**
 * @brief Generic subscriber behaviour for a ros message of type MessageT.
 *
 * The last received message is latched: SUCCESS is returned and the output port is written once any message has been
 * received, FAILURE until then.
 */
template <typename MessageT>
class GenericSubscriber : public BT::RosTopicSubNode<MessageT>
{
public:
  static_assert(rosidl_generator_traits::is_message<MessageT>::value, "GenericSubscriber only accepts ROS Messages!");

  using OutputT = typename SubscriberOutput<MessageT>::type;

  GenericSubscriber(const std::string& instance_name, const BT::NodeConfig& conf, const BT::RosNodeParams& params)
    : BT::RosTopicSubNode<MessageT>::RosTopicSubNode(instance_name, conf, params)
  {
  }

  static BT::PortsList providedPorts()
  {
    return BT::RosTopicSubNode<MessageT>::providedBasicPorts({
        BT::OutputPort<OutputT>("output", "Latest message received."),
    });
  }

  BT::NodeStatus onTick(const std::shared_ptr<MessageT>& last_msg) override
  {
    if (!last_msg)
    {
      return BT::NodeStatus::FAILURE;
    }
    this->setOutput("output", SubscriberOutput<MessageT>::get(*last_msg));
    return BT::NodeStatus::SUCCESS;
  }

  bool latchLastMessage() const override
  {
    return true;
  }
};

}  // namespace samxl::behavior
