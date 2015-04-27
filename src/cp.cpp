// cp implementation 
// Takes 2 additional parameters, input and output
// Extra optional parameter copies 3 times nad gives copy time
#include <iostream>
#include <stdint.h>
#include <istream>
#include <ostream>
#include <fstream>
#include <cstdlib>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "Timer.h"


void outputtime(double wTime, double eTime, double sTime) {
    std::cout << "Wall Clock Time: " << wTime << std::endl;
    std::cout << "Elapsed Time: " << eTime << std::endl;
    std::cout << "System Time: " << sTime << std::endl;
    std::cout << std::endl;
}

// Copying with get() and put() streams
void getput(unsigned long long size, int argc, char **argv) {
    Timer t;
    double wTime, eTime, sTime;
    t.start();
    std::ifstream inf(argv[1]);
    std::ofstream outf(argv[2]);

    for (unsigned long long i = 0; i < size; ++i) {
        outf.put(inf.get());
    }
    t.elapsedTime(wTime, eTime, sTime);
    if (argc > 3) {
        std::cout << "Copying with get() and put(): " << std::endl;
        outputtime(wTime, eTime, sTime);
    }

    inf.close();
    outf.close();
}


// Copies with unix open, read, and write functions, but only
// with a single char buffer
// Very slow.
void readwrite1char(unsigned long long size, int argc, char **argv) {
    Timer t;
    double wTime, eTime, sTime;
    t.start();
    unsigned long long rin, rout;


    // Opens input file for reading
    int fr = open(argv[1], O_RDONLY);
    if (fr == -1) {
        perror("open 2");
        exit(1);
    }

    // Opens output file for writing, sets user permissions as 0644
    int fw = open(argv[2], O_CREAT | O_WRONLY , 0644);
    if (fw == -1) {
        perror("open 2");
        exit(1);
    }
    char buffer[1];
    while ((rin = read(fr, &buffer, 1)) > 0) {
        rout = write(fw, &buffer, (ssize_t) rin);
        if (rout != rin) {
            perror("write");
            exit(1);
        }
    }
    t.elapsedTime(wTime, eTime, sTime);
    if (argc > 3) {
        std::cout << "Copying with 1 char buffer read() write(): " << std::endl;
        outputtime(wTime, eTime, sTime);
    }
    if (-1 == close(fw)) {
        perror("close fw");
        exit(1);
    }
    if (-1 == close(fr)) {
        perror("close fr");
        exit(1);
    }
}

// Copies with unix open, read, and write functions, but with a
// BUFSIZ char buffer
// Fastest.
void readwritebufsiz(unsigned long long size, int argc, char **argv) {
    Timer t;
    double wTime, eTime, sTime;
    t.start();
    unsigned long long rin, rout;

    // Opens input file for reading
    int fr = open(argv[1], O_RDONLY);
    if (fr == -1) {
        perror("open 2");
        exit(1);
    }

    // Opens output file for writing, sets user permissions as 0644
    int fw = open(argv[2], O_CREAT | O_WRONLY , 0644);
    if (fw == -1) {
        perror("open 2");
        exit(1);
    }
    char buffer[BUFSIZ];
    while ((rin = read(fr, &buffer, BUFSIZ)) > 0) {
        rout = write(fw, &buffer, (ssize_t) rin);
        if (rout != rin) {
            perror("write");
            exit(1);
        }
    }
    t.elapsedTime(wTime, eTime, sTime);
    if (argc > 3) {
        std::cout << "Copying with BUFSIZ buffer read() write(): " << std::endl;
        outputtime(wTime, eTime, sTime);
    }
    if (-1 == close(fw)) {
        perror("close fw");
        exit(1);
    }
    if (-1 == close(fr)) {
        perror("close fr");
        exit(1);
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: ./cp <input> <output> <optional>" << std::endl;
        exit(1);
    }

    //  Check to see if files exist 

    if (access(argv[1], F_OK) != 0) {
        std::cerr << "source file doesn't exist or no permissions" << std::endl;
        exit(1);
    }
    if (access(argv[2], F_OK) == 0) {
        std::cerr << "target file already exists" << std::endl;
        exit(1);
    }

    // Get size of file 
    unsigned long long size;
    struct stat st;
    if (stat(argv[1], &st) == 0) {
        size = st.st_size;
    } else {
        perror("stat size check");
        exit(1);
    }


    // Check if arg1 is directory
    if (stat(argv[1], &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            std::cerr << "arg1 is directory" << std::endl;
            exit(1);
        }
    } else {
        perror("stat dir check");
    }

    // Check if arg2 is directory
    if (stat(argv[2], &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            std::cerr << "arg2 is directory" << std::endl;
            exit(1);
        }
    } 

    if(argc == 3){
        readwritebufsiz(size, argc, argv);
    }
    else{
        getput(size, argc, argv);
        readwrite1char(size, argc, argv);
        readwritebufsiz(size, argc, argv);
    }

    return 0;
} 
