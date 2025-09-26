#pragma once

#include "errors.hpp" // Include the consolidated error definitions
#include <string>
#include <vector>

namespace allin1::common {

/**
 * @brief Represents a unified set of permissions.
 */
struct Permissions {
    bool read = false;
    bool write = false;
    bool execute = false;
};

/**
 * @brief Sets the permissions for a given user on a file or directory.
 */
void set_permissions(const std::string& path, const std::string& user, const Permissions& perms, bool recursive, bool output_enabled);

/**
 * @brief Parses a permission string (keyword, octal, or hex) into a Permissions struct.
 */
Permissions parse_permission_string(const std::string& perm_string);

} // namespace allin1::common
