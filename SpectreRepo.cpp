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
#include <map>
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

    bool isInitialized(){
        path spectrePath = p / ".spectre";
        return exists(spectrePath);
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
        z_stream zs;
        memset(&zs, 0, sizeof(zs));
        if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            throw(std::runtime_error("deflateInit failed while compressing."));
        }
        zs.next_in = (Bytef*)inData.data();
        zs.avail_in = inData.size();
        int ret;
        char outbuffer[32768];
        std::string outstring;
        do {
            zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
            zs.avail_out = sizeof(outbuffer);
            ret = deflate(&zs, Z_FINISH);
            if (outstring.size() < zs.total_out) {
                outstring.append(outbuffer, zs.total_out - outstring.size());
            }
        } while (ret == Z_OK);
        deflateEnd(&zs);
        if (ret != Z_STREAM_END) {
            std::ostringstream oss;
            oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
            throw(std::runtime_error(oss.str()));
        }
        std::ofstream outFile(outFilename, std::ios::binary);
        outFile.write(outstring.data(), outstring.size());
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


    vector<Commit> log() {
        std::filesystem::path commitsDir(".spectre/.commits");
        std::vector<Commit> commits;
    for (const auto& entry : std::filesystem::directory_iterator(commitsDir)) {
        if (entry.is_directory()) {
            std::string commitDir = entry.path().string();
            std::string commitFile = commitDir + "/" + entry.path().filename().string() + ".sp.gz";
            cout<<commitFile<<"\n";
            std::vector<uint8_t> decompressedData = decompressFile(commitFile);
            std::string decompressedFileContent(decompressedData.begin(), decompressedData.end());
            cout<<decompressedFileContent<<"\n";
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
            Commit commit(message, "Unknown", date); // Assuming the author is unknown as it's not in the commit info file
            commits.push_back(commit);
        }
    }
    return commits;
    }

    void revert(std::string commitId, std::string message){
        std::filesystem::path commitDir(".spectre/.commits/" + commitId);
        if (!std::filesystem::exists(commitDir)) {
            std::cerr << "Commit ID not found: " << commitId << "\n";
            return;
        }
        std::filesystem::path stageDir(".spectre/.stage");
        if (!std::filesystem::exists(stageDir)) {
            std::filesystem::create_directory(stageDir);
        }

        for (const auto& entry : std::filesystem::directory_iterator(commitDir)) {
            if (entry.is_regular_file()) {
                std::vector<uint8_t> decompressedData = decompressFile(entry.path().string()); // Assuming decompressFile function is available
                std::ofstream outFile(stageDir / entry.path().filename(), std::ios::binary);
                outFile.write(reinterpret_cast<const char*>(decompressedData.data()), decompressedData.size());
            }
        }
        commit(message); 
    }

private:
    path  p;
};