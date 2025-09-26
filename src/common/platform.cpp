#include "common/platform.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <stdexcept>

#if defined(_WIN32)
#include <windows.h>
#include <winreg.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
#include <sys/utsname.h>
#include <unistd.h>
#endif

namespace allin1::common {

OS get_current_os() {
#if defined(_WIN32)
    return OS::Windows;
#elif defined(__APPLE__) && defined(__MACH__)
    return OS::MacOS;
#elif defined(__linux__)
    return OS::Linux;
#elif defined(__FreeBSD__)
    return OS::FreeBSD;
#elif defined(__OpenBSD__)
    return OS::OpenBSD;
#elif defined(__NetBSD__)
    return OS::NetBSD;
#elif defined(__ANDROID__)
    return OS::Android;
#elif defined(__sun)
    return OS::Solaris;
#else
    return OS::Unknown;
#endif
}

std::string get_os_name(OS os) {
    switch (os) {
        case OS::Windows:
            return "Windows";
        case OS::Linux:
            return "Linux";
        case OS::MacOS:
            return "macOS";
        case OS::FreeBSD:
            return "FreeBSD";
        case OS::OpenBSD:
            return "OpenBSD";
        case OS::NetBSD:
            return "NetBSD";
        case OS::Android:
            return "Android";
        case OS::Solaris:
            return "Solaris";
        case OS::Unknown:
        default:
            return "Unknown";
    }
}

std::string trim_and_unquote(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    if (s.length() >= 2 && s.front() == '"' && s.back() == '"' ) {
        s = s.substr(1, s.length() - 2);
    }
    return s;
}

#if defined(__linux__)
void parse_key_value_file(const std::string& filepath, std::map<std::string, std::string>& data) {
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = line.substr(0, eq_pos);
                std::string value = trim_and_unquote(line.substr(eq_pos + 1));
                data[key] = value;
            }
        }
    }
}
#endif

LinuxDistributionInfo get_linux_distribution() {
    LinuxDistributionInfo info;
#if defined(__linux__)
    std::map<std::string, std::string> os_release_data;
    parse_key_value_file("/etc/os-release", os_release_data);

    if (os_release_data.count("ID")) info.id = os_release_data["ID"];
    if (os_release_data.count("ID_LIKE")) info.id_like = os_release_data["ID_LIKE"];
    if (os_release_data.count("NAME")) info.name = os_release_data["NAME"];
    if (os_release_data.count("VERSION")) info.version = os_release_data["VERSION"];
    if (os_release_data.count("VERSION_ID")) info.version_id = os_release_data["VERSION_ID"];
    if (os_release_data.count("PRETTY_NAME")) info.pretty_name = os_release_data["PRETTY_NAME"];
    if (os_release_data.count("BUILD_ID")) info.build_id = os_release_data["BUILD_ID"];
    if (os_release_data.count("VARIANT")) info.variant = os_release_data["VARIANT"];
    if (os_release_data.count("VARIANT_ID")) info.variant_id = os_release_data["VARIANT_ID"];
    if (os_release_data.count("CODENAME")) info.codename = os_release_data["CODENAME"];
    if (os_release_data.count("ANSI_COLOR")) info.ansi_color = os_release_data["ANSI_COLOR"];
    if (os_release_data.count("HOME_URL")) info.home_url = os_release_data["HOME_URL"];
    if (os_release_data.count("BUG_REPORT_URL")) info.bug_report_url = os_release_data["BUG_REPORT_URL"];
    if (os_release_data.count("SUPPORT_URL")) info.support_url = os_release_data["SUPPORT_URL"];
    if (os_release_data.count("PRIVACY_POLICY_URL")) info.privacy_policy_url = os_release_data["PRIVACY_POLICY_URL"];
    if (os_release_data.count("LOGO")) info.logo = os_release_data["LOGO"];
    if (os_release_data.count("CPE_NAME")) info.cpe_name = os_release_data["CPE_NAME"];
    info.other_fields = os_release_data;

    if (info.id.empty()) {
        std::map<std::string, std::string> lsb_release_data;
        parse_key_value_file("/etc/lsb-release", lsb_release_data);
        if (lsb_release_data.count("DISTRIB_ID")) info.id = lsb_release_data["DISTRIB_ID"];
        if (lsb_release_data.count("DISTRIB_DESCRIPTION")) info.pretty_name = lsb_release_data["DISTRIB_DESCRIPTION"];
        if (lsb_release_data.count("DISTRIB_RELEASE")) info.version_id = lsb_release_data["DISTRIB_RELEASE"];
        if (lsb_release_data.count("DISTRIB_CODENAME")) info.codename = lsb_release_data["DISTRIB_CODENAME"];
    }

    if (info.id.empty()) {
        std::ifstream file;
        file.open("/etc/debian_version");
        if (file.is_open()) { info.id = "debian"; std::getline(file, info.version); file.close(); }
        else { file.open("/etc/redhat-release"); if (file.is_open()) { info.id = "redhat"; std::getline(file, info.pretty_name); file.close(); } }
        
        file.open("/etc/arch-release");
        if (file.is_open()) { info.id = "arch"; info.name = "Arch Linux"; info.pretty_name = "Arch Linux"; file.close(); }
        
        file.open("/etc/gentoo-release");
        if (file.is_open()) { info.id = "gentoo"; info.name = "Gentoo"; std::getline(file, info.pretty_name); file.close(); }

        file.open("/etc/alpine-release");
        if (file.is_open()) { info.id = "alpine"; info.name = "Alpine Linux"; std::getline(file, info.version); info.pretty_name = "Alpine Linux " + info.version; file.close(); }

        file.open("/etc/SuSE-release");
        if (file.is_open()) { info.id = "suse"; info.name = "SUSE Linux"; std::getline(file, info.pretty_name); file.close(); }
    }

    if (info.pretty_name.empty()) {
        if (!info.name.empty()) {
            info.pretty_name = info.name;
            if (!info.version.empty()) {
                info.pretty_name += " " + info.version;
            }
        } else {
            info.pretty_name = "Linux (Distribution Unknown)";
        }
    }

#else
#endif
    return info;
}

std::string get_cpu_architecture() {
#if defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: return "x86_64";
        case PROCESSOR_ARCHITECTURE_ARM:   return "ARM";
        case PROCESSOR_ARCHITECTURE_ARM64: return "ARM64";
        case PROCESSOR_ARCHITECTURE_INTEL: return "x86";
        default:                           return "Unknown";
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
    struct utsname buf;
    if (uname(&buf) == 0) {
        return buf.machine;
    }
    return "Unknown";
#else
    return "Unknown";
#endif
}

std::string get_kernel_version() {
#if defined(_WIN32)
    typedef struct _RTL_OSVERSIONINFOEXW {
        ULONG dwOSVersionInfoSize;
        ULONG dwMajorVersion;
        ULONG dwMinorVersion;
        ULONG dwBuildNumber;
        ULONG dwPlatformId;
        WCHAR szCSDVersion[128];
        USHORT wServicePackMajor;
        USHORT wServicePackMinor;
        USHORT wSuiteMask;
        UCHAR wProductType;
        UCHAR wReserved;
    } RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;

    typedef LONG (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOEXW);

    RTL_OSVERSIONINFOEXW osVersionInfo;
    ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

    HMODULE hNtdll = LoadLibraryA("ntdll.dll");
    if (hNtdll) {
        RtlGetVersionPtr RtlGetVersionFunc = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (RtlGetVersionFunc) {
            RtlGetVersionFunc(&osVersionInfo);
            FreeLibrary(hNtdll);
            std::stringstream ss;
            ss << osVersionInfo.dwMajorVersion << "." << osVersionInfo.dwMinorVersion << "." << osVersionInfo.dwBuildNumber;
            return ss.str();
        }
        FreeLibrary(hNtdll);
    }
    return "Windows Kernel (RtlGetVersion failed)";
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
    struct utsname buf;
    if (uname(&buf) == 0) {
        return buf.release;
    }
    return "Unknown";
#else
    return "Unknown";
#endif
}

std::string get_hostname() {
#if defined(_WIN32)
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return buffer;
    }
    return "Unknown";
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, HOST_NAME_MAX + 1) == 0) {
        return hostname;
    }
    return "Unknown";
#else
    return "Unknown";
#endif
}

std::string get_cpu_model_name() {
#if defined(_WIN32)
    HKEY hKey;
    LONG lRes = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey);
    if (lRes == ERROR_SUCCESS) {
        char buffer[256];
        DWORD bufferSize = sizeof(buffer);
        lRes = RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)buffer, &bufferSize);
        if (lRes == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return buffer;
        }
        RegCloseKey(hKey);
    }
    return "Unknown";
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.rfind("model name\t:", 0) == 0) {
                return trim_and_unquote(line.substr(line.find(":") + 1));
            }
        }
    }
    return "Unknown";
#else
    return "Unknown";
#endif
}

int get_cpu_core_count() {
#if defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        int cores = 0;
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.rfind("cpu cores\t:", 0) == 0) {
                cores = std::stoi(trim_and_unquote(line.substr(line.find(":") + 1)));
                break;
            }
        }
        if (cores > 0) return cores;

        cpuinfo.clear();
        cpuinfo.seekg(0);
        int physical_id = -1;
        int processor_count = 0;
        std::map<int, int> cores_per_physical_id;

        while (std::getline(cpuinfo, line)) {
            if (line.rfind("physical id\t:", 0) == 0) {
                physical_id = std::stoi(trim_and_unquote(line.substr(line.find(":") + 1)));
            } else if (line.rfind("cpu cores\t:", 0) == 0 && physical_id != -1) {
                cores_per_physical_id[physical_id] = std::stoi(trim_and_unquote(line.substr(line.find(":") + 1)));
            } else if (line.rfind("processor\t:", 0) == 0) {
                processor_count++;
            }
        }

        if (!cores_per_physical_id.empty()) {
            int total_cores = 0;
            for (const auto& pair : cores_per_physical_id) {
                total_cores += pair.second;
            }
            return total_cores;
        } else if (processor_count > 0) {
            return processor_count;
        }
    }
    return 0;
#else
    return 0;
#endif
}

long long get_total_memory_bytes() {
#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return static_cast<long long>(status.ullTotalPhys);
    }
    return 0;
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        while (std::getline(meminfo, line)) {
            if (line.rfind("MemTotal:", 0) == 0) {
                std::stringstream ss(line);
                std::string key;
                long long value;
                std::string unit;
                ss >> key >> value >> unit;
                if (unit == "kB") return value * 1024;
            }
        }
    }
    return 0;
#else
    return 0;
#endif
}

long long get_available_memory_bytes() {
#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return static_cast<long long>(status.ullAvailPhys);
    }
    return 0;
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun)
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        long long mem_free = 0;
        long long buffers = 0;
        long long cached = 0;

        while (std::getline(meminfo, line)) {
            if (line.rfind("MemFree:", 0) == 0) {
                std::stringstream ss(line);
                std::string key;
                ss >> key >> mem_free;
            } else if (line.rfind("Buffers:", 0) == 0) {
                std::stringstream ss(line);
                std::string key;
                ss >> key >> buffers;
            } else if (line.rfind("Cached:", 0) == 0) {
                std::stringstream ss(line);
                std::string key;
                ss >> key >> cached;
            }
        }
        return (mem_free + buffers + cached) * 1024;
    }
    return 0;
#else
    return 0;
#endif
}

} // namespace allin1::common
