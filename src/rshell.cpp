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



void parse_args(std::string s, std::vector<std::string>& v, int * connector) {
    using namespace boost;
    std::string curr_str = s;
    boost::trim(curr_str);
    curr_str = check_connector(curr_str, connector);

    // Empty, return error
    if (curr_str.size() == 0) {
        v.clear();
        return;
    }

    tokenizer<escaped_list_separator<char> > t(
            curr_str,
            escaped_list_separator<char>("\\", " ", "\"\'"));
    for (
            tokenizer<escaped_list_separator<char> >::iterator i
            = t.begin();
            i != t.end(); ++i) {
        std::string temp = *i;
        // Remove white space and tabs
        boost::trim(temp);
        boost::trim_if(temp, boost::is_any_of("\t"));
        if (temp != "") v.push_back(temp);
    }
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
                std::cerr << "error: no closing double quotation found" << std::endl;
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
                std::cerr << "error: no closing single quotation found" << std::endl;
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
            //std::cout << "comment found, ignoring rest of line" << std::endl;
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

    char e[] = {"exit"};
    std::string prompt;

    // Setup prompt
    // Login and hostname limit is 256, otherwise truncated
    char login[256];
    if (getlogin_r(login, sizeof login)) {
        perror("getlogin error");
        login[0] = '\0';
    }
    char hostname[256];
    if (gethostname(hostname, sizeof hostname)) {
        perror("gethostname error");
        hostname[0] = '\0';
    }


    while(1) {
        if (login[0] != '\0' || hostname[0] != '\0') {
            printf("%s@%s$ ", login, hostname);
        } else {
            printf("$ ");

        }
        getline(std::cin, input_s);
        // Empty input
        boost::trim(input_s);
        if (input_s == "") continue;
        int pid;
        int i = 0;

        while ((unsigned) i < input_s.size() && i >= 0) {
            std::vector<std::string> v;
            int c = 0;
            std::string parse = parse_string(input_s, &i);
            // No closing quotes, break
            if (i < 0) break;
            parse_args(parse, v, &c);

            /*
               for (unsigned b = 0; b < v.size(); ++b) {
               std::cout << "VECTOR[" << b << "]:[" << v.at(b) << "]" << std::endl;
               }
               */

            // Buffer only holds 1000 commands total;
            // Any longer will cause errors
            const char* args[1000];
            // Clear buffer
            for (unsigned i = v.size(); i < 1000 - v.size(); ++i) {
                args[i] = '\0';
            }
            for (unsigned j = 0; j < v.size(); ++j) {
                const char * p = v.at(j).c_str();
                args[j] = p;
            }

            if (v.size() == 0) {
                break;
            }
            // If command is exit
            if (!strcmp(e, args[0])) {
                exit(1);
            }

            pid = fork();
            // Something went wrong
            if (pid == -1) {
                perror("Fork error");
                exit(1);
                // If for some reason there's an error in exit();
                perror("exit error");
            }
            // Child Process
            else if (pid == 0) {
                execvp(args[0], (char * const *) args);
                perror("Command error");
                _exit(1);
            }
            // Parent Process
            else if (pid > 0) {
                // wait(0) till child process finishes
                int status = 0;
                if (wait(&status) == -1) {
                    perror("child process error");
                } else {
                    //child successfully changed
                    if (status == 0) {
                        // return value is true
                        if (c == 2) {
                            break;
                        }
                    }
                    if (status != 0) {
                        // return value is false;
                        if (c == 1) {
                            break;
                        }
                    }

                }
            }
        }
    }
}

int main(int argc, char **argv) {
    rshell_loop();
    return 0;
}
