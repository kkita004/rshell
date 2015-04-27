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




// Struct dirc holds name of directory,
// files in directory, and which ones are dirs
struct dirc {
    std::string name;
    std::vector<std::string> files;
    std::vector<std::string> dirs;
};


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


// Looks through given directory, returns list of files and
// directories in files
// Returns name of directory
void parsedirec(std::string arg, uint8_t flags,
        std::vector<std::string>& files,
        std::vector<std::string>& dirs) {

    // If no other arguments are passed in, only use current directory
    //std::vector<std::string> files;
    //std::vector<std::string> dirs;
    //if (args.size() == 0) files.push_back(".");
    //else {files = args;}

    DIR* dirp;
    if (NULL == (dirp = opendir(arg.c_str()))) {
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
        std::string full = arg + "/" + str;
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
    std::sort(files.begin(), files.end(), caseincomp);
    std::sort(dirs.begin(), dirs.end());
}

// Manages passing in directories
void parsemanager(std::vector<std::string> args, uint8_t flags, std::vector<dirc>& fs) {
    if (args.size() == 0) args.push_back(".");
    for (unsigned i = 0; i < args.size(); ++i) {
        fs.push_back(dirc());
        parsedirec(args.at(i), flags, fs.at(i).files, fs.at(i).dirs);
        fs.at(i).name = args.at(i);

        // Recursive enabled
        if (flags & 0x02) {
            for (unsigned j = 0; j < fs.at(i).dirs.size(); ++j) {
                args.push_back(fs.at(i).name + "/" + fs.at(i).dirs.at(j));
            }
        }
        std::sort(args.begin(), args.end(), caseincomp);
    }
}

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);
    // Remove actual ls program command from argument vector
    args.erase(args.begin());
    uint8_t flags = getFlags(args);
    std::sort(args.begin(), args.end(), caseincomp);

    if (flags & 0x01) std::cout << "Flag -a enabled" << std::endl;
    if (flags & 0x02) std::cout << "Flag -R enabled" << std::endl;
    if (flags & 0x04) std::cout << "Flag -l enabled" << std::endl;
    //std::copy(args.begin(), args.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

    //std::vector<std::string> files;
    //std::vector<std::string> dirs;
    std::vector<dirc> fs;

    parsemanager(args, flags, fs);
    //parsedirec(args, flags, files, dirs);
    /*
       for (unsigned i =0; i < files.size(); ++i) {
       std::cout << files.at(i) << std::endl;
       }
    */

    for (unsigned i = 0; i < fs.size(); ++i) {
        if (fs.size() > 1) {
            std::cout << fs.at(i).name << ":" << std::endl;
        }
        for (auto it = fs.at(i).files.begin(); it != fs.at(i).files.end(); ++it) {
            std::cout << *it << "  ";
        }
        if (i + 1 < fs.size()) std::cout << std::endl << std::endl;
    }

    return 0;
}
