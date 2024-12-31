#pragma once

#include <cstdarg>
#include <cstddef>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

enum errType {
  ex_Delimiter,
  ex_Expression,
  ex_Integer,
  ex_Type,
  ex_Operator,
  ex_Oparen,
  ex_Obrace,
  ex_Cparen,
  ex_Cbrace,
  ms_Type,
  ms_TypeDef,
  un_Type,
  ex_Func,
  ms_Param,
  ms_Scope,
};

struct Error {
  errType type;
  int line, col;
};

enum TokenType {
  SEMI,
  INT_LIT,
  OPAREN,
  CPAREN,
  IDENT,
  EXIT,
  MK,
  ASSIGN,
  STRING_LIT,
  TYPE_DEC,
  TYPE,
  FOR,
  ADD_EQU, // +=
  LTH,     // <
  N_EQU,   // !=
  EQU,     // ==
  SUB_EQU, // -=
  MUL_EQU, // *=
  DIV_EQU, // /=
  CBRACE,
  OBRACE,
  GTH,     // >
  LTH_EQU, // <=
  GTH_EQU, // >=
  FUNC,
  CALL,
  eof,
  ADD,
  SUB,
  DIV,
  MUL,
  EXTERN,
  WHILE,
  RET,
  IF,
  ELSE,
  R_PTR
};

enum DataType {
  STR,
  INT,
  BOOL, // int 0 and 1 ::: translates bools into a possible 0 or 1. 0 beign
        // false, 1 beign true
  R_PTR_T,
};

struct Token {
  std::optional<std::string> value;
  TokenType type;
  int line, col;
  bool is_ptr = false;
};

struct NodeInt {
  Token value;
};

struct NodeString {
  Token value;
};

struct NodeBinaryExpr;
struct NodeExpr;

struct NodeReStmt {
  Token identifier;
  std::shared_ptr<NodeExpr> new_value;
};

struct NodeExitStmt {
  std::shared_ptr<NodeExpr> expr;
};

struct NodeMkStmt {
  Token identifier;
  DataType type;
  std::shared_ptr<NodeExpr> value;
};

struct NodeStmts;
struct NodeFuncCall;

struct NodeCmp {
  std::variant<std::shared_ptr<NodeFuncCall>, NodeInt> lhs;
  std::string cmp_s;
  std::variant<std::shared_ptr<NodeFuncCall>, NodeInt> rhs;
};

struct NodeForStmt {
  Token identifier;
  Token startValue;
  Token targetValue;
  Token incValue;

  std::vector<NodeStmts> body;
};

struct NodeWhileStmt {
  std::vector<NodeCmp> comparisons;
  std::vector<NodeStmts> body;
};

struct NodeIfStmt {
  Token identifier;
  std::shared_ptr<NodeCmp> condition;
  std::vector<NodeStmts> trueBody;
  std::vector<NodeStmts> falseBody;
  bool has_else = false;
};

struct Var {
  Token name;
  size_t stackOffset;
  Token value;
  bool is_prt = false;
};

struct NodeParam {
  Token identifier;
  DataType type;
  Token value;
  Var toVar() { return Var{.name = identifier, .value = value}; }
};

struct NodeRet {
  Token value;
};

struct NodeFuncStmt {
  std::vector<NodeParam> params;
  std::vector<NodeStmts> body;
  DataType ret_type;
  size_t param_count = 0;
  Token identifier;
  NodeRet ret_value;
  bool has_ret = false;
};

struct NodeFuncCall {
  Token identifier;
  std::vector<NodeParam> params;
  size_t param_count = 0;
};

struct NodeExtrnStmt {
  Token identifier;
  std::vector<NodeParam> param;
  size_t param_count = 0;
};

struct NodeCallStmt {
  Token std_lib_value;
  std::vector<NodeParam> params;
};

struct NodeExpr {
  std::variant<std::shared_ptr<std::vector<NodeBinaryExpr>>, NodeInt,
               NodeString, std::shared_ptr<NodeFuncCall>, NodeCmp>
      var;
};

struct NodeBinaryExpr {
  NodeInt lhs;
  std::string op;
  NodeInt rhs;
};
struct NodeStmts {
  std::variant<NodeExitStmt, NodeMkStmt, NodeReStmt, NodeForStmt, NodeFuncStmt,
               NodeFuncCall, NodeCallStmt, NodeExtrnStmt, NodeWhileStmt,
               NodeIfStmt>
      var;
};

struct NodeProg {
  std::vector<NodeStmts> stmt;
};

struct GenPrompt {
  std::vector<NodeStmts> stmts;
  std::stringstream ss;
  std::vector<Var> scope;
};
