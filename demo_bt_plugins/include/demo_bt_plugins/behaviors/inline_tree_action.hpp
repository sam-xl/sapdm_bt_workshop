#pragma once

#include <behaviortree_cpp/action_node.h>
#include <behaviortree_cpp/bt_factory.h>

#include <memory>
#include <string>
#include <utility>

namespace samxl::behavior
{

class InlineTreeAction : public BT::StatefulActionNode
{
public:
  InlineTreeAction(const std::string& name, const BT::NodeConfig& config, BT::Tree tree)
    : BT::StatefulActionNode(name, config)
    , tree_(std::move(tree))
  {
  }

  static BT::PortsList providedPorts()
  {
    return {};
  }

  BT::NodeStatus onStart() override
  {
    return tree_.tickOnce();
  }

  BT::NodeStatus onRunning() override
  {
    return tree_.tickOnce();
  }

  void onHalted() override
  {
    tree_.haltTree();
  }

private:
  BT::Tree tree_;
};

/**
 * Register tree_xml as a leaf action with the given XML tag. The embedded tree
 * uses the caller's blackboard directly and exposes no separate ports.
 */
inline void registerInlineTreeAction(BT::BehaviorTreeFactory& factory, const std::string& node_id,
                                     const std::string& tree_xml)
{
  factory.registerBuilder(BT::CreateManifest<InlineTreeAction>(node_id),
                          [&factory, tree_xml](const std::string& name, const BT::NodeConfig& config) {
                            return std::make_unique<InlineTreeAction>(
                                name, config, factory.createTreeFromText(tree_xml, config.blackboard));
                          });
}

}  // namespace samxl::behavior
