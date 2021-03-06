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

#ifndef DRIFT_LEXER_H
#define DRIFT_LEXER_H

#include <iostream>
#include <vector>

#include "exception.h"
#include "state.h"
#include "token.h"

// lexer structure
class Lexer {
private:
  // current character
  int position = 0;

  // current line
  int line = 1;

  // source code
  std::string source;

  // resolve identifier
  void lexIdent();

  // resolve digit
  void lexDigit();

  // resolve "xxx" string literal
  void lexString(bool longStr);

  // resolve 'x' character literal
  void lexChar();

  // resolve other symbol
  void lexSymbol();

  // return current char of resolve
  inline char now();

  // throw an error
  inline void error(exp::Kind k, std::string m);

  // return next char of resolve
  char peek();

  // judge the current character and process the token
  bool peekEmit(token::Token *t,
                char c,              // current char
                token::Kind k,       // equal token kind
                const std::string &l // equal token literal
  );

  // return resolve is end
  inline bool isEnd();

  // return current char is identifier
  inline bool isIdent();

  // return current char is digit
  inline bool isDigit();

  // return current char is whitespace
  inline bool isSpace();

  // resolve to skip whitespace
  inline void skipWhitespace();

  // resolve to skip line comment
  inline void skipLineComment();

  // resolve to skip block comment
  inline void skipBlockComment();

  std::map<std::string, token::Kind> keyword; // KEYWORDS

  State *state;

public:
  explicit Lexer(std::string source, State *s) : source(std::move(source)) {
    initializeKeyword(&keyword);
    this->state = s;
  }

  // final token list
  std::vector<token::Token> tokens;

  // start
  void tokenizer();

  // final to dissemble tokens list
  void dissembleTokens();
};

#endif