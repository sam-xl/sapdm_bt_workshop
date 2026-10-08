#include "nxtgen_rviz_behaviortree/execute_tree.hpp"

#include <stdio.h>

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <memory>
#include <rviz_common/display_context.hpp>

namespace samxl::rviz
{

ExecuteTreePanel::ExecuteTreePanel(QWidget* parent)
  : rviz_common::Panel(parent)
{
  std::lock_guard lock(ui_mux);

  // Lay out the topic field above the control widget.
  layout = new QVBoxLayout;
  layout->setAlignment(Qt::AlignTop);

  setLayout(layout);
}

void ExecuteTreePanel::onInitialize()
{
  node = this->getDisplayContext()->getRosNodeAbstraction().lock()->get_raw_node();

  std::lock_guard lock(ui_mux);

  auto hlay = new QHBoxLayout;
  combo = new QComboBox();
  combo->setEditable(false);
  combo->setEnabled(false);
  combo->addItem(QString("<no trees found>"));
  hlay->addWidget(combo);
  auto rbutton = new QPushButton(QString("↺"));
  QFont font = rbutton->font();
  font.setPointSize(font.pointSize() + 4);
  rbutton->setFont(font);
  rbutton->setFixedWidth(23);
  rbutton->setFixedHeight(23);
  QObject::connect(rbutton, &QPushButton::pressed, std::bind(&ExecuteTreePanel::refresh_combo, this));
  hlay->addWidget(rbutton);

  layout->addLayout(hlay);

  payload = new QComboBox();
  payload->setEditable(true);
  payload->setEnabled(false);
  layout->addWidget(payload);

  button = new QPushButton(QString("&Execute"));
  button->setEnabled(false);
  layout->addWidget(button);
  QObject::connect(button, &QPushButton::pressed, std::bind(&ExecuteTreePanel::button_callback, this));

  cancel_button = new QPushButton(QString("&Cancel"));
  cancel_button->setEnabled(false);
  layout->addWidget(cancel_button);
  QObject::connect(cancel_button, &QPushButton::pressed, std::bind(&ExecuteTreePanel::cancel_button_callback, this));

  status = new QLabel(QString(""));
  status->setWordWrap(true);
  status->setMinimumHeight(50);
  status->setAlignment(Qt::AlignTop);
  layout->addWidget(status);

  client_get_trees = node->create_client<btcpp_ros2_interfaces::srv::GetTrees>("/get_loaded_trees");
  client_execute_tree =
      rclcpp_action::create_client<btcpp_ros2_interfaces::action::ExecuteTree>(node, "/behavior_server");

  refresh_combo();
}

void ExecuteTreePanel::load(const rviz_common::Config& config)
{
  Panel::load(config);

  const auto default_payload = config.mapGetChild("DefaultPayload");
  const auto default_tree = config.mapGetChild("DefaultTree");

  auto payloads = config.mapGetChild("Payloads");
  if (payloads.getType() != rviz_common::Config::Type::List)
    return;

  for (int ii = 0; ii < payloads.listLength(); ++ii)
  {
    auto child = payloads.listChildAt(ii);
    if (child.getType() != rviz_common::Config::Type::Value)
      continue;
    payload->addItem(child.getValue().toString());
  }

  if (default_payload.getType() == rviz_common::Config::Type::Value)
    payload->setCurrentText(default_payload.getValue().toString());

  if (default_tree.getType() == rviz_common::Config::Type::Value)
  {
    const QString tree = default_tree.getValue().toString();
    int index = combo->findText(tree);
    if (index == -1)
    {
      combo->addItem(tree);
      index = combo->findText(tree);
    }
    combo->setCurrentIndex(index);
  }
}

void ExecuteTreePanel::save(rviz_common::Config config) const
{
  Panel::save(config);

  rviz_common::Config payloads = config.mapMakeChild("Payloads");
  payloads.setType(rviz_common::Config::Type::List);

  for (int ii = 0; ii != payload->count(); ++ii)
    payloads.listAppendNew().setValue(payload->itemText(ii));

  rviz_common::Config default_payload = config.mapMakeChild("DefaultPayload");
  default_payload.setType(rviz_common::Config::Type::Value);
  default_payload.setValue(payload->currentText());

  rviz_common::Config default_tree = config.mapMakeChild("DefaultTree");
  default_tree.setType(rviz_common::Config::Type::Value);
  default_tree.setValue(combo->currentText());
}

void ExecuteTreePanel::refresh_combo()
{
  using namespace std::chrono_literals;
  bool is_online = client_get_trees->wait_for_service(0.1s);
  if (!is_online)
    return;
  auto request = std::make_shared<btcpp_ros2_interfaces::srv::GetTrees::Request>();
  client_get_trees->async_send_request(
      request, [this](std::shared_future<btcpp_ros2_interfaces::srv::GetTrees::Response::SharedPtr> future) {
        std::lock_guard lock(this->ui_mux);
        const auto result = future.get();
        const auto prev_text = this->combo->currentText();
        bool has_prev = false;
        this->combo->clear();
        for (const auto& tree_id : result->tree_ids)
        {
          const auto item_str = QString(tree_id.c_str());
          if (prev_text == item_str)
          {
            has_prev = true;
          }
          this->combo->addItem(item_str);
        }
        if (has_prev)
        {
          this->combo->setCurrentText(prev_text);
        }
        this->combo->setEnabled(true);
        this->button->setEnabled(true);
        this->payload->setEnabled(true);
      });
}

void ExecuteTreePanel::cancel_button_callback()
{
  if (!goal_handle_future)
    return;
  if (!goal_handle_future->get())
    return;
  client_execute_tree->async_cancel_goal(goal_handle_future->get());
}

void ExecuteTreePanel::button_callback()
{
  auto goal_msg = btcpp_ros2_interfaces::action::ExecuteTree::Goal{};

  // Tree name based on dropdown
  goal_msg.target_tree = combo->currentText().toUtf8().toStdString();
  goal_msg.payload = payload->currentText().toUtf8().toStdString();

  auto send_goal_options = rclcpp_action::Client<btcpp_ros2_interfaces::action::ExecuteTree>::SendGoalOptions();
  send_goal_options.goal_response_callback =
      [this](std::shared_ptr<rclcpp_action::ClientGoalHandle<btcpp_ros2_interfaces::action::ExecuteTree>>
                 client_goal_handle) {
        std::lock_guard lock(ui_mux);
        switch (client_goal_handle->get_status())
        {
          case action_msgs::msg::GoalStatus::STATUS_UNKNOWN:
            this->status->setText(QString("Status Unknown"));
            break;
          case action_msgs::msg::GoalStatus::STATUS_ABORTED:
            this->status->setText(QString("Status Aborted"));
            break;
          case action_msgs::msg::GoalStatus::STATUS_ACCEPTED:
            this->status->setText(QString("<font color=' blue'>● Running...</font>"));
            this->button->setEnabled(false);
            this->combo->setEnabled(false);
            this->payload->setEnabled(false);
            this->cancel_button->setEnabled(true);
            break;
          case action_msgs::msg::GoalStatus::STATUS_CANCELED:
            this->status->setText(QString("Status Canceled"));
            break;
          case action_msgs::msg::GoalStatus::STATUS_CANCELING:
            this->status->setText(QString("Status Canceling"));
            break;
          case action_msgs::msg::GoalStatus::STATUS_EXECUTING:
            this->status->setText(QString("Status Executing"));
            break;
          case action_msgs::msg::GoalStatus::STATUS_SUCCEEDED:
            this->status->setText(QString("Status Succeeded"));
            break;
          default:
            this->status->setText(QString("Unknown Status"));
            break;
        }
      };
  send_goal_options.feedback_callback =
      [this](rclcpp_action::ClientGoalHandle<btcpp_ros2_interfaces::action::ExecuteTree>::SharedPtr,
             const std::shared_ptr<const btcpp_ros2_interfaces::action::ExecuteTree::Feedback> feedback) {
        std::lock_guard lock(ui_mux);
        this->status->setText(QString(feedback->message.c_str()));
      };
  send_goal_options.result_callback =
      [this](const rclcpp_action::ClientGoalHandle<btcpp_ros2_interfaces::action::ExecuteTree>::WrappedResult& result) {
        std::lock_guard lock(ui_mux);
        this->button->setEnabled(true);
        this->combo->setEnabled(true);
        this->payload->setEnabled(true);
        this->cancel_button->setEnabled(false);
        std::string str;
        switch (result.result->node_status.status)
        {
          case btcpp_ros2_interfaces::msg::NodeStatus::FAILURE:
            str = "<font color='red'>● Failure</font>";
            break;
          case btcpp_ros2_interfaces::msg::NodeStatus::IDLE:
            str = "<font color='#F54927'>● Error</font>";
            break;
          case btcpp_ros2_interfaces::msg::NodeStatus::SKIPPED:
            str = "<font color='#F54927'>● Skipped</font>";
            break;
          case btcpp_ros2_interfaces::msg::NodeStatus::SUCCESS:
            str = "<font color='green'>● Success</font>";
            break;
          case btcpp_ros2_interfaces::msg::NodeStatus::RUNNING:
            str = "<font color='#F54927'>● Cancelled</font>";
            break;
        }
        str += ": " + result.result->return_message;
        this->status->setText(QString(str.c_str()));
      };

  goal_handle_future = this->client_execute_tree->async_send_goal(goal_msg, send_goal_options);
}

}  // namespace samxl::rviz
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(samxl::rviz::ExecuteTreePanel, rviz_common::Panel)
