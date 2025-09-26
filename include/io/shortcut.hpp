#pragma once

#include <string>

namespace allin1::io {

void handle_shortcut(
    const std::string& target_path,
    const std::string& link_path,
    const std::string& description,
    bool output_enabled
);

} // namespace allin1::io
