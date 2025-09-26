#pragma once

#include <string>
#include <cstdint>

namespace allin1::common {

// Parses a size string (e.g., "1K", "2M", "3G") and returns the size in bytes.
// Throws a StringSizeParseError if the format is invalid.
uint64_t parse_size(const std::string& size_str);

// Parses a hex string (e.g., "0xFF", "FF") and returns the corresponding byte value.
// Throws a HexByteParseError if the format is invalid.
unsigned char parse_hex_byte(const std::string& hex_str);

} // namespace allin1::common
