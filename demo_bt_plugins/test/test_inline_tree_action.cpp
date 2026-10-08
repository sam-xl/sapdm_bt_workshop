#include <gtest/gtest.h>

#include <demo_bt_plugins/behaviors/inline_tree_action.hpp>

TEST(InlineTreeAction, RunsHardcodedXmlAsAnActionNode)
{
  BT::BehaviorTreeFactory factory;
  samxl::behavior::registerInlineTreeAction(factory, "EmbeddedBehavior",
                                            R"(<root BTCPP_format="4">
        <BehaviorTree ID="EmbeddedTree">
          <Sequence>
            <SetBlackboard output_key="embedded_result" value="42" />
            <AlwaysSuccess />
          </Sequence>
        </BehaviorTree>
      </root>)");

  auto tree = factory.createTreeFromText(
      R"(<root BTCPP_format="4">
        <BehaviorTree ID="ConsumerTree">
          <EmbeddedBehavior />
        </BehaviorTree>
      </root>)");

  ASSERT_EQ(tree.rootNode()->registrationName(), "EmbeddedBehavior");
  EXPECT_EQ(tree.tickOnce(), BT::NodeStatus::SUCCESS);
  EXPECT_EQ(tree.rootBlackboard()->get<std::string>("embedded_result"), "42");
}
