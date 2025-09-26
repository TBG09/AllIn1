#include "io/permission.hpp"
#include "common/permission_utils.hpp"
#include "common/error_utils.hpp"

#include <iostream>

namespace allin1::io {

void handle_permission(
    const std::string& path,
    const std::string& user,
    const std::string& perm_string,
    bool recursive,
    bool output_enabled
) {
    if (output_enabled) {
        std::cout << "Settings for io permission:" << std::endl;
        std::cout << "  Path: " << path << std::endl;
        std::cout << "  User: " << user << std::endl;
        std::cout << "  Permissions: " << perm_string << std::endl;
        std::cout << "  Recursive: " << (recursive ? "true" : "false") << std::endl;
    }

    try {

        common::Permissions perms = common::parse_permission_string(perm_string);


        common::set_permissions(path, user, perms, recursive, output_enabled);

        if (output_enabled) {
            std::cout << "Permission operation completed successfully." << std::endl;
        }

    } catch (const common::PermissionError& e) {
        std::cerr << "Permission Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
}

} // namespace allin1::io
