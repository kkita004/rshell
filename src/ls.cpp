// ls program for cs 100
#include <iostream>
#include <vector>
#include <ostream>
#include <iterator>
#include <cstdlib>
#include <algorithm>
#include <cctype>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <boost/algorithm/string.hpp>


// Parses input to find flags argument, returns enabled flags
// Will erase any argument starting with a '-' character
uint8_t getFlags(std::vector<std::string>& args) {
    uint8_t flags = 0x00;
    for (auto it = args.begin(); it != args.end(); ) {
        // Found options flags
        if ((*it).at(0) == '-') {
            if ((*it).find('a')!=std::string::npos) flags = flags | 0x01;
            if ((*it).find('R')!=std::string::npos) flags = flags | 0x02;
            if ((*it).find('l')!=std::string::npos) flags = flags | 0x04;
            it = args.erase(it);
        } else {
          ++it;
        }
    }
    return flags;
}


// Case insensitive string comparison
bool caseincomp(std::string s1, std::string s2) {
    return boost::to_upper_copy(s1) < boost::to_upper_copy(s2);
}

void parsedirec(std::vector<std::string>& args, uint8_t flags,
                std::vector<std::string>& files,
                std::vector<std::string>& dirs) {

    // If no other arguments are passed in, only use current directory
    //std::vector<std::string> files;
    //std::vector<std::string> dirs;
    if (args.size() == 0) files.push_back(".");
    else {files = args;}

    DIR* dirp;
    if (NULL == (dirp = opendir(files.at(0).c_str()))) {
        perror("opendir");
        exit(1);
    }
    struct dirent *filespecs;
    //int result;
    errno = 0;
    // Read in file objects
    while (NULL != (filespecs = readdir(dirp))) {
        // Must keep path of file, otherwise won't find
        std::string str(filespecs->d_name);
        std::string path = files.at(0);
        std::string full = path + "/" + str;
        //std::cout << full << std::endl;

        // If -a flag is not enabled, skip hidden files
        if (!(flags & 0x01)) {
            if (filespecs->d_name[0] == '.') continue;
        }
        // Check if directory or not

        struct stat dstat;
        //std::cout << filespecs->d_name << std::endl;
        // Keep track of directories
        //if (stat(filespecs->d_name, &dstat) == 0 && S_ISDIR(dstat.st_mode)) {
        //    dirs.push_back(str);
        //}

        if (stat(full.c_str(), &dstat)) {
            perror("stat");
            exit(1);
        }
        if (S_ISDIR(dstat.st_mode)) {
            dirs.push_back(str);

        }

        files.push_back(str);
        //std::cout << filespecs->d_name << " ";

    }
    if (errno != 0) {
        perror("readdir");
        exit(1);
    }
    if (-1 == closedir(dirp)) {
        perror("closedir");
        exit(1);
    }
    std::sort(files.begin() + 1, files.end(), caseincomp);
    std::sort(dirs.begin(), dirs.end());
}


int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);
    // Remove actual ls program command from argument vector
    args.erase(args.begin());
    uint8_t flags = getFlags(args);

    if (flags & 0x01) std::cout << "Flag -a enabled" << std::endl;
    if (flags & 0x02) std::cout << "Flag -R enabled" << std::endl;
    if (flags & 0x04) std::cout << "Flag -l enabled" << std::endl;
    //std::copy(args.begin(), args.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

    std::vector<std::string> files;
    std::vector<std::string> dirs;

    parsedirec(args, flags, files, dirs);
/*
    for (unsigned i =0; i < files.size(); ++i) {
        std::cout << files.at(i) << std::endl;
    }
    */

    // Skip first element, it contains the folder name
    for (auto it = files.begin() + 1; it != files.end(); ++it) {
        std::cout << *it << "  ";
    }
    std::cout << std::endl;



    return 0;
}
