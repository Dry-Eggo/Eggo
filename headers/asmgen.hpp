#pragma once

#include "eggoLog.hpp"
#include "token.hpp"
// #include <algorithm>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <ios>
#include <iostream>
// #include <string>
#include <map>
#include <memory>
#include <ostream>
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
  bool main_ret = false;

  // ================== Scopes ======================= //
  std::map<std::string, std::vector<Var>> varScopes;
  std::vector<std::string> scope_stack; // to track current scope

  std::vector<std::string> queue;
  size_t currOffset = 1;
  size_t loop_count = 0;
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
              "std_flush\nextern std_print_int\nextern std_copy\nextern "
              "std_clear_string\n";
    HEADER << "\n_start:\n";
    HEADER << "\n\n\tmov rbp, rsp\n\tcall main\n";

    Logger::Trace("Generating Asm");
    Logger::Trace("Program Size : %d", m_program.stmt.size());

    generate(m_program.stmt, TEXT, TEXT, varStack);

    bool is_good = false;

    for (auto f : funcStack) {
      if (f.identifier.value.value() == "main") {
        is_good = true;
        break;
      }
    }

    if (!is_good) {
      printf("\nFatal : Unable to identify main entry point with label : "
             "main\nterminated with failure\n");
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
    if (!main_ret)
      out_asm << "\n\tmov rdi, 0";

    out_asm << "\n\tcall std_terminate_process\n";
    out_asm << "\nsection .note.GNU-stack";
    printf("DONE\n");
  }

  inline void changeScope(std::string name) {
    std::cout << "Changin Scope to : " << name << std::endl;
    scope_stack.push_back(name);
    if (varScopes.find(name) == varScopes.end()) {
      varScopes[name] = {};
    }
  }

  inline void exitScope() {
    if (scope_stack.empty())
      return;

    std::cout << "Scopes size : " << scope_stack.size() << std::endl;
    std::string cur_scope = scope_stack.back();
    scope_stack.erase(
        std::remove(scope_stack.begin(), scope_stack.end(), scope_stack.back()),
        scope_stack.end());
    std::cout << "Exiting : " << cur_scope << std::endl;
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
          /* Out of commision for Now */

          /* Replaced With std_terminate_process */

          /*   void operator()(NodeInt i) { */
          /*     gen->TEXT << "\n\tmov rdi, " << i.value.value.value() << "\n";
           */
          /*     gen->TEXT << "\tcall std_terminate_process\n"; */
          /*   } */
          /*   void operator()(NodeBinaryExpr i) {} */
          /* }; */
          /* std::visit(exprVisitor{.gen = gen}, e.expr.var); */

          /* Logger::Info("Generating Exit statement"); */
        }

        void operator()(NodeMkStmt m) {
          Logger::Trace("Generating Make statement");
          Logger::Trace("from Make : current Scope : %s",
                        gen->scope_stack.back().c_str());
          struct mkVisitor {
            Token *ident;
            AsmGen *gen;
            std::stringstream *p_ss;
            NodeMkStmt pm;
            std::string var_name =
                gen->scope_stack.back() + "." + pm.identifier.value.value();

            void operator()(NodeString s) {
              gen->DATA << "\n\t" << var_name << " db " << s.value.value.value()
                        << " ,0";
              Var v;
              v.name = {.value = var_name};
              v.value = {.value = s.value.value.value()};
              gen->varScopes[gen->scope_stack.back()].push_back(v);
            }

            void operator()(NodeInt i) { Logger::Trace("FOunc make int"); }
            void
            operator()(const std::shared_ptr<std::vector<NodeBinaryExpr>> &b) {
              Logger::Trace("expression time");
              std::vector<std::string> expression;
              Logger::Trace("SSize : %d", b->size());
              for (int i = 0; i < b->size(); i++) {
                expression.push_back(b->at(i).lhs.value.value.value());
                expression.push_back(b->at(i).op);
                if (b->at(i).rhs.value.value.has_value()) {
                  expression.push_back(b->at(i).rhs.value.value.value());
                }
              }
              printf("--!");
              expression.erase(
                  std::remove(expression.begin(), expression.end(), ""),
                  expression.end());

              Logger::Trace("expression size : %ld", expression.size());
              for (int i = 0; i < expression.size(); i++) {
                Logger::Trace("%s", expression.at(i).c_str());
              }
            }
            void operator()(const std::shared_ptr<NodeFuncCall> &s) {
              gen->BSS << "\n\t" << var_name << " resb 1024";
              Logger::Trace("Found call : make ");
              std::vector<NodeStmts> stmt;
              stmt.push_back({.var = *s});
              gen->generate(stmt, *p_ss, *p_ss,
                            gen->varScopes[gen->scope_stack.back()]);
              *p_ss << "\n\tmov rdi, rax";
              *p_ss << "\n\tmov rsi, " << var_name;
              *p_ss << "\n\tcall std_copy\n";
              gen->varScopes[gen->scope_stack.back()].push_back(
                  {.name = {.value = var_name}, .value = {.value = "rax"}});
            }
            void operator()(NodeCmp &c) {}
          };

          std::visit(mkVisitor{&m.identifier, gen, p_ss, m}, m.value->var);
          printf("done\n");
        }

        void operator()(NodeReStmt r) {
          bool found = false;
          Logger::Trace("Generate Re_Assign statement");

          // Out of Commision for now
          // Changed Some Things. Requires Editing

          /*for (auto v : gen->varScopes[gen->scope_stack.back()]) {*/
          /*  if (v.name.value.value() == r.identifier.value.value()) {*/
          /*    found = true;*/
          /*    *p_ss << "\n\tpush rax";*/
          /*    *p_ss << "\n\tmov rax, [" << r.new_value.value.value() <<
           * "]";*/
          /*    *p_ss << "\n\tmov [" << v.name.value.value() << "], rax";*/
          /*    *p_ss << "\n\tpop rax\n";*/
          /*  }*/
          /*  if (found == false) {*/
          /*    Logger::Error({.type = ms_Scope, .line =
           * r.identifier.line});*/
          /*  }*/
          /*}*/
        }

        void operator()(NodeForStmt f) {
          Logger::Trace("Generating For statement");

          Logger::Trace("For Body size : %d", f.body.size());

          std::vector<Var> scope;
          std::string name("for" + std::to_string(gen->loop_count));
          *p_ss << "\n\tmov rcx, " << f.startValue.value.value() << "\n";
          *p_ss << "for" << gen->loop_count << ":\n";
          *p_ss << "\tcmp rcx, " << f.targetValue.value.value() << "\n";
          *p_ss << "\tjge for" << gen->loop_count << "_end\n";
          *p_ss << "\tadd rcx, " << f.incValue.value.value() << "\n";
          gen->generate(f.body, gen->MAIN, *p_ss, scope);
          *p_ss << "\tjmp for" << gen->loop_count << "\n";
          *p_ss << "\n" << name << "_end:\n";
          gen->loop_count++;

          gen->varScopes[f.identifier.value.value()] = scope;
        }

        void operator()(NodeFuncStmt f) {
          Logger::Trace("Generating Function");
          Logger::Trace("Function params : %d", f.param_count);

          std::vector<Var> scope;
          std::stringstream temp;
          std::stringstream nest;
          gen->scope_stack.push_back(f.identifier.value.value());
          gen->changeScope(f.identifier.value.value());

          size_t scope_offset = 1; // cover the memory of the current scope;

          gen->funcStack.push_back(f);
          temp << "\n" << f.identifier.value.value() << ":\n";
          temp << "\n\tpush rbp\n";
          temp << "\n\tmov rbp, rsp\n";
          // argument name uniqueness with current scope prefix
          std::string param_name;

          for (int i = 0; i < f.param_count; i++) {
            param_name = gen->scope_stack.back() + "." +
                         f.params.at(i).identifier.value.value();
            gen->BSS << "\n\t" << param_name << " resb 1024";
            switch (i) {
            case 0:
              temp << "\n\tmov [rbp -8], rdi\n";
              scope.push_back({.name = {.value = param_name},
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              break;
            case 1:
              scope.push_back({.name = {.value = param_name},
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              temp << "\n\tmov [rbp -16], rsi\n";
              break;
            case 2:
              scope.push_back({.name = {.value = param_name},
                               .stackOffset = scope_offset++,
                               .value = f.params.at(i).value});
              temp << "\n\tmov [rbp -24], rdx\n";
            default:
              break;
            }
            f.params.at(i).identifier.value.value() = param_name.c_str();
          }
          if (f.identifier.value.value() != "main")
            gen->generate(f.body, gen->MAIN, temp,
                          scope); // stream for child statements
          else
            gen->generate(f.body, gen->MAIN, temp, gen->varStack);

          // gen->queue.emplace_back(temp.str());
          // gen->queue.emplace_back(nest.str());
          if (f.ret_value.value.value.has_value()) {
            if (f.ret_value.value.type == TokenType::INT_LIT ||
                f.ret_value.value.type == TokenType::STRING_LIT) {

              if (f.identifier.value.value() == "main")
                temp << "\n\tmov rdi, " << f.ret_value.value.value.value();
              else {
                if (f.ret_value.value.type == INT_LIT)
                  temp << "\n\tmov rax, " << f.ret_value.value.value.value();
                else
                  temp << "\n\tmov rax, " << param_name;
              }
            } else {
              temp << "\n\tmov rax, "
                   << gen->scope_stack.back() + "." +
                          f.ret_value.value.value.value();
            }
          }
          gen->main_ret = true;
          temp << "\n\tpop rbp\n";
          for (int i = 0; i < f.param_count; i++) {
            if (f.params.at(i).type == DataType::STR || 1) {
              temp << "\n\tlea rdi, [" << param_name
                   << "]\n\tmov rsi, 1024\n\tcall std_clear_string\n";
            }
          }
          if (f.identifier.value.value() == "main") {
            gen->MAIN << temp.str();
            /*gen->varScopes["main"] = scope;*/
            printf("size : %d", (int)gen->varScopes["main"].size());
          } else {
            temp << "\tret\n";
            gen->FUNC << temp.str();
            gen->varScopes[f.identifier.value.value()] = scope;
          }

          gen->exitScope();
        }

        void operator()(NodeFuncCall fc) {
          /* gen->changeScope(fc.identifier.value.value()); */
          bool found = false;
          Logger::Trace("Generating Function call");
          Logger::Trace("Function call Params : %d", fc.param_count);

          printf(" -- %s size : %d\n", gen->scope_stack.back().c_str(),
                 (int)gen->varScopes[gen->scope_stack.back()].size());
          std::string param_name;
          for (auto f : gen->funcStack) {
            //  checking if function has been declared
            if (f.identifier.value.value() == fc.identifier.value.value()) {
              // matching parameter count
              if (f.param_count != fc.param_count)
                Logger::Error({.type = ms_Param, .line = fc.identifier.line});
              found = true;
              if (f.params.size() != -1) {
                // Generating parameters for function arguments
                for (int i = 0; i < fc.params.size(); i++) {
                  bool found = false;
                  param_name = gen->scope_stack.back() + "." +
                               fc.params.at(i).value.value.value();
                  printf("param : %s\n", param_name.c_str());
                  for (auto v : gen->varScopes[gen->scope_stack.back()]) {
                    printf(" >> %s\n",
                           fc.params.at(i).value.value.value().c_str());
                    printf("Scope Var Name :  %s\n",
                           v.name.value.value().c_str());
                    if (fc.params.at(i).value.value.has_value() &&
                        param_name == v.name.value.value()) {
                      Logger::Trace("From func call : found : %s",
                                    v.name.value.value().c_str());
                      found = true;
                      break;
                    }
                  }
                  *p_ss << "\n\tmov rdi, " << param_name;
                  *p_ss << "\n\tmov rsi, "
                        << f.identifier.value.value() + "." +
                               f.params.at(i).identifier.value.value();
                  *p_ss << "\n\tcall std_copy\n";
                  if (i == 0) {
                    *p_ss << "\n\tmov rdi, " << param_name << "\n";
                  } else if (i == 1)
                    *p_ss << "\n\tmov rsi, "
                          << fc.params.at(i).value.value.value() << "\n";
                  else
                    *p_ss << "\n\tpush " << fc.params.at(i).value.value.value()
                          << "\n";

                  if (fc.params.at(i).value.value.has_value() &&
                      found == false) {
                    printf(" %s Not found in currentScope\n",
                           param_name.c_str());
                    // exit(1);
                  }
                }
              }
              *p_ss << "\n\tcall " << fc.identifier.value.value() << "\n";
              // gen->TEXT << "_" << fc.identifier.value.value() << ":\n";
            }
          }
          if (found == false) {
            Logger::Error({.type = errType::ex_Func,
                           .line = fc.identifier.line,
                           .col = fc.identifier.col});
          }
        }

        void operator()(NodeCallStmt c) {
          if (gen->varScopes.find(c.std_lib_value.value.value()) ==
              gen->varScopes.end()) {
            Logger::Error({.type = ms_Scope, .line = c.std_lib_value.line});
          } else {
            for (auto v : gen->varScopes[c.std_lib_value.value.value()]) {
              printf(" :: %s\n", v.name.value.value().c_str());
            }
          }
          Logger::Trace("Generating Call Stmt");
          std::string param_name;
          for (int i = 0; i < c.params.size(); i++) {
            param_name = gen->scope_stack.back() + "." +
                         c.params.at(i).value.value.value();
            switch (i) {
            case 0:
              *p_ss << "\n\tmov rdi, " << param_name;
              break;
            case 1:
              *p_ss << "\n\tmov rsi, " << param_name;
              break;
            case 2:
              *p_ss << "\n\tmov rdx, " << param_name;
              break;
            default:
              *p_ss << "\n\tpush " << param_name;
              break;
            }
          }
          *p_ss << "\n\tcall " << c.std_lib_value.value.value() << "\n";
        }
        void operator()(NodeExtrnStmt e) {
          std::vector<Var> scope;
          std::string arg_name;
          for (auto p : e.param) {
            arg_name =
                e.identifier.value.value() + "." + p.identifier.value.value();
            Var v = {.name = {.value = arg_name}};
            gen->BSS << "\n\t" << arg_name << " resb 1024";
            scope.push_back(v);
          }
          gen->HEADER << "\n\textern " << e.identifier.value.value() << "\n";
          gen->varScopes[e.identifier.value.value()] = scope;
        }
        void operator()(NodeWhileStmt w) {

          // Was never In Commission

          /* std::string name("while" + std::to_string(gen->loop_count++)); */
          /* Logger::Trace("While Body size : %d", w.body.size()); */
          /* *p_ss << "\n\tpush rcx"; */
          /* for (auto c : w.comparisons) { */
          /*   for (auto v : gen->varScopes[gen->scope_stack.back()]) { */
          /*     if (c.lhs.value.value() == v.name.value.value()) { */
          /*       *p_ss << "\n\tcmp " << v.value.value.value() << ", " */
          /*             << c.rhs.value.value(); */
          /*     } else */
          /*       *p_ss << "\n\tcmp " << c.lhs.value.value() << ", " */
          /*             << c.rhs.value.value(); */
          /*   } */
          /*   if (c.cmp_s == "<") { */
          /*     *p_ss << "\n\tjl " << name; */
          /*     *p_ss << "\n\tjg " << name << "_end"; */
          /*   } */
          /* } */
          /* *p_ss << "\n\t" << name << ":"; */
          /* gen->generate(w.body, gen->MAIN, *p_ss, scope); */
          /* *p_ss << "\n\t" << name << "_end:"; */
          /* *p_ss << "\n\tpop rcx"; */
        }
        void operator()(NodeIfStmt ifs) {

          // My current goal. completed -- [x]

          Logger::Trace("From if : Doing Work ...");

          // main.if_0:
          //        body
          // main.if_0_else:
          //        body
          // main.if0_end:
          //        rest of program;

          std::string if_name = gen->scope_stack.back() + ".if" +
                                std::to_string(gen->loop_count++);
          gen->changeScope(if_name);

          Logger::Trace("if identifier : %s", if_name.c_str());

          struct lhsv {
            AsmGen *gen;
            std::stringstream *p_ss;
            NodeIfStmt *pm;
            void operator()(const std::shared_ptr<NodeFuncCall> &f) {
              for (auto fs : gen->funcStack) {
                printf("Matching %s : %s\n",
                       f->identifier.value.value().c_str(),
                       fs.identifier.value.value().c_str());

                if (f->identifier.value.value() ==
                    fs.identifier.value.value()) {
                  std::vector<NodeStmts> stmt;
                  stmt.push_back({.var = *f});
                  gen->generate(stmt, *p_ss, *p_ss,
                                gen->varScopes[gen->scope_stack.back()]);
                  *p_ss << "\n\tmov r8, rax";

                  break;
                }
              }
            }

            void operator()(NodeInt i) {}
          };
          struct rhsv {
            AsmGen *gen;
            std::stringstream *p_ss;
            NodeIfStmt *pm;
            void operator()(const std::shared_ptr<NodeFuncCall> &f) {
              for (auto fs : gen->funcStack) {
                printf("Matching %s : %s\n",
                       f->identifier.value.value().c_str(),
                       fs.identifier.value.value().c_str());

                if (f->identifier.value.value() ==
                    fs.identifier.value.value()) {
                  std::vector<NodeStmts> stmt;
                  stmt.push_back({.var = *f});
                  gen->generate(stmt, *p_ss, *p_ss,
                                gen->varScopes[gen->scope_stack.back()]);
                  *p_ss << "\n\tmov r9, rax";

                  break;
                }
              }
            }

            void operator()(NodeInt i) {}
          };
          // TODO replace with one visit
          std::visit(lhsv{gen, p_ss, &ifs}, ifs.condition->lhs);
          std::visit(rhsv{gen, p_ss, &ifs}, ifs.condition->rhs);

          *p_ss << "\n\tcmp r8, r9\n\tje " << if_name
                << "\n\tjne " + if_name + "else\n\t";
          *p_ss << "\n" << if_name << ":";
          gen->generate(ifs.trueBody, *p_ss, *p_ss,
                        gen->varScopes[gen->scope_stack.back()]);

          *p_ss << "\n\tjmp " + if_name + "end";
          if (ifs.has_else) {
            *p_ss << "\n\t" << if_name + "else:";
            gen->generate(ifs.falseBody, *p_ss, *p_ss,
                          gen->varScopes[gen->scope_stack.back()]);
            *p_ss << "jmp " + if_name + "end";
          }
          *p_ss << "\n" + if_name + "end:";
        }
      };

      std::visit(stmtVisitor{this, &curScope, &n_s, &p_s}, stmt.var);
      /* Logger::Info("End of Generating Body"); */
    }
  }
  inline ~AsmGen() {}
};
