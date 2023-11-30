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
 * utilities for modelgen
 */
#ifndef MG_LIB_H
#define MG_LIB_H
/*--------------------------------------------------------------------------*/
#include <set>
/*--------------------------------------------------------------------------*/
template<class T>
struct ptrCmp {
  bool operator()( T const* a, T const* b ) const {
    assert(a);
    assert(b);
    assert(a->key().size());
    assert(b->key().size());
    assert(a==b || (a->key() != b->key()));
    return a->key() < b->key();
  }
};
/*--------------------------------------------------------------------------*/
template<>
struct ptrCmp<Base const> {
  bool operator()( Base const* a, Base const* b ) const {
    return a < b;
  }
};
/*--------------------------------------------------------------------------*/
template<class T>
class pSet {
  typedef std::set<T*, ptrCmp<T>> set;
  set _s;
public:
  typedef typename set::iterator iterator;
  typedef typename set::const_iterator const_iterator;
public:
  explicit pSet(){}
  const_iterator begin()const { return _s.begin(); }
  const_iterator end()const { return _s.end(); }
  std::pair<iterator, bool> insert(T* t) { return _s.insert(t); }
  void merge(pSet const& s) {
     // _s.merge(s._s);// c++17
     for(auto x : s._s){
       _s.insert(x);
     }
  }
  size_t size()const {return _s.size();}
  bool empty()const {return _s.empty();}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
