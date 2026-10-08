#pragma once

/**
 * Utilities to define `BT::convertFromString()` based on `samxl::yaml::msg_from_yaml` functions.
 *
 * These allow BehaviorTree.CPP to automatically convert from string to any type for which the
 * `samxl::yaml::msg_from_yaml` function is defined.
 *
 * To ensure that the functions can be found at runtime, the relevant header for any functions used
 * should be included when declaring a BT::Node which might need it.
 * For example, `conversions/builtin_interfaces.hpp` should be included when using the
 * `builtin_interfaces::msg::Time` type in an InputPort.
 */

#include <behaviortree_cpp/basic_types.h>

#include <optional>
#include <samxl_bt_ros_utils/conversions/yaml_conversions.hpp>

namespace samxl::bt_conversions
{

/**
 * @brief Internal function to convert from string input to some type T.
 *
 * Requires `::samxl::yaml::msg_from_yaml<T>` to be defined for type T.
 *
 * @param string_input String from which to convert. Must contain yaml content.
 * @returns Object of arbitrary type T.
 */
template <typename T>
inline T convertFromString(BT::StringView& string_input)
{
  std::optional<YAML::Node> yaml_node;
  // Convert string to yaml
  try
  {
    yaml_node = YAML::Load(std::string(string_input));
  }
  catch (const YAML::ParserException& ex)
  {
    throw BT::RuntimeError(std::string("Failed to convert string to yaml in convertFromString<") + typeid(T).name() +
                           ">(): " + ex.what());
  }

  // Convert yaml to msg type
  try
  {
    return ::samxl::yaml::msg_from_yaml<T>(*yaml_node);
  }
  catch (const std::exception& ex)
  {
    throw BT::RuntimeError(std::string("Failed to convert yaml string to type '") + typeid(T).name() +
                           "': " + ex.what());
  }
}

}  // namespace samxl::bt_conversions

/**
 * @brief Register a type to be able to have implicit conversions from string in BT.CPP ports
 *
 * For the declaration of this function, use `HEADER_TYPE_CONVERT_FROM_STRING(T)`.
 *
 * Note: the function `T ::samxl::yaml::msg_from_yaml(YAML::Node&)` must exist for this to work
 *
 * Only use this when defining new conversion functions
 *
 * @param T Typename of the type to register
 */
#define REGISTER_TYPE_CONVERT_FROM_STRING(T)                                                                           \
  template <>                                                                                                          \
  __attribute__((used)) [[nodiscard]] T BT::convertFromString(BT::StringView str)                                      \
  {                                                                                                                    \
    return ::samxl::bt_conversions::convertFromString<T>(str);                                                         \
  }

/**
 * @brief Declaration for convert from string function.
 *
 * For the definition of the function, use the `REGISTER_TYPE_CONVERT_FROM_STRING(T)` macro.
 *
 * @param T Typename of the type to register
 */
#define HEADER_TYPE_CONVERT_FROM_STRING(T)                                                                             \
  template <>                                                                                                          \
  T BT::convertFromString(BT::StringView str);
