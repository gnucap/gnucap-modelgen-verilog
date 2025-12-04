/*                       -*- C++ -*-
 * Copyright (C) 2025 Felix Salfelder
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
#include "mg_storage.h"
/*--------------------------------------------------------------------------*/
void Variable_Access::push_init(Token_VAR_REF* v)
{
  assert(v);
  trace1("XS push init", v->name());
  _list.push_back(xs(v, xs_init, true));
}
/*--------------------------------------------------------------------------*/
void Variable_Access::push_assign(Token_VAR_REF* v,
    bool is_const, bool always)
{
  assert(v);
  assert(v->item());
  trace2("XS push assign", v->name(), is_const);
  auto A = prechecked_cast<Assignment*>(v->mutable_item());
  assert(A);

  // _list.push_back(xs(A));
  assert(v);
  xs x(v, is_const?xs_const_assign:xs_assign, always);
  assert(x.is_constant() == is_const);
  _list.push_back(x);
  assert(_list.back().is_constant() == is_const);
}
/*--------------------------------------------------------------------------*/
void Variable_Access::push_use(Token_VAR_REF* v)
{
  assert(v);
  trace1("XS push use", v->name());
  _list.push_back(xs(v, xs_use, false));
}
/*--------------------------------------------------------------------------*/
Block const* Variable_Access::xs::scope() const
{
  assert(_v);
  return _v->scope();
}
/*--------------------------------------------------------------------------*/
Block const* Variable_Access::xs::var_scope() const
{
  assert(_v);
  if(is_assign()){
    if(auto b = dynamic_cast<Variable_Decl const*>(_v->item())){
      // AF output arg hack
      return b->scope();
    }else{
    }

    auto a = prechecked_cast<Assignment const*>(_v->item());
    assert(a);
    return a->lhs_scope();

  }else{
    assert(_v);
    return _v->scope();
  }
}
/*--------------------------------------------------------------------------*/
Variable_Access::xs::xs(Token_VAR_REF* v, Variable_Access::mode_t mode,
    bool always) : _v(v), _mode(mode), _always(always)
{
}
/*--------------------------------------------------------------------------*/
void Variable_Access::propagate(SeqBlock const* scope)
{
  assert(scope);
  if(!scope->is_reachable()){
  }else{
    auto parent_scope = dynamic_cast<SeqBlock*>(scope->scope());
    bool top_level = !parent_scope;
    bool initial = scope->is_initial();
    trace2("XS propagate", top_level, initial);

    for(xs const& i : _list){
      bool always = i.is_always();
      bool reachable = i.scope() && i.scope()->is_reachable();
      bool local = scope == i.var_scope();
      bool constant = i.is_constant();

      assert(i._v);
      if(!reachable) {
	assert(i._v);
	trace2("XS replay, unreachable", i._v->name(), reachable);
	// incomplete(); local var in analog function
      }else if(i.is_assign()) {
	trace5("XS replay, assign", i._v->name(), reachable, always, constant, initial);
	if(!local && !top_level) {
	  trace4("XS push assign, !local", i._v->name(), reachable, always, constant);
	  assert(parent_scope);
	  parent_scope->variable_access().push(i);
	}else if(initial && always) {
	  i._v->init_var();
	}else if(always) {
	  i._v->assign_var();
	  if(!constant) {
	    // temporary.
	    i._v->use_var();
	    i._v->assign_var();
	  }else{
	  }
	}else{
	  trace1("XS replay assign sometimes", i._v->name());
	  // kludge. make it a state. need more analysis
	  i._v->use_var();
	  i._v->assign_var();
	}
      }else if(i.is_use()){
	trace2("XS replay, use", i._v->name(), local);
	local = top_level || dynamic_cast<Variable_Decl const*>(i._v->item());
	if(!local) {
	  trace2("XS replay, push use", i._v->name(), local);
	  assert(parent_scope);
	  parent_scope->variable_access().push(i);
	}else if(reachable) {
	  trace2("XS replay use", i._v->name(), local);
	  i._v->use_var();
	}else{ untested();
	}
      }
    } // loop
  }
}
/*--------------------------------------------------------------------------*/
void STORAGE_TYPE::init()
{
  switch(_actual){
  case s_unknown:
    _actual = s_initial;
    break;
  case s_initial:
    break;
  case s_set:
    _actual = s_initial;
    break;
  case s_used:
    _actual = s_initial;
    break;
  case s_tmp:
    _actual = s_initial;
    break;
  case s_state:
    break;
  }
}
/*--------------------------------------------------------------------------*/
void STORAGE_TYPE::assign()
{
  switch(_actual){
  case s_unknown:
    _actual = s_set;
    break;
  case s_initial:
    _actual = s_state;
    break;
  case s_set:
    break;
  case s_used:
    _actual = s_tmp;
    break;
  case s_tmp:
    break;
  case s_state:
    break;
  }
}
/*--------------------------------------------------------------------------*/
void STORAGE_TYPE::use()
{
  trace0("STORAGE_TYPE::use");
  switch(_actual){
  case s_unknown:
    _actual = s_state;
    break;
  case s_initial:
    break;
  case s_set:
    _actual = s_used;
    break;
  case s_used:
    break;
  case s_tmp:
    break;
  case s_state:
    break;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
