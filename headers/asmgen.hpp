#pragma once

#include "eggoLog.hpp"
#include "token.hpp"
// #include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
// #include <string>
#include <map>
#include <sstream>
// #include <utility>
#include <string>
#include <variant>
#include <vector>

class AsmGen {
public:
  NodeProg m_program;
  std::fstream out_asm;
  std::vector<Var> varStack;
  std::vector<NodeFuncStmt> funcStack;

  // ================== Scopes ======================= //
  std::map<std::string, std::vector<Var>> varScopes;
  std::vector<std::string> scope_stack; // to track current scope

  std::vector<std::string> queue;
  size_t currOffset = 1;
  size_t for_count = 0;
  std::stringstream TEXT, BSS, DATA, FUNC, HEADER, MAIN;

  inline void push(Var v) {
    TEXT << "\n\tmov rax, " << v.value.value.value() << "\n";
    TEXT << "\tpush rax\n";
    v.stackOffset = currOffset++;
  }

  inline void pop(Var v, const char *reg) {
    if (currOffset - 1 != 0 && v.value.type != TokenType::STRING_LIT)
      TEXT << "\n\tmov " << reg << ", [rbp - " << 8 * v.stackOffset << "]\n";
    else
      TEXT << "\n\tmov " << reg << ", " << v.name.value.value() << "\n";
  }

  inline AsmGen(NodeProg program) : m_program(program) {

    varScopes["main"] = varStack;
    scope_stack.push_back("main");

    BSS << "\nsection .bss\n";
    DATA << "\nsection .data\n";

    out_asm.open("d.asm", std::ios::out);

    HEADER << "\nsection .text\n";
    HEADER << "global _start\n";
    HEADER << "\nextern std_terminate_process\nextern std_print_string\nextern "
              "std_flush\nextern std_print_int\nextern std_copy\n";
    HEADER << "\n_start:\n";
    HEADER << "\n\n\tmov rbp, rsp\n\tcall main\n";

    Logger::Info("Generating Asm");
    Logger::Info("Program Size : %d", m_program.stmt.size());

    generate(m_program.stmt, TEXT, TEXT, varStack);

    bool is_good = false;

    for (auto f : funcStack) {
      if (f.identifier.value.value() == "main") {
        is_good = true;
        break;
      }
    }

    if (!is_good) {
      printf("\nmain function not present\n");
      exit(1);
    }

    out_asm << BSS.str();
    out_asm << DATA.str();
    out_asm << HEADER.str();
    out_asm << TEXT.str();

    for (auto &p : queue) {
      FUNC << p;
    }

    out_asm << FUNC.str();
    out_asm << MAIN.str();
    out_asm << "\n\tmov rdi, 0\n\tcall std_terminate_process\n";
    out_asm << "\nsection .note.GNU-stack";
    printf("DONE\n");
  }

  inline void changeScope(std::string name) {
    scope_stack.push_back(name);
    if (varScopes.find(name) == varScopes.end()) {
      varScopes[name] = {};
    }
  }

  inline void exitScope() {
    if (scope_stack.empty())
      return;

    std::string cur_scope = scope_stack.back();
    scope_stack.pop_back();

    varScopes.erase(cur_scope);
  }

  inline void generate(std::vector<NodeStmts> stmts, std::stringstream &n_s,
                       std::stringstream &p_s, std::vector<Var> &curScope) {
    for (auto stmt : stmts) {
      struct stmtVisitor {
        AsmGen *gen;
        std::vector<Var> *curScope = &gen->varScopes[gen->scope_stack.back()];
        std::stringstream *c_ss;
        std::stringstream *p_ss;

        void operator()(NodeExitStmt e) {
          struct exprVisitor {
            AsmGen *gen;
            void operator()(NodeInt i) {
              gen->TEXT << "\n\tmov rdi, " << i.value.value.value() << "\n";
              gen->TEXT << "\tcall std_terminate_process\n";
            }
            void operator()(Token i) {
              for (auto v : gen->varStack) {
                if (i.type == TokenType::IDENT &&
                    i.value.value() == v.name.value.value()) {
                  gen->pop(v, "rdi");
                  gen->TEXT << "\tcall std_terminate_process\n";
                }
              }
            }
          };
          std::visit(exprVisitor{.gen = gen}, e.expr.var);

          Logger::Info("Generating Exit statement");
        }

        void operator()(NodeMkStmt m) {
          Logger::Info("Generating Make statement");
          Var v = {.name = m.identifier,
                   .stackOffset = gen->currOffset,
                   .value = m.value};

          if (m.value.type == TokenType::STRING_LIT) {
            gen->DATA << "\n\t" << m.identifier.value.value() << " db "
                      << m.value.value.value() << ", 0\n";
          } else if (m.value.type == TokenType::INT_LIT) {
            gen->DATA << "\n\t" << m.identifier.value.value() << " dq "
                      << m.value.value.value() << "\n";
          } else if (m.value.type == IDENT) {
            Logger::Info("Passing Ident to make");
            bool found = false;

            for (auto v : gen->varScopes[gen->scope_stack.back()]) {
              if (v.name.value.value() == m.value.value.value()) {
                gen->DATA << "\n\t" << m.identifier.value.value() << " db "
                          << v.value.value.value() << "\n";
                found = true;
              }
            }
            if (!found) {
              Logger::Error({.type = ex_Expression, .line = m.identifier.line});
            }
          } else if (m.value.value.value() == "undef") {
            if (m.type == DataType::STR)
              gen->BSS << "\n\t" << m.identifier.value.value()
                       << " resb 1024\n";
            else if (m.type == DataType::INT) {
              gen->BSS << "\n\t" << m.identifier.value.value() << " resb 1\n";
            }
          } else
            gen->push(v);
          printf(" -- %s\n", gen->scope_stack.back().c_str());

          gen->varScopes[gen->scope_stack.back()].push_back(v);
          for (auto v : gen->varScopes[gen->scope_stack.back()]) {
            printf(" :: %s\n", v.name.value.value().c_str());
          }
        }

        void operator()(NodeReStmt r) {
          bool found = false;
          Logger::Info("Generate Re_Assign statement");
          for (auto v : gen->varScopes[gen->scope_stack.back()]) {
            if (v.name.value.value() == r.identifier.value.value()) {
              found = true;
              *p_ss << "\n\tpush rax";
              *p_ss << "\n\tmov rax, [" << r.new_value.value.value() << "]";
              *p_ss << "\n\tmov [" << v.name.value.value() << "], rax";
              *p_ss << "\n\tpop rax\n";
            }
            if (found == false) {
              Logger::Error({.type = ms_Scope, .line = r.identifier.line});
            }
          }
        }

        void operator()(NodeForStmt f) {
          Logger::Info("Generating For statement");

          Logger::Info("For Body size : %d", f.body.size());

          std::vector<Var> scope;

          *p_ss << "\n\tmov rcx, " << f.startValue.value.value() << "\n";
          *p_ss << "for" << gen->for_count << ":\n";
          *p_ss << "\tcmp rcx, " << f.targetValue.value.value() << "\n";
          *p_ss << "\tjge for" << gen->for_count << "_end\n";
          *p_ss << "\tadd rcx, " << f.incValue.value.value() << "\n";
          gen->generate(f.body, gen->MAIN, *p_ss, scope);
          *p_ss << "\tjmp for" << gen->for_count << "\n";
          *p_ss << "\nfor" << gen->for_count << "_end:\n";
          gen->for_count++;

          gen->varScopes[f.identifier.value.value()] = scope;
        }

        void operator()(NodeFuncStmt f) {
          Logger::Info("Generating Function");
          Logger::Info("Function params : %d", f.param_count);

          std::vector<Var> scope;
          std::stringstream temp;
          std::stringstream nest;
          gen->scope_stack.push_back(f.identifier.value.value());
          gen->changeScope(f.identifier.value.value());

          size_t scope_offset = 1; // cover the memory of the current scope;

          gen->funcStack.push_back(f);
          temp << "\n" << f.identifier.value.value() << ":\n";
          temp << "\n\tmov rbp, rsp\n";

          for (int i = 0; i < f.param_count; i++) {
            switch (i) {
            case 0:
              temp << "\n\tmov [rbp -8], rdi\n";
              scope.push_back({.name = f.params.at(i).identifier,
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              break;
            case 1:
              scope.push_back({.name = f.params.at(i).identifier,
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              temp << "\n\tmov [rbp -16], rsi\n";
              break;
            case 2:
              scope.push_back({.name = f.params.at(i).identifier,
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              temp << "\n\tmov [rbp -24], rdx\n";
            default:
              break;
            }
          }
          gen->generate(f.body, gen->MAIN, temp,
                        scope); // stream for child statements

          // gen->queue.emplace_back(temp.str());
          // gen->queue.emplace_back(nest.str());
          if (f.identifier.value.value() == "main") {
            gen->MAIN << temp.str();
          } else {
            temp << "\tret\n";
            gen->FUNC << temp.str();
          }

          gen->varScopes[f.identifier.value.value()] = scope;
        }

        void operator()(NodeFuncCall fc) {
          /* gen->changeScope(fc.identifier.value.value()); */
          bool found = false;
          Logger::Info("Generating Function call");
          Logger::Info("Function call Params : %d", fc.param_count);
          printf(" -- %s\n", gen->scope_stack.back().c_str());
          for (auto f : gen->funcStack) {

            if (f.identifier.value.value() == fc.identifier.value.value()) {
              if (f.param_count != fc.param_count)
                Logger::Error({.type = ms_Param, .line = fc.identifier.line});
              found = true;
              if (f.params.size() != 0) {
                for (int i = 0; i < fc.params.size(); i++) {
                  bool found = false;
                  for (auto v : gen->varScopes[gen->scope_stack.back()]) {
                    if (fc.params.at(i).value.value.has_value())
                      printf("%s\n",
                             fc.params.at(i).value.value.value().c_str());
                    printf(" :: %s\n", v.name.value.value().c_str());
                    if (fc.params.at(i).value.value.has_value() &&
                        fc.params.at(i).value.value.value() ==
                            v.name.value.value()) {
                      found = true;
                      break;
                    }
                  }
                  if (fc.params.at(i).value.value.has_value() &&
                      found == false) {
                    printf(" %s Not found in currentScope\n",
                           fc.params.at(i).value.value.value().c_str());
                  }
                  if (i == 0) {
                    *p_ss << "\n\tmov rdi, "
                          << fc.params.at(i).value.value.value() << "\n";
                  } else if (i == 1)
                    *p_ss << "\n\tmov rsi, "
                          << fc.params.at(i).value.value.value() << "\n";
                  else
                    *p_ss << "\n\tpush " << fc.params.at(i).value.value.value()
                          << "\n";
                }
              }
              *p_ss << "\n\tcall " << fc.identifier.value.value() << "\n";
              // gen->TEXT << "_" << fc.identifier.value.value() << ":\n";
            }
          }
          if (found == false) {
            Logger::Error(
                {.type = errType::ex_Func, .line = fc.identifier.line});
          }
        }

        void operator()(NodeCallStmt c) {
          Logger::Info("Generating Call Stmt");
          for (int i = 0; i < c.params.size(); i++) {
            switch (i) {
            case 0:
              *p_ss << "\n\tmov rdi, " << c.params.at(i).value.value.value();
              break;
            case 1:
              *p_ss << "\n\tmov rsi, " << c.params.at(i).value.value.value();
              break;
            case 2:
              *p_ss << "\n\tmov rdx, " << c.params.at(i).value.value.value();
              break;
            default:
              *p_ss << "\n\tpush " << c.params.at(i).value.value.value();
              break;
            }
          }
          *p_ss << "\n\tcall " << c.std_lib_value.value.value() << "\n";
        }
        void operator()(NodeExtrnStmt e) {
          gen->HEADER << "\n\textern " << e.identifier.value.value() << "\n";
        }
      };

      std::visit(stmtVisitor{this, &curScope, &n_s, &p_s}, stmt.var);
      /* Logger::Info("End of Generating Body"); */
    }
  }
  inline ~AsmGen() {}
};
