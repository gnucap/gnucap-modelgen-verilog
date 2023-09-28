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
#include <m_base.h>
#include <m_expression.h>
/*--------------------------------------------------------------------------*/
class Probe;
class FUNCTION_;
class Token_ACCESS : public Token_SYMBOL {
  Probe const* _prb;
public:
  explicit Token_ACCESS(const std::string Name, Probe const* data)
    : Token_SYMBOL(Name, ""), _prb(data) {}

private:
  explicit Token_ACCESS(const Token_ACCESS& P)
    : Token_SYMBOL(P), _prb(P._prb) {}
  Token* clone()const override {
    return new Token_ACCESS(*this);
  }
  void stack_op(Expression* e)const override {
    e->push_back(clone());
  }

public:
  Probe const* operator->() const{ return _prb; }
  Probe const* prb() const{ return _prb;}
};
/*--------------------------------------------------------------------------*/
class Parameter_Base;
class Token_PAR_REF : public Token_SYMBOL {
  Parameter_Base const* _item;
public:
  explicit Token_PAR_REF(const std::string Name, Parameter_Base const* item)
    : Token_SYMBOL(Name, ""), _item(item) {}
private:
  explicit Token_PAR_REF(const Token_PAR_REF& P)
    : Token_SYMBOL(P), _item(P._item) {}
  Token* clone()const override {
    return new Token_PAR_REF(*this);
  }
  void stack_op(Expression* e)const override{
    e->push_back(clone());
  }
public:
  std::string code_name()const {
    return "_p_" + name();
  }
  Parameter_Base const* item()const { untested();
    return _item;
  }
};
/*--------------------------------------------------------------------------*/
// ITEM_REF?
class Variable;
//class Deps;
class Filter;
class Token_VAR_REF : public Token_SYMBOL {
  Variable const* _item;
public:
  explicit Token_VAR_REF(const std::string Name, Variable const* item)
    : Token_SYMBOL(Name, ""), _item(item) {}
//   explicit Token_VAR_REF(const std::string Name, Filter const* item)
//     : Token_SYMBOL(Name, ""), _item(item) {}
private:
  explicit Token_VAR_REF(const Token_VAR_REF& P) : Token_SYMBOL(P), _item(P._item) {}
  Token* clone()const  override{return new Token_VAR_REF(*this);}
  void stack_op(Expression* e)const override{
    e->push_back(clone());
  }
public:
  Variable const* item()const { untested(); return _item; }
  Variable const* operator->() const{ return _item; }
};
/*--------------------------------------------------------------------------*/
class FUNCTION_;
class Token_CALL : public Token_SYMBOL {
  FUNCTION_ const* _function{NULL};
  size_t _num_args{size_t(-1)};
public:
  explicit Token_CALL(const std::string Name, FUNCTION_ const* f)
    : Token_SYMBOL(Name, ""), _function(f) { attach(); }
  ~Token_CALL() { detach(); }
protected:
  explicit Token_CALL(const Token_CALL& P)
    : Token_SYMBOL(P), _function(P._function)
    , _num_args(P._num_args) { attach(); }
private:
  Token* clone()const override {return new Token_CALL(*this);}
  void stack_op(Expression* e)const override;
private:
  void attach();
  void detach();
public:
  void set_num_args(size_t n){ _num_args = n; }
  size_t num_args() const;
  virtual /*?*/ std::string code_name() const;
  FUNCTION_ const* f() const{ return _function; }
  bool returns_void() const;
};
/*--------------------------------------------------------------------------*/
class Analog_Function;
// same as Token_CALL? Analog_Function is not a FUNCTION_ (yet?)
class Token_AFCALL : public Token_CALL {
  Analog_Function const* _f{NULL};
public:
  explicit Token_AFCALL(const std::string Name, Analog_Function const* f = NULL)
    : Token_CALL(Name, NULL), _f(f) {}
private:
  explicit Token_AFCALL(const Token_AFCALL& P) : Token_CALL(P), _f(P._f) {}
  Token* clone()const  override{return new Token_AFCALL(*this);}
  void stack_op(Expression* e)const override;
public:
  std::string code_name()const {
    return "af_" + name();
//    assert(_f);
//   return _f->code_name();
  }
};
/*--------------------------------------------------------------------------*/
class Token_SFCALL : public Token_CALL {
public:
  explicit Token_SFCALL(const std::string Name, FUNCTION_ const* f = NULL)
    : Token_CALL(Name, f) {}
private:
  explicit Token_SFCALL(const Token_SFCALL& P) : Token_CALL(P) {}
  Token* clone()const override {
    return new Token_SFCALL(*this);
  }
  void stack_op(Expression* e)const override;
};
/*--------------------------------------------------------------------------*/
#if 0
typedef Token_CALL Token_TASK;
#else
class Token_TASK : public Token_CALL {
public:
  explicit Token_TASK(const std::string Name, FUNCTION_ const* item)
    : Token_CALL(Name, item) {}
private:
  explicit Token_TASK(const Token_TASK& P)
    : Token_CALL(P) {}
  Token* clone()const override{return new Token_TASK(*this);}
  void stack_op(Expression* e)const override;
  std::string code_name()const override;
};
#endif
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
