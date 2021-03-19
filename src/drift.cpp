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

#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "semantic.hpp"
#include "version.hpp"
#include "vm.hpp"

#include "system.hpp"

// DEBUG to output tokens and statements
bool DEBUG = false;
// repl mode
bool REPL = false;

static std::vector<object::Module *> mods; // global modules

vm *mac; // local

// run source code
void run(std::string source) {
  try {
    // lexer
    auto lex = new Lexer(source);

    lex->tokenizer();
    if (DEBUG) lex->dissembleTokens();

    // parser
    auto parser = new Parser(lex->tokens);

    parser->parse();
    if (DEBUG) parser->dissembleStmts();

    // semantic
    auto semantic = new Analysis(&parser->statements);
    // compiler
    auto compiler = new Compiler(parser->statements);
    compiler->compile();

    for (auto i : compiler->entities) i->dissemble();

    // vm
    if (REPL && mac != nullptr) {
      // save the current symbol table
      mac->top()->entity = compiler->entities[0];
      mac->clean();
    } else {
      // new virtual machine
      mac = new vm(compiler->entities[0], &mods);
    }
    mac->evaluate();
    //
  } catch (exp::Exp &e) {
    std::cout << "\033[31m" << e.stringer() << "\033[0m" << std::endl;
    return;
  }
}

// FILE mode
void runFile(const char *path) {
  std::string s;
  fileString(path, &s);

  run(s);
}

// REPL mode
void repl() {
  REPL = true;

  char *line = (char *)malloc(1024);
  std::cout << "\n" << VERS << "\n" << std::endl;

  while (true) {
    std::cout << "ft >> ";
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
    fileString(i.c_str(), &s);

    try {
      // lexer
      auto lex = new Lexer(s);
      lex->tokenizer();

      // parser
      auto parser = new Parser(lex->tokens);
      parser->parse();

      // semantic
      auto semantic = new Analysis(&parser->statements);
      // compiler
      auto compiler = new Compiler(parser->statements);
      compiler->compile();

      // vm
      (new vm(compiler->entities[0], &mods))->evaluate();
      //
    } catch (exp::Exp &e) {
      std::cout << "\033[31m" << e.stringer() << "\033[0m" << std::endl;
      return false;
    }
  }
  return true; // OK
}

// VER
void version() { std::cout << VERS << std::endl; }

// entry
int main(int argc, char **argv) {
  if (!loadStdModules()) return 1; // load standard modules

  // standard modules
  if (!mods.empty()) {
    for (auto i : mods) {
      std::cout << i->stringer() << std::endl;
    }
  }

  if (argc == 2) {
    if (strcmp(argv[1], "-d") == 0) {
      DEBUG = true;
      repl();
    } else if (strcmp(argv[1], "-v") == 0) {
      version();
    } else {
      runFile(argv[1]);
    }
  } else if (argc == 3) {
    if (strcmp("-d", argv[2]) == 0) {
      DEBUG = true;
    }
    runFile(argv[1]);
  } else {
    repl();
  }
  return 0;
}