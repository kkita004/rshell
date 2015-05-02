// ls program for cs 100
#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#include <ostream>
#include <iterator>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <time.h>
#include <stdio.h>

#include <pwd.h>
#include <grp.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <boost/algorithm/string.hpp>

std::string filestats(std::string, int&, unsigned&, unsigned&);


// Struct dirc holds name of directory,
// files in directory, and which ones are dirs
// also holds filestats
struct dirc {
    int blocksize = 0;
    std::string name;
    std::vector<std::pair<std::string, std::string> > files;
    std::vector<std::string> dirs;
    unsigned max_len_size = 1;
    unsigned max_len_link = 1;
};

// Return length of string for formatting
unsigned int numLength(unsigned int i) {
    if (i == 0) return 1;
    return std::to_string(i).length();

}

// Parses input to find flags argument, returns enabled flags
// Will erase any argument starting with a '-' character
uint8_t getFlags(std::vector<std::string>& args) {
    uint8_t flags = 0x00;
    for (auto it = args.begin(); it != args.end(); ) {
        // Found options flags
        if ((*it).at(0) == '-') {
            while ((*it).find('a')!=std::string::npos) {
                flags = flags | 0x01;
                (*it).erase((*it).find('a'), 1);
            }
            while ((*it).find('R')!=std::string::npos) {
                flags = flags | 0x02;
                (*it).erase((*it).find('R'), 1);
            }
            while ((*it).find('l')!=std::string::npos) {
                flags = flags | 0x04;
                (*it).erase((*it).find('l'), 1);
            }
            if (!((*it) == "-")) {
                std::cerr << "invalid option" << (*it) << std::endl;
                exit(1);
            }
            it = args.erase(it);
        } else {
            ++it;
        }
    }
    return flags;
}


// Case insensitive string comparison

bool paircaseincomp(const std::pair<std::string, std::string>& p1,
                const std::pair<std::string, std::string>& p2) {
    std::string s1 = p1.first;
    std::string s2 = p2.first;
    // TODO: Fix sorting for ... directories

    /*
    if (s1.length() < s2.length()) {
        std::swap(s1, s2);
    }*/

    /*
    while (s1.length() > 1 && s1.at(0) == '.') {
        s1.erase(0,1);
    }
    while (s2.length() > 1 && s2.at(0) == '.') {
        s2.erase(0,1);
    }

    */
    /*
    while (s1.length() > 1 && s2.length() > 1 && s1.at(0) == '.' && s2.at(0) == '.') {
        s1.erase(0,1);
        s2.erase(0,1);
    }*/

    return boost::to_upper_copy(s1) < boost::to_upper_copy(s2);
}

bool caseincomp(std::string s1, std::string s2) {
    return boost::to_upper_copy(s1) < boost::to_upper_copy(s2);
}

bool dirccomp(dirc d1, dirc d2) {
    return boost::to_upper_copy(d1.name) < boost::to_upper_copy(d2.name);

}


// Looks through given directory, returns list of files and
// directories in files
// Returns name of directory

void parsedirec(std::string arg, uint8_t flags,
        std::vector<std::pair<std::string, std::string> >& files,
        std::vector<std::string>& dirs) {

    // If no other arguments are passed in, only use current directory
    //std::vector<std::string> files;
    //std::vector<std::string> dirs;
    //if (args.size() == 0) files.push_back(".");
    //else {files = args;}

    DIR* dirp;
    // Couldn't open directory, might not exist
    if (NULL == (dirp = opendir(arg.c_str()))) {
        perror("opendir");
        return;
        //exit(1);
    }
    struct dirent *filespecs;
    //int result;
    errno = 0;
    // Read in file objects
    // Call perror here too
    while (NULL != (filespecs = readdir(dirp))) {
        // Must keep path of file, otherwise won't find
        const std::string str(filespecs->d_name);
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

        // Might need to call perror here
        if (S_ISDIR(dstat.st_mode)) {
            dirs.push_back(str);
        }
        std::pair<std::string, std::string> p(str, "");
        files.push_back(p);
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
    std::sort(files.begin(), files.end(), paircaseincomp);
    std::sort(dirs.begin(), dirs.end());
}

// Manages passing in directories

void parsemanager(std::vector<std::string> args, uint8_t flags, std::vector<dirc>& fs) {
    if (args.size() == 0) args.push_back(".");
    for (unsigned i = 0; i < args.size(); ++i) {
        fs.push_back(dirc());
        parsedirec(args.at(i), flags, fs.at(i).files, fs.at(i).dirs);
        fs.at(i).name = args.at(i);
        // TODO:Need to call filestat, but also when sorting keep track of
        // sorted objects
        // Recursive enabled
        if (flags & 0x04) {
            for (unsigned j = 0; j < fs.at(i).files.size(); ++j) {
                // Skip . and .. files
                //if (fs.at(i).dirs.at(j) == "." || fs.at(i).dirs.at(j) == "..") continue;
                std::string path = fs.at(i).name + "/" + fs.at(i).files.at(j).first;
                //std::cout << "path: " << path << std::endl;
                fs.at(i).files.at(j).second =
                filestats(path,fs.at(i).blocksize,
                          fs.at(i).max_len_link, fs.at(i).max_len_size);
            }
        }
        if (flags & 0x02) {
            for (unsigned j = 0; j < fs.at(i).dirs.size(); ++j) {
                // Skip . and .. files
                if (fs.at(i).dirs.at(j) == "." || fs.at(i).dirs.at(j) == "..") continue;
                std::string path = fs.at(i).name + "/" + fs.at(i).dirs.at(j);
                boost::trim(path);
                args.push_back(path);
         //           if (flags & 0x04) {
                        //std::cout << path << std::endl;
         //               fs.at(i).files.at(j).second = filestats(path);
         //           }
            }
        }
    }
    // Make sure this is compatible with filestats
    std::sort(args.begin(), args.end(), caseincomp);
}



// Returns filemode character
std::string filemode (mode_t m) {
    // Is this even necessary
    if (!m) return "-";
    if S_ISREG(m) return "-";
    if S_ISDIR(m) return "d";
    if S_ISCHR(m) return "c";
    if S_ISBLK(m) return "b";
    if S_ISFIFO(m) return "p";
    if S_ISLNK(m) return "l";
    if S_ISSOCK(m) return "s";
    return "-";
}


// Returns File permissions
std::string permissions(mode_t m) {
    std::string s;
    if (S_IRUSR & m) s.append("r");
    else s.append("-");
    if (S_IWUSR & m) s.append("w");
    else s.append("-");
    if (S_IXUSR & m) s.append("x");
    else s.append("-");

    if (S_IRGRP & m) s.append("r");
    else s.append("-");
    if (S_IWGRP & m) s.append("w");
    else s.append("-");
    if (S_IXGRP & m) s.append("x");
    else s.append("-");

    if (S_IROTH & m) s.append("r");
    else s.append("-");
    if (S_IWOTH & m) s.append("w");
    else s.append("-");
    if (S_IXOTH & m) s.append("x");
    else s.append("-");

    return s;
}

std::string timetostring(time_t& t) {
    // Get Current Time now to decide whether to output
    // Year or Time
    time_t tnow;
    time(&tnow);
    struct tm tnowinfo;;
    localtime_r(&tnow, &tnowinfo);

    struct tm ti;
    localtime_r(&t, &ti);

    char buffer[80];

    // If matches current year, output time
    // else output year instead
    if (tnowinfo.tm_year == ti.tm_year) {
        strftime(buffer, 80, "%h %e %R", &ti);
    } else {
        strftime(buffer, 80, "%h %e %Y", &ti);
    }

    std::string s(buffer);
    boost::trim(s);
    return s;
}

// Get file details
// Take in directory, return string with file details
std::string filestats(std::string s, int& blocksize,
                      unsigned& max_len_link, unsigned& max_len_size) {
    struct stat st;
    if (-1 == stat(s.c_str(), &st)) {
        perror("stat not found");
        return "";
    }

    // Get user and group names
    struct passwd *u = getpwuid(st.st_uid);
    if (NULL == u) {
        perror("getpwuid");
    }

    struct group *g = getgrgid(st.st_gid);
    if (NULL == g) {
        perror("getgrgid");
    }

    std::string r;

    r.append(filemode(st.st_mode));
    r.append(permissions(st.st_mode));
    r.append(" ");
    r.append(std::to_string((int)st.st_nlink));
    r.append(" ");
    r.append(u->pw_name);
    r.append(" ");
    r.append(g->gr_name);
    r.append(" ");
    r.append(std::to_string(st.st_size));
    r.append(" ");
    r.append(timetostring(st.st_mtime));
    r.append(" ");
    // Convert blocks to 1024
    blocksize += st.st_blocks / 2; //ceil((st.st_size + 1) / 1024);
    //std::cout << numLength(st.st_nlink) << std::endl;
    //std::cout << max_len_link << std::endl;
    if (max_len_link < numLength(st.st_nlink)) {
        max_len_link = numLength(st.st_nlink);
    }

    if (max_len_size < numLength(st.st_size)) {
        max_len_size = numLength(st.st_size);
    }

    return r;
}

unsigned getNumCols() {
    struct winsize w;
    if (-1 == ioctl(0, TIOCGWINSZ, &w)) {
        perror("ioctl");
        return 80;
    }

    return (unsigned) w.ws_col;
}

void outputfs(std::vector<dirc>& fs, uint8_t flags) {
    std::sort(fs.begin(), fs.end(), dirccomp);
    for (unsigned i = 0; i < fs.size(); ++i) {
        if (fs.size() > 1) {
            std::cout << fs.at(i).name << ":" << std::endl;
        }
        if (flags & 0x04) {
            std::cout << "total " << fs.at(i).blocksize << std::endl;
        }
        // Keep track if line was empty or not
        bool somethingOutput = false;
        bool hasReset = false;
        unsigned linelength = 0;
        unsigned cols = getNumCols();
        unsigned columncount = 0;
        std::stringstream outputBuffer;

        unsigned longeststring = 0;
        // get longest string
        for (auto it = fs.at(i).files.begin(); it != fs.at(i).files.end(); ++it) {
            if ((*it).first.length() > longeststring) {
                longeststring = (*it).first.length();
            }
        }

        for (auto it = fs.at(i).files.begin(); it != fs.at(i).files.end(); ++it) {
            if ((*it).first == "") continue;
            if (flags & 0x04) {
                std::stringstream ss((*it).second);
                std::string s;
                ss >> s;
                std::cout.width(fs.at(i).max_len_link);
                std::cout << s << ' ';
                //std::cout << fs.at(i).max_len_link;
                ss >> s;
                std::cout << s;
                std::cout << ' ';
                std::cout.width(0);
                // Output user
                ss >> s;
                std::cout << s << ' ';
                // Output Group
                ss >> s;
                std::cout << s << ' ';

                // Output bytes
                std::cout.width(fs.at(i).max_len_size);
                ss >> s;
                std::cout << s << ' ';

                // Output date
                std::cout.width(3);
                ss >> s;
                std::cout << s << ' ' << std::flush;

                ss >> s;
                if (s.length() == 1) {
                    s = " " + s;
                }
                //std::cout << std::right;
                std::cout << s << ' ' << std::flush;

                ss >> s;
                if (s.length() == 4) {
                    s = " " + s;
                }
                std::cout << s << ' ' << std::flush;
                std::cout << (*it).first << std::endl;
                //std::cout << (*it).second << (*it).first << std::endl;
            }
            // Change this to formatting into columns
            else {
                linelength += (*it).first.length() + 2;
                if (linelength <= cols) {
                    //linelength += (*it).first.length() + 2;
                    outputBuffer << (*it).first << "  ";
                } else if (!hasReset) {
                    // if line output is not long enough, go back to beginning
                    // find longest string, divide column count by string length
                    // setwidth that value
                    //
                    it = fs.at(i).files.begin();
                    hasReset = true;
                } else {
                    unsigned ratio = cols / (longeststring + 1);
                    //std::cout << "ratio: " << ratio;
                    // TODO: Fix this to accomodate variable terminal widths
                    std::cout << std::setw(longeststring + 1) << std::left;
                    std::cout << (*it).first << std::flush;
                    columncount++;
                    if (columncount >= ratio) {
                        std::cout << std::endl;
                        columncount = 0;
                    }
                }
                //std::cout << (*it).first << " ";
            }
            somethingOutput = true;

        }
        //std::cout << "cols: " << cols << std::endl;
        if (linelength <= cols) {
            std::cout << outputBuffer.str();
        }
        //std::cout << std::endl;
        if (somethingOutput && flags == 0x02) {
            std::cout << std::endl;
        }
        if (i + 1 < fs.size()) {
            std::cout << std::endl;
        }
    }
    // If -a or empty, then output an extra newline
    if (!flags || (flags == 0x01) ) {
        std::cout << std::endl;
    }
}

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);
    // Remove actual ls program command from argument vector
    args.erase(args.begin());
    uint8_t flags = getFlags(args);
    std::sort(args.begin(), args.end(), caseincomp);

    //if (flags & 0x01) std::cout << "Flag -a enabled" << std::endl;
    //if (flags & 0x02) std::cout << "Flag -R enabled" << std::endl;
    //if (flags & 0x04) std::cout << "Flag -l enabled" << std::endl;
    //std::copy(args.begin(), args.end(), std::ostream_iterator<std::string>(std::cout, "\n"));

    //std::vector<std::string> files;
    //std::vector<std::string> dirs;
    std::vector<dirc> fs;

    parsemanager(args, flags, fs);
    outputfs(fs, flags);
    //parsedirec(args, flags, files, dirs);
    /*
       for (unsigned i =0; i < files.size(); ++i) {
       std::cout << files.at(i) << std::endl;
       }
    */

    return 0;
}
