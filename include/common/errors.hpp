#pragma once

#include <stdexcept>
#include <string>

namespace allin1::common {

class StringSizeParseError : public std::runtime_error {
public:
    explicit StringSizeParseError(const std::string& message)
        : std::runtime_error(message) {}
};

class HexByteParseError : public std::runtime_error {
public:
    explicit HexByteParseError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace allin1::common
