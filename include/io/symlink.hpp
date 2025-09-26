#pragma once

#include <string>

namespace allin1::io {

void handle_symlink(
    const std::string& target_path,
    const std::string& link_path,
    bool is_directory,
    bool output_enabled
);

} // namespace allin1::io
