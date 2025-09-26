#include "io/create.hpp"
#include "common/string_utils.hpp"
#include "common/error_utils.hpp"
#include "common/errors.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

#if defined(_WIN32)
#include <windows.h> // For GetLastError()
#else
#include <cerrno> // For errno
#endif

namespace allin1::io {

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
            throw common::IOCreateError("--fill and --fill-size must be used together.");
        }

        if ((type == "directory") || (type == "folder")) {
            if (use_fill) {
                throw common::IOCreateError("--fill and --fill-size can only be used with type 'file'.");
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
                unsigned long error_code = 0;
#if defined(_WIN32)
                error_code = GetLastError();
#else
                error_code = errno;
#endif
                std::string sys_msg = common::get_system_error_message(error_code);
                std::string ctx_msg = common::get_contextual_error_message(error_code);
                std::string final_msg = "Failed to create file '" + full_path.string() + "'. Code: " + std::to_string(error_code) + ": " + sys_msg;
                if (!ctx_msg.empty()) {
                    final_msg += ". Suggestion: " + ctx_msg;
                }
                throw common::IOCreateError(final_msg);
            }

            if (use_fill) {
                uint64_t size_bytes;
                unsigned char fill_byte;
                try {
                    size_bytes = common::parse_size(fill_size_str);
                    fill_byte = common::parse_hex_byte(fill_str);
                } catch (const common::StringSizeParseError& e) {
                    throw common::IOCreateError("Error parsing size argument: " + std::string(e.what()));
                } catch (const common::HexByteParseError& e) {
                    throw common::IOCreateError("Error parsing fill argument: " + std::string(e.what()));
                }

                const size_t buffer_size = 4096;
                std::vector<char> buffer(buffer_size, static_cast<char>(fill_byte));
                
                uint64_t remaining_bytes = size_bytes;
                while (remaining_bytes > 0) {
                    size_t bytes_to_write = std::min((uint64_t)buffer_size, remaining_bytes);
                    file.write(buffer.data(), bytes_to_write);
                    if (!file) {
                         unsigned long error_code = 0;
#if defined(_WIN32)
                        error_code = GetLastError();
#else
                        error_code = errno;
#endif
                        std::string sys_msg = common::get_system_error_message(error_code);
                        std::string ctx_msg = common::get_contextual_error_message(error_code);
                        std::string final_msg = "Failed to write to file '" + full_path.string() + "'. Code: " + std::to_string(error_code) + ": " + sys_msg;
                        if (!ctx_msg.empty()) {
                            final_msg += ". Suggestion: " + ctx_msg;
                        }
                        throw common::IOCreateError(final_msg);
                    }
                    remaining_bytes -= bytes_to_write;
                }
            }
            if (output_enabled) {
                std::cout << "File created: " << full_path.string() << std::endl;
            }
        } else {
            throw common::IOCreateError("Invalid type: \"" + type + "\". Must be 'file', 'directory', or 'folder'.");
        }

    } catch (const common::IOCreateError&) {
        throw; // Re-throw to be caught in main
    } catch (const std::filesystem::filesystem_error& e) {
        std::string sys_msg = common::get_system_error_message(e.code().value());
        std::string ctx_msg = common::get_contextual_error_message(e.code().value());
        std::string final_msg = "Filesystem error: " + std::string(e.what()) + ". " + sys_msg;
        if (!ctx_msg.empty()) {
            final_msg += ". Suggestion: " + ctx_msg;
        }
        throw common::IOCreateError(final_msg);
    } catch (const std::exception& e) {
        throw common::IOCreateError("An unexpected error occurred: " + std::string(e.what()));
    }
}

} // namespace allin1::io
