cmake_minimum_required(VERSION 3.8)
project(nxtgen_rviz_behaviortree)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rviz_common REQUIRED)
find_package(rviz_rendering REQUIRED)
find_package(rviz_ogre_vendor REQUIRED)
find_package(btcpp_ros2_interfaces REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

# This setting causes Qt's "MOC" generation to happen automatically.
set(CMAKE_AUTOMOC ON)

include_directories(
  include
)

set(HDR_FILES
    include/${PROJECT_NAME}/execute_tree.hpp
)


set(SRC_FILES
  src/execute_tree.cpp
)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES} ${HDR_FILES})

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  rviz_common
  rviz_rendering
  btcpp_ros2_interfaces
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${Qt5Widgets_INCLUDE_DIRS}
  ${button_reconfigure_interface_INCLUDE_DIRS})

pluginlib_export_plugin_description_file(rviz_common plugin_description.xml)
# register_rviz_ogre_media_exports(DIRECTORIES "media")

ament_export_dependencies(
  rclcpp
  rviz_common
  rviz_rendering
  btcpp_ros2_interfaces
)

install(TARGETS
  ${PROJECT_NAME}
  ARCHIVE DESTINATION lib/${PROJECT_NAME}
  LIBRARY DESTINATION lib/${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

# install(DIRECTORY media/
#   DESTINATION share/${PROJECT_NAME}/media
# )

install(DIRECTORY icons/
  DESTINATION share/${PROJECT_NAME}/icons
)

# install(PROGRAMS scripts/send_test_msgs.py
#   DESTINATION lib/${PROJECT_NAME}
# )

ament_package()
