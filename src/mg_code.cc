/*                       -*- C++ -*-
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
// #include "mg_base.h"
#include "mg_.h" // BUG? branch.
#include "mg_code.h"
#include "mg_deps.h" // BUG?
#include "mg_token.h"
#include "mg_module.h"
#include "mg_error.h"
/*--------------------------------------------------------------------------*/
Variable_Decl::~Variable_Decl()
{
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::clear_deps()
{ untested();
  trace2("Variable_Decl::clear_deps", name(), deps().ddeps().size());
  data().clear();
}
/*--------------------------------------------------------------------------*/
// void Variable_Decl::new_deps()
// { untested();
//   assert(!_deps);
//   _deps = new TData;
// }
/*--------------------------------------------------------------------------*/
std::string const& Variable_Decl::name() const
{
  assert(_token);
  return _token->name();
}
/*--------------------------------------------------------------------------*/
Block const* Variable_Decl::scope() const
{ untested();
  auto b = prechecked_cast<Block const*>(owner());
  assert(b);
  return b;
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::new_var_ref()
{ untested();
  incomplete();
}
void Variable_Decl::new_var_ref_()
{
  assert(owner());
  auto l = prechecked_cast<Variable_List*>(owner());
  assert(l);

//  incomplete();
  assert(_token);
  if(auto m = dynamic_cast<Module*>(l->scope())){ untested();
    m->new_var_ref(_token);
  }else if(auto b = dynamic_cast<Block*>(l->scope())){
    b->new_var_ref(_token);
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::is_used_in(Base const*) const
{
  // return _token->is_used_in(b);
  // incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used_in(Base const* b) const
{ untested();
  if(auto p = dynamic_cast<Statement const*>(owner())) { untested();
    return p->is_used_in(b);
  }else{ untested();
  }
  // incomplete(); // later
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used() const
{ untested();
  assert(_token);
  return _token->is_used();
}
/*--------------------------------------------------------------------------*/
std::string Assignment::code_name() const
{ untested();
  assert(_lhsref);
  return _lhsref->code_name();
}
/*--------------------------------------------------------------------------*/
Data_Type const& Assignment::type() const
{
  //assert(_lhs->is_int() == _type.is_int());
  assert(_lhsref);
  return _lhsref->type();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_int() const
{
  return type().is_int();
}
/*--------------------------------------------------------------------------*/
bool Statement::update()
{
  trace0("Statement::update");
//  if(dynamic_cast<Block*>(parent_stmt())){ untested();
//    incomplete();
//  }else{ untested();
//    incomplete();
//  }
  return false;
}
/*--------------------------------------------------------------------------*/
void Statement::set_rdeps(TData const& t)
{ untested();
  for(auto x : t.sensitivities()){ untested();
    if(auto b = dynamic_cast<Branch*>(x)){ untested();
      _rdeps.push_back(b);
    }else{ untested();
      // incomplete(); // later
    }
  }
}
/*--------------------------------------------------------------------------*/
//Statement* Statement::parent_stmt()
//{ untested();
//  Block* b = scope();
//  if(auto x = dynamic_cast<Statement*>(b->owner())){ untested();
//    return x;
//  }else{ untested();
//    incomplete();
//    return NULL;
//  }
//}
/*--------------------------------------------------------------------------*/
bool Statement::is_reachable() const
{ untested();
  assert(scope());
  return scope()->is_reachable();
}
/*--------------------------------------------------------------------------*/
bool Statement::is_always() const
{
  assert(scope());
  return scope()->is_always();
}
/*--------------------------------------------------------------------------*/
bool Statement::is_never() const
{
  assert(scope());
  return scope()->is_never();
}
/*--------------------------------------------------------------------------*/
// generic?
Variable_List* Variable_List::deep_copy_(Block* owner, std::string prefix) const
{
  // return new Variable_List(this);
  // auto n = new Variable_List(*this);
  auto n = new Variable_List();
  attr.set_attributes(tag_t(n)) = attr.attributes(tag_t(this));

  n->_type = type();
  n->set_owner(owner);
  for(Variable_Decl const* i : _l){
    auto j = i->deep_copy(n, prefix);
    // owner->new_var_ref(j);
    n->_l.push_back(j);
  }
  return n;
}
/*--------------------------------------------------------------------------*/
bool SeqBlock::update()
{
  trace0("AnalogSeqBlock::update");
  bool ret = false;
  if(is_reachable()){
    for(auto i: *this){
      if(auto s = dynamic_cast<Statement*>(i)){
	ret |= s->update();
      }else{
      }
    }
  }else{
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
static bool is_output_var(tag_t t)
{
  ATTRIB_LIST_p const& a = attr.attributes(t);
  if(!a) {
  }else if(a->operator[](std::string("desc")) != "0"
         ||a->operator[](std::string("units")) != "0") {
    return true;
  }else{
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_output_var() const
{
  return :: is_output_var(tag_t(_lhsref));
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::new_data()
{
  assert(owner());
  auto l = prechecked_cast<Variable_List const*>(owner());
  assert(l);
  Module const* mod = dynamic_cast<Module const*>(l->owner()); // scope?
  Variable_List_Collection const* p=NULL;
  if(!mod){ untested();
  }else if(is_output_var(tag_t(this))) {
    p = &mod->variables();
  }else{
  }
  _data = new TData();
  if(p){
    _data->add_rdep(p);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
Variable_Decl* Variable_Decl::deep_copy(Base* b, std::string s) const
{
  assert(b);
  auto l = prechecked_cast<Variable_List*>(b);
  assert(l);

  auto n = new Variable_Decl;
  n->set_owner(b);
  assert(type());
  n->set_type(type());
  assert(n->type());
  attr.set_attributes(tag_t(n)) = attr.attributes(tag_t(l));
  n->new_data();
  n->_token = new Token_VAR_DECL(s+_token->name(), n, n->_data);
  attr.set_attributes(tag_t(n->_token)) = attr.attributes(tag_t(l));
  assert(n->_token->type());
  l->scope()->new_var_ref(n->_token);
  assert(n->_token->data());
  return n;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
