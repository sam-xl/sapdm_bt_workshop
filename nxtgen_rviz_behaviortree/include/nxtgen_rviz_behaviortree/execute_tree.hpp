#pragma once

#ifndef Q_MOC_RUN
#include <btcpp_ros2_interfaces/action/execute_tree.hpp>
#include <btcpp_ros2_interfaces/srv/get_trees.hpp>
#include <mutex>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>

#include "rviz_common/panel.hpp"
#endif

class QComboBox;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;

namespace samxl::rviz
{

class ExecuteTreePanel : public rviz_common::Panel
{
  Q_OBJECT

public:
  explicit ExecuteTreePanel(QWidget* parent = 0);
  void onInitialize();
  void load(const rviz_common::Config& config) override;
  void save(rviz_common::Config config) const override;

protected:
  rclcpp::Node::SharedPtr node;
  rclcpp::Client<btcpp_ros2_interfaces::srv::GetTrees>::SharedPtr client_get_trees;
  rclcpp_action::Client<btcpp_ros2_interfaces::action::ExecuteTree>::SharedPtr client_execute_tree;
  std::optional<
      std::shared_future<std::shared_ptr<rclcpp_action::ClientGoalHandle<btcpp_ros2_interfaces::action::ExecuteTree>>>>
      goal_handle_future;

  QVBoxLayout* layout;
  QComboBox* payload;
  QPushButton* button;
  QPushButton* cancel_button;
  QLabel* status;
  QComboBox* combo;

  std::mutex ui_mux;

  void button_callback();
  void cancel_button_callback();
  void refresh_combo();
};

}  // namespace samxl::rviz