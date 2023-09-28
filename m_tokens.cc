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
#include "m_tokens.h"
#include "mg_func.h"
/*--------------------------------------------------------------------------*/
void Token_SFCALL::stack_op(Expression* E)const
{
  assert(E);
  // replace single token with its value
  if (!E->is_empty() && dynamic_cast<const Token_PARLIST*>(E->back())) {
    const Token* T1 = E->back(); // Token_PARLIST
    assert(T1->data()); // expression
    Base const* d = T1->data();
    auto ee = prechecked_cast<Expression const*>(d);
    assert(ee);

    {
      // restore argument.
      E->pop_back();
      E->push_back(new Token_STOP("sf_stop"));
      for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
	trace1("stackop restore arg", (**i).name());
	(**i).stack_op(E);
      }
      E->push_back(new Token_PARLIST("sf_args"));
      E->push_back(clone());
    }
    delete T1;
  }else{
    E->push_back(clone());
  }
}
/*--------------------------------------------------------------------------*/
void Token_AFCALL::stack_op(Expression* E)const
{
  // replace single token with its value
  if (!E->is_empty() && dynamic_cast<const Token_PARLIST*>(E->back())) {
    const Token* T1 = E->back(); // Token_PARLIST
    assert(T1->data()); // expression
    Base const* d = T1->data();
    auto ee = prechecked_cast<Expression const*>(d);
    assert(ee);

    {
      // restore argument.
      E->pop_back();
      E->push_back(new Token_STOP("af_stop"));
      for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
	trace1("stackop restore arg", (**i).name());
	(**i).stack_op(E);
      }
      E->push_back(new Token_PARLIST("af_args"));
      E->push_back(clone());
    }
    delete T1;
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
std::string Token_TASK::code_name()const
{
  assert(f());
  return "/*task*/ " + f()->code_name();
}
/*--------------------------------------------------------------------------*/
bool Token_CALL::returns_void() const
{
  if(_function){
    return _function->returns_void();
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
std::string Token_CALL::code_name()const
{
  if(!_function){
    incomplete();
    return "BUG";
  }else if(_function->code_name()!=""){
    return "/*call1*/" + _function->code_name();
  }else if(_function->label()!=""){
    // incomplete(); // m_va.h, TODO
    return "/*INCOMPLETE*/ va::" + _function->label();
  }else{
    return "Token_CALL::code_name: incomplete";
  }
}
/*--------------------------------------------------------------------------*/
void Token_TASK::stack_op(Expression* E)const
{
  const Token* T1 = E->back(); // Token_PARLIST
  assert(T1->data()); // expression
  Base const* d = T1->data();
  auto ee = prechecked_cast<Expression const*>(d);
  assert(ee);

  { // restore argument.
    E->pop_back();
    E->push_back(new Token_STOP("ta_stop"));
    for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
      trace1("stackop restore arg", (**i).name());
      (**i).stack_op(E);
    }
    E->push_back(new Token_PARLIST("ta_args"));
    E->push_back(clone());
  }
  delete T1;
}
/*--------------------------------------------------------------------------*/
void Token_CALL::stack_op(Expression* E)const
{
  assert(E);
  if (E->is_empty()){
    unreachable();
  }else if (dynamic_cast<const Token_PARLIST*>(E->back())) {
    const Token* T1 = E->back(); // Token_PARLIST
    assert(T1->data()); // expression
    Base const* d = T1->data();
    auto ee = prechecked_cast<Expression const*>(d);
    assert(ee);

    // Expression fee = f->ev(ee); ?

    bool all_float = false;
    for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
      trace1("float?", (**i).name());
      all_float = dynamic_cast<Float const*>((**i).data());
      if(!all_float){
	break;
      }else{
      }
    }

    if(all_float){
      FUNCTION_ const* f = _function;
      // function call as usual
      CS cmd(CS::_STRING, T1->name());
      std::string value = f->eval(cmd, E->_scope); // TODO: pass Expression
      E->pop_back();
      const Float* v = new Float(value);
      E->push_back(new Token_CONSTANT(value, v, ""));
    }else{
      // restore argument.
      E->pop_back();
      E->push_back(new Token_STOP("fn_stop"));
      for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
	trace1("stackop restore arg", (**i).name());
	(**i).stack_op(E);
      }
      E->push_back(new Token_PARLIST("fn_args"));
      E->push_back(clone());
    }
    delete T1;
  }else{
    assert(0);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
