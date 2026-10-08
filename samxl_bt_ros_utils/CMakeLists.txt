cmake_minimum_required(VERSION 3.11)
project(samxl_bt_ros_utils)

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
find_package(control_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(resource_retriever REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(tf2_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(rosbag2_interfaces REQUIRED)
find_package(visualization_msgs REQUIRED)

set(THIS_PACKAGE_DEPS
  rclcpp::rclcpp
  yaml-cpp
  behaviortree_cpp::behaviortree_cpp
  behaviortree_ros2::behaviortree_ros2
  resource_retriever::resource_retriever
  tf2::tf2
  tf2_geometry_msgs::tf2_geometry_msgs
  # FIXME: CHECK IF USED AS BUILD_DEP (or only run)
  tf2_ros::tf2_ros
  ${control_msgs_TARGETS}
  ${geometry_msgs_TARGETS}
  ${rosbag2_interfaces_TARGETS}
  ${sensor_msgs_TARGETS}
  ${std_msgs_TARGETS}
  ${std_srvs_TARGETS}
  ${tf2_msgs_TARGETS}
  ${trajectory_msgs_TARGETS}
  ${visualization_msgs_TARGETS}
)

##########################################################################
# Library for conversions (yaml -> type)
##########################################################################

add_library(${PROJECT_NAME}_conversions
  STATIC
  "src/conversions/builtin_interfaces.cpp"
  "src/conversions/geometry_msgs.cpp"
  "src/conversions/sensor_msgs.cpp"
  "src/conversions/std_msgs.cpp"
)
target_include_directories(${PROJECT_NAME}_conversions
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME}_conversions ${THIS_PACKAGE_DEPS})
set_target_properties(${PROJECT_NAME}_conversions PROPERTIES POSITION_INDEPENDENT_CODE ON)

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
target_link_libraries(${PROJECT_NAME}_plugins PUBLIC ${THIS_PACKAGE_DEPS} PRIVATE ${PROJECT_NAME}_conversions)
target_compile_definitions(${PROJECT_NAME}_plugins PRIVATE BT_PLUGIN_EXPORT)

##########################################################################
# Create an executable for the behavior tree node
##########################################################################
add_executable(bt_action_server src/tree.cpp)
target_include_directories(bt_action_server
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(bt_action_server
  ${THIS_PACKAGE_DEPS}
)

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

install(
  TARGETS ${PROJECT_NAME}_conversions
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

install(
  TARGETS bt_action_server
  DESTINATION lib/${PROJECT_NAME}
)

##########################################################################
# Export targets and dependencies for downstream packages
##########################################################################
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  behaviortree_ros2
  control_msgs
  geometry_msgs
  rclcpp
  resource_retriever
  rosbag2_interfaces
  sensor_msgs
  std_msgs
  std_srvs
  tf2
  tf2_geometry_msgs
  tf2_msgs
  tf2_ros
  trajectory_msgs
  visualization_msgs
)

##########################################################################
# Install files and configurations
##########################################################################

install(DIRECTORY
  launch
  model
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
