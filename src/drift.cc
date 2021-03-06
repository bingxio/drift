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

#include <cstring>
#include <filesystem>
#include <fstream>

#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "version.h"
#include "vm.h"

#include "state.h"
#include "system.h"

// DEBUG to output tokens and statements
bool DEBUG = false;
// repl mode
bool REPL = false;
// dis mode
bool DIS = false;

static State state;                        // global state
static std::vector<object::Module *> mods; // global modules

vm *mac; // local

// run source code
void run(std::string source) {
  try {
    // lexer
    auto lex = new Lexer(source, &state);

    lex->tokenizer();
    if (DEBUG)
      lex->dissembleTokens();

    // parser
    auto parser = new Parser(lex->tokens, &state);

    parser->parse();
    if (DEBUG)
      parser->dissembleStmts();

    // semantic
    auto semantic = new Analysis(&parser->statements, &state);
    // compiler
    auto compiler = new Compiler(parser->statements, parser->lineno);
    compiler->compile();

    if (DIS)
      for (auto i : compiler->entities)
        i->dissemble();

    // vm
    if (REPL && mac != nullptr) {
      // save the current symbol table
      mac->top()->entity = compiler->entities[0];
      mac->clean();
    } else {
      // new virtual machine
      mac = new vm(compiler->entities[0], &mods, REPL, DIS, &state);
    }
    mac->evaluate();

    // if (DIS)
    // mac->main()->tb.dissemble();
    //
  } catch (exp::Exp &e) {
    std::cout << e.stringer() << std::endl;
    return;
  }
}

// FILE mode
void runFile(const char *path) {
  std::string s;
  if (!fileString(path, &s))
    return;

  state.filePath = std::string(path); // current read file

  run(s);
}

// REPL mode
void repl() {
  REPL = true;

  char *line = (char *)malloc(1024);
  std::cout << VERS << std::endl;
  int count = 1;

  while (true) {
    // std::cout << count++ << " >> ";
    std::cout << "> ";
    std::cin.getline(line, 1024);

    if (strlen(line) == 0) {
      continue;
    }
    run(line);
  }
}

// load standard modules
bool loadStdModules() {
  std::vector<std::string> *fs =
      getAllFileWithPath(std::filesystem::current_path().string() + "/std");

  for (auto i : *fs) {
    std::string s;
    if (!fileString(i.c_str(), &s))
      return false;

    try {
      // lexer
      auto lex = new Lexer(s, &state);
      lex->tokenizer();

      // parser
      auto parser = new Parser(lex->tokens, &state);
      parser->parse();

      // semantic
      auto semantic = new Analysis(&parser->statements, &state);
      // compiler
      auto compiler = new Compiler(parser->statements, parser->lineno);
      compiler->compile();

      // vm
      (new vm(compiler->entities[0], &mods, REPL, DIS, &state))->evaluate();
      //
    } catch (exp::Exp &e) {
      // std::cout << "\033[31m" << e.stringer() << "\033[0m" << std::endl;
      std::cout << e.stringer() << std::endl;
      return false;
    }
  }
  return true; // OK
}

// VER
void version() { std::cout << VERS << std::endl; }

// entry
int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "-v") == 0) {
    version();
    return 0;
  }

  if (!loadStdModules())
    return 1; // load standard modules

  if (argc == 2) {
    // D
    if (strcmp(argv[1], "-d") == 0) {
      DEBUG = true;
      repl();
    }
    // B
    else if (strcmp(argv[1], "-b") == 0) {
      DIS = true;
      repl();
    }
    // O
    else {
      runFile(argv[1]);
    }
  } else if (argc == 3) {
    if (strcmp("-d", argv[2]) == 0) {
      DEBUG = true;
    }
    if (strcmp("-b", argv[2]) == 0) {
      DIS = true;
    }
    runFile(argv[1]);
  } else {
    repl();
  }
  return 0;
}