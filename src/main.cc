#include "lexer.h"
#include "token.h"
#include <iostream>
#include <string>

std::string read_line(const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);
    std::string line;
    std::getline(std::cin, line);
    return line;
}

void print_tokens(axe::lexer& lexer) {
    while (true) {
        axe::token tok = lexer.next_token();
        std::cout << tok.string() << '\n';
        if (tok.get_type() == axe::token_type::Eof) {
            break;
        }
    }
}

void main_loop() {
    for (;;) {
        std::string line = read_line(">>> ");
        if (line == "exit") {
            break;
        }
        axe::lexer lexer(line);
        print_tokens(lexer);
    }
}

int main() {
    main_loop();
    return 0;
}
