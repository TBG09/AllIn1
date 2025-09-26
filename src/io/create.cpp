#include "io/create.hpp"
#include "common/string_utils.hpp"
#include "common/errors.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

// Temporary comment to force recompile (create.cpp)

namespace allin1::io {

unsigned char parse_hex_byte(const std::string& hex_str) {
    if (hex_str.empty()) {
        throw allin1::common::HexByteParseError("Fill hex string cannot be empty.");
    }
    std::string processed_str = hex_str;
    if (hex_str.rfind("0x", 0) == 0 || hex_str.rfind("0X", 0) == 0) {
        processed_str = hex_str.substr(2);
    }
    if (processed_str.length() > 2 || processed_str.empty()) {
        throw allin1::common::HexByteParseError("Invalid hex byte format: \"" + hex_str + "\". Must be 1 or 2 hex characters.");
    }
    try {
        return static_cast<unsigned char>(std::stoul(processed_str, nullptr, 16));
    } catch (const std::invalid_argument&) {
        throw allin1::common::HexByteParseError("Invalid hex character in fill string: \"" + hex_str + "\"");
    } catch (const std::out_of_range&) {
        throw allin1::common::HexByteParseError("Hex value out of range for a byte: \"" + hex_str + "\"");
    }
    return 0; 
}

void handle_create(
    const std::string& type,
    const std::string& path_str,
    const std::string& name,
    const std::string& fill_str,
    const std::string& fill_size_str,
    bool output_enabled
) {
    try {
        if (output_enabled) {
            std::cout << "Settings for io create:" << std::endl;
            std::cout << "  Type: " << type << std::endl;
            std::cout << "  Path: " << path_str << std::endl;
            std::cout << "  Name: " << name << std::endl;
            if (!fill_str.empty()) std::cout << "  Fill: " << fill_str << std::endl;
            if (!fill_size_str.empty()) std::cout << "  Fill Size: " << fill_size_str << std::endl;
        }

        std::filesystem::path full_path = std::filesystem::path(path_str) / name;

        bool use_fill = !fill_str.empty();
        bool use_fill_size = !fill_size_str.empty();

        if (use_fill != use_fill_size) {
            throw std::runtime_error("--fill and --fill-size must be used together.");
        }

        if ((type == "directory") || (type == "folder")) {
            if (use_fill) {
                throw std::runtime_error("--fill and --fill-size can only be used with type 'file'.");
            }
            std::filesystem::create_directories(full_path);
            if (output_enabled) {
                std::cout << "Directory created: " << full_path.string() << std::endl;
            }
        } else if (type == "file") {
            if (full_path.has_parent_path()) {
                std::filesystem::create_directories(full_path.parent_path());
            }

            std::ofstream file(full_path, std::ios::binary | std::ios::out);
            if (!file) {
                throw std::runtime_error("Failed to create file: " + full_path.string());
            }

            if (use_fill) {
                uint64_t size_bytes = allin1::common::parse_size(fill_size_str);
                unsigned char fill_byte = parse_hex_byte(fill_str);

                const size_t buffer_size = 4096;
                std::vector<char> buffer(buffer_size, static_cast<char>(fill_byte));
                
                uint64_t remaining_bytes = size_bytes;
                while (remaining_bytes > 0) {
                    size_t bytes_to_write = std::min((uint64_t)buffer_size, remaining_bytes);
                    file.write(buffer.data(), bytes_to_write);
                    if (!file) {
                        throw std::runtime_error("Failed to write to file: " + full_path.string());
                    }
                    remaining_bytes -= bytes_to_write;
                }
            }
            if (output_enabled) {
                std::cout << "File created: " << full_path.string() << std::endl;
            }
        } else {
            throw std::runtime_error("Invalid type: \"" + type + "\". Must be 'file', 'directory', or 'folder'.");
        }

    } catch (const allin1::common::StringSizeParseError& e) {
        std::cerr << "Error parsing size argument: " << e.what() << std::endl;
    } catch (const allin1::common::HexByteParseError& e) {
        std::cerr << "Error parsing fill argument: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

} // namespace allin1::io
