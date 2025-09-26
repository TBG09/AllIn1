#include "io/symlink.hpp"
#include "common/platform.hpp"
#include "common/error_utils.hpp"
#include "common/errors.hpp"

#include <iostream>
#include <filesystem>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace allin1::io {

void handle_symlink(
    const std::string& target_path_str,
    const std::string& link_path_str,
    bool is_directory,
    bool output_enabled
) {
    std::filesystem::path target_path(target_path_str);
    std::filesystem::path link_path(link_path_str);

    if (output_enabled) {
        std::cout << "Settings for io symlink:" << std::endl;
        std::cout << "  Target: " << target_path_str << std::endl;
        std::cout << "  Link: " << link_path_str << std::endl;
        std::cout << "  Type: " << (is_directory ? "directory" : "file") << std::endl;
    }

    try {
#if defined(_WIN32)
        DWORD flags = is_directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
        if (!CreateSymbolicLinkW(link_path.c_str(), target_path.c_str(), flags)) {
            unsigned long error_code = GetLastError();
            std::string error_message = common::get_system_error_message(error_code);
            throw common::IOCreateError("Failed to create symlink on Windows. Code: " + std::to_string(error_code) + ": " + error_message);
        }
#else
        if (is_directory) {
            std::filesystem::create_directory_symlink(target_path, link_path);
        } else {
            std::filesystem::create_symlink(target_path, link_path);
        }
#endif
        if (output_enabled) {
            std::cout << "Symlink created: " << link_path_str << " -> " << target_path_str << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        throw common::IOCreateError("Failed to create symlink: " + std::string(e.what()));
    } catch (const common::IOCreateError&) {
        throw; // Re-throw IOCreateError to be caught in main
    } catch (const std::exception& e) {
        throw common::IOCreateError("An unexpected error occurred: " + std::string(e.what()));
    }
}

} // namespace allin1::io
