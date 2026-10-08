#pragma once

#include <behaviortree_cpp/decorator_node.h>

#include <iomanip>
namespace samxl::behavior
{

/**
 * Decorator that determines how long a behaviour node spends RUNNING.
 *
 * Input ports:
 * - `label` (string): Override the label shown in log messages. Defaults to node name.
 *
 * Output ports:
 * - `string_output` (string): Output time as string.
 * - `time_ms` (double): Time taken to execute child behaviour.
 */
class TimerDecorator : public BT::DecoratorNode
{
public:
  TimerDecorator(const std::string& name, const BT::NodeConfig& config)
    : BT::DecoratorNode(name, config)
  {
  }

  static BT::PortsList providedPorts()
  {
    return { BT::InputPort<std::string>("label", "",
                                        "Override the label shown in log messages. Defaults to node name."),
             BT::OutputPort<std::string>("string_output", "{__string_output}", "Output time as string."),
             BT::OutputPort<double>("time_ms", "{__time_ms}", "Time taken to execute child behaviour.") };
  }

  BT::NodeStatus tick() override
  {
    // executeTick() sets the node status to RUNNING before invoking tick().
    // Start a fresh measurement for every transition into RUNNING.
    if (!timing_active_ && status() == BT::NodeStatus::RUNNING)
    {
      label_ = getInput<std::string>("label").value_or(name());
      if (label_.empty())
        label_ = name();
      start_ = std::chrono::steady_clock::now();
      timing_active_ = true;
      std::cout << "[TIMER] " << label_ << " START RUNNING" << std::endl;
    }

    auto result = child()->executeTick();

    if (result == BT::NodeStatus::SUCCESS || result == BT::NodeStatus::FAILURE)
    {
      auto elapsed = std::chrono::steady_clock::now() - start_;
      double ms = std::chrono::duration<double, std::milli>(elapsed).count();
      setOutput<double>("time_ms", ms);
      timing_active_ = false;
      std::cout << "[TIMER] " << label_ << " END " << std::fixed << std::setprecision(2) << ms << " ms" << std::endl;
      std::stringstream ss;
      ss << label_ << ": " << std::fixed << std::setprecision(2) << ms << " ms" << std::endl;
      setOutput<std::string>("string_output", ss.str());
    }

    return result;
  }

  void halt() override
  {
    timing_active_ = false;
    BT::DecoratorNode::halt();
  }

protected:
  std::chrono::steady_clock::time_point start_;
  std::string label_;
  bool timing_active_{ false };
};
}  // namespace samxl::behavior
