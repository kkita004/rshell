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

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
//using namespace boost;


/* Current bug, will continue doing functions until error is reached,
 * will need to fix in next implementation */

/* TODO:
 * User Input
 * Tokenizing
 * Connectors
 */

/* Removes whitespace from start and end of string */
/*
   std::string trim(std::string s) {
// If null, do nothing
if (s == "")  return s;
std::string str = s;
unsigned i = 0;
while (str.at(i) == ' ') ++i;
str.erase(0, i);

i = str.size() - 1;
while (str.at(i) == ' ') --i;
str.erase(i + 1);

return str;
} */


/* Deallocates memory */
void free_char(char* prog, char** args, unsigned args_num) {
    if (prog != 0) {
        delete[] prog;
        prog = 0;
    }
    if (args != 0) {
        for (unsigned j = 0; j < args_num; ++j)
            delete[] args[j];
        delete[] args;
        args = 0;
    }

}

/* Checks connector, returns string without connector
 * and indicates what connector was at end
 * 0: none or ;
 * 1: &&
 * 2: ||
 * */

std::string check_connector(std::string s, int* con) {
    *con = 0;
    // Single character, special case if it is only ';'
    if (s.size() == 1) {
        if (s.at(0) == ';') {
            return "";
        }
    }
    else if (s.size() > 1) {
        if (s.at(s.size() - 1) == ';') {
            return s.substr(0, s.size() - 1);
        } else if (s.substr(s.size() - 2) == "&&") {
            //std::cout << "This contains \"&&\"" << std::endl;
            *con = 1;
            return s.substr(0, s.size() - 2);
        } else if (s.substr(s.size() - 2) == "||") {
            //std::cout << "This contains \"||\"" << std::endl;
            *con = 2;
            return s.substr(0, s.size() - 2);
        }
    }

    // no connector found, default to 0
    return s;
}


/* Takes string and returns pointers to char* of program,
 * char** of arguments in program,
 * int* to return connector
 * Returns number of arguments
 * */
unsigned return_command(std::string s, char*& prog, char**& args, int* connector) {
    using namespace boost;
    //   unsigned i = 0, j = 0;
    //   need to tokenize string

    std::string curr_str = s;
    //std::cout << "Trimming string" << std::endl;
    trim(curr_str);
    // check_connector will remove end and set connector
    //std::cout << "checking connector" << std::endl;
    curr_str = check_connector(curr_str, connector);
    //std::cout << "Trimming string 2" << std::endl;
    trim(curr_str);

    // Blank command, return error
    if (curr_str.size() == 0) {
    //    std::cout << "empty command" << std::endl;
        prog = 0;
        args = 0;
        return 0;
    }
    // Vector to hold commands and args
    std::vector<std::string> v;
    //std::cout << "Trimmed String: " << curr_str << std::endl;

    // Tokenize string
    tokenizer<escaped_list_separator<char> > t(
            curr_str,
            escaped_list_separator<char>("\\", " ", "\"\'"));
    for (
            tokenizer<escaped_list_separator<char> >::iterator i
            = t.begin();
            i != t.end(); ++i) {
        // Take strings and put them into a vector of strings
        //std::cout << *i << std::endl;
        v.push_back(*i);

    }


    // Take command name
    //std::cout << "Taking command name" << std::endl;
    char * cstr = new char [v.at(0).size() + 1];
    //std::cout << "copying string over" << std::endl;
    std::strcpy(cstr,v.at(0).c_str());
    //std::cout << "pointing to new string" << std::endl;
    prog = cstr;


    // Take arguments if any
    //std::cout << "Taking arguments " << std::endl;
    char ** arguments;
    if (v.size() - 1 > 0) {

        arguments = new char*[v.size() - 1];

        //std::cout << "Converting strings to arguments" << std::endl;
        for (unsigned i = 1; i < v.size(); ++i) {
            char * cstr = new char [v.at(i).size() + 1];
            std::strcpy(cstr, v.at(i).c_str());
            arguments[i-1] = cstr;
        }

        // Output char array

        //std::cout << "Outputting char arrays" << std::endl;
        //for (unsigned i = 1; i < v.size(); ++i) {
        //    for (unsigned j = 0; j < v.at(i).size(); ++j) {
        //        std::cout << arguments[i-1][j];
        //    }
        //    std::cout << std::endl;
        //}
        args = arguments;
    }
    /*
    // program only, no arguments passed in
    if (curr_str.find(' ') == std::string::npos) {
    std::cout << "no spaces found " << std::endl;
    char * cstr = new char [curr_str.length() + 1];
    std::strcpy(cstr, curr_str.c_str());
    prog = cstr;
    std::cout << "Copied string:****";
    for (unsigned i = 0; i < std::strlen(prog); ++i) {
    std::cout << prog[i];
    }
    std::cout << "****" << std::endl;

    args = 0;
    std::cout << "exiting return_command" << std::endl;
    return;
    }*/

    // program and infinitely many arguments
    // first extract the program to be run
    //std::string program;
    //std::stringstream ss(curr_str);
    //program << ss;

    // Get first word: This will be the program to run
    //    unsigned i = 0;
    /*
       std::stringstream ss (s);
       std::string outs;

       while (ss >> outs) {
       std::cout << outs;
       std::cin.get();
       };*/
    //std::cout << "exiting function" << std::endl;
    return v.size() - 1;
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
        // If a quotation mark is found, keep going until
        // closing quotation mark is found
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

        // Same as above, but for a single quote
        // Strings such as Thor's Day and They're
        // will not be accepted
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
        // If comment is found, ignore rest of line
        if (s.at(i) == '#') {
            std::cout << "comment found, ignoring rest of line" << std::endl;
            *index = s.size();

            return s.substr(start, i - start);
        }
        ++i;
    }
    // No delimiters found, return remaining string
    // Set index to last character
    //std::cout << "reached the end" << std::endl;
    *index = i + 1;
    return s.substr(start, i - start);
}

void rshell_loop () {
    std::string input_s;
    std::cout << std::endl;

    int i = 0;
    int c = 0;

    char e[] = {"exit"};

    while(1) {
        std::cout << "$ ";
        getline(std::cin, input_s);
        int pid;
        while ((unsigned) i < input_s.size() && i >= 0) {
            char * prog = 0;
            char ** args = 0;
            std::string parse = parse_string(input_s, &i);

            unsigned args_num = return_command(parse, prog, args, &c);

            // If command is exit
            if (!strcmp(e, prog)) {
                std::cout << "attempt to exit" << std::endl;
                free_char(prog, args, args_num);
                std::exit(1);
            }

            pid = fork();
            // Something went wrong
            if (pid == -1) {
                perror("fork error");
                exit(1);
            }
            // Child Process
            else if (pid == 0) {
                execvp(prog, args);
            }
            // Parent Process
            else if (pid > 0) {
                // wait(0) till child process finishes
                int status = 0;
                if (wait(&status) == -1) {
                    //child failed to change
                    perror("child process error");
                    //exit(1);
                    //
                } else {
                    //child successfully changed
                    if (status == 0) {
                        // return value is true
                        if (c == 2) {
                            // connector is ||, expected failure,
                            // received success
                            // break
                            free_char(prog, args, args_num);
                            break;
                        }
                    }
                    if (status != 0) {
                        // return value is false;
                        if (c == 1) {
                            free_char(prog, args, args_num);
                            break;
                        }
                    }

                }
            }

            // Deallocate char arrays
            free_char(prog, args, args_num);
        }
    }
    return;
}


void test() {
    int i = 0;

    std::string test;
    getline(std::cin, test);
    // std::cout << test << "****";
    int c = 0;
    // return_command(test, 0 ,0);
    std::string parse;
    while ((unsigned) i < test.size() && i >= 0) {
        char * prog = 0;
        char ** args = 0;
        parse = parse_string(test, &i);

        std::cout << "Initial string:****" << parse << "****" << std::endl;
        unsigned args_num = return_command(parse, prog, args, &c);
        //        std::cout << parse << std::endl;
        // unsigned j = 0;
        //        std::cout << "output:****";
        //        for (unsigned j = 0; j < std::strlen(prog); ++j) {
        //            std::cout << prog[j];
        //        }
        //        std::cout << "****" << std::endl;

        //printf("%s \n", prog);

        // Empty command, error
        if (prog == 0 && args == 0 && args_num == 0) {
            std::cout << "error: empty" << std::endl;
            break;

        }



        std::cout << "Deallocating prog" << std::endl;
        if (prog != 0) delete[] prog;
        std::cout << "Deallocating args" << std::endl;
        std::cout << "arg count: " << args_num << std::endl;
        if (args != 0) {
            for (unsigned j = 0; j < args_num; ++j)
                delete[] args[j];
            delete[] args;
        }


        if (c==0) {
            std::cout << "run next command (;)" << std::endl;
        } else if (c==1) {
            std::cout << "if success, run (&&)" << std::endl;
        } else if (c==2) {
            std::cout << "if fail, run (||)" << std::endl;
        }
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
    return;
}

int main(int argc, char **argv) {
    rshell_loop();
    //test();
    return 0;
}
