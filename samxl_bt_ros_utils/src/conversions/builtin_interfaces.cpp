#include <builtin_interfaces/msg/duration.hpp>
#include <builtin_interfaces/msg/time.hpp>
#include <rclcpp/duration.hpp>
#include <regex>
#include <samxl_bt_ros_utils/conversions/bt_conversions.hpp>
#include <samxl_bt_ros_utils/conversions/yaml_conversions.hpp>

namespace samxl::yaml
{

void parse_duration(const std::string& s, builtin_interfaces::msg::Duration& output)
{
  // Split into numeric prefix and unit suffix

  // Match a well-formed number at the very start of the string:
  // optional sign, one or more digits, optional single decimal fraction.
  static const std::regex numberPattern(R"(^[+-]?\d+(\.\d+)?)");

  std::smatch match;
  if (!std::regex_search(s, match, numberPattern))
    throw std::invalid_argument("no numeric value in duration string: " + s);

  std::string numericPart = match[0].str();
  double value = std::stod(numericPart);
  std::string unit = s.substr(numericPart.size());

  using namespace std::chrono;
  nanoseconds ns;

  if (unit == "ns")
    ns = duration_cast<nanoseconds>(duration<double, std::nano>(value));
  else if (unit == "us" || unit == "µs")
    ns = duration_cast<nanoseconds>(duration<double, std::micro>(value));
  else if (unit == "ms")
    ns = duration_cast<nanoseconds>(duration<double, std::milli>(value));
  else if (unit == "s")
    ns = duration_cast<nanoseconds>(duration<double>(value));
  else if (unit == "m" || unit == "min")
    ns = duration_cast<nanoseconds>(duration<double, std::ratio<60>>(value));
  else if (unit == "h")
    ns = duration_cast<nanoseconds>(duration<double, std::ratio<3600>>(value));
  else
    throw std::invalid_argument("unknown duration unit: " + unit);

  // rclcpp::Duration handles the sec/nanosec normalization and overflow
  // guarding; the implicit conversion operator produces the msg.
  output = rclcpp::Duration(ns);
}

// Handles both yaml input and shorthand input (e.g. 1.2min or 15.3us)
template <>
void msg_from_yaml(const YAML::Node& node, builtin_interfaces::msg::Duration& output)
{
  if (node.IsMap())
  {
    output.sec = node["sec"].as<int32_t>();
    output.nanosec = node["nanosec"].as<uint64_t>();
    return;
  }
  else if (node.IsScalar())
  {
    parse_duration(node.as<std::string>(), output);
    return;
  }
  throw std::runtime_error("Bad input for builtin_interfaces::msg::Duration.");
}

template <>
void msg_from_yaml(const YAML::Node& node, builtin_interfaces::msg::Time& output)
{
  output.sec = node["sec"].as<int32_t>();
  output.nanosec = node["nanosec"].as<uint64_t>();
}

}  // namespace samxl::yaml

REGISTER_TYPE_CONVERT_FROM_STRING(builtin_interfaces::msg::Duration)
REGISTER_TYPE_CONVERT_FROM_STRING(builtin_interfaces::msg::Time)
