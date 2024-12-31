#pragma once

#include "asmgen.hpp"
#include "eggoLog.hpp"
#include "token.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

class Parser {
public:
  inline Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {
    Logger::Trace("Parsing");
    while (peek().has_value()) {
      // if(peek().value().type == TokenType::EXIT)
      //{
      //   consume();
      // parse_exit_stmt();
      //}
      if (peek().value().type == TokenType::MK) {
        // consume();
        parse_mk_stmt(stmts.stmt);
      }
      if (peek().value().type == TokenType::IDENT) {
        // consume();
        parse_re_assign_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::EXTERN) {
        parse_extrn_stmt(stmts.stmt);
      }
      if (peek().value().type == TokenType::FOR) {
        Logger::Trace("Found For");
        parse_for_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::FUNC) {
        parse_func_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::CALL) {
        parse_call_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::IF) {
        parse_if_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::WHILE) {
        parse_while_stmt(stmts.stmt);
      }
      if (peek()->type == TokenType::eof) {
        break;
      } else {
        // consume();
        //  e
      }
    }

    AsmGen gen(stmts);
  }
  inline ~Parser() {}

private:
  std::vector<Token> m_tokens;
  size_t m_index = 0;
  NodeProg stmts;

  inline std::optional<Token> peek(int i = 0) {
    if ((m_index + i) >= m_tokens.size()) {
      return {};
    }
    return m_tokens.at(m_index + i);
  }

  inline Token consume() { return m_tokens.at(m_index++); }

  inline void parse_call_stmt(std::vector<NodeStmts> &stmts) {
    Logger::Trace("Parsing Call Stmt");
    NodeStmts stmt;
    NodeCallStmt call;
    consume(); // call
    if (peek().has_value() && peek()->type == TokenType::IDENT) {
      call.std_lib_value = consume();
      // stmt.var = call;
      // stmts.push_back(stmt);
      if (peek().has_value() && peek()->type == OPAREN) {
        consume();

        while (peek().has_value() && peek()->type != CPAREN) {
          NodeParam param;
          Token tkn;
          if (peek()->type == R_PTR) {
            consume();
            if (peek().has_value()) {
              tkn = consume();
              tkn.is_ptr = true;
            }
          } else
            tkn = consume();
          param.value = tkn;
          call.params.push_back(param);
        }
        if (peek().value().type == TokenType::CPAREN) {
          consume();
          if (peek().value().type == TokenType::SEMI) {
            consume();
            stmt.var = call;
            stmts.push_back(stmt);
          }
        }
      }
    }
  }

  void parse_exit_stmt() {
    NodeStmts stmt;
    NodeExitStmt exitstmt;

    /*if (peek().value().type == TokenType::OPAREN) {*/

    /*             peek().value().type == TokenType::IDENT) {*/
    /*    exitstmt.expr.var = consume();*/
    /*    if (peek().has_value() && peek().value().type == TokenType::CPAREN)
     * {*/
    /*      consume();*/
    /*      if (peek().has_value() && peek().value().type == TokenType::SEMI)
     * {*/
    /*        stmt.var = exitstmt;*/
    /*        stmts.stmt.push_back(stmt);*/
    /*      } else {*/
    /*        Logger::Error({.type = errType::ex_Delimiter,*/
    /*                       .line = peek(-1).value().line,*/
    /*                       .col = peek(-1)->col});*/
    /*        exit(1);*/
    /*      }*/
    /*    } else {*/
    /*      Logger::Error({.type = errType::ex_Cparen, .line =
     * peek(-1)->line});*/
    /*      exit(1);*/
    /*    }*/
    /*  }*/
    /*} else {*/
    /*  Logger::Error({.type = errType::ex_Oparen, .line = peek(-1)->line});*/
    /*  exit(1);*/
    /*}*/
  }

  inline void parse_mk_stmt(std::vector<NodeStmts> &stmts) {
    int cc = 0;
    NodeStmts stmt;
    NodeMkStmt mkstmt;
    consume(); // discarding "MK" keyword
    Logger::Trace("Parsing Mk stmt");
    if (peek().has_value() && peek()->type == TokenType::IDENT) {
      mkstmt.identifier = consume();
      if (peek().has_value() && peek()->type == TokenType::TYPE_DEC) {
        consume();
        if (peek().has_value() && peek()->type == TokenType::TYPE) {
          Logger::Trace("Parsing Mk stmt type");
          consume();
          if (peek().has_value() && peek()->type == TokenType::ASSIGN) {
            consume();
            if (peek().has_value()) {
              mkstmt.value =
                  std::make_shared<NodeExpr>(parse_expression(stmts));
              Logger::Trace("Parsed expression");
            }
            if (peek().has_value() && peek()->type == SEMI) {
              consume();
              printf("---\n");
            }
            stmt.var = mkstmt;
            stmts.push_back(stmt);
          }
        }
      }
    }
  }

  inline NodeFuncCall parse_re_assign_stmt(std::vector<NodeStmts> &stmts,
                                           bool push_back = true,
                                           bool check = true) {
    NodeStmts stmt;
    NodeReStmt restmt;

    if (peek(1).has_value() && peek(1).value().type == TokenType::ASSIGN) {
      restmt.identifier = consume();

      consume();
      if (peek().has_value() && peek().value().type == TokenType::INT_LIT ||
          peek().value().type == TokenType::IDENT) {
        if (peek()->type == INT_LIT)
          restmt.new_value =
              std::make_shared<NodeExpr>(parse_expression(stmts));
        else
          restmt.new_value =
              std::make_shared<NodeExpr>(parse_expression(stmts));

        if (peek().has_value() && peek().value().type == TokenType::SEMI) {
          consume();
          stmt.var = restmt;
          stmts.push_back(stmt);
        } else {
          Logger::Error(
              {.type = errType::ex_Delimiter, .line = peek(-1)->line});
          exit(1);
        }
      } else {
        Logger::Error({.type = errType::ex_Expression, .line = peek(-1)->line});
        exit(1);
      }
    } else if (peek(1).has_value() && peek(1)->type == TokenType::OPAREN) {
      Logger::Trace("function call");
      NodeFuncCall fcall;
      int param_count = 0;
      fcall.identifier = consume();
      consume(); // opening parenthesis
      if (peek().has_value() && peek()->type != TokenType::CPAREN) {
        Logger::Trace("Entering Call parenthesis");
        while (peek().has_value() && peek()->type != TokenType::CPAREN) {
          Logger::Trace("Collection call Params");
          Token tkn;
          if (peek()->type == R_PTR) {
            Logger::Trace("From FOR : found pointer");
            consume();
            if (peek().has_value()) {
              tkn = consume();
              tkn.is_ptr = true;
            }
          } else {
            tkn = consume();
          }
          NodeParam param = {.value = tkn};
          fcall.params.push_back(param);
          param_count++;
        }
      }
      if (peek().has_value() && peek()->type == TokenType::CPAREN &&
          peek(1).has_value()) {
        Logger::Trace("No Params Were given");
        consume(); // )
        if (peek()->type == SEMI && check) {
          Logger::Trace("Discard");
          consume();
        } // ;
        fcall.param_count = param_count;
        stmt.var = fcall;
        if (push_back)
          stmts.push_back(stmt);
        Logger::Trace("Done function call");
        return fcall;
      }
    }
    return {};
  }

  inline void parse_for_stmt(std::vector<NodeStmts> &stmts) {
    NodeStmts stmt;
    NodeForStmt for_stmt;
    std::vector<NodeStmts> body;
    consume();
    Logger::Trace("Parsing For Loop");
    // -----------------  for (i : int = 0; i < 10; i += 1) ---------------

    if (peek().has_value() && peek()->type == OPAREN) {
      consume();
      if (peek().has_value() && peek()->type != CPAREN) {
        if (peek()->type == IDENT) {
          for_stmt.identifier = consume();
          if (peek().has_value() && peek()->type == TYPE_DEC &&
              peek(1).has_value() && peek(1)->type == TYPE) {
            consume();
            consume();

            if (peek().has_value() && peek()->type == ASSIGN)
              consume();
            else
              Logger::Error(
                  {.type = errType::ex_Operator, .line = peek(-1)->line});

            if (peek().has_value() && peek()->type == INT_LIT) {
              for_stmt.startValue = consume();
              if (peek().has_value() && peek()->type == SEMI) {
                consume();
                if (peek().has_value() && peek()->type == IDENT) {
                  if (peek()->value.value() !=
                      for_stmt.identifier.value.value()) {
                    printf("Error :  Unknown Identifier : %s\n",
                           peek()->value.value().c_str());
                    exit(1);
                  }
                  consume();
                  if (peek().has_value() && peek()->type == LTH) {
                    consume();
                    if (peek().has_value() && peek()->type == INT_LIT) {
                      for_stmt.targetValue = consume();
                      if (peek().has_value() && peek()->type == SEMI) {
                        consume();

                        if (peek().has_value() && peek()->type == IDENT) {
                          if (peek()->value.value() !=
                              for_stmt.identifier.value.value().c_str()) {
                            printf("Error : UnKnown Identifier : %s\n",
                                   peek()->value.value().c_str());
                            exit(1);
                          }
                          consume(); // TODO check for match
                          if (peek().has_value() && peek()->type == ADD_EQU) {
                            consume();
                            if (peek().has_value() && peek()->type == INT_LIT) {
                              for_stmt.incValue = consume();
                              if (peek().has_value() &&
                                  peek()->type == CPAREN) {
                                consume();
                                if (peek().has_value() &&
                                    peek()->type == OBRACE) {
                                  consume();
                                  Logger::Trace("Parsing For body");
                                  while (peek().has_value() &&
                                         peek()->type != CBRACE) {
                                    switch (peek()->type) {
                                    case MK:
                                      parse_mk_stmt(for_stmt.body);
                                      break;
                                    case IDENT:
                                      parse_re_assign_stmt(for_stmt.body);
                                      break;
                                    case FOR:
                                      parse_for_stmt(for_stmt.body);
                                      break;
                                    case FUNC:
                                      parse_func_stmt(for_stmt.body);
                                      break;
                                    case CALL:
                                      parse_call_stmt(for_stmt.body);
                                      break;
                                    default:
                                      break;
                                    }
                                  }

                                  if (peek().has_value() &&
                                      peek()->type == CBRACE) {
                                    consume();
                                    Logger::Trace("Discard }");
                                    if (peek().has_value() &&
                                        peek()->type == SEMI) {
                                      consume();
                                      Logger::Trace("Discard ;");
                                      stmt.var = for_stmt;
                                      stmts.push_back(stmt);
                                      Logger::Trace("For Loop End");
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      } else
                        Logger::Error({.type = errType::ex_Delimiter,
                                       .line = peek(-1)->line});
                    } else
                      Logger::Error({.type = errType::ex_Expression,
                                     .line = peek(-1)->line});
                  } else
                    Logger::Error(
                        {.type = errType::ex_Operator, .line = peek(-1)->line});
                }
              } else {
                Logger::Error(
                    {.type = errType::ex_Delimiter, .line = peek(-1)->line});
              }
            } else
              Logger::Error({.type = ex_Expression, .line = peek(-1)->line});
          } else
            Logger::Error({.type = ms_Type, .line = peek(-1)->line});
        } else
          Logger::Error(
              {.type = errType::ex_Expression, .line = peek(-1)->line});
        // else {
        //  Logger::Error({.type = errType::ex_Expression, .line =
        //  peek(-1)->line});
        //}
      }
    }
  }

  inline void parse_func_stmt(std::vector<NodeStmts> &stmts) {
    Logger::Trace("Parsing Function");
    NodeStmts stmt;
    NodeFuncStmt func_stmt;
    consume(); // dicarding mk keyword
    int param_count = 0;
    if (peek()->type == TokenType::IDENT)
      func_stmt.identifier = consume();
    if (peek().has_value() && peek()->type == TokenType::OPAREN) {
      consume();
      if (peek()->type != CPAREN) {

        while (peek().has_value() && peek()->type != TokenType::CPAREN) {
          NodeParam param;
          if (peek()->type == TokenType::IDENT) {
            param.identifier = consume();
            if (peek().has_value() && peek()->type == TokenType::TYPE_DEC) {
              consume();
              if (peek().has_value() && peek()->type == TokenType::TYPE) {
                Logger::Trace("Collecting Func params");
                if (peek()->value == "str") {
                  param.type = DataType::STR;
                } else {
                  param.type = DataType::INT;
                }
                consume();
                func_stmt.params.push_back(param);
                param_count++;
              }
            }
          } else if (peek()->type == CPAREN)
            break;
        }
      }
      if (peek()->type == CPAREN)
        consume(); // closing parenthesis
                   //
      if (peek().has_value() && peek()->type == TYPE_DEC) {
        consume();
        if (peek().has_value() && peek()->type == TYPE) {
          Logger::Trace("%s ret type :: %s",
                        func_stmt.identifier.value.value().c_str(),
                        peek()->value->c_str());
          func_stmt.has_ret = true;
          if (peek().has_value() && peek()->value == "str") {
            func_stmt.ret_type = DataType::STR;
          } else if (peek().has_value() && peek()->value == "int") {
            func_stmt.ret_type = DataType::INT;
          }

          consume();
        }
      }
      if (peek().has_value() && peek()->type == TokenType::OBRACE) {
        consume();
      }
      if (peek()->type != CBRACE) {
        while (peek().has_value() && peek()->type != TokenType::CBRACE) {
          if (peek()->type == TokenType::MK) {
            parse_mk_stmt(func_stmt.body);
          } else if (peek()->type == TokenType::IDENT) {
            parse_re_assign_stmt(func_stmt.body);
            Logger::Trace("Found Nested func call");
          } else if (peek()->type == TokenType::FOR) {
            parse_for_stmt(func_stmt.body);
            Logger::Trace("Found Nested For");
          } else if (peek()->type == TokenType::EXIT) {
            parse_exit_stmt(); // ------    out of support ------- //
          } else if (peek()->type == TokenType::FUNC) {
            parse_func_stmt(func_stmt.body);
          } else if (peek()->type == TokenType::CALL) {
            parse_call_stmt(func_stmt.body);
          } else if (peek()->type == WHILE) {
            parse_while_stmt(func_stmt.body);
          } else if (peek()->type == IF) {
            parse_if_stmt(func_stmt.body);
          } else if (peek()->type == CBRACE) {
            break;
          } else if (peek()->type == RET) {
            Logger::Trace("Parsing ret value");
            consume();
            NodeRet ret;
            switch (peek().value().type) {
            case INT_LIT:
              if (func_stmt.ret_type != DataType::INT) {
                Logger::Error({.type = ms_Type,
                               .line = peek(-1)->line,
                               .col = peek(-1)->col});
                exit(1);
              }
              break;
            case STRING_LIT:
              if (func_stmt.ret_type != DataType::STR) {
                Logger::Error({.type = ms_Type,
                               .line = peek(-1)->line,
                               .col = peek(-1)->col});
                exit(1);
              }
              break;
            case IDENT:
              break;

            default:
              break;
            }
            ret.value = consume();
            printf("from Parser : ret value : %s\n",
                   ret.value.value.value().c_str());
            func_stmt.ret_value = ret;
            if (peek().has_value() && peek()->type == SEMI)
              consume();
          } // consume();
        }
      }
      if (peek()->type == CBRACE) {
        consume(); // closing curly
        if (peek()->type == SEMI)
          consume();
      }
      // consume();
      func_stmt.param_count = param_count;
      stmt.var = func_stmt;
      stmts.push_back(stmt);
    }
  }
  inline NodeExpr parse_expression(std::vector<NodeStmts> &stmts) {
    NodeExpr expr;
    NodeStmts stmt;
    // Parsing general Experession
    if (peek().has_value() && peek()->type == TokenType::IDENT) {
      expr.var = std::make_shared<NodeFuncCall>(
          parse_re_assign_stmt(stmts, false, false));
      Logger::Trace("found mk func-ret expr");
    } else if (peek().has_value() && peek()->type == TokenType::INT_LIT) {
      if (peek(1).has_value() && peek(1)->type != SEMI)
        expr.var = std::make_shared<std::vector<NodeBinaryExpr>>(parse_bexpr());
      else
        expr.var = NodeInt{.value = consume()};
    } else if (peek().has_value() && peek()->type == TokenType::STRING_LIT) {
      expr.var = NodeString{.value = consume()};
      Logger::Trace("Expr :: string_lit");
    }
    Logger::Trace("done");
    return expr;
  }
  std::vector<NodeBinaryExpr> parse_bexpr() {
    std::vector<NodeBinaryExpr> b_expr;
    NodeExpr expr;
    Logger::Trace("Parsing BinaryExpr");

    NodeBinaryExpr ex;
    bool lhs = true;
    while (peek().has_value() && peek()->type != SEMI) {
      if (peek().has_value() && peek()->type == INT_LIT) {
        if (lhs) {
          ex.lhs = {consume()};
          lhs = false;
        } else {
          ex.rhs = {consume()};
          lhs = true;
        }
      } else {

        switch (peek().value().type) {
        case TokenType::MUL:
          ex.op = "*";
          lhs = true;
          break;
        default:
          break;
        }
        consume();
      }
      b_expr.push_back(std::move(ex));
    }
    return b_expr;
  }
  inline void parse_extrn_stmt(std::vector<NodeStmts> &stmts) {
    NodeExtrnStmt extrnStmt;
    NodeStmts stmt;
    consume(); // dmk
    if (peek().has_value() && peek()->type == TokenType::IDENT) {
      extrnStmt.identifier = consume();
      if (peek().has_value() && peek()->type == OPAREN) {
        consume();
        while (peek().has_value() && peek()->type != CPAREN) {
          NodeParam param;
          if (peek()->type == IDENT) {
            param.identifier = consume();
            if (peek().has_value() && peek()->type == TYPE_DEC) {
              consume();
              if (peek().has_value() && peek()->type == TYPE) {
                param.type =
                    consume().type == INT_LIT ? DataType::INT : DataType::STR;
              }
              extrnStmt.param.push_back(param);
            }
          }
        }
        if (peek().has_value() && peek()->type == CPAREN) {
          consume();
        }
        if (peek().has_value() && peek()->type == SEMI) {
          consume();
          stmt.var = extrnStmt;
          stmts.push_back(stmt);
        }
      }
    }
  }
  inline void parse_while_stmt(std::vector<NodeStmts> &stmts) {

    /* NodeStmts stmt; */
    /* Logger::Trace("Parsing While"); */
    /* consume(); // while */
    /* if (peek().has_value() && peek()->type == OPAREN) { */
    /*   consume(); */
    /*   Logger::Trace("Checking While Conditions"); */
    /*   while (peek().has_value() && peek()->type != CPAREN) { */
    /*     NodeCmp cmp; */
    /*     if (peek().has_value() && peek()->type == IDENT) { */
    /*       if (peek(1).has_value() && peek(1)->type == LTH) { */
    /*         cmp.lhs = consume(); */
    /*         switch (peek().value().type) { */
    /*         case LTH: */
    /*           cmp.cmp_s = "<"; */
    /*           break; */
    /*         case GTH: */
    /*           cmp.cmp_s = ">"; */
    /*           break; */
    /*         case N_EQU: */
    /*           cmp.cmp_s = "!="; */
    /*           break; */
    /*         default: */
    /*           break; */
    /*         } */
    /*         consume(); */
    /*         if (peek().has_value() && peek()->type == INT_LIT) { */
    /*           cmp.rhs = consume(); */
    /*           while_Stmt.comparisons.push_back(cmp); */
    /*         } */
    /*       } */
    /*     } */
    /*   } */
    /*   if (peek().has_value() && peek()->type == CPAREN) { */
    /*     consume(); */
    /*     if (peek().has_value() && peek()->type == OBRACE) { */
    /*       consume(); */
    /*       Logger::Trace("Parsing While Body"); */
    /*       if (peek().has_value() && peek()->type == CBRACE) { */
    /*         while (peek().has_value() && peek()->type != CBRACE) { */
    /*           Logger::Trace("In the loop"); */
    /*           switch (peek()->type) { */
    /*           case MK: */
    /*             parse_mk_stmt(while_Stmt.body); */
    /*             break; */
    /*           case FUNC: */
    /*             parse_func_stmt(while_Stmt.body); */
    /*             break; */
    /*           case IDENT: */
    /*             parse_re_assign_stmt(while_Stmt.body); */
    /*             break; */
    /*           case CALL: */
    /*             parse_call_stmt(while_Stmt.body); */
    /*             break; */
    /*           case FOR: */
    /*             parse_for_stmt(while_Stmt.body); */
    /*             break; */
    /*           case CPAREN: */
    /*             break; */
    /*           default: */
    /*             Logger::Trace("undefined"); */
    /*             break; */
    /*           } */
    /*         } */
    /*         Logger::Trace("Done with while body"); */
    /*         if (peek().has_value() && peek()->type == CBRACE) { */
    /*           consume(); */
    /*           if (peek().value().type == SEMI) { */
    /*             consume(); */
    /*             stmt.var = while_Stmt; */
    /*             stmts.push_back(stmt); */
    /*             Logger::Trace("Done With While"); */
    /*           } */
    /*         } */
    /*       } */
    /*     } */
    /*   } */
    /* } */
    /* } */
  }
  void parse_if_stmt(std::vector<NodeStmts> &stmts) {
    NodeIfStmt if_stmt;
    NodeStmts stmt;

    if (peek().has_value() && peek()->type == TokenType::IF) {
      consume();
      if (peek().has_value() && peek()->type == OPAREN) {
        consume();
        if_stmt.condition =
            std::make_shared<NodeCmp>(parse_condition(if_stmt.trueBody));
        printf("Done with condition\n");
        if (peek().has_value() && peek()->type == CPAREN) {
          consume();
          if (peek().has_value() && peek()->type == OBRACE) {
            consume();
            printf("Parsing IF body\n");
            while (peek().has_value() && peek()->type != CBRACE) {
              switch (peek()->type) {
              case MK:
                parse_mk_stmt(if_stmt.trueBody);
                break;
              case FOR:
                parse_for_stmt(if_stmt.trueBody);
                break;

              case IDENT:
                parse_re_assign_stmt(if_stmt.trueBody);
                break;
              default:
                printf(
                    "Un supported or Invlaid Stmt\nTerminate with failure\n");
                exit(-1);
                break;
              }
            }
            if (peek().has_value() && peek()->type == CBRACE) {
              consume();
              if (peek().value().type == SEMI) {
                consume();
                stmt.var = if_stmt;
                stmts.push_back(stmt);
              } else {
                if (peek().value().type == TokenType::ELSE) {
                  consume();
                  Logger::Trace("Parsing Else Block");
                  if_stmt.has_else = true;
                  if (peek().has_value() && peek()->type == OBRACE)
                    consume();
                  while (peek().has_value() && peek()->type != CBRACE) {
                    switch (peek()->type) {
                    case MK:
                      parse_mk_stmt(if_stmt.falseBody);
                      break;
                    case FOR:
                      parse_for_stmt(if_stmt.falseBody);
                      break;

                    case IDENT:
                      parse_re_assign_stmt(if_stmt.falseBody);
                      break;
                    default:
                      printf("Un supported or Invlaid Stmt\nTerminate with "
                             "failure\n");
                      exit(-1);
                      break;
                    }
                  }
                  if (peek().has_value() && peek()->type == CBRACE) {
                    consume();
                    if (peek().has_value() && peek()->type == SEMI) {
                      consume();
                      stmt.var = if_stmt;
                      stmts.push_back(stmt);
                    }
                  }
                }
              }
            }
          } else {
            printf("Fatal : expected Matching parenthesis\n");
            exit(1);
          }
        }
      }
    }
  }
  NodeCmp parse_condition(std::vector<NodeStmts> &stmts) {
    NodeCmp cmp;
    if (peek().has_value() && peek()->type == INT_LIT) {
      cmp.lhs = NodeInt{.value = consume()};
    } else if (peek().has_value() && peek()->type == IDENT) {
      cmp.lhs = std::make_shared<NodeFuncCall>(
          parse_re_assign_stmt(stmts, false, false));

      Logger::Trace("lhs is funccall");

    } else {
      printf("Fatal : Expected and Expression");
      exit(1);
    }

    if (peek().has_value() && peek()->type == EQU) {
      Logger::Trace("found equ");
      cmp.cmp_s = "==";
      consume();
    }

    if (peek().has_value() && peek()->type == INT_LIT) {
      cmp.rhs = NodeInt{.value = consume()};
    } else if (peek().has_value() && peek()->type == IDENT) {
      cmp.rhs = std::make_shared<NodeFuncCall>(
          parse_re_assign_stmt(stmts, false, false));

      Logger::Trace("rhs is funccall");

    } else {
      printf("Fatal : Expected and Expression");
      exit(1);
    }

    return cmp;
  }
}

;
