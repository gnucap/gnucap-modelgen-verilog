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
{
  auto b = prechecked_cast<Block const*>(owner());
  assert(b);
  return b;
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::new_var_ref()
{ untested();
  assert(owner());
  owner()->new_var_ref(this);
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::is_used_in(Base const*) const
{
  // return _token->is_used_in(b);
  incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used_in(Base const*) const
{
  // return _expression->is_used_in(b);
  incomplete();
  return false;
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
bool Assignment::is_module_variable() const
{ untested();
  assert(_lhsref);
  return _lhsref->is_module_variable();
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::is_module_variable() const
{
  if(dynamic_cast<Module const*>(owner())){
    return true;
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
// TData const& Assignment::data() const
// { untested();
//   incomplete();
//   if(_my_lhs){ untested();
// //    assert(_my_lhs->deps());
//     return _my_lhs->deps();
// //  }else if(_deps){ untested();
// //    return *_deps;
//   }else{ untested();
//     return _rhs.deps();
//   }
// }
/*--------------------------------------------------------------------------*/
bool Statement::update()
{
//  if(dynamic_cast<Block*>(parent_stmt())){
//    incomplete();
//  }else{
//    incomplete();
//  }
  return false;
}
/*--------------------------------------------------------------------------*/
void Statement::set_rdeps(TData const& t)
{
  for(auto x : t.sensitivities()){
    if(auto b = dynamic_cast<Branch*>(x)){
      _rdeps.push_back(b);
    }else{
      incomplete();
    }
  }
}
/*--------------------------------------------------------------------------*/
//Statement* Statement::parent_stmt()
//{
//  Block* b = scope();
//  if(auto x = dynamic_cast<Statement*>(b->owner())){
//    return x;
//  }else{
//    incomplete();
//    return NULL;
//  }
//}
/*--------------------------------------------------------------------------*/
void BlockVarIdentifier::update()
{ untested();
  clear_deps();
  new_var_ref();
}
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
// vim:ts=8:sw=2:noet
