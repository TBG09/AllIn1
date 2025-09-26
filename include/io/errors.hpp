#pragma once

#include <stdexcept>
#include <string>

namespace allin1::io {

class IOCreateError : public std::runtime_error {
public:
    explicit IOCreateError(const std::string& message, int code = 0) 
        : std::runtime_error(message), error_code(code) {}

    int code() const {
        return error_code;
    }

private:
    int error_code;
};

std::string get_error_message(int error_code);

} // namespace allin1::io
