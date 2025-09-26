#include "common/permission_utils.hpp"
#include "common/error_utils.hpp"
#include "common/errors.hpp"

#ifdef _WIN32
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#endif

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <filesystem>

namespace allin1::common {

Permissions parse_permission_string(const std::string& perm_string) {
    std::string lower_perm = perm_string;
    std::transform(lower_perm.begin(), lower_perm.end(), lower_perm.begin(), ::tolower);

    if (lower_perm == "full") return {true, true, true};
    if (lower_perm == "modify") return {true, true, true};
    if (lower_perm == "readexec") return {true, false, true};
    if (lower_perm == "read") return {true, false, false};
    if (lower_perm == "write") return {false, true, false};

    try {
        size_t pos;
        unsigned long value = std::stoul(perm_string, &pos, 0);
        if (pos != perm_string.length()) throw std::invalid_argument("Invalid chars");

        Permissions perms;
        if (perm_string.rfind("0x", 0) == 0) { // Hex (Windows)
            if ((value & GENERIC_ALL) == GENERIC_ALL || (value & 0x1f01ff) == 0x1f01ff) return {true, true, true};
            perms.read = (value & GENERIC_READ) == GENERIC_READ;
            perms.write = (value & GENERIC_WRITE) == GENERIC_WRITE;
            perms.execute = (value & GENERIC_EXECUTE) == GENERIC_EXECUTE;
        } else { // Octal (Linux)
            perms.read = (value & 4) != 0;
            perms.write = (value & 2) != 0;
            perms.execute = (value & 1) != 0;
        }
        return perms;
    } catch (const std::exception&) {
        throw PermissionError("Invalid permission string: \"" + perm_string + "\".");
    }
}

#ifdef _WIN32
void set_single_win_permission(const std::string& path, const std::string& user, const Permissions& perms, bool output_enabled) {
    DWORD access_mask = 0;
    if (perms.read) access_mask |= FILE_GENERIC_READ;
    if (perms.write) access_mask |= FILE_GENERIC_WRITE;
    if (perms.execute) access_mask |= FILE_GENERIC_EXECUTE;
    if (perms.read && perms.write && perms.execute) access_mask = GENERIC_ALL;

    auto sid_buffer = std::make_unique<BYTE[]>(SECURITY_MAX_SID_SIZE);
    PSID p_sid = (PSID)sid_buffer.get();
    DWORD sid_size = SECURITY_MAX_SID_SIZE;
    auto domain_buffer = std::make_unique<char[]>(256);
    DWORD domain_size = 256;
    SID_NAME_USE sid_name_use;

    if (!LookupAccountName(NULL, user.c_str(), p_sid, &sid_size, domain_buffer.get(), &domain_size, &sid_name_use)) {
        throw PermissionError("LookupAccountName failed for user '" + user + "': " + get_system_error_message(GetLastError()));
    }

    EXPLICIT_ACCESS ea;
    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = access_mask;
    ea.grfAccessMode = SET_ACCESS;
    ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_USER;
    ea.Trustee.ptstrName = (LPTSTR)p_sid;

    PACL p_old_dacl = nullptr, p_new_dacl = nullptr;
    PSECURITY_DESCRIPTOR p_sd = nullptr;
    GetNamedSecurityInfo(path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &p_old_dacl, NULL, &p_sd);

    if (SetEntriesInAcl(1, &ea, p_old_dacl, &p_new_dacl) != ERROR_SUCCESS) {
        if(p_sd) LocalFree(p_sd);
        throw PermissionError("SetEntriesInAcl failed: " + get_system_error_message(GetLastError()));
    }

    if (SetNamedSecurityInfo((LPSTR)path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, p_new_dacl, NULL) != ERROR_SUCCESS) {
        if(p_new_dacl) LocalFree(p_new_dacl);
        if(p_sd) LocalFree(p_sd);
        throw PermissionError("SetNamedSecurityInfo failed: " + get_system_error_message(GetLastError()));
    }

    if (output_enabled) std::cout << "Set permissions for " << user << " on " << path << std::endl;
    if(p_new_dacl) LocalFree(p_new_dacl);
    if(p_sd) LocalFree(p_sd);
}
#else
void set_single_linux_permission(const std::string& path, const std::string& user, const Permissions& perms, bool output_enabled) {
    struct passwd *pw = getpwnam(user.c_str());
    if (!pw) throw PermissionError("User '" + user + "' not found.");

    uid_t uid = pw->pw_uid;
    gid_t gid = pw->pw_gid;

    if (chown(path.c_str(), uid, gid) != 0) {
        throw PermissionError("chown failed on '" + path + "': " + std::string(strerror(errno)));
    }

    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        throw PermissionError("stat failed on '" + path + "': " + std::string(strerror(errno)));
    }

    mode_t new_mode = st.st_mode;
    new_mode &= ~(S_IRWXU); // Clear user permissions
    if (perms.read) new_mode |= S_IRUSR;
    if (perms.write) new_mode |= S_IWUSR;
    if (perms.execute) new_mode |= S_IXUSR;

    if (chmod(path.c_str(), new_mode) != 0) {
        throw PermissionError("chmod failed on '" + path + "': " + std::string(strerror(errno)));
    }
    if (output_enabled) std::cout << "Set permissions for " << user << " on " << path << std::endl;
}
#endif

void set_permissions(const std::string& path, const std::string& user, const Permissions& perms, bool recursive, bool output_enabled) {
    std::filesystem::path fs_path(path);
    if (!std::filesystem::exists(fs_path)) {
        throw PermissionError("Path does not exist: " + path);
    }

    if (recursive && std::filesystem::is_directory(fs_path)) {
        // Apply to the directory itself first
        try {
#ifdef _WIN32
            set_single_win_permission(fs_path.string(), user, perms, output_enabled);
#else
            set_single_linux_permission(fs_path.string(), user, perms, output_enabled);
#endif
        } catch (const PermissionError& e) {
            std::cerr << "Error setting permission on directory " << fs_path.string() << ": " << e.what() << std::endl;
        }


        for (const auto& entry : std::filesystem::recursive_directory_iterator(fs_path)) {
            try {
#ifdef _WIN32
                set_single_win_permission(entry.path().string(), user, perms, output_enabled);
#else
                set_single_linux_permission(entry.path().string(), user, perms, output_enabled);
#endif
            } catch (const PermissionError& e) {
                // Report error and continue to the next file
                std::cerr << "Error setting permission on " << entry.path().string() << ": " << e.what() << std::endl;
            }
        }
    } else {
#ifdef _WIN32
        set_single_win_permission(path, user, perms, output_enabled);
#else
        set_single_linux_permission(path, user, perms, output_enabled);
#endif
    }
}

} // namespace allin1::common
