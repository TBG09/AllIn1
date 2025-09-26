#include "common/error_utils.hpp"
#include <map>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstring> // For strerror
#include <cerrno>  // For POSIX error constants
#endif

namespace allin1::common {

std::string get_system_error_message(unsigned long error_code) {
#ifdef _WIN32
    if (error_code == 0) {
        return "No error message generated.";
    }

    LPSTR message_buffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message_buffer, 0, NULL);

    if (size == 0) {
        return "Unable to format error message for error code " + std::to_string(error_code);
    }

    std::string message(message_buffer, size);
    LocalFree(message_buffer);

    while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) {
        message.pop_back();
    }

    return message;
#else
    return strerror(error_code);
#endif
}

std::string get_contextual_error_message(unsigned long error_code) {
    static const std::map<unsigned long, std::string> contextual_messages = {
#ifdef _WIN32
        {ERROR_ACCESS_DENIED, "Access Denied. Try running as an administrator or check file/directory permissions."},
        {ERROR_FILE_NOT_FOUND, "The system cannot find the file specified. Check if the path is correct."},
        {ERROR_PATH_NOT_FOUND, "The system cannot find the path specified. Check if the directory exists."},
        {ERROR_SHARING_VIOLATION, "The file is being used by another process. Close other programs that might be using the file."},
        {ERROR_ALREADY_EXISTS, "The file or directory already exists and cannot be created."},
        {ERROR_INVALID_NAME, "The filename, directory name, or volume label syntax is incorrect. Check for invalid characters."},
        {ERROR_DIRECTORY, "The directory name is invalid. Ensure it does not contain illegal characters."},
        {ERROR_WRITE_PROTECT, "The media is write-protected. You cannot write to this disk."},
        {ERROR_DISK_FULL, "There is not enough space on the disk."},
        {ERROR_INVALID_DRIVE, "The system cannot find the drive specified. Ensure the drive letter is correct."},
        {ERROR_BAD_NETPATH, "The network path was not found. Check your network connection and the path."},
        {ERROR_BAD_PATHNAME, "The specified path is invalid. Review the full path for errors."},
        {1314, "A required privilege is not held by the client. This operation requires administrator privileges. Try running as an administrator."}
#else
        {EACCES, "Permission denied. Try running with sudo or check file/directory permissions."},
        {ENOENT, "No such file or directory. Check if the path is correct."},
        {EEXIST, "File or directory already exists."},
        {ENOSPC, "No space left on device."},
        {EROFS, "Read-only file system."},
        {EINVAL, "Invalid argument. One of the parameters provided to a system call was invalid."},
        {EISDIR, "Is a directory. You cannot perform a file operation on a directory."},
        {ENOTDIR, "Not a directory. A component of the path prefix is not a directory."},
        {EMLINK, "Too many links. The link count of a file would exceed the maximum allowed."},
        {ENFILE, "Too many open files in the system. The system-wide limit on the total number of open files has been reached."},
        {EMFILE, "Too many open files. The per-process limit on the number of open file descriptors has been reached."},
        {EPERM, "Operation not permitted. You do not have the necessary permissions for this operation."}
#endif
    };

    auto it = contextual_messages.find(error_code);
    if (it != contextual_messages.end()) {
        return it->second;
    }
    return "";
}

} // namespace allin1::common
