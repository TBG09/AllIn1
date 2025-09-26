#include <iostream>
#include <string_view>
#include <vector>
#include "cppParse/parser.hpp"
#include "cppParse/help_formatter.hpp"
#include "io/io.hpp"
#include "io/version.hpp"
#include "common/platform.hpp"
#include "io/create.hpp"
#include "io/shortcut.hpp"
#include "io/symlink.hpp"
#include "io/permission.hpp"

constexpr std::string_view app_version = "0.1.0a";

int main(int argc, char *argv[]) {
    cppParse::Parser program("AllIn1-alpha", app_version.data());
    program.add_description("A collection of command-line tools.");
    program.add_argument(std::vector<std::string>{"-v", "--version"}).store_true().help("shows version information and exits");
    program.add_argument(std::vector<std::string>{"--output"}).store_true().help("Enable output messages");

    auto& io_parser = program.add_subparser("io");
    io_parser.add_description("Perform I/O operations.");

    allin1::io::register_io_commands(io_parser);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        return 1;
    }

    if (program.get<bool>("version")) {
        std::cout << "AllIn1 version " << app_version << std::endl;
        std::cout << "  - allin1_io version " << allin1::io::version << std::endl;
        return 0;
    }

    if (program.is_subcommand_used("io")) {
        auto& used_io_parser = program.get_subparser("io");
        if (used_io_parser.is_subcommand_used("create")) {
            auto& used_create_parser = used_io_parser.get_subparser("create");

            bool output_enabled = program.get<bool>("output");
            std::string type = used_create_parser.get<std::string>("type");
            std::string path = used_create_parser.get<std::string>("path");
            std::string name = used_create_parser.get<std::string>("name");
            std::string fill = used_create_parser.get<std::string>("fill");
            std::string fill_size = used_create_parser.get<std::string>("fill-size");

            allin1::io::handle_create(type, path, name, fill, fill_size, output_enabled);
        } else if (used_io_parser.is_subcommand_used("symlink")) {
            auto& used_symlink_parser = used_io_parser.get_subparser("symlink");

            bool output_enabled = program.get<bool>("output");
            std::string target_path = used_symlink_parser.get<std::string>("target_path");
            std::string link_path = used_symlink_parser.get<std::string>("link_path");
            bool is_directory = used_symlink_parser.get<bool>("directory");

            allin1::io::handle_symlink(target_path, link_path, is_directory, output_enabled);
        } else if (used_io_parser.is_subcommand_used("shortcut")) {
            auto& used_shortcut_parser = used_io_parser.get_subparser("shortcut");

            bool output_enabled = program.get<bool>("output");
            std::string target_path = used_shortcut_parser.get<std::string>("target_path");
            std::string link_path = used_shortcut_parser.get<std::string>("link_path");
            std::string description = used_shortcut_parser.get<std::string>("description");

            allin1::io::handle_shortcut(target_path, link_path, description, output_enabled);
        } else if (used_io_parser.is_subcommand_used("permission")) {
            auto& used_permission_parser = used_io_parser.get_subparser("permission");

            bool output_enabled = program.get<bool>("output");
            std::string path = used_permission_parser.get<std::string>("path");
            std::string user = used_permission_parser.get<std::string>("user");
            std::string permissions = used_permission_parser.get<std::string>("permissions");
            bool recursive = used_permission_parser.get<bool>("recursive");

            allin1::io::handle_permission(path, user, permissions, recursive, output_enabled);
        } else {
            cppParse::HelpFormatter formatter(used_io_parser);
            std::cout << formatter.format();
        }
    } else {
        if (argc == 1) {
            std::cout << "Welcome to AllIn1. Use --help to see available commands." << std::endl;
        }
    }

    return 0;
}
