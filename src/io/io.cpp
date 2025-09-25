#include "io/io.hpp"
#include "common/string_utils.hpp"
#include "common/errors.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>

void register_io_commands(argparse::ArgumentParser& parser) {
    auto& create = parser.add_subparser("create", "Create a file or directory");
    create.add_argument("type", "The type of object to create (file, directory, folder)");
    create.add_argument("path", "The path where the object should be created");

    create.add_argument("--fill")
        .help("Fill the file with a certain hex code (e.g., 0xFF)")
        .default_value(std::string(""))
        .required(false);

    create.add_argument("--fill-size")
        .help("The size to initialize the file to (e.g., 1K, 2M, 3G)")
        .default_value(std::string(""))
        .required(false);
}

void handle_io_create(const argparse::ArgumentParser& create_command, bool output_enabled) {
    try {
        auto type = create_command.get<std::string>("type");
        auto path_str = create_command.get<std::string>("path");
        auto fill_str = create_command.get<std::string>("--fill");
        auto fill_size_str = create_command.get<std::string>("--fill-size");

        if (output_enabled) {
            std::cout << "Settings for io create:" << std::endl;
            std::cout << "  Type: " << type << std::endl;
            std::cout << "  Path: " << path_str << std::endl;
            if (!fill_str.empty()) std::cout << "  Fill: " << fill_str << std::endl;
            if (!fill_size_str.empty()) std::cout << "  Fill Size: " << fill_size_str << std::endl;
        }

        std::filesystem::path path(path_str);

        // Argument validation
        bool use_fill = !fill_str.empty();
        bool use_fill_size = !fill_size_str.empty();

        if (use_fill != use_fill_size) {
            throw std::runtime_error("--fill and --fill-size must be used together.");
        }

        if ((type == "directory") || (type == "folder")) {
            if (use_fill) {
                throw std::runtime_error("--fill and --fill-size can only be used with type 'file'.");
            }
            std::filesystem::create_directories(path);
            if (output_enabled) {
                std::cout << "Directory created: " << path << std::endl;
            }
        } else if (type == "file") {
            if (path.has_parent_path()) {
                std::filesystem::create_directories(path.parent_path());
            }

            std::ofstream file(path, std::ios::binary | std::ios::out);
            if (!file) {
                throw std::runtime_error("Failed to create file: " + path_str);
            }

            if (use_fill) {
                uint64_t size_bytes = parse_size(fill_size_str);
                unsigned char fill_byte = parse_hex_byte(fill_str);

                const size_t buffer_size = 4096;
                std::vector<char> buffer(buffer_size, static_cast<char>(fill_byte));
                
                uint64_t remaining_bytes = size_bytes;
                while (remaining_bytes > 0) {
                    size_t bytes_to_write = std::min((uint64_t)buffer_size, remaining_bytes);
                    file.write(buffer.data(), bytes_to_write);
                    if (!file) {
                        throw std::runtime_error("Failed to write to file: " + path_str);
                    }
                    remaining_bytes -= bytes_to_write;
                }
            }
            if (output_enabled) {
                std::cout << "File created: " << path << std::endl;
            }
        } else {
            throw std::runtime_error("Invalid type: \"" + type + "\". Must be 'file', 'directory', or 'folder'.");
        }

    } catch (const StringSizeParseError& e) {
        std::cerr << "Error parsing size argument: " << e.what() << std::endl;
    } catch (const HexByteParseError& e) {
        std::cerr << "Error parsing fill argument: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
