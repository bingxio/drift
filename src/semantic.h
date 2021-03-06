//
// Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.
//

// GNU General Public License, more to see file: LICENSE
// https://www.gnu.org/licenses

//          THE DRIFT PROGRAMMING LANGUAGE
//
//          https://github.com/bingxio/drift
//
//          https://www.drift-lang.fun/
//

#ifndef DRIFT_SEMANTIC_H
#define DRIFT_SEMANTIC_H

#include <algorithm>

#include "ast.h"
#include "exception.h"
#include "state.h"

// analysis
class Analysis {
private:
  int position = 0;
  // stmts
  std::vector<ast::Stmt *> *statements;

  // return the kind of current statement
  inline ast::Kind look() { return statements->at(position)->kind(); }

  // return the current statement
  inline ast::Stmt *now() { return statements->at(position); }

  State *state;

  // throw semantic analysis exception
  void error(exp::Kind k, std::string message, int line) {
    state->kind = k;
    state->message = message;
    state->line = line;

    throw exp::Exp(state);
  }

public:
  explicit Analysis(std::vector<ast::Stmt *> *stmts, State *state) {
    this->statements = stmts;
    this->state = state;

    while (position < statements->size()) {
      this->analysisStmt(now());
      this->position++;
    }
  }

  // statement
  void analysisStmt(ast::Stmt *stmt);
  // expression
  void analysisExpr(ast::Expr *expr);
};

#endif