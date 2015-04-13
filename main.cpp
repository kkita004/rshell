#include <iostream>
#include <unistd.h>


void rshell_loop () {
    std::cout << std::endl;
    std::string input;

    do {
        std::cout << "$ ";
        std::cin >> input;
        execvp(input.c_str());
    } while (input != "exit");
    return;
}

int main(int argc, char **argv) {
    rshell_loop();
    return 0;
}
