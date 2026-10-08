#include <behaviortree_cpp/action_node.h>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <fstream>
#include <sstream>

namespace samxl::behavior

{
class LoadTextFile : public BT::SyncActionNode
{
public:
  LoadTextFile(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("ros_package", "", "If set, the file_path is relative to this package's share."),
      BT::InputPort<std::string>("file_path", "File path to the file which should be loaded."),
      BT::InputPort<std::string>("file_name", "Name of the file which should be loaded."),
      BT::InputPort<bool>("debug_print", false, "Print loaded file's contents for debugging"),
      BT::OutputPort<std::string>("output", "Where to put the file contents string"),
    };
  }

  BT::NodeStatus tick()
  {
    std::string path;
    std::string file_name;
    auto path_result = getInput<std::string>("file_path", path);
    auto filename_result = getInput<std::string>("file_name", file_name);

    // Check before using
    if (!path_result)
    {
      std::cerr << "Missing input [file_path]: " << path_result.error() << std::endl;
      return BT::NodeStatus::FAILURE;
    }
    if (!filename_result)
    {
      std::cerr << "Missing input [file_name]: " << filename_result.error() << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    std::string ros_package;
    getInput<std::string>("ros_package", ros_package);
    if (!ros_package.empty())
    {
      // If a ros_package is specified, then interpret the path relative to it.
      try
      {
        path = ament_index_cpp::get_package_share_directory(ros_package) + "/" + path;
      }
      catch (const std::exception& ex)
      {
        std::cerr << "Could not find ros package '" << ros_package << "': " << ex.what() << std::endl;
        return BT::NodeStatus::FAILURE;
      }
    }

    const std::string full_path = path + "/" + file_name;
    std::ifstream file_stream(full_path);
    if (!file_stream)
    {
      std::cerr << "Could not open file '" << full_path << "'." << std::endl;
      return BT::NodeStatus::FAILURE;
    }
    std::ostringstream ss;
    ss << file_stream.rdbuf();

    // Write result to blackboard
    setOutput("output", ss.str());

    bool debug_print = getInput<bool>("debug_print").value();
    if (debug_print)
    {
      std::cout << "FILE CONTENTS" << std::endl;
      std::cout << ">>>>BEGIN>>>>" << std::endl;
      std::cout << ss.str() << std::endl;
      std::cout << "<<<<<END<<<<<" << std::endl;
      std::cout << "FILE CONTENTS" << std::endl;
    }
    return BT::NodeStatus::SUCCESS;
  }
};
}  // namespace samxl::behavior
