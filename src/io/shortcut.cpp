#include "io/shortcut.hpp"
#include "common/platform.hpp"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector> // Added for std::vector

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
        IShellLink* psl;

        hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hres)) {
            throw std::runtime_error("Failed to initialize COM library.");
        }

        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
        if (FAILED(hres)) {
            CoUninitialize();
            throw std::runtime_error("Failed to create IShellLink instance.");
        }

        // Set the path to the shortcut target
        hres = psl->SetPath(target_path_str.c_str());
        if (FAILED(hres)) {
            psl->Release();
            CoUninitialize();
            throw std::runtime_error("Failed to set shortcut target path.");
        }

        // Set the description (comments) of the shortcut
        if (!description.empty()) {
            hres = psl->SetDescription(description.c_str());
            if (FAILED(hres)) {
                psl->Release();
                CoUninitialize();
                throw std::runtime_error("Failed to set shortcut description.");
            }
        }

        // Resolve the link path to a wide character string for IPersistFile
        int wchars_num = MultiByteToWideChar(CP_UTF8, 0, link_path_str.c_str(), -1, NULL, 0);
        if (wchars_num == 0) {
            psl->Release();
            CoUninitialize();
            throw std::runtime_error("Failed to convert link path to wide char (size check).");
        }
        std::vector<wchar_t> w_link_path(wchars_num);
        MultiByteToWideChar(CP_UTF8, 0, link_path_str.c_str(), -1, w_link_path.data(), wchars_num);

        // Save the shortcut
        IPersistFile* ppf;
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (FAILED(hres)) {
            psl->Release();
            CoUninitialize();
            throw std::runtime_error("Failed to query IPersistFile interface.");
        }

        hres = ppf->Save(w_link_path.data(), TRUE);
        if (FAILED(hres)) {
            ppf->Release();
            psl->Release();
            CoUninitialize();
            throw std::runtime_error("Failed to save shortcut file.");
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

        // Ensure the directory for the shortcut exists
        if (link_path.has_parent_path()) {
            std::filesystem::create_directories(link_path.parent_path());
        }

        std::ofstream desktop_file(link_path.string() + ".desktop");
        if (!desktop_file.is_open()) {
            throw std::runtime_error("Failed to create .desktop file: " + link_path.string() + ".desktop");
        }

        desktop_file << "[Desktop Entry]\n";
        desktop_file << "Type=Application\n";
        desktop_file << "Name=" << link_path.stem().string() << "\n";
        desktop_file << "Exec=" << target_path.string() << "\n";
        if (!description.empty()) {
            desktop_file << "Comment=" << description << "\n";
        }
        desktop_file << "Terminal=false\n";
        desktop_file << "Categories=Utility;\n";
        desktop_file.close();

        // Make the .desktop file executable
        std::filesystem::permissions(link_path.string() + ".desktop",
                                      std::filesystem::perms::owner_exec |
                                      std::filesystem::perms::group_exec |
                                      std::filesystem::perms::others_exec,
                                      std::filesystem::perm_options::add);

        if (output_enabled) {
            std::cout << "Linux .desktop shortcut created: " << link_path.string() << ".desktop -> " << target_path_str << std::endl;
        }
#else
        throw std::runtime_error("Shortcut creation not supported on this OS.");
#endif
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

} // namespace allin1::io
