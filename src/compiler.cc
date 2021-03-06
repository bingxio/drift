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

#include "compiler.h"

// return the current statement
ast::Stmt *Compiler::look() { return this->statements.at(this->position); }

// compile statements to entities
void Compiler::compile() {
  // std::cout << "LINE: " << this->line << std::endl;

  while (this->position < this->statements.size()) {
    this->line = this->lineno.at(this->position);

    this->stmt(look()); // START
    this->position++;
  }
  this->emitCode(byte::RET); // RET
}

// push bytecode to entity
void Compiler::emitCode(byte::Code co) {
  this->now->codes.push_back(co);
  this->now->lineno.push_back(this->line);
}

// push offset to entity
void Compiler::emitOffset(int off) { this->now->offsets.push_back(off); }

// push constant to entity
void Compiler::emitConstant(object::Object *obj) {
  this->now->constants.push_back(obj);
  this->emitOffset(this->icf++);
}

// push name to entity
void Compiler::emitName(std::string v) {
  std::vector<std::string>::iterator iter =
      std::find(now->names.begin(), now->names.end(), v);
  if (iter != now->names.end()) {
    // found
    this->emitOffset(
        std::distance(now->names.begin(), iter)); // only push offset
  } else {
    // not found
    this->now->names.push_back(v); // push new name
    this->emitOffset(this->inf++); // push new offset
  }
}

// push names type to entity
void Compiler::emitType(Type *t) {
  this->now->types.push_back(t);
  this->emitOffset(this->itf++);
}

void Compiler::emitJumpOffset(int off) {
  this->now->jumpOffsets.push_back(off);
}

// insert position with current counts of bytecode
void Compiler::insertPosOffset(int pos) {
  this->now->offsets.insert(now->offsets.begin() + pos, now->codes.size());
  this->emitJumpOffset(now->codes.size());
}

// with custom value
void Compiler::insertPosOffset(int pos, int val) {
  this->now->offsets.insert(now->offsets.begin() + pos, val);
  this->emitJumpOffset(val);
}

// replace placeHolder
void Compiler::replaceHolder(int original) {
  for (std::vector<int>::iterator iter = now->offsets.begin();
       iter != now->offsets.end(); iter++) {
    // out statement
    if (*iter == -1) {
      *iter = now->codes.size();
      this->emitJumpOffset(now->codes.size());
    }
    // tin statement
    if (*iter == -2) {
      *iter = original;
      this->emitJumpOffset(original);
    }
  }
}

// expression
void Compiler::expr(ast::Expr *expr) {
  switch (expr->kind()) {
  case ast::EXPR_LITERAL: {
    ast::LiteralExpr *l = static_cast<ast::LiteralExpr *>(expr);
    token::Token tok = l->token;

    if (tok.kind == token::NUM) {
      this->emitConstant(new object::Int(std::stoi(tok.literal)));
    }
    if (tok.kind == token::FLOAT) {
      this->emitConstant(new object::Float(std::stof(tok.literal)));
    }
    if (tok.kind == token::STR) {
      this->emitConstant(
          // judge long characters at here
          new object::Str(tok.literal, tok.literal.back() == '`'));
    }
    if (tok.kind == token::CHAR) {
      this->emitConstant(new object::Char(tok.literal.at(0)));
    }
    this->emitCode(byte::CONST);
  } break;
  case ast::EXPR_BINARY: {
    ast::BinaryExpr *b = static_cast<ast::BinaryExpr *>(expr);

    this->expr(b->left);
    this->expr(b->right);

    switch (b->op.kind) {
    case token::ADD:
    case token::AS_ADD:
      this->emitCode(byte::ADD);
      break;
    case token::SUB:
    case token::AS_SUB:
      this->emitCode(byte::SUB);
      break;
    case token::MUL:
    case token::AS_MUL:
      this->emitCode(byte::MUL);
      break;
    case token::DIV:
    case token::AS_DIV:
      this->emitCode(byte::DIV);
      break;
    case token::SUR:
    case token::AS_SUR:
      this->emitCode(byte::SUR);
      break;

    case token::GREATER:
      this->emitCode(byte::GR);
      break;
    case token::LESS:
      this->emitCode(byte::LE);
      break;
    case token::GR_EQ:
      this->emitCode(byte::GR_E);
      break;
    case token::LE_EQ:
      this->emitCode(byte::LE_E);
      break;
    case token::EQ_EQ:
      this->emitCode(byte::E_E);
      break;
    case token::BANG_EQ:
      this->emitCode(byte::N_E);
      break;
    case token::ADDR:
      this->emitCode(byte::AND);
      break;
    case token::OR:
      this->emitCode(byte::OR);
      break;
    }

    if (b->op.kind == token::AS_ADD || b->op.kind == token::AS_SUB ||
        b->op.kind == token::AS_MUL || b->op.kind == token::AS_DIV ||
        b->op.kind == token::AS_SUR) {
      ast::NameExpr *n = static_cast<ast::NameExpr *>(b->left);

      this->emitName(n->token.literal);
      this->emitCode(byte::ASSIGN);
    }
  } break;
  //
  case ast::EXPR_GROUP: {
    ast::GroupExpr *g = static_cast<ast::GroupExpr *>(expr);

    this->expr(g->expr);
  } break;
  //
  case ast::EXPR_UNARY: {
    ast::UnaryExpr *u = static_cast<ast::UnaryExpr *>(expr);

    this->expr(u->expr);

    if (u->token.kind == token::BANG) {
      this->emitCode(byte::BANG);
    }
    if (u->token.kind == token::SUB) {
      this->emitCode(byte::NOT);
    }
  } break;
  //
  case ast::EXPR_NAME: {
    ast::NameExpr *n = static_cast<ast::NameExpr *>(expr);

    this->emitCode(byte::LOAD);
    this->emitName(n->token.literal); // new name
  } break;
  //
  case ast::EXPR_CALL: {
    ast::CallExpr *c = static_cast<ast::CallExpr *>(expr);

    this->expr(c->callee);

    for (int i = c->arguments.size(); i > 0; i--)
      this->expr(c->arguments.at(i - 1)); // arguments

    this->emitCode(byte::CALL);
    this->emitOffset(c->arguments.size());
  } break;
  //
  case ast::EXPR_GET: {
    ast::GetExpr *g = static_cast<ast::GetExpr *>(expr);

    this->expr(g->expr);

    this->emitCode(byte::GET);
    this->emitName(g->name.literal); // name
  } break;
  //
  case ast::EXPR_SET: {
    ast::SetExpr *s = static_cast<ast::SetExpr *>(expr);

    this->expr(s->value); // right expression
    this->expr(s->expr);  // left expression

    this->emitCode(byte::SET);
    this->emitName(s->name.literal); // name
  } break;
  //
  case ast::EXPR_ASSIGN: {
    ast::AssignExpr *a = static_cast<ast::AssignExpr *>(expr);

    this->expr(a->value);

    if (a->expr->kind() == ast::EXPR_NAME) {
      this->emitCode(byte::ASSIGN);
      this->emitName(static_cast<ast::NameExpr *>(a->expr)->token.literal);
    } else {
      this->expr(a->expr); // index
      // index replace
      if (now->codes.back() == byte::INDEX) {
        this->now->codes.pop_back();   // pop
        this->emitCode(byte::REPLACE); // push
      }
    }
  } break;
  //
  case ast::EXPR_ARRAY: {
    ast::ArrayExpr *a = static_cast<ast::ArrayExpr *>(expr);

    // push elements from right to left
    for (int i = a->elements.size(); i > 0; i--)
      this->expr(a->elements.at(i - 1));

    this->emitCode(byte::B_ARR);
    this->emitOffset(a->elements.size()); // length
  } break;
  //
  case ast::EXPR_TUPLE: {
    ast::TupleExpr *t = static_cast<ast::TupleExpr *>(expr);

    for (int i = t->elements.size(); i > 0; i--)
      this->expr(t->elements.at(i - 1));

    this->emitCode(byte::B_TUP);
    this->emitOffset(t->elements.size()); // length
  } break;
  //
  case ast::EXPR_MAP: {
    ast::MapExpr *m = static_cast<ast::MapExpr *>(expr);

    for (std::map<ast::Expr *, ast::Expr *>::reverse_iterator iter =
             m->elements.rbegin();
         iter != m->elements.rend(); iter++) {
      //  from right to left by iterator
      this->expr(iter->first);
      this->expr(iter->second);
    }

    this->emitCode(byte::B_MAP);
    this->emitOffset(m->elements.size() * 2); // length
  } break;
  //
  case ast::EXPR_INDEX: {
    ast::IndexExpr *i = static_cast<ast::IndexExpr *>(expr);

    this->expr(i->right);
    this->expr(i->left);

    this->emitCode(byte::INDEX);
  } break;
  //
  case ast::EXPR_NEW: {
    ast::NewExpr *n = static_cast<ast::NewExpr *>(expr);

    for (auto i : n->builder) {
      this->emitCode(byte::NAME);
      this->emitName(i.first->literal); // K

      this->expr(i.second); // V
    }

    this->emitCode(byte::NEW);
    this->emitName(n->name.literal);         // name
    this->emitOffset(n->builder.size() * 2); // fields
  } break;
  }
}

// statements
void Compiler::stmt(ast::Stmt *stmt) {
  switch (stmt->kind()) {
  case ast::STMT_EXPR:
    this->expr(static_cast<ast::ExprStmt *>(stmt)->expr); // expression
    break;
  //
  case ast::STMT_VAR: {
    ast::VarStmt *v = static_cast<ast::VarStmt *>(stmt);

    if (v->expr != nullptr)
      this->expr(v->expr); // initial value
    else
      this->emitCode(byte::ORIG); // original value

    this->emitCode(byte::STORE);
    this->emitName(v->name.literal);

    this->emitType(v->T); // type
  } break;
  //
  case ast::STMT_BLOCK: {
    ast::BlockStmt *b = static_cast<ast::BlockStmt *>(stmt);

    for (auto i : b->block)
      this->stmt(i);
  } break;
  //
  case ast::STMT_IF: {
    ast::IfStmt *i = static_cast<ast::IfStmt *>(stmt);
    /**
     * Moisture regain algorithm
     */
    this->expr(i->condition);
    this->emitCode(byte::F_JUMP);
    int ifPos = now->offsets.size();

    this->stmt(i->ifBranch);

    if (!i->efBranch.empty() ||
        i->nfBranch != nullptr)   // ignore single expr to JUMP
      this->emitCode(byte::JUMP); // jump out after

    int ifOff = now->offsets.size(); // jump after execution if branch
    std::vector<int> tempEfOffs;     // ef condition offsets

    // ef branch
    if (!i->efBranch.empty()) {
      bool firstStmt = true;

      for (auto i : i->efBranch) {
        // if jump to the first ef
        if (firstStmt) {
          this->insertPosOffset(ifPos); // TO: if (F_JUMP)
          firstStmt = false;
        }

        this->expr(i.first); // condition
        this->emitCode(byte::F_JUMP);
        int efPos = now->offsets.size();

        this->stmt(i.second); // block
        this->insertPosOffset(efPos,
                              now->codes.size() + 1); // TO: ef (F_JUMP)

        this->emitCode(byte::JUMP); // jump out after
        tempEfOffs.push_back(now->offsets.size());
      }
      // nf branch
      if (i->nfBranch != nullptr)
        this->stmt(i->nfBranch);
    }
    // nf branch
    else {
      if (i->nfBranch != nullptr) {
        this->insertPosOffset(ifPos); // TO: if (F_JUMP)
        this->stmt(i->nfBranch);
      } else {
        // no ef and nf statement
        this->insertPosOffset(ifPos); // TO: if (F_JUMP)
      }
    }

    // for (auto i : tempEfOffs) std::cout << i << std::endl;
    for (int i = 0; i < tempEfOffs.size(); i++) {
      // insertion increment successively
      this->insertPosOffset(tempEfOffs.at(i) + i);
    }

    if (!i->efBranch.empty() ||
        i->nfBranch != nullptr)         // ignore single expr to JUMP
      this->insertPosOffset(ifOff + 1); // TO: if (JUMP)
  } break;
  //
  case ast::STMT_FOR: {
    ast::ForStmt *f = static_cast<ast::ForStmt *>(stmt);

    this->stmt(f->init); // initializer

    int original = now->codes.size(); // original state: for callback loops

    this->stmt(f->cond); // condition
    this->emitCode(byte::F_JUMP);
    int ePos = now->offsets.size(); // skip loop for FALSE

    this->stmt(f->block); // block
    this->stmt(f->more);  // update

    this->insertPosOffset(ePos, now->codes.size() + 1); // TO: (F_JUMP)

    this->emitCode(byte::JUMP);     // back to original state
    this->emitOffset(original);     // offset
    this->emitJumpOffset(original); // DEBUG

    this->replaceHolder(original); // REPLACE
  } break;
  //
  case ast::STMT_AOP: {
    ast::AopStmt *a = static_cast<ast::AopStmt *>(stmt);

    int original = now->codes.size(); // original state

    if (a->expr == nullptr)
      this->stmt(a->block); // skip condition
    //
    else {
      this->expr(a->expr);
      this->emitCode(byte::F_JUMP);
      int ePos = now->offsets.size(); // skip loop for FALSE

      this->stmt(a->block); // block
                            // jump to next bytecode
      this->insertPosOffset(ePos, now->codes.size() + 1); // TO: (F_JUMP)
    }

    this->emitCode(byte::JUMP);     // back to original state
    this->emitOffset(original);     // offset
    this->emitJumpOffset(original); // DEBUG

    this->replaceHolder(original); // REPLACE
  } break;
  //
  case ast::STMT_OUT: {
    ast::OutStmt *o = static_cast<ast::OutStmt *>(stmt);

    if (o->expr != nullptr)
      this->expr(o->expr);

    // jump straight out
    this->emitCode(o->expr == nullptr ? byte::JUMP : byte::T_JUMP);
    // place holder
    this->emitOffset(-1);
  } break;
  //
  case ast::STMT_GO: {
    ast::GoStmt *t = static_cast<ast::GoStmt *>(stmt);

    if (t->expr != nullptr)
      this->expr(t->expr);

    // jump straight out
    this->emitCode(t->expr == nullptr ? byte::JUMP : byte::T_JUMP);
    // place holder
    this->emitOffset(-2);
  } break;
  //
  case ast::STMT_FUNC: {
    ast::FuncStmt *f = static_cast<ast::FuncStmt *>(stmt);

    int entitiesSize = this->entities.size() - 1; // original

    this->entities.push_back(
        new Entity(f->name.literal)); // new entity for function statement
    this->now = this->entities.back();

    object::Func *obj = new object::Func;

    obj->name = f->name.literal;   // function name
    obj->arguments = f->arguments; // function arguments
    obj->ret = f->ret;             // function return

    int x = this->icf;
    int y = this->inf;
    int z = this->itf;

    this->icf = 0; // x
    this->inf = 0; // y
    this->itf = 0; // z

    this->stmt(f->block);

    this->icf = x;
    this->inf = y;
    this->itf = z;

    obj->entity = this->now; // function entity

    this->entities.pop_back(); // lose

    // if more than one it points to the last one
    this->now = this->entities.at(entitiesSize); // restore to main entity

    // TO main ENTITY
    this->emitCode(byte::FUNC);
    this->emitConstant(obj); // push to constant object
  } break;
  //
  case ast::STMT_WHOLE: {
    ast::WholeStmt *w = static_cast<ast::WholeStmt *>(stmt);

    int entitiesSize = this->entities.size() - 1; // original

    this->entities.push_back(
        new Entity(w->name.literal)); // new entity for whole statement
    this->now = this->entities.back();

    object::Whole *obj = new object::Whole;

    obj->name = w->name.literal; // whole name

    // whole inherit
    if (w->inherit != nullptr) {
      ast::InheritStmt *i = static_cast<ast::InheritStmt *>(w->inherit);
      for (auto iter : i->names) {
        obj->inherit.push_back(iter->literal);
      }
    }

    int x = this->icf;
    int y = this->inf;
    int z = this->itf;

    this->icf = 0; // x
    this->inf = 0; // y
    this->itf = 0; // z

    // block statement
    for (auto i : w->body->block) {
      // interface definition
      if (i->kind() == ast::STMT_INTERFACE) {
        ast::InterfaceStmt *inter = static_cast<ast::InterfaceStmt *>(i);
        obj->interface.push_back(
            std::make_tuple(inter->name.literal, inter->arguments, inter->ret));
        continue;
      }
      this->stmt(i);
    }

    this->icf = x;
    this->inf = y;
    this->itf = z;

    obj->entity = this->now; // whole entity

    this->entities.pop_back(); // lose

    // if more than one it points to the last one
    this->now = this->entities.at(entitiesSize); // restore to main entity

    // TO main ENTITY
    this->emitCode(byte::WHOLE);
    this->emitConstant(obj); // push to constant object
  } break;
  //
  case ast::STMT_MOD: {
    ast::ModStmt *m = static_cast<ast::ModStmt *>(stmt);

    this->emitCode(byte::MOD);
    this->emitName(m->name.literal);
  } break;
  //
  case ast::STMT_USE: {
    ast::UseStmt *u = static_cast<ast::UseStmt *>(stmt);

    this->emitCode(byte::USE);
    this->emitName(u->name.literal);
  } break;
  //
  case ast::STMT_RET: {
    ast::RetStmt *r = static_cast<ast::RetStmt *>(stmt);

    if (r->stmt != nullptr)
      this->stmt(r->stmt);

    this->emitCode(r->stmt == nullptr ? byte::RET_N : byte::RET);
  } break;
  //
  case ast::STMT_ENUM: {
    ast::EnumStmt *e = static_cast<ast::EnumStmt *>(stmt);

    object::Enum *obj = new object::Enum;
    obj->name = e->name.literal;

    for (int i = 0; i < e->field.size(); i++) {
      obj->elements.insert(std::make_pair(i, e->field.at(i)->literal));
    }

    this->emitCode(byte::ENUM);
    this->emitConstant(obj); // push to constant object
  } break;
  //
  case ast::STMT_DEL: {
    this->emitCode(byte::DEL);
    this->emitName(static_cast<ast::DelStmt *>(stmt)->name.literal); // name
  } break;
  //
  default:
    break;
  }
}