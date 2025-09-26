#pragma once

#include <string>

namespace allin1::common {

/**
 * @brief Translates a system-specific error code into a human-readable string.
 */
std::string get_system_error_message(unsigned long error_code);

/**
 * @brief Provides a more contextual message for common I/O error codes.
 */
std::string get_contextual_error_message(unsigned long error_code);

} // namespace allin1::common
