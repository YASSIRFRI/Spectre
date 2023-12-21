#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <stdexcept>
#include <Windows.h>
#include <Winhttp.h>
#include <filesystem>

using namespace std;
using namespace experimental::filesystem;
namespace fs = std::filesystem;

class SpectreRepo {
public:
    SpectreRepo(const string& path) : p(path) {
        if (!exists(path)) {
            create_directories(path);
        }
        initRepo();
    }
    SpectreRepo(){
    }


    void initRepo(){
        path spectrePath = p / ".spectre";
        if (!exists(spectrePath)) {
            if (create_directory(spectrePath)) {
                cout << "Successfully initialized Spectre repository in " << p << "\n";
            } else {
                cerr << "Failed to initialize Spectre repository in " << p << "\n";
            }
        } else {
            cout << "Spectre repository already exists in " << p << "\n";
        }
    }

    void downloadFile(const std::wstring& url, const fs::path& outputPath) {
        HINTERNET hSession = WinHttpOpen(L"User-Agent", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (hSession == NULL) {
            throw std::runtime_error("Failed to open WinHTTP session");
        }

        HINTERNET hConnect = WinHttpOpenRequest(hSession, L"GET", url.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        if (hConnect == NULL) {
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to open WinHTTP request");
        }

        if (!WinHttpSendRequest(hConnect, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to send WinHTTP request");
        }

        if (!WinHttpReceiveResponse(hConnect, NULL)) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to receive WinHTTP response");
        }

        std::ofstream outfile(outputPath, std::ofstream::binary);
        if (!outfile.is_open()) {
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            throw std::runtime_error("Failed to open file for writing");
        }

        DWORD bytesRead;
        BYTE buffer[4096];
        while (WinHttpReadData(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            outfile.write(reinterpret_cast<char*>(buffer), bytesRead);
        }
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        outfile.close();
    }


    void cloneRepo(const string& url) {
        cout<<"Cloning Spectre repository from "<<url<<"\n";
        fs::path currentPath = fs::current_path();
        fs::path spectrePath = currentPath / L".spectre";
        if (!fs::exists(spectrePath)) {
            throw std::runtime_error(".spectre directory not found");
        }
        try {
            wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
            wstring urlStr(url.begin(), url.end());
            downloadFile(urlStr, spectrePath);
            std::wcout << L"Successfully cloned Spectre repository from " << url << L"\n";
        } catch (const std::exception& e) {
            std::cerr << "Failed to clone Spectre repository: " << e.what() << "\n";
        }
    }



private:
    path  p;
};