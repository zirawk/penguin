#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "Scanner.h"
#include "Token.h"
#include "base.h"


void run(std::string source)
{
    Scanner scanner(source);
    std::vector<Token> tokens = scanner.scan_tokens();

    for (const auto &token : tokens) {
        std::cout << token.str() << '\n';
    }
}

void run_file(std::string path)
{
    std::ifstream ifs(path);
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

