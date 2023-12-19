#include "lib/CLI11.hpp"
#include "SpectreRepo.cpp"

using namespace std;

class SpectreCLI {
public:
    SpectreCLI(int argc, char* argv[]) : app("spectre"), argc(argc), argv(argv) {
        initCommand();
        cloneCommand();
        //addRemoteCommand();
        //addCommand();
        //commitCommand();
        //pushCommand();
    }

    void parse() {
        try {
            app.parse(argc, argv);
        } catch (const CLI::ParseError &e) {
            cout << e.what() << endl;
            return;
        }
    }

    void initRepo() {
        cout << "Initializing Spectre repository at " << repoPath << endl;
        SpectreRepo repo = SpectreRepo("../");
        repo.initRepo();
    }

    void cloneRepo() {
        cout << "Cloning Spectre repository from " << repoPath << endl;
    }



private:
    CLI::App app;
    string repoPath;
    int argc;
    char** argv;
    void initCommand() {
        auto init = app.add_subcommand("init", "Initialize a Spectre repository");
        init->callback([this] { initRepo(); });
    }

    void cloneCommand() {
        auto clone = app.add_subcommand("clone", "Clone a remote Spectre repository");
        // Add options for clone command
        string remoteURL;
        clone->add_option("--remote", remoteURL, "URL of the remote repository")->required();
        clone->callback([this] { cloneRepo(); });
    }
};


int main(int argc, char* argv[]){
    SpectreCLI cli(argc, argv);
    SpectreRepo repo = SpectreRepo();
    cli.parse();
    return 0;
}