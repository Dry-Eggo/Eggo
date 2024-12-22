#pragma once

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include "token.hpp"

namespace Logger {

  inline void Info(const char* msg, ...)
  {
    if(msg == NULL)
    {
      printf("Null msg pointer\n");
    }

    va_list args;
    va_start(args, msg);
    
    printf("INFO : ");
    vprintf(msg, args); 
    va_end(args);

    printf("\n");
  }

  inline void Error(Error err)
  {
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
      default:
        break;
    }
    printf(" at line : %d\n", err.line);
    exit(1);
  }

}
