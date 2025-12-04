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
// void Variable_Decl::new_deps()
// { untested();
//   assert(!_deps);
//   _deps = new TData;
// }
/*--------------------------------------------------------------------------*/
Block const* Variable_Decl::scope() const
{
  assert(owner());
  if(auto vs = dynamic_cast<Variable_Stmt const*>(owner())) {
      // AF hack
    return vs->scope();
  }else{ untested();
    auto b = prechecked_cast<Block const*>(owner());
    assert(b);
    return b;
  }
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::new_var_ref()
{
  assert(owner());
  auto l = prechecked_cast<Variable_Stmt*>(owner());
  assert(l);

//  incomplete();
  if(auto m = dynamic_cast<Module*>(l->scope())){ untested();
    m->new_var_ref(&token());
  }else if(auto b = dynamic_cast<Block*>(l->scope())){
    b->new_var_ref(&token());
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
  if(attr.has_attributes(tag_t(this))) {
    attr.set_attributes(tag_t(n)) = attr.attributes(tag_t(this));
  }else{
  }

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
Variable_Decl* Variable_Decl::deep_copy(Base* b, std::string s) const
{
  assert(b);
  auto l = prechecked_cast<Variable_Stmt*>(b);
  assert(l);

  auto n = new Variable_Decl(s + token().name());
  n->set_owner(b);
  n->new_data();
  assert(type());
  n->set_type(type());
  assert(n->type());
  if(attr.has_attributes(tag_t(l))) {
    attr.set_attributes(tag_t(n)) = attr.attributes(tag_t(l));
  }else{
  }
  if(attr.has_attributes(tag_t(l))) {
    attr.set_attributes(tag_t(&n->token())) = attr.attributes(tag_t(l));
  }else{
  }
  if(n->token().type()){
  }else{
    // bug?
  }
  l->scope()->new_var_ref(&n->token());

  if(n->token().data()){
  }else{ untested();
  }

  if(is_override_var()){
    n->_stt = _stt;
  }else{
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
