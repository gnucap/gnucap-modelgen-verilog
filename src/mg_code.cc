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
  auto l = prechecked_cast<Variable_Stmt*>(owner());
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
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// generic?
Variable_Stmt* Variable_Stmt::deep_copy_(Block* owner, std::string prefix) const
{
  // return new Variable_Stmt(this);
  // auto n = new Variable_Stmt(*this);
  auto n = new Variable_Stmt();
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
static bool is_output_var(tag_t t)
{
  ATTRIB_LIST_p const& a = attr.attributes(t);
  if(!a) {
  }else if(a->operator[](std::string("desc")) != "0"
         ||a->operator[](std::string("units")) != "0") {
    return true;
  }else{ untested();
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_output_var() const
{
  return :: is_output_var(tag_t(_lhsref));
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_state_var() const
{
  return _lhsref->is_state_var();
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::new_data()
{
  assert(owner());
  auto l = prechecked_cast<Variable_Stmt const*>(owner());
  assert(l);
  Module const* mod = dynamic_cast<Module const*>(l->owner()); // scope?
  Variable_List_Collection const* p=NULL;
  if(!mod){ untested();
  }else if(is_output_var(tag_t(this))) {
    p = &mod->variables();
  }else{
  }
  // _rdeps.add(p);
   _data = new TData();
   if(p){
  incomplete();
   }
 //    _data->add_rdep(p);
 //  }else{ untested();
 //  }
}
/*--------------------------------------------------------------------------*/
Variable_Decl* Variable_Decl::deep_copy(Base* b, std::string s) const
{
  assert(b);
  auto l = prechecked_cast<Variable_Stmt*>(b);
  assert(l);

  auto n = new Variable_Decl;
  n->set_owner(b);
  assert(type());
  n->set_type(type());
  assert(n->type());
  attr.set_attributes(tag_t(n)) = attr.attributes(tag_t(l));
  n->new_data();
  assert(n->_data);
  n->_token = new Token_VAR_DECL(s+_token->name(), n, n->_data);
  attr.set_attributes(tag_t(n->_token)) = attr.attributes(tag_t(l));
  assert(n->_token->type());
  l->scope()->new_var_ref(n->_token);

  if(n->_token->data()){
  }else{ untested();
  }
  return n;
}
/*--------------------------------------------------------------------------*/
// inline?
bool Expression_::is_used_in(Base const*) const
{
 //  for(auto& i : data().rdeps()) { untested();
 //    trace2("AnalogEvtCtrlStmt::is_used_in", i, b);
 //    if(i == b){ untested();
 //      return true;
 //    }else{ untested();
 //    }
 //  }
  {
    return false;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
