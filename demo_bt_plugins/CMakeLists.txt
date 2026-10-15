cmake_minimum_required(VERSION 3.11)
project(demo_bt_plugins)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(behaviortree_ros2 REQUIRED)
find_package(conveyorbelt_msgs REQUIRED)

add_library(${PROJECT_NAME}_plugins SHARED src/plugins.cpp)
target_include_directories(${PROJECT_NAME}_plugins
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(${PROJECT_NAME}_plugins PUBLIC
  behaviortree_cpp::behaviortree_cpp
  behaviortree_ros2::behaviortree_ros2
)
ament_target_dependencies(${PROJECT_NAME}_plugins PUBLIC conveyorbelt_msgs)
target_compile_definitions(${PROJECT_NAME}_plugins PRIVATE BT_PLUGIN_EXPORT)

install(DIRECTORY include/ DESTINATION include)

# behaviortree_ros2 looks for plugins in share/<package>/bt_plugins
install(
  TARGETS ${PROJECT_NAME}_plugins
  LIBRARY DESTINATION share/${PROJECT_NAME}/bt_plugins
  ARCHIVE DESTINATION lib
)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_inline_tree_action test/test_inline_tree_action.cpp)
  target_include_directories(test_inline_tree_action PRIVATE include)
  target_link_libraries(test_inline_tree_action behaviortree_cpp::behaviortree_cpp)
endif()

ament_package()
