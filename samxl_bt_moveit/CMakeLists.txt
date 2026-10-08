cmake_minimum_required(VERSION 3.11)
project(samxl_bt_moveit)

# Recommended by https://docs.ros.org/en/rolling/How-To-Guides/Ament-CMake-Documentation.html
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(behaviortree_ros2 REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(moveit_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(samxl_bt_ros_utils REQUIRED)

set(THIS_PACKAGE_DEPS
  rclcpp::rclcpp
  yaml-cpp
  behaviortree_cpp::behaviortree_cpp
  behaviortree_ros2::behaviortree_ros2
  ${geometry_msgs_TARGETS}
  ${moveit_msgs_TARGETS}
)

##########################################################################
# Create a library for the behavior tree plugins
##########################################################################
add_library(${PROJECT_NAME}_plugins
  SHARED
  src/plugins.cpp
)
target_include_directories(${PROJECT_NAME}_plugins
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME}_plugins PUBLIC ${THIS_PACKAGE_DEPS} PRIVATE samxl_bt_ros_utils::samxl_bt_ros_utils_conversions)
target_compile_definitions(${PROJECT_NAME}_plugins PRIVATE BT_PLUGIN_EXPORT)


##########################################################################
# Install targets and headers
##########################################################################

install(
  DIRECTORY include/
  DESTINATION include
)

# The below is a bit unusual, but that's where behaviourtree_ros2 looks for plugins
install(
  TARGETS ${PROJECT_NAME}_plugins
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION share/${PROJECT_NAME}/bt_plugins
  ARCHIVE DESTINATION lib
)

##########################################################################
# Export targets and dependencies for downstream packages
##########################################################################
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  behaviortree_cpp
  behaviortree_ros2 
  geometry_msgs
  moveit_msgs
  rclcpp
  samxl_bt_ros_utils
)

##########################################################################
# Install files and configurations
##########################################################################

install(DIRECTORY
  model
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
