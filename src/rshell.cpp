#include <iostream>
#include <cstdlib>

// fork()
#include <unistd.h>

// wait()
#include <sys/types.h>
#include <sys/wait.h>

// perror()
// strtok()
#include <stdio.h>

/* TODO:
 * User Input
 * Tokenizing
 * Connectors
 */




/* Should store commands in a queue of Commands */
struct Command {
    char * program;
    char ** args;
}

/* Parses user input, sets pointers to programs and args
 * Don't forget to deallocate pointers after calling this
 * function */
void user_input(char * prog, char ** args) {
    return;
}

void rshell_loop (char ** argv) {
    std::cout << std::endl;
    std::string input_s;
    int pid;
    char ** args;

    do {
        std::cout << "$ ";
        user_input(args);
        //std::cin >> input_s;

        pid = fork();
        // Something went wrong
        if (pid == -1) {
            perror("fork error");
            exit(1);
        }
        // Child Process
        else if (pid == 0) {
            execvp(input_s.c_str(), argv);
        }
        // Parent Process
        else if (pid > 0) {
            // wait(0) till child process finishes
            if (wait(0) == -1) {
               perror("child process error");
               exit(1);
            }
        }
    } while (input_s != "exit");
    return;
}

int main(int argc, char **argv) {

   // rshell_loop(argv);
    return 0;
}
