#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "Scanner.h"
#include "Parser.h"
#include "Interpreter.h"
#include "Token.h"
#include "error.h"
#include "AstPrinter.h"

namespace {
Interpreter interpreter;

void run(std::string source)
{
    Scanner scanner(source);
    std::vector<Token> tokens = scanner.scan_tokens();

    Parser parser(tokens);
    auto stmts = parser.parse();

    if (has_error) return;

    interpreter.interpret(std::move(stmts));

    //AstPrinter printer;
    //std::cout << printer.print(*expr) << '\n';
}

void run_file(std::string path)
{
    std::ifstream ifs(path);
    if (!ifs) {
        std::cerr << "Cannot open file: " << path << '\n';
        exit(1);
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    run(ss.str());
    if (has_error) exit(1);
}

void run_prompt()
{
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!getline(std::cin, line)) break;
        run(line);
        has_error = false;
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [script]\n";
        return 1;
    } else if (argc == 2) {
        run_file(argv[1]);
    } else {
        run_prompt();
    }

    return 0;
}

