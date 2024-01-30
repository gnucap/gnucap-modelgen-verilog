/*                        -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 * Author: Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 * arithmetic tokens
 */
#ifndef M_TOKENS_H
#define M_TOKENS_H
#include "mg_expression.h"
#include <m_base.h>
#include "mg_deps.h" // BUG?
#include "mg_token.h"
/*--------------------------------------------------------------------------*/
class Deps;
/*--------------------------------------------------------------------------*/
Deps* copy_deps(Base const* b);
/*--------------------------------------------------------------------------*/
static Token* copy(Token const* b)
{
  if(auto t=dynamic_cast<Token const*>(b)){
    return t->clone();
  }else if(b){ untested();
    return NULL;
  }else{
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
class Token_PARLIST_ : public Token_PARLIST {
public:
  explicit Token_PARLIST_(Token_PARLIST const& p)
    : Token_PARLIST(p) { }
  explicit Token_PARLIST_(const std::string Name, Base* L=NULL)
    : Token_PARLIST(Name, L) {}
public:
  void stack_op(Expression* E)const override;
  Token* clone()const override{
    return new Token_PARLIST_(*this);
  }
  Deps* new_deps()const;
};
/*--------------------------------------------------------------------------*/
class Token_UNARY_ : public Token_UNARY {
  Token const* _op{NULL}; // stuff into data?
public:
  explicit Token_UNARY_(const std::string Name, Token const* op, Base const* d=NULL)
    : Token_UNARY(Name, d), _op(op) {}
  explicit Token_UNARY_(Token_UNARY const& b)
    : Token_UNARY(b.name()) { } // ,( copy_deps(b.data())) { }
  Token* clone()const override { untested();
    return new Token_UNARY_(*this); // name(), copy(data()));
  }
  ~Token_UNARY_(){ delete _op; }

  void stack_op(Expression*)const override;
  void pop(){ _op = NULL; }
  Token const* op1()const { return _op; };
};
/*--------------------------------------------------------------------------*/
class Token_BINOP_ : public Token_BINOP {
private: // stuff into data?
  Token const* _op1{NULL};
  Token const* _op2{NULL};
public:
  explicit Token_BINOP_(Token_BINOP const& b, Base const* d)
    : Token_BINOP(b.name(), d) { }
  explicit Token_BINOP_(std::string const& b, Base const* d)
    : Token_BINOP(b, d) { }
  explicit Token_BINOP_(std::string const& b,
      Token const* t1, Token const* t2, Base const* d)
    : Token_BINOP(b, d), _op1(t1), _op2(t2) { }
  explicit Token_BINOP_(Token_BINOP const& b)
    : Token_BINOP(b.name()) { assert(!b.data()); }
  ~Token_BINOP_(){
    delete _op1;
    delete _op2;
  }
  void pop(){
    _op1 = _op2 = NULL;
  }
  void pop1(){
    _op1 = NULL;
  }
  void pop2(){
    _op2 = NULL;
  }

  Token* clone()const override { untested();
    return new Token_BINOP_(name(), copy(_op1), copy(_op2), copy_deps(data()));
  }
  Token const* op1()const { return _op1; };
  Token const* op2()const { return _op2; };

  void stack_op(Expression* E)const override;
//  Deps const* op_deps(Base const* t1, Base const* t2)const;
  Deps const* op_deps(Token const* d1, Token const* d2)const;
private:
  void stack_op_(Expression* E)const;
};
/*--------------------------------------------------------------------------*/
class Token_TERNARY_ : public Token_TERNARY {
  Token const* _cond{NULL}; // stuff into data?
public:
//  explicit Token_TERNARY_(Token_TERNARY const& b, Base const* d)
//    : Token_TERNARY(b.name(), d) { }
  explicit Token_TERNARY_(std::string const& b, Base const* d)
    : Token_TERNARY(b, d) { }
  explicit Token_TERNARY_(std::string const& b, Token const* Cond,
      Expression const* t1, Expression const* t2, Base const* d=NULL)
    : Token_TERNARY(b, t1, t2, d), _cond(Cond) {}
  ~Token_TERNARY_(){
    delete _cond;
    _cond = NULL;
  }

  Token* clone()const override { untested();
    unreachable();
    return NULL;
    // new Token_TERNARY_(name(), dup(true_part()), dup(false_part()), copy(data()));
  }

  void stack_op(Expression* E)const override;
//  Deps const* op_deps(Base const* t1, Base const* t2)const;
  Token const* op_deps(Token const* d1, Token const* d2)const;
  Token const* cond()const {return _cond; }
private:
  void stack_op_(Expression* E)const;
};
/*--------------------------------------------------------------------------*/
class Token_SYMBOL_ : public Token_SYMBOL {
public:
  explicit Token_SYMBOL_(Token_SYMBOL const& p)
    : Token_SYMBOL(p) { untested();
  }
  ~Token_SYMBOL_(){
//    _data?
    incomplete();
  }
public:
  void stack_op(Expression* E)const override;
  Token* clone()const override{ untested();
    return new Token_SYMBOL(*this);
  }
};
/*--------------------------------------------------------------------------*/
class Probe;
class FUNCTION_;
class Token_ACCESS : public Token_SYMBOL {
  mutable /*BUG*/ FUNCTION_ const* _prb;
public:
  explicit Token_ACCESS(const std::string Name, Base const* Data, FUNCTION_ const* f=NULL)
    : Token_SYMBOL(Name, Data), _prb(f) {}

private:
  explicit Token_ACCESS(const Token_ACCESS& P)
    : Token_SYMBOL(P.name(), copy_deps(P.data())), _prb(P._prb) {}
  Token* clone()const override {
    return new Token_ACCESS(*this);
  }

public:
  void stack_op(Expression* e)const override;
  Probe const* prb() const;
  bool is_short() const;
  bool is_reversed() const;
  std::string code_name()const;
  std::string code_name_()const;
private:
  size_t num_deps() const;
};
/*--------------------------------------------------------------------------*/
class Parameter_Base;
class Token_PAR_REF : public Token_CONSTANT {
  Parameter_Base const* _item;
public:
  explicit Token_PAR_REF(const std::string Name, Parameter_Base const* item)
    : Token_CONSTANT(Name, NULL, ""), _item(item) {}
private:
  explicit Token_PAR_REF(const Token_PAR_REF& P, Base const* d=NULL)
    : Token_CONSTANT(P.name(), d, ""), _item(P._item) {}
  Token* clone()const override {
    return new Token_PAR_REF(*this);
  }
public:
  void stack_op(Expression* e)const override;
public:
  std::string code_name()const {
    return "_p_" + name();
  }
  Parameter_Base const* item()const {
    return _item;
  }
  Parameter_Base const* operator->() const{ return _item; }
};
/*--------------------------------------------------------------------------*/
// ITEM_REF?
class Variable;
//class Deps;
class Token_VAR_REF : public Token_SYMBOL {
  Variable const* _item;
public:
  explicit Token_VAR_REF(const std::string Name, Variable const* item)
    : Token_SYMBOL(Name, ""), _item(item) {}
private:
  explicit Token_VAR_REF(const Token_VAR_REF& P, Base const* d=NULL)
    : Token_SYMBOL(P.name(), d), _item(P._item) {}
private:
  Token* clone()const override { return new Token_VAR_REF(*this);}
public:
  void stack_op(Expression* e)const override;
  Variable const* item()const { untested(); return _item; }
  Variable const* operator->() const{ return _item; }
private:
  size_t num_deps() const;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#if 1
// typedef Token_CALL Token_SFCALL;
#else
class Token_SFCALL : public Token_CALL {
public:
  explicit Token_SFCALL(const std::string Name, FUNCTION_ const* f = NULL)
    : Token_CALL(Name, f) {}
private:
  explicit Token_SFCALL(const Token_SFCALL& P, Base const* data=NULL)
    : Token_CALL(P, data) {}
  Token* clone()const override {
    return new Token_SFCALL(*this);
  }
  void stack_op(Expression* e)const override{
    assert(e);
    Token_CALL::stack_op(e);
    assert(e->back());
    Base const* dd = e->back()->data();
    if(auto cc=dynamic_cast<Token_SFCALL const*>(e->back())){
      if(auto tt=dynamic_cast<Token const*>(dd)){
	dd = tt->clone();
      }else{
      }
      auto t = new Token_SFCALL(*cc, dd);
      delete(e->back());
      e->pop_back();
      e->push_back(t);
    }else{
      // reachable?
    }
  }
};
#endif
/*--------------------------------------------------------------------------*/
#if 0
class Token_FILTER : public Token_CALL {
public:
  explicit Token_FILTER(const std::string Name, FUNCTION_ const* item)
    : Token_CALL(Name, item) {}
private:
  explicit Token_FILTER(const Token_FILTER& P)
    : Token_CALL(P) {} // , _item(P._item) {}
  Token* clone()const override {return new Token_FILTER(*this);}

  // incomplete.
//  void stack_op(Expression* e)const override{ untested();
//    e->push_back(clone());
//  }
public:
  std::string code_name() const;
};
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
