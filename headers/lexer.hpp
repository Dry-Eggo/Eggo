#pragma once

#include "eggoLog.hpp"
#include "parser.hpp"
#include "token.hpp"
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

class Lexer {
public:
  inline Lexer(const char *filePath) : m_srcPath(std::move(filePath)) {
    fs::path p = m_srcPath;
    if (!fs::exists(fs::absolute(p))) {
      printf("No Such File or Directory : %s", p.c_str());
      exit(1);
    }

    std::ifstream file(p);
    if (!file.is_open()) {
      printf("Error : Unable to open file");
      exit(0);
    }

    std::stringstream ss;
    ss << file.rdbuf();
    m_srcFile = ss.str();

    file.close();

    lex();
  }
  inline ~Lexer() {}

private:
  std::string m_srcFile;
  std::string m_srcPath;

  size_t m_index = 0;
  int curline = 1;
  int column = 1;

  inline std::optional<char> peek(int i = 0) {
    if ((m_index + i) >= m_srcFile.length()) {
      return {};
    } else {
      return m_srcFile.at(m_index + i);
    }
  }

  inline char consume() { return m_srcFile.at(m_index++); }

  inline void lex() {
    std::string buf;
    std::vector<Token> tokens;

    while (peek().has_value()) {
      if (std::isalpha(peek().value())) {
        buf.push_back(consume());
        column++;
        while (peek().has_value() && isalnum(peek().value()) ||
               peek().value() == '_') {
          buf.push_back(consume());
          column++;
        }

        Logger::Trace("Parse token : %s", buf.c_str());

        if (buf == "exit") {
          tokens.push_back(
              {.type = TokenType::EXIT, .line = curline, .col = column});
        }

        else if (buf == "mk") {
          tokens.push_back(
              {.type = TokenType::MK, .line = curline, .col = column});
        }

        else if (buf == "ret") {
          tokens.push_back(
              {.type = TokenType::RET, .line = curline, .col = column});
        }

        else if (buf == "str" || buf == "int" || buf == "bool") {
          tokens.push_back({.value = buf,
                            .type = TokenType::TYPE,
                            .line = curline,
                            .col = column});
        }

        else if (buf == "for") {
          tokens.push_back(
              {.type = TokenType::FOR, .line = curline, .col = column});
        } else if (buf == "mkf") {
          tokens.push_back(
              {.type = TokenType::FUNC, .line = curline, .col = column});
        } else if (buf == "call") {
          tokens.push_back(
              {.type = TokenType::CALL, .line = curline, .col = column});
        } else if (buf == "extern") {
          tokens.push_back(
              {.type = TokenType::EXTERN, .line = curline, .col = column});
        } else if (buf == "while") {
          tokens.push_back(
              {.type = TokenType::WHILE, .line = curline, .col = column});
        } else if (buf == "if") {
          tokens.push_back(
              {.type = TokenType::IF, .line = curline, .col = column});
        } else if (buf == "else") {
          tokens.push_back({.type = ELSE, .line = curline, .col = column});
        }

        else {
          tokens.push_back({.value = buf,
                            .type = TokenType::IDENT,
                            .line = curline,
                            .col = column});
        }

        buf.clear();
      }

      if (peek().value() == '\"') {
        Logger::Trace("Parsing string lit");
        buf.push_back(consume());
        column++;
        // "Hello"
        while (peek().has_value() && peek().value() != '\"') {
          buf.push_back(consume());
          column++;
          // if(peek().has_value() && peek().value() == '\"')
          //{
          // buf.push_back('\"');
          //}
        }

        if (peek().value() == '\"') {
          buf.push_back(consume());
          column++;
        }

        tokens.push_back({.value = buf,
                          .type = TokenType::STRING_LIT,
                          .line = curline,
                          .col = column});
        Logger::Trace("Buf : %s", buf.c_str());
        buf.clear();
      }

      if (peek().value() == '#') {
        consume();
        if (peek().value() == '!') {
          consume();
          while (peek().value() != '\n') {
            consume();
          }
        } else {
          while (peek().value() != '!') {
            consume();
            if (peek().value() == '\n') {
              curline++;
              column = 1;
            }
          }
          column++;
          consume();
        }
      }

      if (isdigit(peek().value())) {
        column++;
        buf.push_back(consume());
        // Logger::Info("%s", buf.c_str());
        while (peek().has_value() && isdigit(peek().value())) {
          buf.push_back(consume());
          column++;
        }
        Logger::Trace("Parse Token : %s", buf.c_str());
        tokens.push_back({.value = buf,
                          .type = TokenType::INT_LIT,
                          .line = curline,
                          .col = column});
        buf.clear();
      }

      if (peek().value() == '(') {
        consume();
        tokens.push_back(
            {.type = TokenType::OPAREN, .line = curline, .col = column});
        Logger::Trace("Parse Token : '('");
      }

      if (peek().value() == ':') {
        consume();
        tokens.push_back(
            {.type = TokenType::TYPE_DEC, .line = curline, .col = column});
      }

      if (peek().value() == ')') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::CPAREN, .line = curline, .col = column});
      }
      if (peek().value() == ';') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::SEMI, .line = curline, .col = column});
        // printf("Found semi at line : %d\n", curline);
      }
      if (peek().value() == '+') {
        if (peek(1).has_value() && peek(1).value() == '=') {
          consume();
          consume();
          column += 2;
          tokens.push_back(
              {.type = TokenType::ADD_EQU, .line = curline, .col = column});
        } else {
          consume();
          column++;
          tokens.push_back(
              {.type = TokenType::ADD, .line = curline, .col = column});
        }
      }

      if (peek().value() == '<') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::LTH, .line = curline, .col = column});
      }
      if (peek().value() == '>') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::GTH, .line = curline, .col = column});
      }

      if (peek().value() == '>') {
        if (peek(1).has_value() && peek(1).value() == '=')
          tokens.push_back(
              {.type = TokenType::GTH_EQU, .line = curline, .col = column});

        consume();
        consume();
        column += 2;
      }

      if (peek().has_value() && peek().value() == '>') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::GTH, .line = curline, .col = column});
      }
      if (peek().has_value() && peek().value() == '{') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::OBRACE, .line = curline, .col = column});
      }
      if (peek().value() == '}') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::CBRACE, .line = curline, .col = column});
      }

      if (peek().value() == '=') {
        consume();
        if (peek().has_value() && peek().value() == '=') {
          consume();
          column += 2;
          tokens.push_back(
              {.type = TokenType::EQU, .line = curline, .col = column});
          Logger::Trace("Lexed Equ");
        } else {
          column++;
          tokens.push_back(
              {.type = TokenType::ASSIGN, .line = curline, .col = column});
        }
      }

      if (peek().value() == ',') {
        consume();
        column++;
      }
      if (isspace(peek().value())) {
        if (peek().value() == '\n') {
          curline++;
          column = 1;
        }
        consume();
        column++;
      }
      // if(peek().value() == '+')
      //{
      //  consume();
      // tokens.push_back({.type =  TokenType::ADD, .line  curline, .col =
      // column});
      //}
      if (peek().has_value() && peek().value() == '-') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::SUB, .line = curline, .col = column});
      } else if (peek().has_value() && peek().value() == '*') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::MUL, .line = curline, .col = column});
        printf("INFO : *\n");
      } else if (peek().has_value() && peek().value() == '/') {
        consume();
        column++;
        tokens.push_back(
            {.type = TokenType::DIV, .line = curline, .col = column});
      }
      /*if ((m_index - column) >= 1 && m_index != 0) {*/
      /*  // we have moved a column*/
      /*  column += m_index - column;*/
      /*}*/
    }
    printf("End of Lexing\n");
    tokens.push_back({.type = TokenType::eof, .line = curline, .col = column});
    column++;
    Logger::Trace("Token Size : %d", tokens.size());
    Parser parse(tokens);
  }
};
