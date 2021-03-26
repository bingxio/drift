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

#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h> // LINUX

#elif defined(_WIN32)
#include <windows.h> // WINDOWS

#endif

#include "builtin.h"

// throw an exception
void error(std::string m) {
  State *state = new State;

  state->filePath = "BUILTIN";
  state->kind = exp::RUNTIME_ERROR;
  state->message = m;
  state->line = -1;

  throw exp::Exp(state);
}

// print to screen
void puts(object::Object *obj, Frame *) {
  object::Func *f = static_cast<object::Func *>(obj);

  // new line
  if (f->builtin.empty()) {
    std::cout << std::endl;
    return;
  }

  for (std::vector<object::Object *>::reverse_iterator iter =
           f->builtin.rbegin();
       iter != f->builtin.rend(); iter++) {
    std::cout << (*iter)->stringer() << std::endl;
  }
}

// print to screen but no new line
void put(object::Object *obj, Frame *) {
  object::Func *f = static_cast<object::Func *>(obj);
  for (std::vector<object::Object *>::reverse_iterator iter =
           f->builtin.rbegin();
       iter != f->builtin.rend(); iter++) {
    std::cout << (*iter)->stringer() << "\t";
  }
}

// print to screen and end new line
void putl(object::Object *obj, Frame *) {
  object::Func *f = static_cast<object::Func *>(obj);
  for (std::vector<object::Object *>::reverse_iterator iter =
           f->builtin.rbegin();
       iter != f->builtin.rend(); iter++) {
    std::cout << (*iter)->stringer() << "\t";
  }
  std::cout << std::endl;
}

// return the length of object
void len(object::Object *obj, Frame *f) {
  object::Func *fu = static_cast<object::Func *>(obj);

  if (fu->builtin.empty() || fu->builtin.size() > 1)
    error("the <len> function receives one object");

  obj = fu->builtin.front();

  switch (obj->kind()) {
  case object::ARRAY:
    f->data.push(
        new object::Int(static_cast<object::Array *>(obj)->elements.size()));
    break;
  case object::TUPLE:
    f->data.push(
        new object::Int(static_cast<object::Tuple *>(obj)->elements.size()));
    break;
  case object::MAP:
    f->data.push(
        new object::Int(static_cast<object::Map *>(obj)->elements.size()));
    break;
  case object::STR:
    f->data.push(
        new object::Int(static_cast<object::Str *>(obj)->value.size()));
    break;
  case object::CHAR:
    f->data.push(new object::Int(1));
    break;

  default:
    error(obj->stringer() + " not contain length type");
  }
}

// sleep time
void bsleep(object::Object *obj, Frame *) {
  object::Func *fu = static_cast<object::Func *>(obj);

  if (fu->builtin.empty() || fu->builtin.size() > 1 ||
      fu->builtin.front()->kind() != object::INT)
    error("the <sleep> function receives one <int> object");

  object::Int *i = static_cast<object::Int *>(fu->builtin.front());

#if defined(__linux__) || defined(__APPLE__)
  sleep(i->value);
#elif defined(_WIN32)
  Sleep(i->value);
#endif
}

constexpr int l = 5;                        // length of builtin names
static builtin bu[l] = {{"puts", puts},     // print to screen
                        {"put", put},       // print to screen but no new line
                        {"putl", putl},     // print to screen and end new line
                        {"len", len},       // return the length of object
                        {"sleep", bsleep}}; // sleep time

// return it is builtin function name
bool isBuiltinName(std::string name) {
  for (int i = 0; i < l; i++)
    if (bu[i].name == name)
      return true;
  return false;
}

// if its builtin function to call it
void builtinFuncCall(std::string name, object::Object *obj, Frame *f) {
  for (int i = 0; i < l; i++)
    if (bu[i].name == name)
      bu[i].to(obj, f); // CALL
}

// regist the name of builtin
void regBuiltinName(Frame *f) {
  f->tb.emit("T", new object::Bool(1));
  f->tb.emit("F", new object::Bool(0));

  f->tb.emit("_VERSION_", new object::Str("DRIFT 0.0.1"));
  f->tb.emit("_AUTHOR_", new object::Str("BINGXIO - 黄菁"));
  f->tb.emit("_LICENSE_", new object::Str("GPL 3.0"));
  f->tb.emit("_WEBSITE_", new object::Str("https://drift-lang.fun/"));
}