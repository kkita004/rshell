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
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

// perror()
// strtok()
#include <stdio.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

struct io {
    io() {}
    io(std::string e, std::string i, std::string o) {
        exec = e;
        input = i;
        output = o;
    }
    std::string exec;
    std::string input;
    std::string output;
    bool isAppend = false;
};

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


// takes string, returns vector of commands separated
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
bool check_piping(const std::string s, std::vector<std::string>& v) {
    //unsigned j = 0;
    //bool unfinishedQuotes = false;
    bool doubleQuote = false, singleQuote = false;
    unsigned j = 0;
    for (unsigned i = 0; i < s.size(); ++i) {
        if (s.at(i) == '\"' && !singleQuote) doubleQuote = !doubleQuote;
        if (s.at(i) == '\'' && !doubleQuote) singleQuote = !singleQuote;

        if (!doubleQuote && !singleQuote) {
            if (s.at(i) == '|') {
                if (i + 1 < s.size() && s.at(i+1) != '|') {
                    v.push_back(s.substr(j,i - j));
                    j = i + 1;
                } else if (i + 1 >= s.size()) {
                    // string ended with a pipe to nowhere, return error
                    return false;
                } else i++;
            } else if (i == s.size() - 1) {
                v.push_back(s.substr(j));
            }
            // If j == 0, then there was no piping character
            //if (j == 0 && i == s.size() - 1) v.push_back(s);
        }
    }

    // check if quotes ended, otherwise it is error
    return (!doubleQuote && !singleQuote);


    /* boost::tokenizer<boost::escaped_list_separator<char> > t(
       s,
       boost::escaped_list_separator<char>("\\", "|", "\"\'"));
       for (
       boost::tokenizer<boost::escaped_list_separator<char> >::iterator i
       = t.begin();
       i != t.end(); ++i) {
       std::string temp = *i;
    // Remove white space and tabs
    boost::trim(temp);
    boost::trim_if(temp, boost::is_any_of("\t"));
    if (temp != "") v.push_back(temp);
    }*/

    /*
       char * str = new char[s.size()];
       strcpy(str, s.c_str());
       char * pch;
       pch = strtok(str, "|");
       std::vector<std::string> temp;
       while (pch != NULL) {
       std::string t(pch);
       boost::trim(t);
       temp.push_back(t);
       pch = strtok(NULL, "|");
       }

       v = temp;
       if(str) delete[] str;

*/

    /*for (unsigned i = 0; i < s.size(); ++i) {
      if (s.at(i) == '\"') {
      while (i < s.size()) {
      if (i == s.size()) unfinishedQuotes = true;
      i++;
      }
      } else if (s.at(i) == '\'') {
      while (i < s.size()) {
      if ( i == s.size()) unfinishedQuotes = true;
      i++;
      }
      } else if (s.at(i) == '|')  {
      v.push_back(s.substr(j,i - j));
      j = i + 1;
      }
      }*/
    /*
       if (unfinishedQuotes) return false;
       return true;*/
}


//boost::char_separator<char> delim("|");
/*boost::tokenizer<boost::escaped_list_separator<char> > tok(
  s, boost::escaped_list_separator<char>("\\", "|", "\'\""));
  for (std::string t : tok) {
  boost::trim(t);
  v.push_back(t);
  }*/
//for (std::string s : v) {
//    std::cout << s << std::endl;
//}


// Searches for character but not in quotes
unsigned find_without_quotes(const std::string s, const char c, unsigned index = -1) {
    for (index++; index < s.size(); ++index) {
        if (s.at(index) == '\"') {
            while (index + 1< s.size()) {
                index++;
                if (s.at(index) == '\"') break;
            }
        }
        else if (s.at(index) == '\'') {
            while (index + 1 < s.size()) {
                index++;
                if (s.at(index) == '\'') break;
            }
        }
        if (s.at(index) == c) return index;
        if (index >= s.size()) return s.size();
    }
    // Searched entire string, did not find
    return index;
}

// Finds a substring, can probably replace above function
unsigned find_without_quotes(const std::string s, const std::string c, unsigned index = -1) {
    for (index++; index < s.size(); ++index) {
        if (s.at(index) == '\"') {
            while (index + 1 < s.size()) {
                index++;
                if (s.at(index) == '\"') break;
            }
        }
        else if (s.at(index) == '\'') {
            while (index + 1 < s.size()) {
                index++;
                if (s.at(index) == '\'') break;
            }
        }
        if (index >= s.size()) return s.size();
        if (s.substr(index, c.size()) == c) return index;
    }
    // Searched entire string, did not find
    return index;
}


bool separate_by_char_without_quotes(const std::string s, const char c,
        std::vector<std::string>& v) {
    boost::tokenizer<boost::escaped_list_separator<char> > tok(
            s, boost::escaped_list_separator<char>('\\', c, '\"'));
    for (std::string t : tok) if (t != "") v.push_back(t);
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
bool check_redirect(const std::string s, io& f) {
    // holds entries
    std::vector<std::string> temp;
    // keep delimiters
    // TODO: fix quotation mark parsing

    // separate by input
    //boost::char_separator<char> delim("", "<>");

    // Check if characters exist

    bool inputExists, outputExists, inputFirst, append;
    unsigned inputIndex = find_without_quotes(s, '<');
    unsigned outputIndex = find_without_quotes(s, '>');
    unsigned appendIndex = find_without_quotes(s, ">>");

    if (inputIndex == s.size()) inputExists = false;
    else inputExists = true;
    if (outputIndex == s.size()) outputExists = false;
    else outputExists = true;
    if (appendIndex < s.size()) {
        append = true;
        outputExists = true;
        outputIndex = appendIndex;
    } else append = false;

    if (inputIndex < outputIndex) inputFirst = true;
    else inputFirst = false;

    try {
        std::vector<std::string> v;
        if (inputExists && !outputExists) {
            if(!separate_by_char_without_quotes(s, '<', v)) return false;
            f.exec = v.at(0);
            //boost::trim(exec);
            f.input= v.at(1);
            //boost::trim(input);
        } else if (!inputExists && outputExists) {
            if(!separate_by_char_without_quotes(s, '>', v)) return false;
            f.exec = v.at(0);
            //boost::trim(exec);
            f.output = v.at(1);
            //boost::trim(output);
        } else if (inputExists && outputExists) {
            if (inputFirst) {
                if(!separate_by_char_without_quotes(s, '<', v)) return false;
                f.exec = v.at(0);
                //boost::trim(exec);
                std::vector<std::string> v2;
                if(!separate_by_char_without_quotes(v.at(1), '>', v2)) return false;
                f.input = v2.at(0);
                f.output = v2.at(1);
            } else {
                if(!separate_by_char_without_quotes(s, '>', v)) return false;
                f.exec = v.at(0);
                //boost::trim(exec);
                std::vector<std::string> v2;
                if(!separate_by_char_without_quotes(v.at(1), '<', v2)) return false;
                f.output = v2.at(0);
                f.input = v2.at(1);
            }
        } else {
            // no redirection found, set string as executable
            f.exec = s;
        }

        boost::trim(f.exec);
        boost::trim(f.input);
        boost::trim(f.output);
        f.isAppend = append;}
        catch (...) { std::cerr << "bad input\n"; return false;}

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
void change_descriptors(int newfd, int oldfd) {
    // do nothing if same
    if (newfd != oldfd) {
        if(-1 == close(oldfd)) {
            perror("close");
            exit(1);
        }
        if(-1 == dup(newfd)) {
            perror("dup");
            exit(1);
        }
    }
}

void run_command(const char **args, int in, int out) {
    pid_t pid= fork();
    if (-1 == pid) {
        perror("fork");
        exit(1);
    } else if (0 == pid) {
        change_descriptors(in, 0);
        change_descriptors(out, 1);
        execvp(args[0], (char * const *) args);
        perror("execvp");
        _exit(1);
    }
    // if we get here we're in the parent
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
        // end of ctrl-d
        if (!std::cin.good()) input_s = "exit";
        // Empty input
        boost::trim(input_s);
        if (input_s == "") continue;
        //int pid;

        std::vector<std::string> v_pipe, v_args;
        bool inputOn = false, outputOn = false;
        io f;
        std::string parse;
        // Buffer only holds 1000 commands total;
        // Any longer will cause errors
        const char* args[1000] = { NULL };
        // file descriptor holders
        // set to default
        int in = 0, out = 1;

        if (!check_piping(input_s, v_pipe)) {
            std::cerr << "bad pipe input\n";
            continue;
        }
        for (auto p = v_pipe.begin(); p != v_pipe.end(); ++p) {
            int i = 0;
            bool quit_loop = false;
            while ((unsigned) i < (*p).size() && i >= 0) {
            v_args.clear();
                // parse piping
                // break if something goes wrong
                int c = 0;

                //std::cout << "p: [" << *p <<  "]" << std::endl;
                //std::cout << "parsing string" << std::endl;

                // Check if i/o redirection is necessary
                check_redirect(*p, f);
                if (f.input != "") inputOn = true;
                if (f.output != "") outputOn = true;

                parse = parse_string(f.exec, &i);
                // No closing quotes, break
                if (i < 0) {
                    quit_loop = true;
                    break;
                }
                //std::cout << "parsing args" << std::endl;
                parse_args(parse, v_args, &c);

                // if blank command, break
                if (v_args.size() == 0) {
                    quit_loop = true;
                    break;
                }

                if (inputOn) in = open(f.input.c_str(), O_RDONLY);
                if (in == -1) {
                    perror("input file open");
                    quit_loop = true;
                    break;
                }
                if (outputOn) out = open(f.output.c_str(),
                        O_CREAT | O_WRONLY | O_TRUNC, 0644);

                if (out == -1) {
                    perror("output file open");
                    quit_loop = true;
                    break;
                }

                // initialize argument array
                for (unsigned j = 0; j < 1000; ++j) args[j] = NULL;

                for (unsigned j = 0; j < v_args.size(); ++j) {
                    const char * p = v_args.at(j).c_str();
                    args[j] = p;
                }

                // If command is exit
                if (!strcmp(e, args[0])) {
                    exit(1);
                }

                //pid = fork();
                // Something went wrong
                /*if (pid == -1) {
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
                }*/
                run_command(args, in, out);
                // Parent Process

                //run_command((const char **) args, in, out);
                //else if (pid > 0) {
                //    wait(0) till child process finishes
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

                    //}
                }
            }
            if (quit_loop) break;
        }
    }
}

int main(int argc, char **argv) {
    rshell_loop();
    return 0;
}
