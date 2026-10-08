#include <behaviortree_cpp/action_node.h>

#include <ament_index_cpp/get_package_share_directory.hpp>
#include <fstream>
#include <sstream>

namespace samxl::behavior

{
class WriteTextFile : public BT::SyncActionNode
{
public:
  WriteTextFile(const std::string& name, const BT::NodeConfig& config)
    : BT::SyncActionNode(name, config)
  {
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<std::string>("file_path", "File path to the file which should be loaded."),
      BT::InputPort<std::string>("file_name", "Name of the file which should be loaded."),
      BT::InputPort<bool>("debug_print", false, "Print loaded file's contents for debugging"),
      BT::InputPort<bool>("append", false, "Whether to append to the new file. Otherwise overwrites."),
      BT::InputPort<std::string>("text", "What to put in the file"),
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

    auto input_result = getInput<std::string>("text");
    if (!input_result)
    {
      std::cerr << "Missing input [text]: " << input_result.error() << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    const std::string full_path = path + "/" + file_name;
    const bool append = getInput<bool>("append").value();
    std::ofstream file_stream(full_path, append ? (std::ios::out | std::ios::app) : (std::ios::out | std::ios::trunc));
    if (!file_stream)
    {
      std::cerr << "Could not open file '" << full_path << "'." << std::endl;
      return BT::NodeStatus::FAILURE;
    }
    file_stream << input_result.value();
    if (!file_stream)
    {
      std::cerr << "Could not write to file '" << full_path << "'." << std::endl;
      return BT::NodeStatus::FAILURE;
    }

    bool debug_print = getInput<bool>("debug_print").value();
    if (debug_print)
    {
      std::cout << "FILE CONTENTS" << std::endl;
      std::cout << ">>>>BEGIN>>>>" << std::endl;
      std::cout << input_result.value() << std::endl;
      std::cout << "<<<<<END<<<<<" << std::endl;
      std::cout << "FILE CONTENTS" << std::endl;
    }
    return BT::NodeStatus::SUCCESS;
  }
};
}  // namespace samxl::behavior
