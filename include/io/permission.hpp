#pragma once

#include <string>

namespace allin1::io {

void handle_permission(
    const std::string& path,
    const std::string& user,
    const std::string& perm_string,
    bool recursive,
    bool output_enabled
);

} // namespace allin1::io
