#include "base.h"
#include <iostream>

bool has_error = false;

void error(int line, std::string msg)
{
    std::cerr << "Error at line " << line << ": " << msg;
    has_error = true;
}

