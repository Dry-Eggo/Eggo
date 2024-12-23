#pragma once

#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>


enum errType {
  ex_Delimiter, ex_Expression, ex_Integer, ex_Type,
  ex_Operator, ex_Oparen, ex_Obrace, ex_Cparen, ex_Cbrace,
  ms_Type, ms_TypeDef, un_Type, ex_Func, ms_Param, ms_Scope,
};

struct Error {
  errType type;
  int line;
};

enum TokenType
{
    SEMI, INT_LIT, OPAREN, CPAREN,
    IDENT, EXIT, MK, ASSIGN, STRING_LIT,
    TYPE_DEC, TYPE, FOR, ADD_EQU, LTH,
    CBRACE, OBRACE, GTH, LTH_EQU, GTH_EQU,
    FUNC, CALL, eof,

};

enum DataType {
  STR, INT,
};

struct Token 
{
  std::optional<std::string> value;
  TokenType type;
  int line;
};

struct NodeCallStmt
{
  Token std_lib_value;
};

struct NodeInt {
  Token value;
};

struct NodeExpr {

  std::variant<Token, NodeInt> var;
  
};

struct NodeReStmt {
  Token identifier;
  Token new_value;
};

struct NodeExitStmt {
  NodeExpr expr;
};

struct NodeMkStmt {
  Token identifier;
  DataType type;
  Token value;
};

struct NodeStmts;

struct NodeForStmt {
  Token identifier;
  Token startValue;
  Token targetValue;
  Token incValue;

  std::vector<NodeStmts> body;
};

struct NodeParam
{
  Token identifier;
  DataType type;
  Token value;
};

struct NodeFuncStmt {
  std::vector<NodeParam> params;
  std::vector<NodeStmts> body;
  size_t param_count = 0;
  Token identifier;
};

struct NodeFuncCall
{
  Token identifier;
  std::vector<NodeParam> params;
  size_t param_count = 0;
};

struct NodeStmts {
  std::variant<NodeExitStmt, NodeMkStmt,NodeReStmt, NodeForStmt, NodeFuncStmt, NodeFuncCall, NodeCallStmt> var;
};

struct NodeProg {
  std::vector<NodeStmts> stmt;
};


struct Var
{
  Token name;
  size_t stackOffset;
  Token value;
};

struct GenPrompt
{
  std::vector<NodeStmts> stmts;
  std::stringstream ss;
  std::vector<Var> scope;
};
