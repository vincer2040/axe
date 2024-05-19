#include "argparse.hpp"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("axec");

    program.add_argument("file").required().help("the file to compile");

    program.add_argument("-o", "--output")
        .required()
        .help("specifiy the output file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        std::cerr << program;
        exit(1);
    }

    auto file = program.get<std::string>("file");
    auto out = program.get<std::string>("-o");

    std::cout << file << '\n' << out << '\n';
    return 0;
}
