#include "io/symlink.hpp"
#include "common/platform.hpp"

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
            throw std::runtime_error("Failed to create symlink on Windows. Error code: " + std::to_string(GetLastError()));
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
        std::cerr << "Error creating symlink: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

} // namespace allin1::io
