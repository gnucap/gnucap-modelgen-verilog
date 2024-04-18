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
{ untested();
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
{ untested();
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
  assert(owner());
  incomplete();
//   owner()->new_var_ref(this);
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::is_used_in(Base const*) const
{ untested();
  // return _token->is_used_in(b);
  incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used_in(Base const* b) const
{ untested();
  if(auto p = dynamic_cast<Statement const*>(owner())) { untested();
    return p->is_used_in(b);
  }else{ untested();
  }
  incomplete();
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
{ untested();
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
      incomplete();
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
Variable_List* Variable_List::deep_copy(Block* owner, std::string prefix) const
{
  // return new Variable_List(this);
  // auto n = new Variable_List(*this);
  auto n = new Variable_List();
  attributes(n) = attributes(this);
  n->_type = type();
  n->set_owner(owner);
  for(auto i : *this){
    auto j = i->deep_copy(n, prefix);
    attributes(j) = attributes(n);
    owner->new_var_ref(j);
    n->push_back(j);
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
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
