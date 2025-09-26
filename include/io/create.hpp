#pragma once

#include <string>

namespace allin1::io {

void handle_create(
    const std::string& type,
    const std::string& path,
    const std::string& name,
    const std::string& fill,
    const std::string& fill_size,
    bool output_enabled
);

} // namespace allin1::io
