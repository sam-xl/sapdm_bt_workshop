#pragma once

/**
 * Declarations of generic functions for converting from yaml to arbitrary objects/ros messages.
 *
 * Definitions for these functions must be written manually for each type T.
 */

#include <behaviortree_cpp/behavior_tree.h>
#include <yaml-cpp/yaml.h>

namespace samxl::yaml
{

/**
 * @brief A generic function to convert yaml to arbitrary types.
 *
 * This function should be specialised for all required custom types.
 *
 * @param node A yaml node input, to be converted to type T.
 * @param output The output of type T, passed by reference.
 */
template <typename T>
void msg_from_yaml(const YAML::Node& node, T& output);

/**
 * @brief A generic function to convert yaml to arbitrary types.
 *
 * This function is automatically derived for types where:
 *
 *   - The type T is default constructable
 *
 *   - There exists a declaration of `void msg_from_yaml(const YAML::Node&, T&)` for the type
 *
 * @param node A yaml node input, to be converted to type T.
 * @return A new object of type T.
 */
template <typename T>
T msg_from_yaml(const YAML::Node& node)
{
  T output;
  msg_from_yaml(node, output);
  return output;
}

}  // namespace samxl::yaml
