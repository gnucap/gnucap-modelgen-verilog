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
 * mockup component generator for analog block testing
 */
#ifndef M_TOKENS_H
#define M_TOKENS_H
#include <gnucap/m_base.h>
/*--------------------------------------------------------------------------*/

class Token_PROBE : public Token_SYMBOL {
  Probe const* _item;
public:
  explicit Token_PROBE(const std::string Name, Probe const* data)
    : Token_SYMBOL(Name, ""), _item(data) {}

private:
  explicit Token_PROBE(const Token_PROBE& P)
	  : Token_SYMBOL(P), _item(P._item) {}
  Token* clone()const  override{return new Token_PROBE(*this);}
  void stack_op(Expression* e)const override{
    e->push_back(clone());
  }

public:
  std::string code_name() const{
	  assert(_item);
	  return _item->code_name();
  }
};
/*--------------------------------------------------------------------------*/
class Token_PAR_REF : public Token_SYMBOL {
  Parameter_Base const* _item;
public:
  explicit Token_PAR_REF(const std::string Name, Parameter_Base const* item)
    : Token_SYMBOL(Name, ""), _item(item) {}
private:
  explicit Token_PAR_REF(const Token_PAR_REF& P) : Token_SYMBOL(P), _item(P._item) {}
  Token* clone()const  override{return new Token_PAR_REF(*this);}
  void stack_op(Expression* e)const override{
    e->push_back(clone());
  }
public:
  Parameter_Base const* item()const {
	  return _item;
  }
};
/*--------------------------------------------------------------------------*/
class Token_VAR_REF : public Token_SYMBOL {
  Base const* _item;
public:
  explicit Token_VAR_REF(const std::string Name, Variable const* item)
    : Token_SYMBOL(Name, ""), _item(item) {}
private:
  explicit Token_VAR_REF(const Token_VAR_REF& P) : Token_SYMBOL(P), _item(P._item) {}
  Token* clone()const  override{return new Token_VAR_REF(*this);}
  void stack_op(Expression* e)const override{
    e->push_back(clone());
  }
public:
  Variable const* item()const {
	  return dynamic_cast<Variable const*>(_item);
  }
  std::set<Probe const*> const& deps() const{
    if(item()){
		 return item()->deps();
	 }else{
		 static const std::set<Probe const*> s;
		 return s;
	 }
  }
};
/*--------------------------------------------------------------------------*/
#endif
