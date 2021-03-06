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

#ifndef DRIFT_VM_H
#define DRIFT_VM_H

#include "exp.hpp"
#include "frame.hpp"
#include "object.hpp"
#include "opcode.hpp"

#include "entity.hpp"

// structure
class vm {
private:
  std::vector<Frame *> frames; // execute frames
  // push object to the current frame
  void pushData(object::Object *);
  // pop the top of data stack
  object::Object *popData();
  // emit new name of table to the current frame
  void emitTable(std::string, object::Object *);
  // look up a name from current top frame
  object::Object *lookUp(std::string);
  // first to end iterator
  object::Object *retConstant();
  // first to end iterator
  ast::Type *retType();
  // first to end iterator
  std::string retName();
  // first to end iterator
  int retOffset();
  // are the comparison types the same
  void typeChecker(ast::Type *, object::Object *);

  int op = 0; // offset pointer

public:
  explicit vm(Entity *main) {
    // to main frame as main
    this->frames.push_back(new Frame(main));
  }

  // top frame
  Frame *top();

  // repl mode to clean pointer for offset
  void clean() { this->op = 0; }

  void evaluate(); // evaluate the top of frame
};

#endif