#include "Scanner.h"

#include "Token.h"
#include "base.h"
#include <string>
#include <array>
#include <variant>
#include <vector>
#include <string_view>

constexpr std::array<std::pair<std::string_view, TokenType>, 16> keywords = {{
    { "and", TokenType::AND },
    { "class", TokenType::CLASS },
    { "else", TokenType::ELSE },
    { "false", TokenType::FALSE },
    { "for", TokenType::FOR },
    { "fun", TokenType::FUN },
    { "if", TokenType::IF },
    { "nil", TokenType::NIL },
    { "or", TokenType::OR },
    { "print", TokenType::PRINT },
    { "return", TokenType::RETURN },
    { "super", TokenType::SUPER },
    { "this", TokenType::THIS },
    { "true", TokenType::TRUE },
    { "var", TokenType::VAR },
    { "while", TokenType::WHILE },
}};

constexpr TokenType find_keyword(const std::string_view sv)
{
    for (const auto e : keywords) {
        if (e.first == sv) return e.second;
    }
    return TokenType::IDENTIFIER;
}

constexpr bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

constexpr bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

constexpr bool is_alnum(char c)
{
    return is_alpha(c) || is_digit(c);
}

Scanner::Scanner(std::string_view src) : source(src), tokens(), start(0), current(0), line(1) {}

bool Scanner::at_end()
{
    return current >= source.length();
}

bool Scanner::match(char c)
{
    if (at_end() || source[current] != c) return false;
    ++current;
    return true;
}

char Scanner::peek()
{
    if (at_end()) return '\0';
    return source[current];
}

char Scanner::peek_next()
{
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

void Scanner::add_token(TokenType type, const LitVal &lit)
{
    tokens.push_back({type, std::string(source.substr(start, current - start)), std::move(lit), line});
}

void Scanner::add_token(TokenType type)
{
    add_token(type, std::monostate{});
}

void Scanner::string()
{
    while (peek() != '"' && !at_end()) {
        if (peek() == '\n') ++line;
        ++current;
    }

    if (at_end()) {
        error(line, "Unterminated string.");
        return;
    }
    // current -> "

    ++current;
    // [start + 1, current - 1) => npos == current - start - 2
    add_token(STRING, std::string(source.substr(start + 1, current - start - 2)));
}

void Scanner::number()
{
    // TODO 精度问题
    double value = source[start] - '0';
    while (is_digit(peek())) value = value * 10 + (source[current++] - '0');
    // 此刻，current停在第一个非数字字符上
    if (peek() == '.' && is_digit(peek_next())) {
        ++current;
        double frac = 0.1;
        while (is_digit(peek())) {
            value += frac * (source[current++] - '0');
            frac *= 0.1;
        }
    }
    add_token(NUMBER, value);
}

void Scanner::identifier()
{
    while (is_alnum(peek())) ++current;
    add_token(find_keyword(source.substr(start, current - start)));
}

void Scanner::scan_token()
{
    char c = source[current++];
    switch (c) {
    case '(': add_token(LEFT_PAREN); break;
    case ')': add_token(RIGHT_PAREN); break;
    case '{': add_token(LEFT_BRACE); break;
    case '}': add_token(RIGHT_BRACE); break;
    case ',': add_token(COMMA); break;
    case '.': add_token(DOT); break;
    case '-': add_token(MINUS); break;
    case '+': add_token(PLUS); break;
    case ';': add_token(SEMICOLON); break;
    case '*': add_token(STAR); break;
    case '!':
        add_token(match('=') ? BANG_EQUAL : BANG);
        break;
    case '=':
        add_token(match('=') ? EQUAL_EQUAL : EQUAL);
        break;
    case '<':
        add_token(match('=') ? LESS_EQUAL : LESS);
        break;
    case '>':
        add_token(match('=') ? GREATER_EQUAL : GREATER);
        break;
    case '/':
        if (match('/')) {
            // 处理注释
            // 关于为什么要把 !at_end() 放在后面：出现情况更少
            while (peek() != '\n' && !at_end()) ++current;
        } else {
            add_token(SLASH);
        }
        break;

    case '\n':
        ++line;
    case ' ':
    case '\r':
    case '\t':
        break;

    case '"': string(); break;

    default:
        if (is_digit(c)) {
            number();
        } else if (is_alpha(c)) {
            identifier();
        } else {
            error(line, "Unexpected character.");
        }
        break;
    }

}

std::vector<Token> Scanner::scan_tokens()
{
    while (!at_end()) {
        start = current;
        scan_token();
    }

    tokens.push_back({END_OF_FILE, "", std::monostate{}, line});
    return std::move(tokens);
}
