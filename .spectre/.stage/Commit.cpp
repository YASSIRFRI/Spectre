#include <string>
#include <functional>
#include <vector>

class Commit {
public:
    Commit(const std::string& message, const std::string& author, const std::string& date)
        : message(message), author(author), date(date) {}

    std::string hash() const {
        std::hash<std::string> hasher;
        return std::to_string(hasher(message + author + date));
    }

    std::string getMessage() const {
        return message;
    }
     
    std::string getAuthor() const {
        return author;
    }

    std::string getDate() const {
        return date;
    }

    void addFile(const std::string& file) {
        files.push_back(file);
    }

    std::vector<std::string> getFiles() const {
        return files;
    }

    std::string toString() const {
        std::string commitString = "commit " + hash() + "\n";
        commitString += "Author: " + author + "\n";
        commitString += "Date: " + date + "\n";
        commitString += "\n";
        commitString += message + "\n";
        commitString += "\n";
        for (const std::string& file : files) {
            commitString += file + "\n";
        }
        return commitString;
    }

    void setFiles(const std::vector<std::string>& files) {
        this->files = files;
    }
    
    void setMessage(const std::string& message) {
        this->message = message;
    }

private:
    std::string message;
    std::string author;
    std::string date;
    std::vector<std::string> files={};
};