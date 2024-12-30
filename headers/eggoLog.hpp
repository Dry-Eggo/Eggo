#pragma once

#include "token.hpp"
#include <cstdarg>
#include <cstddef>
#include <cstdio>

namespace Logger {

inline void Trace(const char *msg, ...) {
  if (msg == NULL) {
    printf("Null msg pointer\n");
  }

  va_list args;
  va_start(args, msg);

  printf("INFO : ");
  vprintf(msg, args);
  va_end(args);

  printf("\n");
}

inline void Error(Error err) {
  switch (err.type) {
  case errType::ex_Delimiter:
    printf("Error : Expected a delimiter ';'");
    break;
  case errType::ex_Oparen:
    printf("Error : Expected a \"{\"");
    break;
  case errType::ex_Obrace:
    printf("Error : Missing Parenthesis");
    break;
  case errType::ex_Expression:
    printf("Error : Expected an Expression");
    break;
  case errType::ex_Func:
    printf("Error : Undefined Function");
    break;
  case errType::ms_Param:
    printf("Error : Invalid Arguments");
    break;
  case errType::ms_Scope:
    printf("Error : Not Declared in this scope");
    break;
  case ms_Type:
	printf("Error : Specified Return Type does not match return value");
  default:
    break;
  }
  printf(" at line : %d,  column : %d\n", err.line, err.col);
  exit(1);
}

} // namespace Logger
