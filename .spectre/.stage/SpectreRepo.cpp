#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <set>
#include "Commit.cpp"
#include <zlib.h>
//#include "lib/libarchive/include/archive.h"
//#include "lib/libarchive/include/archive_entry.h"
#include <fcntl.h>


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
        path spectreIgnorePath = p / ".spectreignore";
        if (!exists(spectrePath)) {
            if (create_directory(spectrePath)) {
                cout << "Successfully initialized Spectre repository in " << p << "\n";
            } else {
                cerr << "Failed to initialize Spectre repository in " << p << "\n";
            }
        } else {
            cout << "Spectre repository already exists in " << p << "\n";
        }
        if (!exists(spectreIgnorePath)) {
            ofstream spectreIgnore(spectreIgnorePath);
            spectreIgnore << ".spectre\n";
            spectreIgnore.close();
        }
    }

    string hashFile(const std::filesystem::path& path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::hash<std::string> hasher;
        return std::to_string(hasher(buffer.str()));
    }

  
    std::vector<std::string> getChangedFiles() {
        set<string> ignoreFiles;
        std::ifstream ignoreFile(".spectreignore");
        std::string line;
        while (std::getline(ignoreFile, line)) {
            ignoreFiles.insert(line);
        }
        std::vector<std::string> changedFiles;
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            if (entry.is_regular_file() && ignoreFiles.find(entry.path().filename().string()) == ignoreFiles.end()) {
                std::string hash = hashFile(entry.path());
                //cout<<entry.path().filename().string()<<"\n";
                //cout<<hash<<"\n";
                std::string stagedHash = hashFile(".spectre/.stage/" + entry.path().filename().string());
                cout<<stagedHash<<"\n";
                if (hash != stagedHash) {
                    changedFiles.push_back(entry.path().filename().string());
                }
            }
        }
        return changedFiles;
    }


    void addFiles(const std::vector<std::string>& filesToAdd) {
    std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);
    std::filesystem::path stageDir(".spectre/.stage");
    if (!std::filesystem::exists(stageDir)) {
        std::filesystem::create_directory(stageDir);
    }
    if (filesToAdd[0] == ".") {
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
            if (entry.is_regular_file()) {
                std::filesystem::copy(entry, stageDir / entry.path().filename(), std::filesystem::copy_options::overwrite_existing);
            }
        }
    } else {
        for (const auto& file : filesToAdd) {
            std::filesystem::path src(file);
            if (std::filesystem::exists(src) && std::filesystem::is_regular_file(src)) {
                std::filesystem::copy(src, stageDir / src.filename(), std::filesystem::copy_options::overwrite_existing);
            } else {
                std::cout << "File does not exist or is not a regular file: " << file << std::endl;
            }
        }
        }
    }

    void compressFile(const std::string& inFilename, const std::string& outFilename) {
        std::ifstream inFile(inFilename, std::ios::binary);
        std::stringstream ss;
        ss << inFile.rdbuf();
        std::string inData = ss.str();

        std::vector<char> outData(compressBound(inData.size()));
        uLongf outDataSize = outData.size();

        compress((Bytef*)outData.data(), &outDataSize, (const Bytef*)inData.data(), inData.size());

        std::ofstream outFile(outFilename, std::ios::binary);
        outFile.write(outData.data(), outDataSize);
    }

    void compressDirectory(const std::filesystem::path& directory) {
    for(auto& p: std::filesystem::recursive_directory_iterator(directory)) {
        if(p.is_regular_file()) {
            std::string inFilename = p.path().string();
            std::string outFilename = inFilename + ".gz";
            compressFile(inFilename, outFilename);
            std::filesystem::remove(p); // remove the original file
        }
    }
    }


    void commit(string commitMessage) {
    std::filesystem::path stageDir(".spectre/.stage");
    std::filesystem::path commitsDir(".spectre/.commits");
    std::filesystem::create_directories(stageDir);
    std::filesystem::create_directories(commitsDir);
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    cout<<now<<"\n";
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");
    std::string date = ss.str();
    Commit commit(commitMessage, "author", date);  
    std::string commitHash = commit.hash();
    std::filesystem::path commitDir = commitsDir / commitHash;
    std::filesystem::create_directories(commitDir);
    for (const auto& entry : std::filesystem::directory_iterator(stageDir)) {
        if (entry.is_regular_file()) {
            std::filesystem::copy(entry, commitDir / entry.path().filename(), std::filesystem::copy_options::overwrite_existing);
        }
    }
    std::ofstream commitInfoFile(commitDir / (commitHash + ".txt"));
    if (!commitInfoFile) {
        std::cerr << "Could not open commit info file for writing.\n";
        return;
    }
    commitInfoFile << "Commit message: " << commitMessage << "\n";
    commitInfoFile << "Timestamp: " << date << "\n";
    commitInfoFile << "Hash: " << commitHash << "\n";
    commitInfoFile.close();
    compressDirectory(commitDir);
}

    



    //void cloneRepo(const string& url) {
        //cout<<"Cloning Spectre repository from "<<url<<"\n";
        //fs::path currentPath = fs::current_path();
        //fs::path spectrePath = currentPath / L".spectre";
        //if (!fs::exists(spectrePath)) {
            //throw std::runtime_error(".spectre directory not found");
        //}
        //try {
            //wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
            //wstring urlStr(url.begin(), url.end());
            //std::wcout << L"Successfully cloned Spectre repository from " << url << L"\n";
        //} catch (const std::exception& e) {
            //std::cerr << "Failed to clone Spectre repository: " << e.what() << "\n";
        //}
    //}


private:
    path  p;
};