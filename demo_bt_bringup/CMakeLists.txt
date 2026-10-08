cmake_minimum_required(VERSION 3.8)
project(demo_bt_bringup NONE)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY launch config trees
  DESTINATION share/${PROJECT_NAME}/
)

install(
  PROGRAMS
    scripts/box_proximity_sensor.py
    scripts/conveyor_speed_overlay.py
    scripts/gazebo_box_tf_publisher.py
    scripts/gated_box_attacher.py
    scripts/reset_boxes.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
