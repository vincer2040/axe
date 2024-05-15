#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <string>

std::string read_line(const char* prompt) {
    printf("%s", prompt);
    fflush(stdout);
    std::string line;
    std::getline(std::cin, line);
    return line;
}

bool check_errors(const axe::parser& parser) {
    for (auto& err : parser.get_errors()) {
        std::cout << err << '\n';
    }
    return parser.get_errors().size() != 0;
}

void main_loop() {
    while (true) {
        std::string line = read_line(">>> ");
        if (line == "exit") {
            break;
        }
        axe::lexer lexer(line);
        axe::parser parser(lexer);
        auto ast = parser.parse();
        if (check_errors(parser)) {
            continue;
        }
        auto ast_str = ast.string();
        std::cout << ast_str << '\n';
    }
}

int main() {
    main_loop();
    return 0;
}
