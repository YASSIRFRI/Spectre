#include <string>
#include <functional>

class Commit {
public:
    Commit(const std::string& message, const std::string& author, const std::string& date)
        : message(message), author(author), date(date) {}

    std::string hash() const {
        std::hash<std::string> hasher;
        return std::to_string(hasher(message + author + date));
    }

private:
    std::string message;
    std::string author;
    std::string date;
};