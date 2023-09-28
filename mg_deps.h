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
#ifndef MG_DEPS_H
#define MG_DEPS_H
#include <m_base.h>
/*--------------------------------------------------------------------------*/
class Probe;
class Dep {
public: // types
  typedef enum {
    _NONE = 0,
    _LINEAR,
    _QUADRATIC,
    _ANY,
  } dep_order;
private:
  Probe const* _prb{NULL};
  dep_order _order{_ANY};
public:
  Dep(Dep const& p) : _prb(p._prb), _order(p._order) {}
  explicit Dep(Probe const* p) : _prb(p) {}
  explicit Dep(Probe const* p, dep_order) : _prb(p), _order() {}

  bool is_linear() const{ return _order<=_LINEAR; }
  bool is_quadratic() const{ return _order<=_QUADRATIC; }

public:
  Probe const* operator->() const{ return _prb; }
  Probe const* operator*() const{ return _prb; }
  operator Probe const*() const{ return _prb; }
};
/*--------------------------------------------------------------------------*/
class Deps : public Base {
  typedef std::vector<Dep> S;
  typedef S::const_iterator const_iterator;
  /* mutable */ S _s;
public:
  explicit Deps() : Base() {}
  explicit Deps(Deps const& d) : Base(), _s(d._s) { untested(); }
  ~Deps();
  Deps* clone()const {
    return new Deps(*this);
  }
  std::pair<const_iterator, bool> insert(Dep const&);
  void update(Deps const& other){
    for(auto& i : other){
      insert(i);
    }
  }
  Dep back(){
    return _s.back();
  }
  void clear(){
    _s.clear();
  }
private:
  void parse(CS&)override {unreachable();}
  void dump(std::ostream&)const override {unreachable();}
public:
  Deps& operator=(Deps const& d){
    clear();
    update(d);
    return *this;
  }
  const_iterator begin() const;
  const_iterator end() const;
  size_t size() const{
    return _s.size();
  }
  bool empty() const{
    return _s.empty();
  }
  Base* multiply(const Base* X)const override	{ untested();
    auto n = clone();
    auto o = prechecked_cast<Deps const*>(X);
    assert(o);
    n->update(*o);
    return n;
  }
  Base* subtract(const Base*)const override	{incomplete(); return NULL;}
  Base* r_subtract(const Base*)const override	{incomplete(); return NULL;}
  Base* divide(const Base*)const override	{incomplete(); return NULL;}
  Base* r_divide(const Base*)const override   {incomplete(); return NULL;}	
  Base* modulo(const Base*)const override     {incomplete(); return NULL;}	

  Base* multiply(const Float*)const override	{unreachable(); return NULL;}
  Base* subtract(const Float*)const override	{unreachable(); return NULL;}
  Base* r_subtract(const Float*)const override	{unreachable(); return NULL;}
  Base* divide(const Float*)const override	{unreachable(); return NULL;}
  Base* r_divide(const Float*)const override    {unreachable(); return NULL;}	
  Base* modulo(const Float*)const override      {unreachable(); return NULL;}	

  Base* multiply(const String*)const override	{unreachable(); return NULL;}
  Base* subtract(const String*)const override	{unreachable(); return NULL;}
  Base* r_subtract(const String*)const override	{unreachable(); return NULL;}
  Base* divide(const String*)const override	{unreachable(); return NULL;}
  Base* r_divide(const String*)const override   {unreachable(); return NULL;}	
  Base* modulo(const String*)const override     {unreachable(); return NULL;}	
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
