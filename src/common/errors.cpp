#include "common/errors.hpp"

namespace allin1::common {

StringSizeParseError::StringSizeParseError(const std::string& message) : std::runtime_error(message) {}

HexByteParseError::HexByteParseError(const std::string& message) : std::runtime_error(message) {}

IOCreateError::IOCreateError(const std::string& message) : std::runtime_error(message) {}

PermissionError::PermissionError(const std::string& message) : std::runtime_error(message) {}

} // namespace allin1::common
