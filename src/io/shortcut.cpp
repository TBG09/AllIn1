#include "io/shortcut.hpp"
#include "common/platform.hpp"
#include "common/error_utils.hpp"
#include "common/errors.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <shlobj.h> // For IShellLink, IPersistFile
#include <winnls.h> // For MultiByteToWideChar
#include <objbase.h> // For CoInitializeEx, CoUninitialize
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#endif

namespace allin1::io {

void handle_shortcut(
    const std::string& target_path_str,
    const std::string& link_path_str,
    const std::string& description,
    bool output_enabled
) {
    if (output_enabled) {
        std::cout << "Settings for io shortcut:" << std::endl;
        std::cout << "  Target: " << target_path_str << std::endl;
        std::cout << "  Link: " << link_path_str << std::endl;
        std::cout << "  Description: " << description << std::endl;
    }

    try {
#if defined(_WIN32)
        HRESULT hres;
        IShellLink* psl = nullptr;
        IPersistFile* ppf = nullptr;

        hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hres)) {
            throw common::IOCreateError("Failed to initialize COM library. Code: " + std::to_string(hres) + ": " + common::get_system_error_message(hres));
        }

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
        if (FAILED(hres)) {
            CoUninitialize();
            throw common::IOCreateError("Failed to create IShellLink instance. Code: " + std::to_string(hres) + ": " + common::get_system_error_message(hres));
        }

        hres = psl->SetPath(target_path_str.c_str());
        if (FAILED(hres)) {
            psl->Release();
            CoUninitialize();
            throw common::IOCreateError("Failed to set shortcut target path. Code: " + std::to_string(hres) + ": " + common::get_system_error_message(hres));
        }

        if (!description.empty()) {
            hres = psl->SetDescription(description.c_str());
            if (FAILED(hres)) {
                psl->Release();
                CoUninitialize();
                throw common::IOCreateError("Failed to set shortcut description. Code: " + std::to_string(hres) + ": " + common::get_system_error_message(hres));
            }
        }

        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, link_path_str.c_str(), -1, NULL, 0);
        if (wchars_num == 0) {
            unsigned long error_code = GetLastError();
            psl->Release();
            CoUninitialize();
            std::string sys_msg = common::get_system_error_message(error_code);
            std::string ctx_msg = common::get_contextual_error_message(error_code);
            std::string final_msg = "Failed to convert link path to wide char. Code: " + std::to_string(error_code) + ": " + sys_msg;
            if (!ctx_msg.empty()) {
                final_msg += ". Suggestion: " + ctx_msg;
            }
            throw common::IOCreateError(final_msg);
        }
        std::vector<wchar_t> w_link_path(wchars_num);
        MultiByteToWideChar(CP_UTF8, 0, link_path_str.c_str(), -1, w_link_path.data(), wchars_num);

        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (FAILED(hres)) {
            psl->Release();
            CoUninitialize();
            throw common::IOCreateError("Failed to query IPersistFile interface. Code: " + std::to_string(hres) + ": " + common::get_system_error_message(hres));
        }

        hres = ppf->Save(w_link_path.data(), TRUE);
        if (FAILED(hres)) {
            unsigned long error_code = hres;
            ppf->Release();
            psl->Release();
            CoUninitialize();
            std::string sys_msg = common::get_system_error_message(error_code);
            std::string ctx_msg = common::get_contextual_error_message(error_code);
            std::string final_msg = "Failed to save shortcut file. Code: " + std::to_string(error_code) + ": " + sys_msg;
            if (!ctx_msg.empty()) {
                final_msg += ". Suggestion: " + ctx_msg;
            }
            throw common::IOCreateError(final_msg);
        }

        ppf->Release();
        psl->Release();
        CoUninitialize();

        if (output_enabled) {
            std::cout << "Windows shortcut created: " << link_path_str << " -> " << target_path_str << std::endl;
        }
#elif defined(__linux__)
        std::filesystem::path link_path(link_path_str);
        std::filesystem::path target_path(target_path_str);

        if (link_path.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(link_path.parent_path(), ec);
            if (ec) {
                throw common::IOCreateError("Failed to create parent directories for shortcut: " + ec.message());
            }
        }

        std::ofstream desktop_file(link_path.string() + ".desktop");
        if (!desktop_file.is_open()) {
            throw common::IOCreateError("Failed to create .desktop file: " + link_path.string() + ".desktop");
        }

        desktop_file << "[Desktop Entry]\\n";
        desktop_file << "Type=Application\\n";
        desktop_file << "Name=" << link_path.stem().string() << "\\n";
        desktop_file << "Exec=" << target_path.string() << "\\n";
        if (!description.empty()) {
            desktop_file << "Comment=" << description << "\\n";
        }
        desktop_file << "Terminal=false\\n";
        desktop_file << "Categories=Utility;\\n";
        desktop_file.close();

        std::error_code ec;
        std::filesystem::permissions(link_path.string() + ".desktop",
                                      std::filesystem::perms::owner_exec |
                                      std::filesystem::perms::group_exec |
                                      std::filesystem::perms::others_exec,
                                      std::filesystem::perm_options::add, ec);
        if (ec) {
            throw common::IOCreateError("Failed to set executable permissions on .desktop file: " + ec.message());
        }

        if (output_enabled) {
            std::cout << "Linux .desktop shortcut created: " << link_path.string() << ".desktop -> " << target_path_str << std::endl;
        }
#else
        throw common::IOCreateError("Shortcut creation not supported on this OS.");
#endif
    } catch (const common::IOCreateError&) {
        throw; // Re-throw to be caught in main
    } catch (const std::filesystem::filesystem_error& e) {
        throw common::IOCreateError("A filesystem error occurred: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw common::IOCreateError("An unexpected error occurred: " + std::string(e.what()));
    }
}

} // namespace allin1::io
