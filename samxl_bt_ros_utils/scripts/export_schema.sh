#!/bin/bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ros2 launch samxl_bt_ros_utils bt_action_server.launch \
    parameter_file:=$(ros2 pkg prefix samxl_bt_ros_utils --share)/test/config/bt_action_server_parameters.yaml \
    tree_schema_path:=$SCRIPT_DIR/../model/schema.xsd \
    tree_nodes_model_path:=$SCRIPT_DIR/../model/tree_nodes_model.xml \
    exit_after_writing_schema:=true
