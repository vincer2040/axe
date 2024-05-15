#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include <iostream>
#include <string>
#include <unistd.h>

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
        axe::compiler compiler;
        auto err = compiler.compile(ast);
        if (err.has_value()) {
            std::cout << *err << '\n';
            continue;
        }
        axe::vm vm(compiler.get_byte_code());
        err = vm.run();
        if (err.has_value()) {
            std::cout << *err << '\n';
            continue;
        }

        auto stack_top = vm.last_popped_stack_element();
        auto str = stack_top.string();
        std::cout << str << '\n';
    }
}

int main() {
    main_loop();
    return 0;
}
