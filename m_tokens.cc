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
#include "mg_.h" // BUG, Probe
/*--------------------------------------------------------------------------*/
std::string Token_TASK::code_name()const
{
  assert(f());
  return "/*task*/ " + f()->code_name();
}
/*--------------------------------------------------------------------------*/
void Token_CALL::attach()
{
  if(_function) {
    _function->inc_refs();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Token_CALL::detach()
{
  if(_function) {
    _function->dec_refs();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool Token_CALL::returns_void() const
{
//  assert(_function);
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
static bool is_constant(Token const* t, double val=NOT_VALID)
{
  auto tt = dynamic_cast<Token_CONSTANT const*>(t);
  if(!tt){
    return false;
  }else if(val == NOT_VALID){
    return tt;
  }else if(auto f = dynamic_cast<const Float*>(tt->data())){
    return f->value() == val;
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class stash_op{
  Token* _op{NULL};
  Token* _args{NULL};
public:
  stash_op(Expression* E){
    assert(!E->is_empty());
    _op = E->back();
    E->pop_back();
    if(E->is_empty()){
    }else if(dynamic_cast<Token_PARLIST_*>(E->back())){
      _args = E->back();
      E->pop_back();
    }else{
      assert(!dynamic_cast<Token_PARLIST*>(E->back()));
    }
  }
  bool is_constant(double val=NOT_VALID) const {
    assert(_op);
    return !_args && ::is_constant(_op, val);
  }
  void restore(Expression* E){
    if(_args){
      E->push_back(_args);
    }else{
    }
    E->push_back(_op);
  }
  void drop(){
    delete(_op);
    delete(_args);
    _op = _args = NULL;
  }

};
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
void Token_CALL::stack_op(Expression* E)const
{
  assert(E);

  if (E->is_empty()){
    // SFCALL
      E->push_back(clone());
  }else if(!dynamic_cast<const Token_PARLIST*>(E->back())) {
    // SFCALL
      E->push_back(clone());
  }else if (E->back()->data()) {
    const Token* T1 = E->back(); // Token_PARLIST?
    assert(T1->data()); // expression
    Base const* d = T1->data();
    auto ee = prechecked_cast<Expression const*>(d);
    assert(ee);

    FUNCTION_ const* f = _function;
    assert(ee);

    E->pop_back();
    if(f){
      try{
	f->stack_op(*ee, E);
      }catch(Exception const&){
	f = NULL;
      }
    }else{
    }
    if(f){
    }else if(1){
      auto EE = new Expression_;
      for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
	trace1("stackop restore arg", (**i).name());
	(**i).stack_op(EE);
      }
      E->push_back(new Token_PARLIST_("fn_args", EE));
      E->push_back(clone());

    }else{
      // put it back.
      E->push_back(new Token_STOP("fn_stop"));
      for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
	trace1("stackop restore arg", (**i).name());
	(**i).stack_op(E);
      }
      E->push_back(new Token_PARLIST("fn_args"));
      E->push_back(clone());
    }

    delete T1;
  }
}
/*--------------------------------------------------------------------------*/
void Token_ACCESS::stack_op(Expression* e) const
{
  Expression& E = *e;
  auto SE = prechecked_cast<Expression_*>(e);
  assert(SE);

  if(_prb){
    // already resolved
    assert(e->is_empty() || !dynamic_cast<Token_PARLIST const*>(e->back()));
    e->push_back(clone());
  }else{
    assert(!e->is_empty() && dynamic_cast<Token_PARLIST const*>(e->back()));

    delete E.back(); // PARLIST
    E.pop_back();
    assert(!E.is_empty());
    if(dynamic_cast<Token_STOP*>(E.back())) { untested();
      throw Exception("syntax error");
    }else{
    }
    std::string arg0 = E.back()->name();
    size_t na=1;
    std::string arg1;
    delete E.back();
    E.pop_back();
    assert(!E.is_empty());

    while(!dynamic_cast<Token_STOP*>(E.back())) {
      arg1 = arg0;
      ++na;
      arg0 = E.back()->name();
      delete E.back();
      E.pop_back();
      assert(!E.is_empty());
    }

    delete E.back();
    E.pop_back();
    // BUG: push dep?
    //
    VAMS_ACCESS f(name(), arg0, arg1);
//    assert(ds.top());
    assert(SE->owner());
    Deps dd;
    Token* t = SE->owner()->new_token(&f, dd, na);

#if 0
  Branch_Ref br = m.new_branch(_arg0, _arg1);
  //  br->set_owner(this);
  assert(br);
  assert(const_cast<Branch const*>(br.operator->())->owner());
  // Probe const* p = m.new_probe(_name, _arg0, _arg1);
  //
  // install clone?
  FUNCTION_ const* p = m.new_probe(_name, br);

  return p->new_token(m, na, d);
#endif

    assert(dd.size() == 1);
    _prb = *dd.begin();
    assert(t);
    e->push_back(t);
  }
}
/*--------------------------------------------------------------------------*/
Probe const* Token_ACCESS::prb() const
{
  auto p = prechecked_cast<Probe const*>(_prb);
  assert(p);
  return p;
}
/*--------------------------------------------------------------------------*/
bool Token_ACCESS::is_reversed() const
{
  auto p = prb();
  assert(p);
  return p->is_reversed();
}
/*--------------------------------------------------------------------------*/
std::string Token_ACCESS::code_name() const
{
  auto p = prb();
  assert(p);
  return p->code_name();
}
/*--------------------------------------------------------------------------*/
bool Token_ACCESS::is_short() const
{
  auto p = prb();
  assert(p);
  assert(p->branch());
  return p->branch()->is_short();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
