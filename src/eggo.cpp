#include "eggoLog.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include <cstdlib>
#include <sstream>

int main (int argc, char *argv[]) {

  if(argc < 3)
  {
    Logger::Error({.type = errType::ex_Expression, .line = 0});
    exit(1);
  }

  Lexer lexer(argv[1]);

  std::stringstream ss;
  ss << "nasm -f elf64 d.asm -o d.o && ld d.o ../Eggo/std/std_h.o -o " << argv[2];
  system(ss.str().c_str());
  
  return 0;
}
