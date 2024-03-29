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
#include "mg_code.h"
#include "mg_deps.h" // BUG?
#include "mg_token.h"
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
std::string const& Variable::name() const
{
  assert(_token);
  return _token->name();
}
/*--------------------------------------------------------------------------*/
Block const* Variable::scope() const
{
  auto b = prechecked_cast<Block const*>(owner());
  assert(b);
  return b;
}
/*--------------------------------------------------------------------------*/
void Variable::new_var_ref()
{ untested();
  assert(owner());
  owner()->new_var_ref(this);
}
/*--------------------------------------------------------------------------*/
std::string Assignment::code_name() const
{ untested();
  assert(_lhsref);
  return _lhsref->code_name();
}
/*--------------------------------------------------------------------------*/
Block const* Assignment::scope() const
{
  auto b = prechecked_cast<Block const*>(owner());
  assert(b);
  return b;
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
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
