#pragma once

#include <string>
#include <map>

namespace allin1::common {

enum class OS {
    Unknown,
    Windows,
    Linux,
    MacOS,
    FreeBSD,
    OpenBSD,
    NetBSD,
    Solaris,
    Android,
    iOS
};

struct LinuxDistributionInfo {
    std::string id;
    std::string id_like;
    std::string name;
    std::string version;
    std::string version_id;
    std::string pretty_name;
    std::string build_id;
    std::string variant;
    std::string variant_id;
    std::string codename;
    std::string ansi_color;
    std::string home_url;
    std::string bug_report_url;
    std::string support_url;
    std::string privacy_policy_url;
    std::string logo;
    std::string cpe_name;
    std::map<std::string, std::string> other_fields; // For any other fields in os-release

    bool is_detected() const { return !id.empty(); }
};

OS get_current_os();
std::string get_os_name(OS os);

LinuxDistributionInfo get_linux_distribution();

// --- Cross-Platform System Information ---
std::string get_cpu_architecture();
std::string get_kernel_version();
std::string get_hostname();

// --- More Extensive System Information ---
std::string get_cpu_model_name();
int get_cpu_core_count();
long long get_total_memory_bytes();
long long get_available_memory_bytes();

} // namespace allin1::common
