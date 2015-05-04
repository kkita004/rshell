// rm utility for rshell
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// Removes regular files
void removeFiles(std::vector<std::string> files) {

    for (auto it = files.begin(); it != files.end(); ++it) {
        if (-1 == unlink((*it).c_str())) {
            perror("unlink");
            continue;
        }
    }
}


// Returns all entries in a directory
bool checkDir(std::string arg, std::vector<std::string>& files) {

    DIR* dirp;
    if (NULL == (dirp = opendir(arg.c_str()))) {
        perror("opendir");
        return false;
    }
    struct dirent *filespecs;
    errno = 0;
    while (NULL != (filespecs = readdir(dirp))) {
        std::string str(filespecs->d_name);
        if (str == "." || str == "..") continue;
        std::string full = arg + "/" + str;
        files.push_back(full);
    }
    if (0 != errno) {
        perror("readdir");
        return false;
    }
    if (-1 == closedir(dirp)) {
        perror("closedir");
        return false;
    }

    return true;
}


// Requires a vector of files, returns associated vector of stat
void statFiles(std::vector<std::string>& args, std::vector<struct stat>& s) {

    for (auto it = args.begin(); it != args.end(); ++it) {
        struct stat st;
        if(-1 == stat((*it).c_str(), &st)) {
            perror("stat");
            std::cerr << (*it) << std::endl;
            args.erase(it);
            --it;
        } else {
            s.push_back(st);
        }
    }

}

// Separates entries into directories and regular files, rEnabled is for -r flag
void separateFiles(bool rEnabled, std::vector<std::string> args, std::vector<struct stat> s,
        std::vector<std::string>& dir, std::vector<std::string>& files) {
    for (unsigned i = 0; i < s.size(); ++i) {
        if S_ISDIR((s.at(i)).st_mode) {
            if (!rEnabled) {
                std::cerr << "cannot remove directory without -r flag" << std::endl;
                continue;
            }
            dir.push_back(args.at(i));
        } else {
            files.push_back(args.at(i));
        }
    }
}

// Pass in a list of directories, will remove them all
void dirManager(std::vector<std::string> dir) {
    for (unsigned j = 0; j < dir.size(); ++j) {
        std::vector<std::string> dirs;
        dirs.push_back(dir.at(j));
        for (unsigned i = 0; i < dirs.size(); ++i) {
            std::vector<std::string> newDirs;
            std::vector<std::string> files;
            std::vector<std::string> toDelete;
            std::vector<struct stat> s;
            if(!checkDir(dirs.at(i), toDelete)) {
                continue;
            }
            statFiles(toDelete, s);
            separateFiles(true, toDelete, s, newDirs, files);
            removeFiles(files);
            dirs.insert(dirs.end(), newDirs.begin(), newDirs.end());
        }
        for (unsigned i = dirs.size() - 1; i > 0; --i) {
            if (-1 == rmdir(dirs.at(i).c_str())) {
                perror("rmdir");
            }
        }
    }
    for (unsigned i = 0; i < dir.size(); ++i) {
        if (-1 == rmdir(dir.at(i).c_str())) {
            perror("rmdir");
        }
    }

}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: rm <flags> <files>" << std::endl;
        exit(1);
    }

    std::vector<std::string> args(argv, argv + argc);
    args.erase(args.begin());

    auto p = std::find(args.begin(), args.end(), "-r");
    bool rEnabled = false;
    if (p != args.end()) {
        rEnabled = true;
        args.erase(p);
    }

    std::vector<struct stat> s;
    std::vector<std::string> dir;
    std::vector<std::string> files;

    statFiles(args, s);

    separateFiles(rEnabled, args, s, dir, files);

    removeFiles(files);
    dirManager(dir);

    return 0;
}
