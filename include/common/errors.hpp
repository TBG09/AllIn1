#pragma once

#include <stdexcept>
#include <string>

namespace allin1::common {

// For parsing errors in string_utils
class StringSizeParseError : public std::runtime_error {
public:
    explicit StringSizeParseError(const std::string& message);
};

class HexByteParseError : public std::runtime_error {
public:
    explicit HexByteParseError(const std::string& message);
};

// For I/O operation errors in create, symlink, etc.
class IOCreateError : public std::runtime_error {
public:
    explicit IOCreateError(const std::string& message);
};

// For permission-related errors in permission_utils
class PermissionError : public std::runtime_error {
public:
    explicit PermissionError(const std::string& message);
};

} // namespace allin1::common
