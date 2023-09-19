//Author: Artner Patrick, Mahdi Fabian
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>

// helper function
void print_usage(std::string programName)
{
    std::cout << "Usage: " << programName << " [-R] [-i] searchpath filename1 [filename2] … [filenameN]\n";
}

void waitForAllChildren(int startedChildren)
{
    int finishedChildren = 0;

    while(finishedChildren < startedChildren)
    {
        pid_t pid = wait(NULL);

        if(pid == -1)
        {
            std::cerr << "myfork: error when waiting for child process\n";
            return;
        }
        ++finishedChildren;
    }
}

int search(std::string path, std::string file, bool caseInsensitive, bool recursive)
{
    if(recursive)
    {
        for(auto const& dir_entry : std::filesystem::recursive_directory_iterator(path))
        {
            if(std::filesystem::is_regular_file(dir_entry) &&
                std::filesystem::path(dir_entry).filename() == file)
            {
                std::cout << getpid() << ": " << file << ": " << dir_entry << "\n";
            }
        }
    }
    else
    {
        for(auto const& dir_entry : std::filesystem::directory_iterator(path))
        {
            if(std::filesystem::is_regular_file(dir_entry) &&
                std::filesystem::path(dir_entry).filename() == file)
            {
                std::cout << getpid() << ": " << file << ": " << dir_entry << "\n";
            }
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    std::string programName = argv[0];

    int shortOption;
    int iCounter = 0;
    int RCounter = 0;

    while ((shortOption = getopt(argc, argv, "Ri")) != EOF) {
        switch(shortOption)
        {
        case '?':
            print_usage(programName);
            return EXIT_FAILURE;
            break;
        case 'i':
            ++iCounter;
            break;
        case 'R':
            ++RCounter;
            break;
        }
    }

    // check for duplicate short options
    if(iCounter > 1 || RCounter > 1)
    {
        print_usage(programName);
        return EXIT_FAILURE;
    }

    // read path to search in
    std::string searchPath = argv[optind++];
    int startedChildren = 0;

    // start child-processes for each given file
    while(optind < argc)
    {
        pid_t pid = fork();

        switch(pid)
        {
        case -1: // error
            std::cerr << "myfork: error when forking child process\n";
            waitForAllChildren(startedChildren);
            return EXIT_FAILURE;
        case 0: // child process
            return search(searchPath, argv[optind], iCounter, RCounter);
        }
        ++optind;
        ++startedChildren;
    }

    // wait for children to finish
    waitForAllChildren(startedChildren);

    return EXIT_SUCCESS;
}













