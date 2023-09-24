//Author: Artner Patrick, Mahdi Fabian
#include <iostream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <cstring>

// helper function
void print_usage(std::string programName)
{
    std::cout << "Usage: " << programName << " [-R] [-i] searchpath filename1 [filename2] … [filenameN]\n";
}

void waitForAllChildren(int startedChildren, int pipe[2])
{
    int finishedChildren = 0;
    char buffer[PIPE_BUF];
    close(pipe[1]);

    // read data from pipe
    while(read(pipe[0], buffer, PIPE_BUF) != 0)
    {
        std::cout << buffer;
        memset(buffer, 0, sizeof(buffer));
    }

    // wait for all children to finish
    while(finishedChildren < startedChildren)
    {
        pid_t pid = wait(NULL);

        if(pid == -1)
        {
            std::cerr << "error when waiting for child process\n";
            return;
        }
        ++finishedChildren;
    }
}

int search(std::string path, std::string file, bool caseInsensitive, bool recursive, int pipe[2])
{
    pid_t pid = getpid();
    close(pipe[0]);

    if(recursive)
    {
        for(auto const& dir_entry : std::filesystem::recursive_directory_iterator(path))
        {
            if(std::filesystem::is_regular_file(dir_entry) &&
                std::filesystem::path(dir_entry).filename() == file)
            {
                std::string toWrite = std::to_string(pid) + ": " + file + ": " + dir_entry.path().string() + "\n";
                write(pipe[1], toWrite.c_str(), toWrite.length());
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
                std::string toWrite = std::to_string(pid) + ": " + file + ": " + dir_entry.path().string() + "\n";
                write(pipe[1], toWrite.c_str(), toWrite.length());
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

    // read searchpath
    std::string searchPath = argv[optind++];
    int startedChildren = 0;

    // create pipe
    int fd[2];
    if(pipe(fd) != 0)
    {
        std::cerr << "error when creating pipe\n";
        return EXIT_FAILURE;
    }

    // start child-processes for each given file
    while(optind < argc)
    {
        pid_t pid = fork();

        switch(pid)
        {
        case -1: // error
            std::cerr << "error when forking child process\n";
            waitForAllChildren(startedChildren, fd);
            return EXIT_FAILURE;
        case 0: // child process
            return search(searchPath, argv[optind], iCounter, RCounter, fd);
        }
        ++optind;
        ++startedChildren;
    }

    // wait for children to finish
    waitForAllChildren(startedChildren, fd);

    return EXIT_SUCCESS;
}













