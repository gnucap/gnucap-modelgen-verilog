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
 * dependencies in expressions
 */
#include <m_base.h>
#include "mg_analog.h" // Probe
#include "mg_deps.h"
/*--------------------------------------------------------------------------*/
TData TData::_no_deps;
Range Range::_unknown;
/*--------------------------------------------------------------------------*/
// is linear, as in "map"
bool TData::is_linear() const
{
  if(is_offset()) {
    return false;
  }else{
  }
  for(auto const& i : ddeps()) {
    if (i.is_linear()) {
    }else{
      return false;
    }
  }
  return true;
}
/*--------------------------------------------------------------------------*/
// is quadratic, as in "form"
bool TData::is_quadratic() const
{ untested();
  if(is_offset()) { untested();
    return false;
  }else{ untested();
  }
  for(auto const& i : ddeps()) { untested();
    if (i.is_quadratic()) { untested();
    }else{ untested();
      return false;
    }
  }
  return true;
}
/*--------------------------------------------------------------------------*/
Base* TData::combine(const Base* X)const
{
  auto n = clone();
  auto o = prechecked_cast<TData const*>(X);
  assert(o);
  n->update(*o);
  n->set_offset();
  return n;
}
/*--------------------------------------------------------------------------*/
TData* TData::multiply(const Base* X)const
{
  auto n = clone();
  auto o = prechecked_cast<TData const*>(X);
  assert(o);
  n->update(*o);
  if (o->is_offset()){
    n->set_offset();
  }else{
  }

  if(is_linear()){
    // TODO incomplete();
  }else{
  }
  if(o->is_linear()){
    // TODO incomplete();
  }else{
  }

  return n;
}
/*--------------------------------------------------------------------------*/
Base* TData::divide(const Base* X)const
{
  auto n = clone();
  auto o = prechecked_cast<TData const*>(X);
  assert(o);
  n->update(*o);
  if (is_offset()){
    // denominator is offset
    n->set_offset();
  }else if (o->is_offset()){
  }else{
  }

  if(o->ddeps().size()==0){
    // divide by constant...
    assert(is_linear() == n->is_linear());
  }else{
  }
  return n;
}
/*--------------------------------------------------------------------------*/
void TData::update(TData const& other)
{
  for(auto& i : other.ddeps()){
    insert(i);
  }
  trace1("update", other.sensitivities().empty());
  _sens.merge(other.sensitivities());
}
/*--------------------------------------------------------------------------*/
TData::~TData()
{
  _s.clear();
}
/*--------------------------------------------------------------------------*/
std::pair<DDeps::const_iterator, bool> DDeps::insert(Dep const& x)
{
  for(auto s = begin(); s!=end(); ++s){
    if (s->same_data(x)){
      return std::make_pair(s, false);
    }else{
    }
  }
  _s.push_back(x);
  return std::make_pair(begin()+(int(size())-1), true);
}
/*--------------------------------------------------------------------------*/
bool Dep::same_data(Dep const& o) const
{
  assert(_prb);
  assert(o._prb);
  return _prb->same_data(*o._prb);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
