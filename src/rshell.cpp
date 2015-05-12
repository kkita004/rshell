#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <pwd.h>

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

std::string check_connector(const std::string s, int* con) {
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



void parse_args(const std::string s, std::vector<std::string>& v, int * connector) {
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

std::string parse_string(const std::string s, int* index) {
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


/* Takes string and splits based on piping*/
void check_piping(const std::string s, std::vector<std::string>& v) {
    //boost::char_separator<char> delim("|");
    boost::tokenizer<boost::escaped_list_separator<char> > tok(
            s, boost::escaped_list_separator<char>('\\', '|', '\"'));
    for (std::string t : tok) {
        boost::trim(t);
        v.push_back(t);
    }
    //for (std::string s : v) {
    //    std::cout << s << std::endl;
    //}
}

// Searches for character but not in quotes
unsigned find_without_quotes(const std::string s, const char c, unsigned index = -1) {
    for (index++; index < s.size(); ++index) {
        if (s.at(index) == c) return index;
        if (s.at(index) == '\"') {
            while (index < s.size()) {
                if (s.at(index) == '\"') break;
                index++;
            }
        }
        else if (s.at(index) == '\'') {
            while (index < s.size()) {
                if (s.at(index) == '\'') break;
                index++;
            }
        }
        if (index >= s.size()) return s.size();
    }
    // Searched entire string, did not find
    return index;
}


bool separate_by_char_without_quotes(const std::string s, const char c,
    std::vector<std::string>& v) {
    boost::tokenizer<boost::escaped_list_separator<char> > tok(
            s, boost::escaped_list_separator<char>('\\', c, '\"'));
    for (std::string t : tok) v.push_back(t);
    // Multiple redirection characters found
    // Return error
    if (v.size() > 2) return false;
    return true;
}



/* Takes string and creates pairs of input/output
 *
 */
//void check_redirect(const std::string s,
//        std::vector<std::pair<std::string, std::pair<std::string, std::string> > >& v) {
bool check_redirect(const std::string s,
                        std::string& exec,
                        std::string& input,
                        std::string& output) {
    // holds entries
    std::vector<std::string> temp;
    // keep delimiters
    // TODO: fix quotation mark parsing

    // separate by input
    //boost::char_separator<char> delim("", "<>");

    // Check if characters exist

    bool inputExists, outputExists;
    bool inputFirst;
    unsigned inputIndex = find_without_quotes(s, '<');
    unsigned outputIndex = find_without_quotes(s, '>');

    if (inputIndex == s.size()) inputExists = false;
    else inputExists = true;
    if (outputIndex == s.size()) outputExists = false;
    else outputExists = true;

    if (inputIndex < outputIndex) inputFirst = true;

    std::vector<std::string> v;
    if (inputExists && !outputExists) {
        if(!separate_by_char_without_quotes(s, '<', v)) return false;
        exec = v.at(0);
        boost::trim(exec);
        input = v.at(1);
        boost::trim(input);
    } else if (!inputExists && outputExists) {
        if(!separate_by_char_without_quotes(s, '>', v)) return false;
        exec = v.at(0);
        boost::trim(exec);
        output = v.at(1);
        boost::trim(output);
    } else if (inputExists && outputExists) {
        if (inputFirst) {
            if(!separate_by_char_without_quotes(s, '<', v)) return false;
            exec = v.at(0);
            boost::trim(exec);
            std::vector<std::string> v2;
            if(!separate_by_char_without_quotes(v.at(1), '>', v2)) return false;
            input = v2.at(0);
            output = v2.at(1);
        }
    }
    return true;
    /*
    std::vector<std::string> combo;
    for (unsigned i = 0; i < temp.size(); ++i) {
        if (temp.at(i).at(0) == '\"') {
            unsigned j = i++;
            std::string combostring = "";
            while (j < temp.size()) {
                j++;
                combostring.append(temp.at(j));
                if (temp.at(j).at(temp.at(j).size() - 1) == '\"');
                    break;
            }
            i = j;
            combo.push_back(combostring);
        }
        combo.push_back(temp.at(i));
    }

    for (std::string t : combo) {
        std::cout << t << std::endl;
    }


    for (unsigned i = 0; i < temp.size(); ++i) {
        if (temp.at(i) == "<") {
            // input
            std::cout << "input" << std::endl;

        } else if (temp.at(i) == ">") {
            // output
            std::cout << "output" << std::endl;

        }
    }*/
}



void rshell_loop () {
    std::string input_s;
    std::cout << std::endl;

    char e[] = {"exit"};
    std::string prompt;

    // Setup prompt
    // Login and hostname limit is 256, otherwise truncated
    char *login;
    struct passwd *pwd;
    if (!(pwd = getpwuid(getuid()))) {
        perror("getlogin error");
    } else {
        login = pwd->pw_name;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof hostname)) {
        perror("gethostname error");
        hostname[0] = '\0';
    }

    while(1) {
        if ((pwd && pwd->pw_name) || hostname[0] != '\0') {
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
    //std::vector<std::string> v;
    std::string s;
    std::getline(std::cin, s);
    //check_piping(s, v);
    //std::vector<std::pair<std::string, std::pair<std::string, std::string> > > v;
    //std::vector<std::string> v;
    std::string input, output, exec;
    check_redirect(s, exec, input, output);
    std::cout << "exec: " << exec << std::endl;
    std::cout << "input: " << input << std::endl;
    std::cout << "output: " << output << std::endl;
    std::cout << s << std::endl;
    //rshell_loop();

    return 0;
}
