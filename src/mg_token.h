/*                                      -*- C++ -*-
 * Copyright (C) 2023, 2024 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
 */
/*--------------------------------------------------------------------------*/
#ifndef MG_TOKEN_H
#define MG_TOKEN_H
#include "m_tokens.h"
#include <m_expression.h>
#include "mg_func.h"
#include "mg_base.h"
class FUNCTION_;
class Token_CALL : public Token_SYMBOL {
private: // stuff into data?
  FUNCTION_ const* _function{NULL};
  Expression const* _args{NULL};
  size_t _num_args{size_t(-1)};
private:
  Token* clone()const override {
    return new Token_CALL(*this);
  }
public:
  explicit Token_CALL(const std::string Name, FUNCTION_ const* f, Expression const* e=NULL)
    : Token_SYMBOL(Name, ""), _function(f), _args(e) { attach(); }
  ~Token_CALL() { detach(); delete _args; }
protected:
  explicit Token_CALL(const Token_CALL& P)
    : Token_SYMBOL(P.name(), ""), _function(P._function), _num_args(P._num_args) { attach(); }
  explicit Token_CALL(const Token_CALL& P, Base const* data, Expression const* e=NULL)
    : Token_SYMBOL(P.name(), data), _function(P._function), _args(e)
    , _num_args(P._num_args) { attach(); }
private:
  void attach();
  void detach();
public:
  void pop(){ untested(); assert(_args); _args = NULL; }
  void push(Expression const* e){ assert(!_args); _args = e; }
  void stack_op(Expression* e)const override;
  void set_num_args(size_t n){ _num_args = n; } // expression size?
  void set_args(Expression* e){ assert(!_args); _args = e; } // needed?
 // size_t num_args() const;
  Expression const* args()const { return _args; }
  virtual /*?*/ std::string code_name() const;
  FUNCTION_ const* f() const{ return _function; }
  bool returns_void() const;
}; // Token_CALL
/*--------------------------------------------------------------------------*/
class Port_3; // New_Port?
class Token_PORT_BRANCH : public Token_SYMBOL {
  Port_3 const* _port{NULL}; // use _data?
public:
  Token_PORT_BRANCH(Token_PORT_BRANCH const& s)
    : Token_SYMBOL(s), _port(s._port) {}
  Token_PORT_BRANCH(Token_SYMBOL const& s, Port_3 const* p)
    : Token_SYMBOL(s), _port(p) {}

  Token_PORT_BRANCH* clone()const override { return new Token_PORT_BRANCH(*this); }

  void stack_op(Expression* E)const override{
    E->push_back(clone());
  }
};
/*--------------------------------------------------------------------------*/
class Token_NODE : public Token_SYMBOL {
  Node_Ref const _node;
public:
  Token_NODE(Token_NODE const& s)
    : Token_SYMBOL(s), _node(s._node) {}
  Token_NODE(Token_SYMBOL const& s, Node_Ref p)
    : Token_SYMBOL(s), _node(p) {}

  Token_NODE* clone()const override { return new Token_NODE(*this); }

  void stack_op(Expression* E)const override{
    E->push_back(clone());
  }
};
/*--------------------------------------------------------------------------*/
inline void Token_CALL::attach()
{
  assert(_function);
  _function->inc_refs();
}
/*--------------------------------------------------------------------------*/
inline void Token_CALL::detach()
{
  assert(_function);
  _function->dec_refs();
}
/*--------------------------------------------------------------------------*/
inline bool Token_CALL::returns_void() const
{
//  assert(_function);
  if(_function){
    return _function->returns_void();
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
inline std::string Token_CALL::code_name()const
{
  assert(_function);
  if(_function->code_name()!=""){
    return "/*call1*/" + _function->code_name();
  }else if(_function->label()!=""){
    // incomplete(); // m_va.h, TODO
    return "/*INCOMPLETE*/ va::" + _function->label();
  }else{ untested();
    return "Token_CALL::code_name: incomplete";
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
