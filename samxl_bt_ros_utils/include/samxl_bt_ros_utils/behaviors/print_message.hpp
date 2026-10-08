#pragma once

#include <behaviortree_cpp/action_node.h>

#include <iostream>
#include <rosidl_runtime_cpp/traits.hpp>

namespace samxl::behavior
{

template <typename MessageT>
class PrintMessage : public BT::SyncActionNode
{
private:
  static_assert(rosidl_generator_traits::is_message<MessageT>::value, "PrintMessage only excepts ROS Messages!");

public:
  PrintMessage(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<MessageT>("input", "message to print."),
      BT::InputPort<bool>("inline", false, "Whether to use inline YAML (flow style) or full YAML (block style)"),
    };
  }

  BT::NodeStatus tick()
  {
    auto input_val = getInput<MessageT>("input");
    if (!input_val)
    {
      std::cerr << "Failed to get 'input' key in '" << name() << "': " << input_val.error() << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    MessageT& input{ input_val.value() };

    auto use_inline_val = getInput<bool>("inline");
    if (!use_inline_val)
    {
      std::cerr << "Failed to get 'inline' key in '" << name() << "': " << use_inline_val.error() << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    bool use_inline = use_inline_val.value();

    std::cout << "[" << name() << "] input (" << rosidl_generator_traits::name<MessageT>() << ")";

    if (use_inline)
    {
      std::cout << " = ";
      to_flow_style_yaml(input, std::cout);
      std::cout << std::endl;
    }
    else
    {
      std::cout << ":" << std::endl;
      to_block_style_yaml(input, std::cout, 2);
    }

    return BT::NodeStatus::SUCCESS;
  }
};

}  // namespace samxl::behavior
