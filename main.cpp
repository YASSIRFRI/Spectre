#include "SpectreRepo.cpp"
#include <string>

using namespace std;

class SpectreCLI {
public:
    SpectreCLI(int argc, char* argv[]) : argc(argc), argv(argv) {
        parse();
    }

    void parse() {
        if (argc < 2) {
            cout << "No command provided" << endl;
            return;
        }
        string command = argv[1];
        if (command == "init") {
            initRepo();
        } else if (command == "add") {
            if (argc < 3) {
                cout << "No file provided for add command" << endl;
                return;
            }
            vector<string> filesToAdd;
            for (int i = 2; i < argc; i++) {
                filesToAdd.push_back(argv[i]);
            }
            addCommand(filesToAdd);
        } else if (command == "help") {
            string helpcommand = argv[2];
            helpCommand(helpcommand);
        } else if (command == "log") {
            logCommand();
        } else if (command == "status") {
            statusCommand();
        } else if (command == "commit") {
            cout << "Commit " << argv[2] << "\n";
            cout<<argc<<"\n";
            if(argc < 3 ){
                cout<<(argc<3)<<"\n";
                cout<<(argv[2] != "-m")<<"\n";
                cout<<"-m option required"<<"\n";
                return;
            }
            if (argc < 4) {
                cout << "No commit message provided" << endl;
                return;
            }
            string commitMessage = argv[3];
            commitCommand(commitMessage);
        } else {
            cout << "Unknown command: " << command << endl;
        }
    }

    void initRepo() {
        if (repo.isInitialized()) {
            cout << "Spectre repository already initialized" << endl;
            return;
        }
        cout << "Initializing Spectre repository at " << repoPath << endl;
        SpectreRepo repo = SpectreRepo("./");
        repo.initRepo();
    }

    void addCommand(vector<string> fileToAdd) {
        if (!repo.isInitialized()) {
            cout<<"Fatal: Not a Spectre repository (or any of the parent directories): .spectre"<<"\n";
            return;
        }
        cout << "Adding files to Spectre repository" << "\n";
        for (string file : fileToAdd) {
            cout << file << "\n";
        }
        repo.addFiles(fileToAdd);
    }


    void helpCommand(string helpCommand) {
        if (helpCommand == "init") {
            cout << "init: Initializes a Spectre repository in the current directory" << "\n";
        } else if (helpCommand == "add") {
            cout << "add: Adds files to the Spectre repository" << "\n";
        } else if (helpCommand == "log") {
            cout << "log: Displays the commit history of the Spectre repository" << "\n";
        } else if (helpCommand == "status") {
            cout << "status: Displays the status of the Spectre repository" << "\n";
        } else {
            cout << "Unknown command: " << helpCommand << "\n";
        }
    }

    void logCommand() {
        if (!repo.isInitialized()) {
            cout<<"Fatal: Not a Spectre repository (or any of the parent directories): .spectre"<<"\n";
            return;
        }
        vector<Commit> commits = repo.log();
        cout<<"Log"<<"\n";
        for (Commit commit : commits) {
            cout << commit.getMessage() << "\n";
        }
    }

    void statusCommand() {
        if (!repo.isInitialized()) {
            cout<<"Fatal: Not a Spectre repository (or any of the parent directories): .spectre"<<"\n";
            return;
        }
        vector<string> changed= repo.getChangedFiles();
        for (string file : changed) {
            cout << file << "\n";
        }
    }


    void commitCommand(string commitMessage) {
        cout << "Commit" << "\n";
        cout<<commitMessage<<"\n";
        repo.commit(commitMessage);
        cout<<"Commit successful"<<"\n";
    }

private:
    string repoPath;
    SpectreRepo repo;
    int argc;
    char** argv;
};

int main(int argc, char* argv[]){
    SpectreCLI cli(argc, argv);
    return 0;
}