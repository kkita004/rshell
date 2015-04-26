#include <iostream>
#include <vector>
#include <ostream>
#include <iterator>
#include <cstdlib>

// Parses input to find flags argument, returns enabled flags
uint8_t getFlags(const std::vector<std::string>& args) {
    uint8_t flags = 0x00;
    for (auto it = args.begin(); it != args.end(); ++it) {
        // Found options flags
        if ((*it).at(0) == '-') {
            if ((*it).find('a')!=std::string::npos) flags = flags | 0x01;
            if ((*it).find('l')!=std::string::npos) flags = flags | 0x02;
            if ((*it).find('R')!=std::string::npos) flags = flags | 0x04;
        }
    }
    return flags;
}

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);
    //std::copy(args.begin(), args.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
    uint8_t flags = getFlags(args);
    if (flags & 0x01) std::cout << "Flag -a enabled" << std::endl;
    if (flags & 0x02) std::cout << "Flag -l enabled" << std::endl;
    if (flags & 0x04) std::cout << "Flag -R enabled" << std::endl;



    return 0;
}
