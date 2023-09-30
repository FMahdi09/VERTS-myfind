//Author: Artner Patrick, Mahdi Fabian
#include <iostream>
#include <string>
#include <filesystem>
#include <cctype>

#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <cstring>

namespace fs = std::filesystem;

/*
 * prints usage help to cout
*/
void print_usage(const std::string& programName)
{
    std::cout << "Usage: " << programName << " [-R] [-i] searchpath filename1 [filename2] … [filenameN]\n";
}

/*
 * compares two strings case insensitive
 *
 * params:
 *  word1 & word2: strings to compare
 *
 * return:
 *  true: strings are the same disregarding upper & lowercase
 *
 *  false: strings are different size / contain different symbols
*/
const bool caseInsensitiveCompare(const std::string& word1, const std::string& word2)
{
    if(word1.size() != word2.size())
    {
        return false;
    }

    for(size_t i = 0; i < word1.size(); ++i)
    {
        if(std::tolower(word1[i]) != std::tolower(word2[i]))
        {
            return false;
        }
    }
    return true;
}

/*
 * reads from given pipe until all writing processes have disconnected and
 * waits for given number of childprocesses to finish
 *
 * params:
 *  startedChildren: number of childprocesses to wait for
 *
 *  pipe: pipe to read from
*/
void waitForAllChildren(const int startedChildren, const int pipe[2])
{
    int finishedChildren = 0;

    char buffer[PIPE_BUF];
    memset(buffer, 0, sizeof(buffer));

    // close writing end of pipe
    close(pipe[1]);

    // read data from pipe until all children close their writing end
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

/*
 * searches for given file in the given path, writes results in given pipe
 *
 * params:
 *  path: path to search in (relative or absolute)
 *
 *  toSearch: filename to search
 *
 *  caseInsensitive: flag to enable case insensitive search
 *
 *  recursice: flag to enable recursive search
 *
 *  pipe: pipe to write in
*/
int search(const std::string& path, const std::string& toSearch, const bool caseInsensitive, const bool recursive, const int pipe[2])
{
    pid_t pid = getpid();
    std::string filename;

    // close reading end of pipe
    close(pipe[0]);

    if(recursive)
    {
        for(auto const& dir_entry : fs::recursive_directory_iterator(path))
        {
            filename = fs::path(dir_entry).filename();

            if(fs::is_regular_file(dir_entry) &&
               (filename == toSearch || (caseInsensitive && caseInsensitiveCompare(toSearch, filename))))
            {
                // format: <pid>: <filename>: <complete-path-to-found-file>\n
                std::string toWrite = std::to_string(pid) + ": " + filename + ": " + fs::absolute(fs::canonical(dir_entry)).string() + "\n";
                write(pipe[1], toWrite.c_str(), toWrite.length());
            }
        }
    }
    else
    {
        for(auto const& dir_entry : fs::directory_iterator(path))
        {
            filename = fs::path(dir_entry).filename();

            if(fs::is_regular_file(dir_entry) &&
               (filename == toSearch || (caseInsensitive && caseInsensitiveCompare(toSearch, filename))))
            {
                // format: <pid>: <filename>: <complete-path-to-found-file>\n
                std::string toWrite = std::to_string(pid) + ": " + filename + ": " + fs::absolute(fs::canonical(dir_entry)).string() + "\n";
                write(pipe[1], toWrite.c_str(), toWrite.length());
            }
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    std::string programName = argv[0];

    if(argc <= 1)
    {
        print_usage(programName);
        return EXIT_FAILURE;
    }

    int shortOption;
    int iCounter = 0;
    int RCounter = 0;
    int startedChildren = 0;

    while ((shortOption = getopt(argc, argv, "Ri")) != EOF) {
        switch(shortOption)
        {
        case '?':
            print_usage(programName);
            return EXIT_FAILURE;
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

    if(!fs::exists(searchPath))
    {
        std::cerr << "\"" << searchPath << "\" no such file or directory\n";
        return EXIT_FAILURE;
    }

    // create pipe
    int fd[2];
    if(pipe(fd) != 0)
    {
        std::cerr << "error when creating pipe\n";
        return EXIT_FAILURE;
    }

    // start child-processes for each given filename
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













