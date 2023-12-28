#include "SpectreRepo.cpp"
#include <string>

using namespace std;

class SpectreCLI {
public:
    string red = "\033[31m";
    string green = "\033[32m";
    string yellow = "\033[33m";
    string purple = "\033[35m";
    string reset = "\033[0m";
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
            string helpcommand;
            if (argc < 3) {
                helpcommand = "help";
            }else{
                helpcommand = argv[2];
            }
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
        } else if(command == "revert") {
            cout << "Revert" << "\n";
            if (argc < 4) {
                if (argc < 3) {
                    cout << "No commit hash provided" << endl;
                    return;
                }
                cout << "No commit message provided" << endl;
                return;
            }
            string commitHash = argv[2];
            string commitMessage = argv[3];
            repo.revert(commitHash, commitMessage);
        } else if (command=="ignore"){
            if (argc < 3) {
                cout << "No file provided for ignore command" << endl;
                helpCommand("ignore");
                return;
            }
            vector<string> filesToIgnore;
            for (int i = 2; i < argc; i++) {
                filesToIgnore.push_back(argv[i]);
            }
            ignoreCommand(filesToIgnore);
        }else{
            cout<<"Command not found"<<"\n";
        }
    }

    void initRepo() {
        if (repo.isInitialized()) {
            cout << "Spectre repository already initialized" << endl;
            return;
        }
        cout << "Initializing Spectre repository at " << repoPath << endl;
        SpectreRepo repo = SpectreRepo("./");
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
        } else if (helpCommand == "commit") {
            cout << "commit: Creates a commit with the staged files" << "\n";
        } else if (helpCommand == "revert") {
            cout << "revert: Reverts the repository to a previous commit" << "\n";
        } else if (helpCommand == "ignore") {
            cout << "ignore: Adds files to the .spectreignore file" << "\n";
        } else if (helpCommand == "config") {
            cout << "config: Sets the author and email for the repository" << "\n";
        } else {
            cout<<yellow<<"Spectre Help"<<"\n"<<reset;
            cout<<"Usage: spectre <command> [<args>]"<<"\n";
            cout<<yellow<<"\tcommands:"<<"\n"<<reset;
            cout<<"\t\tinit: Initializes a Spectre repository in the current directory"<<"\n";
            cout<<"\t\tadd: Adds files to the Spectre repository"<<"\n";
            cout<<"\t\tlog: Displays the commit history of the Spectre repository"<<"\n";
            cout<<"\t\tstatus: Displays the status of the Spectre repository"<<"\n";
            cout<<"\t\tcommit: Creates a commit with the staged files"<<"\n";
            cout<<"\t\trevert: Reverts the repository to a previous commit"<<"\n"<<reset;
            cout<<"\t\tignore: Adds files to the .spectreignore file"<<"\n"<<reset;
            cout<<"\t\tconfig: Sets the author and email for the repository"<<"\n"<<reset;
            cout<<yellow<<"\targs:"<<"\n"<<reset;
        }
    }

    void logCommand() {
        if (!repo.isInitialized()) {
            cout<<"Fatal: Not a Spectre repository (or any of the parent directories): .spectre"<<"\n";
            return;
        }
        vector<Commit> commits = repo.log();
        if (commits.size() == 0) {
            cout<<red<<"\tEmpty Commit History"<<"\n";
            cout<<"Run 'spectre commit -m <message>' to add a commit"<<"\n"<<reset;
            return;
        }
        cout<<purple<<"Commit History"<<reset<<"\n";
        for (Commit commit : commits) {
            cout<<commit.getAuthor()<<"\n";
            cout<<"Commit Date:";
            cout<<commit.getDate()<<"\n";
            cout<<"Commit Message:";
            cout << commit.getMessage() << "\n";
            cout<<"Commit Hash: ";
            cout << commit.hash() << "\n";
        }
    }

    void ignoreCommand(vector<string> filesToIgnore) {
        if (!repo.isInitialized()) {
            cout<<"Fatal: Not a Spectre repository (or any of the parent directories): .spectre"<<"\n";
            return;
        }
        cout << "Ignoring files in Spectre repository" << "\n";
        for (string file : filesToIgnore) {
            cout << file << "\n";
        }
        repo.ignoreFiles(filesToIgnore);
    }

    void statusCommand() {
        if (!repo.isInitialized()) {
            cout <<red<< "Fatal: Not a Spectre repository (or any of the parent directories): .spectre" <<reset<< "\n";
            return;
        }
        map<string,string> info = repo.getInfo();
        vector<string> changed = repo.getChangedFiles();
        vector<string> staged = repo.getStagedFiles();
        cout<<"\t*****Status*****:"<<"\n"; 
        if (info["author"] == "Unknown"|| info.find("author") == info.end()){
            cout<<yellow<<"Warning: Author not set"<<"\n";
            cout<<"Run 'spectre config --author <author>' to set author"<<"\n"<<reset;
        }else{
            cout<<"\tAuthor: "<<info["author"]<<"\n";
        }
        if(info["email"] == "Unknown"|| info.find("email") == info.end()){
            cout<<yellow<<"Warning: Email not set"<<"\n";
            cout<<"Run 'spectre config --email <email>' to set email"<<"\n"<<reset;
        }else{
            cout<<"\tEmail: "<<info["email"]<<"\n";
        }
        cout << green << "Staged Files" << reset << "\n";
        for (string file : staged) {
            cout << green << "\t" << file << reset << "\n";
        }
        cout << red << "Changed Files" << reset << "\n";
        for (string file : changed) {
            cout << red << "\t" << file << reset << "\n";
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