#pragma once

#include <behaviortree_cpp/action_node.h>

#include <iomanip>

namespace samxl::behavior
{

class DateString : public BT::SyncActionNode
{
public:
  explicit DateString(const std::string& name, const BT::NodeConfig& conf)
    : BT::SyncActionNode(name, conf)
  {
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("input"),
      BT::OutputPort<std::string>("output"),
    };
  }

  BT::NodeStatus tick() override
  {
    std::string input;
    const auto success = this->getInput("input", input);
    if (!success.has_value())
    {
      std::cout << name() << " could not get input port 'input' value" << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    // Get current date/time for expanding specifiers
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    // Expand date/time specification
    std::stringstream ss;
    ss << std::put_time(&tm, input.c_str());

    this->setOutput<std::string>("output", ss.str());

    return BT::NodeStatus::SUCCESS;
  }
};

}  // namespace samxl::behavior
