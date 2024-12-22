#pragma once

#include "eggoLog.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class Lexer
{
public:
  inline Lexer(const char* filePath) 
    : m_srcPath(std::move(filePath))
  {
    fs::path p = m_srcPath;
    if(!fs::exists(fs::absolute(p)))
    {
      printf("No Such File or Directory : %s", p.c_str());
      exit(1);
    }

    std::ifstream file(p);
    if(!file.is_open())
    {
        printf("Error : Unable to open file");
        exit(0);
    }
    
    std::stringstream ss;
    ss << file.rdbuf();
    m_srcFile = ss.str();

    file.close();

    lex();

  }
  inline ~Lexer(){}

private:
  std::string m_srcFile;
  std::string m_srcPath;

  size_t m_index = 0;
  int cuurLine = 1;


  inline std::optional<char> peek(int i = 0)
  {
    if((m_index + i) >= m_srcFile.length())
    {
        return {};
    } else {
        return m_srcFile.at(m_index + i);
    }
  }

  inline char consume(){return m_srcFile.at(m_index++);}
  
  inline void lex()
  {
    std::string buf;
    std::vector<Token> tokens;

    while(peek().has_value())
    {
      if(std::isalpha(peek().value()))
      {
          buf.push_back(consume());
          while (peek().has_value() && isalnum(peek().value()) || peek().value() == '_') 
          {
              buf.push_back(consume());
          }

          Logger::Info("Parse token : %s", buf.c_str());
          
          if (buf == "exit")
          {
            tokens.push_back({.type = TokenType::EXIT, .line = cuurLine});
          }

          else if(buf == "mk")
          {
            tokens.push_back({.type = TokenType::MK, .line=cuurLine});
          }

          else if (buf == "int") 
          {
            tokens.push_back({.value = buf,.type = TokenType::TYPE, .line = cuurLine});
          }

          else if (buf == "str") {
            tokens.push_back({.value = buf, .type = TokenType::TYPE, .line = cuurLine});
          }

          else if (buf == "for") {
            tokens.push_back({.type = TokenType::FOR, .line = cuurLine});
          }
          else if (buf == "mkf") {
            tokens.push_back({.type = TokenType::FUNC, .line = cuurLine});
          }
          else if (buf == "call") {
            tokens.push_back({.type = TokenType::CALL, .line = cuurLine});
          }

          else {
            tokens.push_back({.value = buf,.type = TokenType::IDENT, .line = cuurLine});
          }

          buf.clear();
      }

      if (peek().value() == '\"') 
      {
        Logger::Info("Parsing string lit");
        buf.push_back(consume());
        // "Hello"
        while (peek().has_value() && peek().value() != '\"') {
          buf.push_back(consume());
          //if(peek().has_value() && peek().value() == '\"')
          //{
            //buf.push_back('\"');
          //}

        }

        if(peek().value() == '\"')
        {
          buf.push_back(consume());
        }

        tokens.push_back({.value = buf, .type = TokenType::STRING_LIT, .line = cuurLine});
        Logger::Info("Buf : %s", buf.c_str());
        buf.clear();
      }

      if(peek().value() == '#')
      {
        consume();
        if(peek().value() == '!')
        {
          consume();
          while (peek().value() != '\n') {
            consume();
          } 
        } else {
          while (peek().value() != '!') {
            consume();
          }
          consume();
        }

      }
    
      if(isdigit(peek().value()))
      {
        buf.push_back(consume());
        //Logger::Info("%s", buf.c_str());
        while ( peek().has_value() && isdigit(peek().value())) {
            buf.push_back(consume());
        }
        Logger::Info("Parse Token : %s", buf.c_str());
        tokens.push_back({ .value = buf,.type = TokenType::INT_LIT, .line = cuurLine});
        buf.clear();
      }

      if(peek().value() == '(')
      {
        consume();
        tokens.push_back({.type = TokenType::OPAREN, .line = cuurLine});
        Logger::Info("Parse Token : '('");
      }

      if(peek().value() == ':')
      {
        consume();
        tokens.push_back({.type = TokenType::TYPE_DEC, .line = cuurLine});
      }

      if(peek().value() == ')')
      {
        consume();
        tokens.push_back({.type = TokenType::CPAREN, .line = cuurLine});
      }
      if (peek().value() == ';')
      {
        consume();
        tokens.push_back({.type = TokenType::SEMI, .line = cuurLine});
        //printf("Found semi at line : %d\n", cuurLine);
      }
      if (peek().value() == '+' && peek(1).value() == '=') {
        consume();
        consume();
        tokens.push_back({.type = TokenType::ADD_EQU, .line = cuurLine});
      }

      if(peek().value() == '<')
      {
        consume();
        tokens.push_back({.type = TokenType::LTH, .line = cuurLine});
      }

      if (peek().value() == '{') {
        consume();
        tokens.push_back({.type = TokenType::OBRACE, .line = cuurLine});
      }
      if (peek().value() == '}') {
        consume();
        tokens.push_back({.type = TokenType::CBRACE, .line = cuurLine});
      }

      if (peek().value() == '=') 
      {
        consume();
        tokens.push_back({.type = TokenType::ASSIGN, .line = cuurLine});
      }
      if (peek().value() == ',') {
        consume();
      }
      if(isspace(peek().value())) {
        if(peek().value() == '\n')
        {
          cuurLine++;
          //printf("Newline\n");
        }
        consume();
      }      
    }

    Logger::Info("Token Size : %d", tokens.size());
    Parser parse(tokens);

  }

};
