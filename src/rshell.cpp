#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>

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

/* Removes whitespace from start and end of string */
std::string trim(std::string s) {
    /* If null, do nothing */
    if (s == "")  return s;
    std::string str = s;
    unsigned i = 0;
    while (str.at(i) == ' ') ++i;
    str.erase(0, i);

    i = str.size() - 1;
    while (str.at(i) == ' ') --i;
    str.erase(i + 1);

    return str;
}

/* Checks connector, returns string without connector
 * and indicates what connector was at end
 * 0: none or ;
 * 1: &&
 * 2: ||
 * */
std::string check_connector(std::string s, int* con) {
    *con = 0;
    if (s.at(s.size() - 1) == ';') {
        return s.substr(0, s.size() - 1);
    } else if (s.substr(s.size() - 2) == "&&") {
        *con = 1;
        return s.substr(0, s.size() - 2);
    } else if (s.substr(s.size() - 2) == "||") {
        *con = 2;
        return s.substr(0, s.size() - 2);
    }

    // no connector found, default to 0
    return s;
}


/* Takes string and returns pointers to char* of program,
 * char** of arguments in program,
 * int* to return connector
 * */
void return_command(std::string s, char* prog, char** args, int* connector) {
 //   unsigned i = 0, j = 0;
 //   need to tokenize string
    std::string curr_str = trim(s);
    check_connector(curr_str, connector);
    curr_str = trim(curr_str);

    // Blank command, return error
    if (curr_str.size() == 0) {
        prog = 0;
        args = 0;
        return;
    }

    // program only, no arguments passed in
    if (curr_str.find(' ') == std::string::npos) {
        char * cstr = new char [curr_str.length() + 1];
        std::strcpy(cstr, curr_str.c_str());
        prog = cstr;

        args = 0;
        return;
    }




    // Get first word: This will be the program to run
    //    unsigned i = 0;
    /*
    std::stringstream ss (s);
    std::string outs;

    while (ss >> outs) {
        std::cout << outs;
        std::cin.get();
    };*/
    return;
}


/* Parses string, returns substring of first command,
 * changes index pointer to after substring.
 * Substring contains connector if applicable
 * Returns a negative value for index if error has been
 * found i.e. ("&&&" or "|||") */

std::string parse_string(std::string s, int* index) {
    /* If index < 0, meant error was returned last parse
     * Quit parsing */
    if (*index < 0) {
        return s;
    }
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
            /* No closing quotation found */
            if (i == s.size()) {
                std::cout << "error: no closing double quotation found" << std::endl;
                *index = -1;
                return s;
            }
        }

        /* Same as above, but for a single quote
         * Strings such as Thor's Day and They're
         * will not be accepted */
        if (s.at(i) == '\'') {
            //std::cout << "found a single quote" << std::endl;
            do {
                i++;
            } while (i < s.size() && s.at(i) != '\'');
            if (i == s.size()) {
                std::cout << "error: no closing single quotation found" << std::endl;
                *index = -2;
                return s;
            }
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
                    // If this is commented out, then &&& will be split
                    // return error
                    if (i+1 < s.size() && s.at(i+1) == '&') {
                        *index = -3;
                        return s;
                    }
                  //  std::cout << "i: " << i << std::endl;
                    *index = i + 1;
                    return s.substr(start, i - start + 1);
                }
            } else if (s.at(i) == '|') {
                // Check if next character is also '|'
                i++;
                if (i < s.size() && s.at(i) == '|')  {
                    // Check if third character is also a '|', if so
                    // return error
                    if (i+1 < s.size() && s.at(i+1) == '|') {
                        *index = -4;
                        return s;
                    }
                    *index = i + 1;
                    return s.substr(start, i - start + 1);
                }
            }
        }
        ++i;
    }
    // No delimiters found, return remaining string
    // Set index to last character
    //std::cout << "reached the end" << std::endl;
    *index = i + 1;
    return s.substr(start, i - start);
}

void rshell_loop (char ** argv) {
    std::cout << std::endl;
    std::string input_s;
    int pid;
    //char ** args = 0;
    //char * c = 0;

    do {
        std::cout << "$ ";
        //user_input(c, args);
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
    char * prog = 0;
    char ** args = 0;

    std::string test;
    getline(std::cin, test);
   // std::cout << test << "****";
    int c = 0;
   // return_command(test, 0 ,0);
    std::string parse;
     while ((unsigned) i < test.size() && i >= 0) {
        parse = parse_string(test, &i);

//TODO: Fix return_command
        std::cout << "Initial string:****" << parse << "****" << std::endl;
        return_command(parse, prog, args, &c);
        std::cout << parse;
        printf("%s \n", prog);


        if (prog != 0) delete prog;
        if (args != 0) delete[] args;
        /*
        std::cout << "Trim(1):****" << trim(parse) << "****" << std::endl;
        parse = check_connector(parse, &c);
        std::cout << "Remove connectors:****" << parse << "****" << std::endl;
        std::cout << "c: " << c << std::endl;
        std::cout << "Trim(2):****" << trim(parse) << "****" << std::endl;
        */

        //std::cout << "RETURN COMMAND" << std::endl;
        std::cout << "===============================================" << std::endl;

        //if (i == -1) std::cout << "error: &&& found" << std::endl;
        //else if (i == -2) std::cout << "error: ||| found" << std::endl;
    }

    // rshell_loop(argv);
    return 0;
}
