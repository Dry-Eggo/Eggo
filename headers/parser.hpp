#pragma once

#include "asmgen.hpp"
#include "eggoLog.hpp"
#include "token.hpp"
#include <cstddef>
#include <cstdlib>
#include <optional>
#include <utility>
#include <vector>

class Parser {
public:
  inline Parser(std::vector<Token> tokens)
    : m_tokens(std::move(tokens))
  {
    Logger::Info("Parsing");
      while (peek().has_value()) 
      {
        if(peek().value().type == TokenType::EXIT)
        {
            consume();
            parse_exit_stmt();
        }
        if(peek().value().type == TokenType::MK)
        {
          //consume();
          parse_mk_stmt(stmts.stmt);
        }
        if(peek().value().type == TokenType::IDENT)
        {
          //consume();
          parse_re_assign_stmt(stmts.stmt);
        }
        if (peek().value().type == TokenType::FOR) {
          consume();
          parse_for_stmt(stmts.stmt);
        }
        if (peek()->type == TokenType::FUNC) {
          parse_func_stmt(stmts.stmt);
        }
        if(peek()->type == TokenType::CALL)
        {
          parse_call_stmt(stmts.stmt);
        }
        else {
          consume();
        }
      }

      AsmGen gen(stmts);
  }
  inline ~Parser() {}

private:
  std::vector<Token> m_tokens;
  size_t m_index = 0;
  NodeProg stmts;
  
  inline std::optional<Token> peek(int i = 0)
  {
    if((m_index + i) >= m_tokens.size())
    {
      return {};
    }
    return m_tokens.at(m_index + i);

  }

  inline Token consume()
  {
    return m_tokens.at(m_index++);
  }

  inline void parse_call_stmt(std::vector<NodeStmts> &stmts)
  {
    NodeStmts stmt;
    NodeCallStmt call;
    consume(); // call
    if(peek().has_value() && peek()->type == TokenType::IDENT)
    {
      call.std_lib_value = consume();
      stmt.var = call;
      stmts.push_back(stmt);
    }
  }

  void parse_exit_stmt() 
  {
    NodeStmts stmt;
    NodeExitStmt exitstmt;
    
    if(peek().value().type == TokenType::OPAREN)
    {
      consume();
      if(peek().has_value() && peek().value().type == TokenType::INT_LIT)
      {
          NodeInt i = {.value = consume()};
          exitstmt.expr.var = i;
          if(peek().has_value() && peek().value().type == TokenType::CPAREN)
          {
            consume();
            if(peek().has_value() && peek().value().type == TokenType::SEMI)
            {
              stmt.var = exitstmt;
              stmts.stmt.push_back(stmt);
               
            } else {Logger::Error({.type = errType::ex_Delimiter, .line = peek(-1).value().line}); exit(1);}
          } else {Logger::Error({.type = errType::ex_Cparen, .line = peek(-1).value().line}); exit(1);}

      }else if(peek().has_value() && peek().value().type == TokenType::IDENT)
      {
        exitstmt.expr.var = consume();
        if(peek().has_value() && peek().value().type == TokenType::CPAREN)
        {
          consume();
          if(peek().has_value() && peek().value().type == TokenType::SEMI)
          {
             stmt.var = exitstmt;
            stmts.stmt.push_back(stmt);
            
          } else {Logger::Error({.type = errType::ex_Delimiter, .line = peek(-1).value().line}); exit(1);}
        } else {Logger::Error({.type = errType::ex_Cparen, .line = peek(-1)->line}); exit(1);}


      }
    } else {Logger::Error({.type = errType::ex_Oparen, .line = peek(-1)->line}); exit(1);} 

  }

  inline void parse_mk_stmt(std::vector<NodeStmts> &stmts)
  {
    NodeStmts stmt;
    NodeMkStmt mkstmt;
    consume();  // discarding "MK" keyword
    if(peek().has_value() && peek().value().type == TokenType::IDENT)
    {
      mkstmt.identifier.value = consume().value;
      if(peek().has_value() && peek().value().type == TokenType::TYPE_DEC)
      {
        consume();
        if (peek().has_value() && peek().value().type == TokenType::TYPE) {
          if(peek().value().value.value() == "str")
          {
            mkstmt.type = DataType::STR;
          }   
          else if (peek().value().value.value() == "int") {
            mkstmt.type = DataType::INT;
          } else 
          {
            Logger::Error({.type = un_Type, .line = peek(-1)->line});
          }

          consume();

          if(peek().has_value() && peek().value().type == TokenType::ASSIGN)
          {
            consume();
            if(peek().has_value() && peek().value().type == TokenType::INT_LIT)
            {
              mkstmt.value = consume();
              if(peek().has_value() && peek().value().type == TokenType::SEMI)
              {
                stmt.var = mkstmt;
                stmts.push_back(stmt);
                //consume();
              } else {
                Logger::Error({.type = ex_Delimiter, .line = peek(-1)->line});
                exit(-1);
              }
            } else if(peek().has_value() && peek().value().type == TokenType::STRING_LIT)
            {
              mkstmt.value = consume();
              if(peek().has_value() && peek().value().type == TokenType::SEMI)
              {
                stmt.var = mkstmt;
                stmts.push_back(stmt);
                //consume();
              }
            } else {
              Logger::Error({.type = ex_Expression, .line = peek(-1)->line});
            } 
          } //else { TODO
            //Logger::Error({.type = ex_Operator, .line = peek(-1)->line});
          //}
          else if(peek().has_value() && peek().value().type == TokenType::SEMI){
              mkstmt.value.value = "undef";
              //consume();
              stmt.var = mkstmt;
              stmts.push_back(stmt);
          } else {
            Logger::Error({.type = ex_Delimiter, .line = peek(-1)->line});
            exit(-1);
          }

          //consume();
        }
      }  
    }
  }
  
  inline void parse_re_assign_stmt(std::vector<NodeStmts> &stmts)
  {
    NodeStmts stmt;
    NodeReStmt restmt;

   
    if(peek(1).has_value() && peek(1).value().type == TokenType::ASSIGN)
    {
      restmt.identifier = consume();

      consume();
      if(peek().has_value() && peek().value().type == TokenType::INT_LIT || peek().value().type == TokenType::IDENT)
      {
        restmt.new_value = consume();
        if(peek().has_value() && peek().value().type == TokenType::SEMI)
        {
          consume();
          stmt.var = restmt;
          stmts.push_back(stmt);
        } else {Logger::Error({.type = errType::ex_Delimiter, .line = peek(-1)->line});exit(1);}
      } else {Logger::Error({.type = errType::ex_Expression, .line = peek(-1)->line});exit(1);}

    } else if (peek(1).has_value() && peek(1)->type == TokenType::OPAREN){
      Logger::Info("function call");
      NodeFuncCall fcall;
      int param_count = 0;
      fcall.identifier = consume();
      consume(); // opening parenthesis
      if(peek().has_value() && peek()->type != TokenType::CPAREN)
      {
        while (peek().has_value() && peek()->type != TokenType::CPAREN) {
          NodeParam param = {.value = consume()};
          fcall.params.push_back(param);
          param_count++;
          
        }
      }
      if(peek().has_value() && peek()->type == TokenType::CPAREN && peek(1).has_value() && peek(1)->type == TokenType::SEMI)
      {
        consume();
        //consume();
        fcall.param_count = param_count;
        stmt.var = fcall;
        stmts.push_back(stmt);
        Logger::Info("Done function call");
      }
    }
    
  }

  inline void parse_for_stmt(std::vector<NodeStmts> &stmts)
  {
    NodeStmts stmt;
    NodeForStmt for_stmt;
    std::vector<NodeStmts> body;

    if (peek().has_value() && peek().value().type == TokenType::OPAREN) {
      consume();
      if (peek().has_value() && peek().value().type == TokenType::IDENT) {
        for_stmt.identifier = consume();
        if(peek().has_value() && peek().value().type == TokenType::TYPE_DEC && peek(1).value().type == TokenType::TYPE &&
            peek(2).value().type == TokenType::ASSIGN && peek(3).value().type == TokenType::INT_LIT && peek(4).value().type == 
            TokenType::SEMI) 
        {
          consume(); // ":"
          consume(); // "type : int"
          consume(); // "="
          for_stmt.startValue = consume(); // "value"
          consume(); // ";"

          if (peek().has_value() && peek().value().type == TokenType::IDENT) {
            consume();
            if (peek().value().type == TokenType::LTH) {
              consume();
              if(peek().value().type == TokenType::INT_LIT)
              {
                for_stmt.targetValue = consume();
                if(peek().value().type == TokenType::SEMI)
                {
                  consume();
                  if(peek().value().type == TokenType::IDENT && peek(1).value().type == ADD_EQU)
                  {
                    consume();
                    consume();
                    if(peek().value().type == TokenType::INT_LIT && peek(1).value().type == TokenType::CPAREN && peek(2).value().type == TokenType::OBRACE)
                    {
                      for_stmt.incValue = consume();
                      consume();
                      consume();
                      Logger::Info("Parsing For Body");
                      while (peek().value().type != TokenType::CBRACE) {
                        if(peek().value().type == TokenType::MK){
                          parse_mk_stmt(body);
                          Logger::Info("Found For-Make statement");
                        }
                        else if (peek().value().type == TokenType::IDENT)
                          parse_re_assign_stmt(body);
                        else if (peek().value().type == TokenType::EXIT)
                          parse_exit_stmt();
                        else if (peek().value().type == TokenType::FOR)
                          parse_for_stmt(body);
                        else if (peek()->type == TokenType::FUNC) {
                          parse_func_stmt(body);
                        }
                        
                        consume();
                      }
                      //consume();
                      for_stmt.body = body;
                      stmt.var = for_stmt;
                      stmts.push_back(stmt);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  inline void parse_func_stmt(std::vector<NodeStmts> &stmts)
  {

    NodeStmts stmt;
    NodeFuncStmt func_stmt;
    consume();
    int param_count = 0;
    if(peek()->type == TokenType::IDENT)
      func_stmt.identifier = consume();
     if(peek().has_value() && peek()->type == TokenType::OPAREN) {
        consume();
        if(peek()->type != CPAREN){

          while (peek().has_value() && peek()->type != TokenType::CPAREN) {
            NodeParam param;
            if(peek()->type == TokenType::IDENT)
            {
              param.identifier = consume();
              if (peek().has_value() && peek()->type == TokenType::TYPE_DEC) {
                consume();
                if(peek().has_value() && peek()->type == TokenType::TYPE)
                {
                  if(peek()->value == "str")
                  {
                    param.type = DataType::STR;
                  } else {
                    param.type = DataType::INT;
                  }
                  consume();
                  func_stmt.params.push_back(param);
                  param_count++;
                  
                }
              }
            }
            else if(peek()->type == CPAREN)
              break;
          }
        }
        if(peek()->type == CPAREN)
          consume();  // closing parenthesis
        if (peek().has_value() && peek()->type ==TokenType::OBRACE) {
          consume();
        }
        if(peek()->type != CBRACE){
          while (peek().has_value() && peek()->type!= TokenType::CBRACE) {
              if(peek()->type== TokenType::MK)
              {
                parse_mk_stmt(func_stmt.body);
              } else if(peek()->type == TokenType::IDENT)
                parse_re_assign_stmt(func_stmt.body);
              else if (peek()->type == TokenType::FOR){
                parse_for_stmt(func_stmt.body);
                Logger::Info("Found Nested For");
              }
              else if (peek()->type == TokenType::EXIT) {
                parse_exit_stmt(); // ------    out of support ------- //
              }
              else if (peek()->type == TokenType::FUNC) {
                parse_func_stmt(func_stmt.body);
              }
              else if (peek()->type == TokenType::CALL) {
                parse_call_stmt(func_stmt.body);
              }
              else if(peek()->type == CBRACE)
              {
                break;
              }
              consume();
          }
        }
        if(peek()->type == CBRACE)
          consume(); //closing curly
        //consume();
        func_stmt.param_count = param_count;
        stmt.var = func_stmt;
        stmts.push_back(stmt);
      }
    }     
};
