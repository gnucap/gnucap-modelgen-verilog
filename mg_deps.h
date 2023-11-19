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
  explicit Dep(Probe const* p, dep_order d) : _prb(p), _order(d) {}

  int order()const {return _order;}
  bool is_linear()const { return _order<=_LINEAR; }
  bool is_quadratic()const { return _order<=_QUADRATIC; }

  bool same_data(Dep const& o)const;

public:
  Probe const* operator->() const{ return _prb; }
  Probe const* operator*() const{ return _prb; }
  operator Probe const*() const{ return _prb; }
};
extern Dep mg_const_dep; // REMOVE?
/*--------------------------------------------------------------------------*/
class Deps : public Base {
  typedef std::vector<Dep> S;
  typedef S::const_iterator const_iterator;
  /* mutable */ S _s;
  bool _offset{false};
public:
  static Deps _no_deps;
public:
  explicit Deps() : Base() {}
  explicit Deps(Deps const& o) : Base(), _s(o._s), _offset(o._offset) { }
  ~Deps();
  Deps* clone()const {
    return new Deps(*this);
  }
  std::pair<const_iterator, bool> insert(Dep const&);
  void update(Deps const& other);
  Dep back(){
    return _s.back();
  }
  void clear(){
    _s.clear();
  }
  void set_offset(bool v = true){_offset = v;}
  bool is_offset() const {return _offset;}
  bool is_linear() const;
  bool is_quadratic() const;
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
  Base* combine(const Base* X)const;
  Deps* multiply(const Base* X)const override; // {incomplete(); return NULL;}
  Base* subtract(const Base*)const override	{incomplete(); return NULL;}
  Base* r_subtract(const Base*)const override	{incomplete(); return NULL;}
  Base* divide(const Base* X)const override;  //{incomplete(); return NULL;}
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
// use ValueRange from mg_.h?
class Range : public Base {
public:
  static Range _unknown;
  explicit Range() : Base() {}
public:
  Range* multiply(const Base*)const override {
    return NULL;
  }
  Base* multiply(const Float*)const override	{untested(); return NULL;}
  Base* multiply(const String*)const override	{untested(); return NULL;}

private:
  void parse(CS&)override {unreachable();}
  void dump(std::ostream&)const override {unreachable();}
};
/*--------------------------------------------------------------------------*/
class Attrib : public Base {
  Deps const* _deps{NULL};
  Range const* _range{NULL};
//  DataType _type;?
public:
  explicit Attrib() : Base() {}
  ~Attrib() {
    delete _deps;
    _deps = NULL;
    delete _range;
    _range = NULL;
  }
private:
  explicit Attrib(Attrib const&) : Base() { }
  Attrib* clone()const {
    return new Attrib(*this);
  }
  void set_deps(Deps const* d) { assert(!_deps); _deps=d; }
  void set_range(Range const* r) { assert(!_range); _range=r; }
public:
  Deps const& deps() const{
    if(_deps){
      return *_deps;
    }else{
      return Deps::_no_deps;
    }
  }
  Range const& range() const{
    if(_range){
      return *_range;
    }else{
      return Range::_unknown;
    }
  }
private:
  void parse(CS&)override {unreachable();}
  void dump(std::ostream&)const override {unreachable();}
public:
//  Attrib& operator=(Attrib const& o){
//    _deps = o._deps;
//    _range = o._range;
//    return *this;
//  }
  Base* multiply(const Base* X)const override	{
    Attrib* n = clone();
    n->set_deps(deps().multiply(X));
    n->set_range(range().multiply(X));
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
