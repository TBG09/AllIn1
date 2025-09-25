#pragma once

#include <stdexcept>
#include <string>

// Custom exception for errors during size string parsing.
class StringSizeParseError : public std::runtime_error {
public:
    explicit StringSizeParseError(const std::string& message)
        : std::runtime_error(message) {}
};

// Custom exception for errors during hex byte string parsing.
class HexByteParseError : public std::runtime_error {
public:
    explicit HexByteParseError(const std::string& message)
        : std::runtime_error(message) {}
};
