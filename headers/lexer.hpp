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
  int curline = 1;


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
            tokens.push_back({.type = TokenType::EXIT, .line = curline});
          }

          else if(buf == "mk")
          {
            tokens.push_back({.type = TokenType::MK, .line=curline});
          }

          else if (buf == "int") 
          {
            tokens.push_back({.value = buf,.type = TokenType::TYPE, .line = curline});
          }

          else if (buf == "str") {
            tokens.push_back({.value = buf, .type = TokenType::TYPE, .line = curline});
          }

          else if (buf == "for") {
            tokens.push_back({.type = TokenType::FOR, .line = curline});
          }
          else if (buf == "mkf") {
            tokens.push_back({.type = TokenType::FUNC, .line = curline});
          }
          else if (buf == "call") {
            tokens.push_back({.type = TokenType::CALL, .line = curline});
          }
		  else if (buf == "dmk")
		  {
			  tokens.push_back({.type = TokenType::EXTERN, .line = curline});
		  }

          else {
            tokens.push_back({.value = buf,.type = TokenType::IDENT, .line = curline});
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

        tokens.push_back({.value = buf, .type = TokenType::STRING_LIT, .line = curline});
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
            if(peek().value() == '\n')
              curline++;
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
        tokens.push_back({ .value = buf,.type = TokenType::INT_LIT, .line = curline});
        buf.clear();
      }

      if(peek().value() == '(')
      {
        consume();
        tokens.push_back({.type = TokenType::OPAREN, .line = curline});
        Logger::Info("Parse Token : '('");
      }

      if(peek().value() == ':')
      {
        consume();
        tokens.push_back({.type = TokenType::TYPE_DEC, .line = curline});
      }

      if(peek().value() == ')')
      {
        consume();
        tokens.push_back({.type = TokenType::CPAREN, .line = curline});
      }
      if (peek().value() == ';')
      {
        consume();
        tokens.push_back({.type = TokenType::SEMI, .line = curline});
        //printf("Found semi at line : %d\n", curline);
      }
      if (peek().value() == '+'){
		 if(peek(1).has_value() && peek(1).value() == '=') {
			consume();
			consume();
			tokens.push_back({.type = TokenType::ADD_EQU, .line = curline});
		 }else{
			 consume();
		 	 tokens.push_back({.type = TokenType::ADD, .line = curline});
		 }
      }

      if(peek().value() == '<')
      {
        consume();
        tokens.push_back({.type = TokenType::LTH, .line = curline});
      }
	  if(peek().value() == '>')
	  {
		  consume();
		  tokens.push_back({.type = TokenType::GTH, .line = curline});
	  }

	  if(peek().value() == '>')
	  {
		  if(peek(1).has_value() && peek(1).value() == '=')
			  tokens.push_back({.type = TokenType::GTH, .line = curline});

		  consume();
		  consume();
	  }
	  
	  if(peek().has_value() && peek().value() == '>')
	  {
		  consume();
		  tokens.push_back({.type = TokenType::GTH, .line = curline});
	  }
      if (peek().has_value() && peek().value() == '{') {
        consume();
        tokens.push_back({.type = TokenType::OBRACE, .line = curline});
      }
      if (peek().value() == '}') {
        consume();
        tokens.push_back({.type = TokenType::CBRACE, .line = curline});
      }

      if (peek().value() == '=') 
      {
        consume();
        tokens.push_back({.type = TokenType::ASSIGN, .line = curline});
      }
      if (peek().value() == ',') {
        consume();
      }
      if(isspace(peek().value())) {
        if(peek().value() == '\n')
        {
          curline++;
          //printf("Newline\n");
        }
        consume();
      }     
	  //if(peek().value() == '+')
	  //{
		 // consume();
		  //tokens.push_back({.type =  TokenType::ADD, .line = curline});
	  //}
	  if(peek().has_value() && peek().value() == '-')
	  {
		consume();
		tokens.push_back({.type = TokenType::SUB, .line = curline});
	  }
	  else if(peek().has_value() && peek().value() == '*')
	  {
		 consume();
		  tokens.push_back({.type = TokenType::MUL, .line = curline});
		  printf("INFO : *\n");
	  }
	  else if(peek().has_value() && peek().value() == '/')
	  {
		consume();
		  tokens.push_back({.type = TokenType::DIV, .line = curline});
	  }
    }
   	printf("End of Lexing\n"); 
    tokens.push_back({.type = TokenType::eof, .line = curline});
    Logger::Info("Token Size : %d", tokens.size());
    Parser parse(tokens);

  }

};
