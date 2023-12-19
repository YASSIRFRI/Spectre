#include <iostream>
#include <experimental/filesystem>

using namespace std;
using namespace experimental::filesystem;

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

private:
    path  p;
};