#include <iostream>
#include <string>

#include <unistd.h>

//test Pat
// helper function
void print_usage(std::string programName)
{
    std::cout << "Usage: " << programName << " [-R] [-i] searchpath filename1 [filename2] … [filenameN]\n";
}

//test
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
            exit(-1);
            break;
        case 'i':
            ++iCounter;
            break;
        case 'R':
            ++RCounter;
            break;
        }
    }
    return 0;
}
