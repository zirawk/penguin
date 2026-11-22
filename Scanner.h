#ifndef SCANNER_H
#define SCANNER_H

#include <string_view>
#include <vector>
#include "Token.h"

class Scanner {
    using enum TokenType;
private:
    const std::string_view source;
    std::vector<Token> tokens;
    int start;
    int current;
    int line;

    bool at_end();
    bool match(char c);
    char peek();
    char peek_next();
    void scan_token();
    void add_token(TokenType type, const LitVal &lit);
    void add_token(TokenType type);
    void string();
    void number();
    void identifier();
public:
    Scanner(std::string_view src);
    ~Scanner() = default;
    std::vector<Token> scan_tokens();
};

#endif // SCANNER_H
