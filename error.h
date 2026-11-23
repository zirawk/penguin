#ifndef ERROR_H
#define ERROR_H

#include <string_view>
#include "Token.h"

extern bool has_error;

void error(int line, std::string_view message);
void error(const Token &token, std::string_view message);

struct RuntimeError {};

#endif // ERROR_H
