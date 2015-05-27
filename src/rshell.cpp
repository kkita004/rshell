#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <limits>
#include <pwd.h>

// getenv()
#include <stdlib.h>

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

// signal()
#include <signal.h>

//PATH_MAX
#include <linux/limits.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

// Globals
//bool isChild = false;
pid_t lastChild = 0;
bool childExists = false;

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


/* Given a string with a command, stores inside a vector
 * the command and its arguments, and also gets the type of connector
 */
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
}


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
        catch (...) { std::cerr << "error: bad input\n"; return false;}

        return true;
}

void change_descriptors(int newfd, int oldfd) {
    if (newfd != oldfd) {
        if (-1 == close(oldfd)) {
            perror("close");
            exit(1);
        }
        if (-1 == dup(newfd)) {
            perror("dup");
            exit(1);
        }
    }
}


void run_command(const char **args, int pin, int pout) {
    childExists = true;
    pid_t pid = fork();
    if (-1 == pid) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        //isChild = true;
        change_descriptors(pin, 0);
        change_descriptors(pout, 1);
        execvp(args[0], (char * const *)args);
        perror("execvp");
        _exit(1);
    }
    lastChild = pid;
}

bool replace(std::string& source, const std::string to_replace, const std::string with) {
    size_t pos = source.find(to_replace);
    if (pos == std::string::npos) return false;
    source.replace(pos, to_replace.size(), with);
    return true;
}

bool change_directory(const char ** args) {
    static char oldp[PATH_MAX];
    static char p[PATH_MAX];
    strcpy(p, getenv("PWD"));
    if (0 == p) {
        std::cerr << "Could not get $PWD";
        return false;
    }

    if (args[1] == '\0') {
        strcpy(oldp, p);
        strcpy(p, getenv("HOME"));
        if (0 == p) {
            std::cerr << "Could not get $HOME\n";
            return false;
        }
    } else if (!strcmp(args[1], "-\0")) {
        strcpy(p, getenv("OLDPWD"));
        if (0 == p) {
            std::cerr << "Could not get $OLDPWD\n";
            return false;
        }
        strcpy(oldp, getenv("PWD"));
        if (0 == oldp) {
            std::cerr << "Could not get $PWD\n";
            return false;
        }
    } else {
        strcpy(oldp, getenv("PWD"));
        if (0 == oldp) {
            std::cerr << "Could not get $PWD\n";
            return false;
        }
        strcpy(p, args[1]);
    }

    if (-1 == chdir(p)) {
        perror("chdir");
        return false;
        //exit(1);
    }

    if (-1 == setenv("OLDPWD", oldp, 1)) {
        perror("setenv");
        exit(1);
    }
    if (-1 == setenv("PWD", p, 1)) {
        perror("setenv");
        exit(1);
    }

    return true;
}



static void sighandle(int signum) {
    switch(signum) {
        case SIGINT:
        if (lastChild != 0 && childExists) {
            //std::cerr << "in child" << std::endl;
            kill(lastChild, SIGINT);
            int status;
            wait(&status);
            if (-1 == status) {
                perror("wait");
                exit(1);
            }
        }
        //std::cerr << "in parent" << std::endl;
        break;
        default:
        break;
    }
}


void rshell_loop () {
    std::string input_s;
    std::cout << std::endl;

    char e[]={"exit"};
    char cd[]={"cd"};
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
        // Get current working directory
        char cwd[PATH_MAX];
        if (0 == getcwd(cwd, PATH_MAX)) {
            perror("getcwd");
            exit(1);
        }

        // Check if path contains HOME
        //std::string cwd_str(cwd);
        char* pHome;
        //std::string home_char "~";
        if (0 == (pHome = getenv("HOME"))) {
            perror("getenv");
            exit(1);
        }

        // path contains HOME
        std::string tcwd(cwd);
        std::string thome(pHome);

        replace(tcwd, thome, "~");

        if ((pwd && pwd->pw_name) || hostname[0] != '\0') {
            printf("%s@%s:%s$ ", login, hostname, tcwd.c_str());
        } else {
            printf("$ ");

        }
        getline(std::cin, input_s);
        // end of ctrl-d
        if (!std::cin.good()) {
            std::cin.clear();
            //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << std::endl;
            continue;
        }
        // Empty input
        boost::trim(input_s);
        if (input_s == "") continue;
        //int pid;

        //unsigned pipecount = 0;

        int pipe_in = 0;
        int pipe_out = 1;

        bool inputOn = false, outputOn = false, pipeOn = false;
        std::string parse;
        std::string parse_nocon;
        const char* args[1000] = {NULL};
        std::vector<std::string> v_args;
        io f;
        // file descriptor holders
        int fin = 0, fout = 1;
        int c = 0;
        // initialize fds
        int fd[2];

        // last prog will be run separately
        bool quit_loop = false;
        std::vector<std::string> v_pipe;

        // first get each chain of commands
        // separate by connectors

        int i = 0;
        while ((unsigned) i < (input_s).size() && i >= 0) {
            // Grab the first set of commands separated by a connector
            parse = parse_string(input_s, &i);
            boost::trim(parse);
            if (parse == "") {
                quit_loop = true;
                break;
            }
            parse_nocon = check_connector(parse, &c);
            // No closing quotes, break
            if (i < 0) {
                quit_loop = true;
                break;
            }
            v_pipe.clear();
            if(!check_piping(parse_nocon, v_pipe)) {
                std::cerr << "error: bad pipe input";
                continue;
            }
            // after unfinished quotes
            for (auto p = v_pipe.begin(); p != v_pipe.end(); ++p) {
                // Check if i/o redirection is necessary
                if (!check_redirect(*p, f)) {
                    quit_loop = true;
                    break;
                }
                // take command to execute, and create vector
                // of arguments
                v_args.clear();
                // discard variable, need args, don't need connectors
                int d;
                parse_args(f.exec, v_args, &d);

                // parse piping
                // break if something goes wrong
                if (f.input != "") inputOn = true;
                if (f.output != "") outputOn = true;
                if (v_pipe.size() > 1) pipeOn = true;
                // Buffer only holds 1000 commands total;
                // Any longer will cause errors
                // create argument array
                for (unsigned j = 0; j < 1000; ++j) args[j] = NULL;

                for (unsigned j = 0; j < v_args.size(); ++j) {
                    const char * p = v_args.at(j).c_str();
                    args[j] = p;
                }
                // if blank command, break
                if (v_args.size() == 0) {
                    quit_loop = true;
                    break;
                }
                // If command is exit
                if (!strcmp(e, args[0])) exit(1);

                if (!strcmp(cd, args[0])) {
                    change_directory(args);
                    break;
                }

                if (inputOn) fin = open(f.input.c_str(), O_RDONLY);
                if (fin == -1) {
                    perror("opening input file");
                    quit_loop = true;
                    break;
                }
                int outputFlags = O_CREAT | O_WRONLY | O_TRUNC;
                if (f.isAppend) outputFlags = O_CREAT | O_WRONLY | O_APPEND;
                if (outputOn) fout = open(f.output.c_str(), outputFlags, 0644);

                if (fout == -1) {
                    perror("opening output file");
                    quit_loop = true;
                    break;
                }

                // if only one command break
                //if (v_pipe.size() == 1) break;
                // exit loop if one command remaining
                if (p == v_pipe.end() - 1) break;

                // create pipes
                if (pipeOn) {
                    if (-1 == pipe(fd)) {
                        perror("pipe");
                        exit(1);
                    }
                }

                //pipe_in = fd[0];

                // original fork
                if (inputOn && !outputOn && pipeOn) {
                    run_command((const char **) args, fin, fd[1]);
                    if (-1 == close(fd[0])) {
                        perror("close");
                        exit(1);
                    }
                } else if (!outputOn && pipeOn)  {
                    run_command((const char **) args, pipe_in, fd[1]);
                } else if (outputOn && pipeOn) {
                    if(-1 == close(fd[0])) {
                        perror("close fd0");
                        exit(1);
                    }
                    if(-1 == close(fd[1])) {
                        perror("close fd1");
                        exit(1);
                    }
                    std::cerr << "error: output redirect before pipe\n";
                    quit_loop = true;
                    break;
                }
                // at this point the process is in the parent

                if (v_pipe.size() > 1) {
                    if (-1 == close(fd[1])) {
                        perror("close");
                        exit(1);
                    }
                    pipe_in = fd[0];
                }
                // closing pipe_in?
            }
            //child successfully changed
            if (quit_loop) break;

            if (inputOn) {
                pipe_in = fin;
            }
            if (outputOn) {
                pipe_out = fout;
            }
            if (!strcmp(cd, args[0])) continue;
            run_command((const char **) args, pipe_in, pipe_out);
            int status = 0;
            for (unsigned k = 0; k < v_pipe.size() - 1; ++k) {
                wait(&status);
                if (-1 == status) {
                    perror("wait");
                    exit(1);
                }
            }
            // final wait will determine what ends
            wait(&status);
            if (-1 == status) {
                perror("wait");
                exit(1);
            } else {
                if (status == 0) {
                    if (c == 2) break;
                } else if (status != 0) {
                    if (c == 1) break;
                }
            }

           childExists = false;
        }
        //if (quit_loop) continue;
        // outside of loop
    }
}

int main(int argc, char **argv) {

    struct sigaction act;
    act.sa_handler = &sighandle;
    sigaction(SIGINT, &act, NULL);
    /*
       if (-1 == signal(SIGINT, handle)) {
       perror("signal");
       exit(1);
       }*/
    //std::cerr << "PID: " << getpid() << std::endl;
    rshell_loop();
    // mian
    return 0;
}
