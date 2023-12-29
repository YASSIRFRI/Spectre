#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <thread>
#include <filesystem>
#include <set>
#include "Commit.cpp"
#include <zlib.h>
#include <map>
#include <fcntl.h>
#include <windows.h>
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif



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

    bool isInitialized(){
        path spectrePath = p / ".spectre";
        return exists(spectrePath);
    }

    void initRepo(){
        path spectrePath = p / ".spectre";
        cout << "Initializing Spectre repository in " << p << "\n";
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        mode &= ~ENABLE_ECHO_INPUT;
        string username;
        string email;
        std::cout<<"Enter your username:";
        getline(cin, username);
        if (username =="" || username.size() == 0){
            username = "Unknown";
        }
        std::cout<<"Enter your email:";
        getline(cin, email);
        if (email=="" || email.size() == 0){
            email = "Unknown";
        }
        path spectreIgnorePath = p / ".spectreignore";
        if (!exists(spectrePath)) {
            if (create_directory(spectrePath)) {
                ofstream initFile(spectrePath / "init");
                initFile << "author: " << username << "\n";
                initFile << "email: " << email << "\n";
                initFile.close();
                cout << "Spectre repository Initialized in " << p << "\n";
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


    map<std::string, std::string> getInfo() {
        std::map<std::string, std::string> info;
        std::ifstream file(".spectre/init");
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key, value;
            string line;
            getline(iss,line);
            key = line.substr(0, line.find(":"));
            value = line.substr(line.find(":") + 2);
            info[key] = value;
        }
        return info;
    }


    string hashFile(const std::filesystem::path& path) {
        std::ifstream file(path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::hash<std::string> hasher;
        return std::to_string(hasher(buffer.str()));
    }


    void decompressFile(const std::string& inFilename, const std::string& outFilename) {
        gzFile inFile = gzopen(inFilename.c_str(), "rb");
        if (!inFile) {
            std::cerr << "Could not open " << inFilename << " for reading.\n";
            return;
        }
        std::ofstream outFile(outFilename, std::ios::binary);
        if (!outFile) {
            std::cerr << "Could not open " << outFilename << " for writing.\n";
            gzclose(inFile);
            return;
        }
        char buffer[128];
        int numRead = 0;
        while ((numRead = gzread(inFile, buffer, sizeof(buffer))) > 0) {
            outFile.write(buffer, numRead);
        }
        gzclose(inFile);
    }

    std::vector<uint8_t> decompressFile(const std::string& inFilename) {
        gzFile inFile = gzopen(inFilename.c_str(), "rb");
        if (!inFile) {
            std::cerr << "Could not open " << inFilename << " for reading.\n";
            return {};
        }

        uint8_t buffer[128];
        int numRead = 0;
        std::vector<uint8_t> decompressedContent;
        while ((numRead = gzread(inFile, buffer, sizeof(buffer))) > 0) {
            decompressedContent.insert(decompressedContent.end(), buffer, buffer + numRead);
        }

        gzclose(inFile);

        return decompressedContent;
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
                std::string stagedHash = hashFile(".spectre/.stage/" + entry.path().filename().string());
                cout<<stagedHash<<"\n";
                if (hash != stagedHash) {
                    changedFiles.push_back(entry.path().filename().string());
                }
            }
        }
        return changedFiles;
    }

    bool isIgnored(const std::string& file) {
        std::ifstream ignoreFile(".spectreignore");
        std::string line;
        while (std::getline(ignoreFile, line)) {
            if (line == file) {
                return true;
            }
        }
        return false;
    }

    void addFiles(const std::vector<std::string>& filesToAdd) {
        std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        std::filesystem::path stageDir(".spectre/.stage");
        if (!std::filesystem::exists(stageDir)) {
            std::filesystem::create_directory(stageDir);
        }

        auto copyFileToStage = [&](const std::filesystem::path& src) {
            if (isIgnored(src.string())) {
                return;
            }
            std::filesystem::path relativePath = std::filesystem::relative(src, std::filesystem::current_path());
            std::filesystem::path dest = stageDir / relativePath;
            if (std::filesystem::exists(dest)) {
                std::filesystem::remove(dest);
            }
            std::filesystem::create_directories(dest.parent_path());
            std::filesystem::copy(src, dest);
        };

        auto handlePath = [&](const std::filesystem::path& path) {
            if (path.filename().string()[0] == '.'&&path.filename().string() != ".spectreignore") {
                return;
            }
            if (std::filesystem::is_regular_file(path)) {
                copyFileToStage(path);
            } else if (std::filesystem::is_directory(path)) {
                for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
                    if (entry.is_regular_file()) {
                        copyFileToStage(entry.path());
                    }
                }
            }
        };
        if (filesToAdd[0] == ".") {
            for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
                handlePath(entry.path());
            }
        } else {
            for (const auto& file : filesToAdd) {
                handlePath(file);
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
        map<std::string, std::string> info = getInfo();
        string author = info["author"];
        Commit commit(commitMessage, author, date);  
        std::string commitHash = commit.hash();
        std::filesystem::path commitDir = commitsDir / commitHash;
        std::filesystem::create_directories(commitDir);
        for (const auto& entry : std::filesystem::recursive_directory_iterator(stageDir)) {
            if (entry.is_regular_file()) {
                std::filesystem::path relativePath = std::filesystem::relative(entry.path(), stageDir);
                std::filesystem::path dest = commitDir / relativePath;
                std::filesystem::create_directories(dest.parent_path());
                std::filesystem::copy(entry, dest, std::filesystem::copy_options::overwrite_existing);
            }
        }
        std::ofstream commitInfoFile(commitDir / (commitHash + ".sp"));
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

    void compressFile(const std::string& inFilename, const std::string& outFilename) {
        std::ifstream inFile(inFilename, std::ios::binary);
        std::stringstream ss;
        ss << inFile.rdbuf();
        std::string inData = ss.str();
        size_t numThreads = std::thread::hardware_concurrency();
        size_t chunkSize = (inData.size() + numThreads - 1) / numThreads;
        std::vector<std::string> outStrings(numThreads);
        std::vector<std::thread> threads(numThreads);
        for (size_t i = 0; i < numThreads; ++i) {
            threads[i] = std::thread([&, i]() {
                z_stream zs;
                memset(&zs, 0, sizeof(zs));
                if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
                    throw(std::runtime_error("deflateInit failed while compressing."));
                }
                zs.next_in = (Bytef*)inData.data() + i * chunkSize;
                zs.avail_in = std::min(chunkSize, inData.size() - i * chunkSize);
                int ret;
                char outbuffer[32768];
                do {
                    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
                    zs.avail_out = sizeof(outbuffer);
                    ret = deflate(&zs, Z_FINISH);
                    if (outStrings[i].size() < zs.total_out) {
                        outStrings[i].append(outbuffer, zs.total_out - outStrings[i].size());
                    }
                } while (ret == Z_OK);
                deflateEnd(&zs);
            });
        }
        for (auto& thread : threads) {
            thread.join();
        }
        std::string outData;
        for (const auto& outString : outStrings) {
            outData += outString;
        }

        std::ofstream outFile(outFilename, std::ios::binary);
        outFile.write(outData.data(), outData.size());
    } 

    std::vector<std::string> getStagedFiles() {
        std::vector<std::string> unchangedFiles;
        std::filesystem::path stageDir(".spectre/.stage");
        if (!std::filesystem::exists(stageDir)) {
            std::cerr << "Stage directory does not exist.\n";
            return unchangedFiles;
        }

        for (const auto& entry : std::filesystem::directory_iterator(stageDir)) {
            if (entry.is_regular_file()) {
                std::string stagedFilePath = entry.path().string();
                std::string currentFilePath = entry.path().filename().string();
                std::string stagedFileHash = hashFile(stagedFilePath);
                std::string currentFileHash = hashFile(currentFilePath);
                if (stagedFileHash == currentFileHash) {
                    unchangedFiles.push_back(currentFilePath);
                }
            }
        }

        return unchangedFiles;
    }

    void compressDirectory(const std::filesystem::path& directory) {
        for(auto& p: std::filesystem::recursive_directory_iterator(directory)) {
            if(p.is_regular_file()) {
                std::string inFilename = p.path().string();
                std::string outFilename = inFilename + ".gz";
                compressFile(inFilename, outFilename);
                std::filesystem::remove(p); 
            }
        }
    }



    void ignoreFiles(vector<string> filesToIgnore){
        for (string file : filesToIgnore) {
            if (!std::filesystem::exists(file)) {
                std::cerr << "File does not exist: " << file << "\n";
                return;
            }
            std::filesystem::path ignorePath(".spectreignore");
            std::ofstream ignoreFile(ignorePath, std::ios::app);
            ignoreFile << file << "\n";
            ignoreFile.close();
        }
    }

    vector<Commit> log() {
        std::filesystem::path commitsDir(".spectre/.commits");
        if (!std::filesystem::exists(commitsDir)) {
            return {};
        }
        std::vector<Commit> commits;
        for (const auto& entry : std::filesystem::directory_iterator(commitsDir)) {
        if (entry.is_directory()) {
            std::string commitDir = entry.path().string();
            std::string commitFile = commitDir + "/" + entry.path().filename().string() + ".sp.gz";
            std::string commitId = entry.path().filename().string();
            std::vector<uint8_t> decompressedData = decompressFile(commitFile);
            std::string decompressedFileContent(decompressedData.begin(), decompressedData.end());
            std::istringstream inFile(decompressedFileContent);
            std::string line;
            std::string message, date, hash;
            std::vector<std::string> files;
            while (std::getline(inFile, line)) {
                if (line.substr(0, 16) == "Commit message: ") {
                    message = line.substr(16);
                } else if (line.substr(0, 11) == "Timestamp: ") {
                    date = line.substr(11);
                } else if (line.substr(0, 6) == "Hash: ") {
                    hash = line.substr(6);
                }
            }
            map<std::string, std::string> info = getInfo();
            if(info["author"] == "Unknown" || info.find("author") == info.end()){
                info["author"] = "Unknown";
            }
            Commit newCommit(message,info["author"], date); 
            newCommit.setId(commitId);
            commits.push_back(newCommit);
        }
    }
    return commits;
    }


    void revert(std::string commitId, std::string message, bool hardReset = false) {
        if (!commitExists(commitId)) {
            std::cerr << "Commit ID not found: " << commitId << "\n";
            return;
        }

        if (!stageExists()) {
            std::filesystem::create_directory(".spectre/.stage");
        }

        std::vector<std::string> filesToCommit = getFilesToCommit();

        if (hardReset) {
            performHardReset(filesToCommit, commitId, message);
        } else {
            checkForUncommittedChanges();
        }

        recreateCommitDirectory(commitId);
    }

    bool commitExists(const std::string& commitId) {
        return std::filesystem::exists(".spectre/.commits/" + commitId);
    }

    bool stageExists() {
        return std::filesystem::exists(".spectre/.stage");
    }

    std::vector<std::string> getFilesToCommit() {
        std::vector<std::string> filesToCommit;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(".")) {
            if (!isIgnored(entry.path().string()) && entry.path().parent_path().filename().string()[0] != '.') {
                filesToCommit.push_back(entry.path().string());
            }
        }
        return filesToCommit;
    }

    void performHardReset(const std::vector<std::string>& filesToCommit, const std::string& commitId, const std::string& message) {
        addFiles(filesToCommit);
        commit("Commit before revert" + commitId + " " + message);
        for (const auto& entry : std::filesystem::recursive_directory_iterator(".")) {
            if (!isIgnored(entry.path().string()) && entry.path().parent_path().filename().string()[0] != '.') {
                std::filesystem::remove_all(entry.path());
            }
        }
    }

    void checkForUncommittedChanges() {
        if (stageExists()) {
            std::vector<std::string> filesToCommit = getChangedFiles();
            if (!filesToCommit.empty()) {
                throw std::runtime_error("You have uncommitted changes. Please commit or stash them before reverting.");
            }
        }
    }

    void recreateCommitDirectory(const std::string& commitId) {
        std::filesystem::path commitDir(".spectre/.commits/" + commitId);
        for (const auto& entry : std::filesystem::recursive_directory_iterator(commitDir)) {
            if (entry.is_regular_file()) {
                std::vector<uint8_t> decompressedData = decompressFile(entry.path().string()); // Assuming decompressFile function is available
                std::filesystem::path relativePath = std::filesystem::relative(entry.path(), commitDir);
                std::filesystem::path dest = "." / relativePath;
                std::filesystem::create_directories(dest.parent_path());
                std::ofstream outFile(dest, std::ios::binary);
                outFile.write(reinterpret_cast<const char*>(decompressedData.data()), decompressedData.size());
            }
        }
    }

    
private:
    path  p;
};