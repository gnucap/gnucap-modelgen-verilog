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
#include "mg_.h" // Probe
#include "mg_deps.h"
/*--------------------------------------------------------------------------*/
Deps Deps::_no_deps;
Range Range::_unknown;
/*--------------------------------------------------------------------------*/
// is linear, as in "map"
bool Deps::is_linear() const
{
  if(is_offset()) {
    return false;
  }else{
  }
  for(auto const& i : *this) {
    if (i.is_linear()) {
    }else{
      return false;
    }
  }
  return true;
}
/*--------------------------------------------------------------------------*/
// is quadratic, as in "form"
bool Deps::is_quadratic() const
{ untested();
  if(is_offset()) { untested();
    return false;
  }else{ untested();
  }
  for(auto const& i : *this) { untested();
    if (i.is_quadratic()) { untested();
    }else{ untested();
      return false;
    }
  }
  return true;
}
/*--------------------------------------------------------------------------*/
Base* Deps::combine(const Base* X)const
{
  auto n = clone();
  auto o = prechecked_cast<Deps const*>(X);
  assert(o);
  n->update(*o);
  n->set_offset();
  return n;
}
/*--------------------------------------------------------------------------*/
Deps* Deps::multiply(const Base* X)const
{
  auto n = clone();
  auto o = prechecked_cast<Deps const*>(X);
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
Base* Deps::divide(const Base* X)const
{
  auto n = clone();
  auto o = prechecked_cast<Deps const*>(X);
  assert(o);
  n->update(*o);
  if (is_offset()){
    // denominator is offset
    n->set_offset();
  }else if (o->is_offset()){
  }else{
  }

  if(o->size()==0){
    // divide by constant...
    assert(is_linear() == n->is_linear());
  }else{
  }
  return n;
}
/*--------------------------------------------------------------------------*/
void Deps::update(Deps const& other)
{
  for(auto& i : other){
    insert(i);
  }
}
/*--------------------------------------------------------------------------*/
Deps::~Deps()
{
  _s.clear();
}
/*--------------------------------------------------------------------------*/
std::pair<Deps::const_iterator, bool> Deps::insert(Dep const& x)
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
