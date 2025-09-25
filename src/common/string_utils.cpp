#include "common/string_utils.hpp"
#include "common/errors.hpp"
#include <stdexcept>
#include <cctype>

// Parses a size string (e.g., "1K", "2M", "3G") and returns the size in bytes.
// Throws a StringSizeParseError if the format is invalid.
uint64_t parse_size(const std::string& size_str) {
    if (size_str.empty()) {
        throw StringSizeParseError("Size string cannot be empty.");
    }

    char last_char = static_cast<char>(std::toupper(size_str.back()));
    std::string num_part = size_str;
    uint64_t multiplier = 1;

    if (!std::isdigit(last_char)) {
        num_part.pop_back(); // Remove the suffix
        switch (last_char) {
            case 'B':
                multiplier = 1;
                break;
            case 'K':
                multiplier = 1024;
                break;
            case 'M':
                multiplier = 1024 * 1024;
                break;
            case 'G':
                multiplier = 1024 * 1024 * 1024;
                break;
            default:
                throw StringSizeParseError("Invalid size suffix: " + std::string(1, last_char));
        }
    }

    try {
        // Use std::stoull for unsigned 64-bit integer conversion
        size_t pos;
        uint64_t base_size = std::stoull(num_part, &pos);
        if (pos != num_part.length()) {
            // This case handles inputs like "123xyzK"
            throw std::invalid_argument("Invalid number format");
        }
        return base_size * multiplier;
    } catch (const std::invalid_argument&) {
        throw StringSizeParseError("Invalid number format in size string: \"" + num_part + "\"");
    } catch (const std::out_of_range&) {
        throw StringSizeParseError("Size value out of range: \"" + num_part + "\"");
    }
}

// Parses a hex string (e.g., "0xFF", "FF") and returns the corresponding byte value.
// Throws a HexByteParseError if the format is invalid.
unsigned char parse_hex_byte(const std::string& hex_str) {
    if (hex_str.empty()) {
        throw HexByteParseError("Fill hex string cannot be empty.");
    }
    // Allow "0x" prefix
    std::string processed_str = hex_str;
    if (hex_str.rfind("0x", 0) == 0 || hex_str.rfind("0X", 0) == 0) {
        processed_str = hex_str.substr(2);
    }
    if (processed_str.length() > 2 || processed_str.empty()) {
        throw HexByteParseError("Invalid hex byte format: \"" + hex_str + "\". Must be 1 or 2 hex characters.");
    }
    try {
        return static_cast<unsigned char>(std::stoul(processed_str, nullptr, 16));
    } catch (const std::invalid_argument&) {
        throw HexByteParseError("Invalid hex character in fill string: \"" + hex_str + "\"");
    } catch (const std::out_of_range&) {
        throw HexByteParseError("Hex value out of range for a byte: \"" + hex_str + "\"");
    }
}
