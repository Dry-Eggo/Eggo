#pragma once

#include "eggoLog.hpp"
#include "token.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <ios>
#include <iostream>
// #include <string>
#include <sstream>
#include <map>
//#include <utility>
#include <string>
#include <variant>
#include <vector>

class AsmGen
{
public:
  NodeProg m_program;
  std::fstream out_asm;
  std::vector<Var> varStack;
  std::vector<NodeFuncStmt> funcStack;
  std::map<Token, std::vector<Var>> varScopes;
  std::vector<std::string> queue;
  size_t currOffset = 1;
  std::stringstream TEXT, BSS, DATA, FUNC;

  inline void push(Var v)
  {
    TEXT << "\n\tmov rax, " << v.value.value.value() << "\n";
    TEXT << "\tpush rax\n";
    v.stackOffset = currOffset++;
  }

  inline void pop(Var v, const char *reg)
  {
    if (currOffset - 1 != 0 && v.value.type != TokenType::STRING_LIT)
      TEXT << "\n\tmov " << reg << ", [rbp - " << 8 * v.stackOffset << "]\n";
    else
      TEXT << "\n\tmov " << reg << ", " << v.name.value.value() << "\n";
  }

  inline AsmGen(NodeProg program)
      : m_program(program)
  {
    BSS << "\nsection .bss\n";
    DATA << "\nsection .data\n";

    out_asm.open("d.asm", std::ios::out);

    TEXT << "\nsection .text\n";
    TEXT << "global _start\n";
    TEXT << "\nextern std_terminate_process\nextern std_print_string\nextern std_flush\nextern std_print_int\nextern std_copy\n";
    TEXT << "\n_start:\n";
    TEXT << "\n\n\tmov rbp, rsp\n";

    Logger::Info("Generating Asm");
    Logger::Info("Program Size : %d", m_program.stmt.size());
    
    generate(m_program.stmt, TEXT, TEXT,varStack);

    out_asm << BSS.str();
    out_asm << DATA.str();
    out_asm << TEXT.str();

    out_asm << "\n\tmov rdi, 0\n\tcall std_terminate_process\n";
    for(auto &p : queue)
    {
      FUNC << p;
    }

    out_asm << FUNC.str();
    out_asm << "\nsection .note.GNU-stack";
  }
  inline void generate(std::vector<NodeStmts> stmts, std::stringstream &n_s, std::stringstream &p_s, std::vector<Var> &curScope)
  {
    for (auto stmt : stmts)
    {
      struct stmtVisitor
      {
        AsmGen *gen;
        std::vector<Var> *curScope;
        std::stringstream *c_ss;
        std::stringstream *p_ss;

        void operator()(NodeExitStmt e)
        {
          struct exprVisitor
          {
            AsmGen *gen;
            void operator()(NodeInt i)
            {
              gen->TEXT << "\n\tmov rdi, " << i.value.value.value() << "\n";
              gen->TEXT << "\tcall std_terminate_process\n";
            }
            void operator()(Token i)
            {
              for (auto v : gen->varStack)
              {
                if (i.type == TokenType::IDENT && i.value.value() == v.name.value.value())
                {
                  gen->pop(v, "rdi");
                  gen->TEXT << "\tcall std_terminate_process\n";
                }
              }
            }
          };
          std::visit(exprVisitor{.gen = gen}, e.expr.var);

          Logger::Info("Generating Exit statement");
        }

        void operator()(NodeMkStmt m)
        {
          Logger::Info("Generating Make statement");
          Var v = {.name = m.identifier, .stackOffset = gen->currOffset, .value = m.value};

          if (m.value.type == TokenType::STRING_LIT || m.value.type == TokenType::INT_LIT)
          {
            gen->DATA << "\n\t" << m.identifier.value.value() << " db " << m.value.value.value() << ", 0\n";
          }
          else if (m.value.value.value() == "undef")
          {
            if (m.type == DataType::STR)
              gen->BSS << "\n\t" << m.identifier.value.value() << " resb 1024\n";
            else if (m.type == DataType::INT)
            {
              gen->BSS << "\n\t" << m.identifier.value.value() << " resb 1\n";
            }
          }
          else
            gen->push(v);
          curScope->push_back(v);
        }

        void operator()(NodeReStmt r)
        {
          bool found = false;
          Logger::Info("Generate Re_Assign statement");
          for (auto v : *curScope)
          {
            if (v.name.value.value() == r.identifier.value.value())
            {
              found = true;
              gen->TEXT << "\n\tmov " << v.name.value.value() << ", " << r.new_value.value.value() << "\n";
                            
            }
            if(found == false)
            {
              Logger::Error({.type = ms_Scope, .line = r.identifier.line});
            }
          }
        }

        void operator()(NodeForStmt f)
        {
          Logger::Info("Generating For statement");

          Logger::Info("For Body size : %d", f.body.size());
        
          std::vector<Var> scope;

          gen->TEXT << "\n\tmov rcx, " << f.startValue.value.value() << "\n";
          gen->TEXT << "for:\n";
          gen->TEXT << "\tcmp rcx, " << f.targetValue.value.value() << "\n";
          gen->TEXT << "\tjge for_end\n";
          gen->TEXT << "\tinc rcx\n";
          gen->generate(f.body, gen->TEXT, gen->TEXT, scope);
          gen->TEXT << "\tjmp for\n";
          gen->TEXT << "\nfor_end:\n";

          //gen->varScopes[f.identifier] = scope;
          
        }

        void operator()(NodeFuncStmt f)
        {
          Logger::Info("Generating Function");
          Logger::Info("Function params : %d", f.param_count);

          std::vector<Var> scope;
          std::stringstream temp;
          std::stringstream nest;
        
          gen->funcStack.push_back(f);
          temp << "\n" << f.identifier.value.value() << ":\n";
          temp << "\n\tmov rbp, rsp\n";

          for(int i = 0; i < f.param_count; i++) 
          {
            switch (i) {
              case 0:
                temp << "\n\tmov [rbp -8], rdi\n";
                break;
              case 1:
                temp << "\n\tmov [rbp -16], rsi\n";
                break;
              case 2:
                temp << "\n\tmov [rbp -24], rdx\n";
              default:
                break;
            }
          }
          gen->generate(f.body, gen->FUNC , temp, scope); // stream for child statements

          //gen->queue.emplace_back(temp.str());
          temp << "\tret\n";
          //gen->queue.emplace_back(nest.str());

          gen->FUNC << temp.str();

          //gen->varScopes[f.identifier] = scope;
        }

        void operator()(NodeFuncCall fc)
        {
          bool found = false;
          Logger::Info("Generating Function call");
          Logger::Info("Function call Params : %d", fc.param_count);
          for(auto f : gen->funcStack)
          {

            if(f.identifier.value.value() == fc.identifier.value.value())
            { 
              if(f.param_count != fc.param_count)
                Logger::Error({.type = ms_Param, .line = fc.identifier.line});
              found = true;
              if(f.params.size() != 0)
              {
                    for(int i = 0; i < fc.params.size(); i++)
                    {
                      if(i == 0)
                      {
                        *p_ss << "\n\tmov rdi, " << fc.params.at(i).value.value.value() << "\n";
                      }
                      else if(i == 1)
                        *p_ss << "\n\tmov rsi, " << fc.params.at(i).value.value.value() << "\n";
                      else
                        *p_ss << "\n\tpush " << fc.params.at(i).value.value.value() << "\n";
                    }
              }
              *p_ss << "\n\tcall " << fc.identifier.value.value() << "\n";
              //gen->TEXT << "_" << fc.identifier.value.value() << ":\n";
            }
          }
          if(found == false)
          {
            Logger::Error({.type = errType::ex_Func, .line = fc.identifier.line});
          }
        }

        void operator()(NodeCallStmt c)
        {
          Logger::Info("Generating Call Stmt");
          *p_ss << "\n\tcall " << c.std_lib_value.value.value() << "\n";
        }
      };

      std::visit(stmtVisitor{this, &curScope, &n_s, &p_s}, stmt.var);
    }

  }
  inline ~AsmGen() {}
};
