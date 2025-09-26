#include "io/io.hpp"
#include "io/create.hpp"
#include "io/symlink.hpp"
#include "io/shortcut.hpp"
#include <vector>

// Temporary comment to force recompile

namespace allin1::io {

void register_io_commands(cppParse::Parser& io_parser) {
    auto& create_parser = io_parser.add_subparser("create");
    create_parser.add_description("Create a file or directory.");
    create_parser.add_argument(std::vector<std::string>{"type"}).help("The type of object to create (file, directory, folder)").required();
    create_parser.add_argument(std::vector<std::string>{"path"}).help("The directory where the object should be created").required();
    create_parser.add_argument(std::vector<std::string>{"name"}).help("The name of the file or directory to create").required();
    create_parser.add_argument(std::vector<std::string>{"--fill"}).takes_value().help("Fill the file with a certain hex code (e.g., 0xFF)");
    create_parser.add_argument(std::vector<std::string>{"--fill-size"}).takes_value().help("The size to initialize the file to (e.g., 1K, 2M, 3G)");

    auto& symlink_parser = io_parser.add_subparser("symlink");
    symlink_parser.add_description("Create a symbolic link.");
    symlink_parser.add_argument(std::vector<std::string>{"target_path"}).help("The original file or directory to link to.").required();
    symlink_parser.add_argument(std::vector<std::string>{"link_path"}).help("The path where the symlink will be created.").required();
    symlink_parser.add_argument(std::vector<std::string>{"--directory"}).store_true().help("Specify if the target is a directory (Windows only).");

    auto& shortcut_parser = io_parser.add_subparser("shortcut");
    shortcut_parser.add_description("Create a platform-specific shortcut.");
    shortcut_parser.add_argument(std::vector<std::string>{"target_path"}).help("The original file or directory to link to.").required();
    shortcut_parser.add_argument(std::vector<std::string>{"link_path"}).help("The path where the shortcut will be created.").required();
    shortcut_parser.add_argument(std::vector<std::string>{"--description"}).takes_value().help("A description for the shortcut.");
}

} // namespace allin1::io
