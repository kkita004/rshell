#include <iostream>
#include <cstdlib>

// fork()
#include <unistd.h>

// wait()
#include <sys/types.h>
#include <sys/wait.h>

// perror()
#include <stdio.h>

void rshell_loop (char ** argv) {
    std::cout << std::endl;
    std::string input;
    int pid;

    do {
        std::cout << "$ ";
        std::cin >> input;

        pid = fork();
        // Something went wrong
        if (pid == -1) {
            perror("fork error");
            exit(1);
        }
        // Child Process
        else if (pid == 0) {
            execvp(input.c_str(), argv);
        }
        // Parent Process
        else if (pid > 0) {
            // wait(0) till child process finishes
            if (wait(0) == -1) {
               perror("wait error");
               exit(1);
            }
        }
    } while (input != "exit");
    return;
}

int main(int argc, char **argv) {
    rshell_loop(argv);
    return 0;
}
