#include <iostream>
#include <cstdlib>
#include <string>

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
};

/* Parses user input, sets pointers to programs and args
 * Don't forget to deallocate pointers after calling this
 * function */
void user_input(char * prog, char ** args) {
    return;
}


/* Parses string, returns substring of first command,
 * changes index pointer to after substring.
 * Substring contains connector if applicable
 * Returns a negative value for index if error has been
 * found i.e. ("&&&" or "|||") */

std::string parse_string(std::string s, int* index) {
    // Start at 0 index by default
    // Otherwise, start at index
    // i traverses string
    // start marks beginning index
    unsigned i = 0;
    unsigned start = *index;
    if (*index != 0) {
        i = *index;
    }
    // std::cout << s.size();

    while (i < s.size()) {
        /* If a quotation mark is found, keep going until
         * closing quotation mark is found */
        if (s.at(i) == '\"') {
            //std::cout << "found a double quote" << std::endl;
            do {
                i++;
            } while (i < s.size() && s.at(i) != '\"');
        }

        /* Same as above, but for a single quote
         * Strings such as Thor's Day and They're
         * will not be accepted */
        if (s.at(i) == '\'') {
            //std::cout << "found a single quote" << std::endl;
            do {
                i++;
            } while (i < s.size() && s.at(i) != '\'');
        }
        if (i < s.size()) {
            // Delimiters
            // If ';', "&&", or "||" is found,
            // return substring from initial index to
            // delimiter
            if (s.at(i) == ';') {
                *index = i + 1;
                return s.substr(start, i - start + 1);
            } else if (s.at(i) == '&') {
                // Check if next character is also '&'
                i++;
                if (i < s.size() && s.at(i) == '&')  {
                    // Check if third character is also a '&', if so
                    // return error
                    if (i+1 < s.size() && s.at(i+1) == '&') {
                        *index = -1;
                        return s;
                    }
                  //  std::cout << "i: " << i << std::endl;
                    *index = i + 1;
                    return s.substr(start,i - start + 1);
                }
            } else if (s.at(i) == '|') {
                // Check if next character is also '|'
                i++;
                if (i < s.size() && s.at(i) == '|')  {
                    // Check if third character is also a '|', if so
                    // return error
                    if (i+1 < s.size() && s.at(i+1) == '|') {
                        *index = -2;
                        return s;
                    }
                    *index = i + 1;
                    return s.substr(start,i - start + 1);
                }
            }
        }
        ++i;
    }
    // No delimiters found, return original string
    return s;
}

void rshell_loop (char ** argv) {
    std::cout << std::endl;
    std::string input_s;
    int pid;
    char ** args = 0;
    char * c = 0;

    do {
        std::cout << "$ ";
        user_input(c, args);
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
    int i = 0;
    std::string test;
    getline(std::cin, test);
    while ((unsigned) i < test.size()) {
        std::cout << parse_string(test, &i) << "****" << std::endl;
        if (i == -1) std::cout << "error: &&& found" << std::endl;
        else if (i == -2) std::cout << "error: ||| found" << std::endl;
        //std::cout << parse_string(test, &i) << "****" << std::endl;
        //std::cout << parse_string(test, &i) << "****" << std::endl;
        //std::cout << i << std::endl;
        //std::cin.get();
    }
    // rshell_loop(argv);
    return 0;
}
