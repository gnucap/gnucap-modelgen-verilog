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
#include "mg_lib.h"
/*--------------------------------------------------------------------------*/
class RDeps;
class Probe;
class Dep /*Dynamic_Dep, DDep?*/{
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

  int order()const { untested();return _order;}
  bool is_linear()const { return _order<=_LINEAR; }
  bool is_quadratic()const { return _order<=_QUADRATIC; }

  bool same_data(Dep const& o)const;
  void set_any(){
    _order = _ANY;
  }
  bool propagate_rdeps(RDeps const&) const;

public:
  Probe const* operator->() const{ return _prb; }
  Probe const* operator*() const{ return _prb; }
  operator Probe const*() const{ return _prb; }
  std::string code_name()const;
};
/*--------------------------------------------------------------------------*/
class Sensitivities {
  typedef pSet<Base> set;
  typedef set::iterator iterator;
  typedef set::const_iterator const_iterator;
  set _s;
public:
  Sensitivities(Sensitivities const& o) : _s(o._s) {}
  explicit Sensitivities(){}
  ~Sensitivities(){}
public:
  void add(Base* x) { _s.insert(x); }
  void merge(Sensitivities const& s) { _s.merge(s._s); }
  bool empty()const {return !_s.size();}
  size_t size()const {return _s.size();}

  iterator begin() { untested();
    return _s.begin();
  }
  iterator end() { untested();
    return _s.end();
  }

  const_iterator begin()const {
    return _s.begin();
  }
  const_iterator end()const {
    return _s.end();
  }
};
/*--------------------------------------------------------------------------*/
class DDeps {
  typedef std::vector<Dep> D;
  D _s;
public:
  typedef D::const_iterator const_iterator;

  const_iterator begin() const;
  const_iterator end() const;
  size_t size() const{
    return _s.size();
  }
  Dep const& operator[](size_t i) const{
    return _s[i];
  }
  bool empty() const{
    return _s.empty();
  }
  Dep back(){ untested();
    return _s.back();
  }
  void clear(){
    _s.clear();
  }
  void set_any() {
    for(auto& d: _s) {
      d.set_any();
    }
  }
  std::pair<DDeps::const_iterator, bool> insert(Dep const&);
}; // DDeps
/*--------------------------------------------------------------------------*/
// use ValueRange from mg_.h?
class Range : public Base {
public:
  static Range _unknown;
  explicit Range() : Base() {}
public:
  Range* multiply(const Base*)const override { untested();
    return NULL;
  }
  Base* multiply(const Float*)const override	{ untested();untested(); return NULL;}
  Base* multiply(const String*)const override	{ untested();untested(); return NULL;}

private:
  void parse(CS&)override { untested();unreachable();}
  void dump(std::ostream&)const override { untested();unreachable();}
};
/*--------------------------------------------------------------------------*/
class RDeps : public pSet<Base const> {

};
/*--------------------------------------------------------------------------*/
class Branch;
class TData : public Base {
  DDeps _ddeps;
  Sensitivities _sens;
  // V _sens; // discrete_deps?
  // R _range; // discrete_deps?
  bool _offset{false}; // -> dynamic_deps.
  bool _constant{false};
public:
  static TData _no_deps;
public:
  explicit TData() : Base() {}
  explicit TData(TData const& o) : Base(), _ddeps(o._ddeps),
    _sens(o._sens),
    _offset(o._offset), _constant(o._constant) { }
  ~TData();
  TData* clone()const {
    return new TData(*this);
  }
  std::pair<DDeps::const_iterator, bool> insert(Dep const& d){
    return _ddeps.insert(d);
  }
  void update(TData const& other);
  Dep back(){ untested(); return _ddeps.back(); }
  void clear();

  DDeps& ddeps() {return _ddeps;}
  DDeps const& ddeps()const {return _ddeps;}

  void set_offset(bool v = true){_offset = v;}
  void set_constant(bool v = true){_constant = v;} // attrib/sens?
  size_t size()const {return _ddeps.size(); } // { + _rdeps.size(); }
  bool is_offset()const {return _offset;}
  bool is_constant()const {return _constant;}
  bool is_linear()const;
  bool is_quadratic()const;
  void set_any() { _ddeps.set_any(); }
public: // sens
  void add_sens(Base* x) {
    _sens.add(x);
  }
  void add_sens(Sensitivities const& s){
    _sens.merge(s);
  }
  bool has_sensitivities()const;
  Sensitivities const& sensitivities()const {return _sens; }
  Sensitivities& sensitivities() {return _sens; }
private:
  void parse(CS&)override { untested();unreachable();}
  void dump(std::ostream&)const override { untested();unreachable();}
public:
  TData& operator=(TData const& d){
    clear();
    update(d);
    return *this;
  }
  Base* combine(const Base* X)const;
  TData* multiply(const Base* X)const override; // { untested();incomplete(); return NULL;}
  Base* subtract(const Base*)const override	{ untested();incomplete(); return NULL;}
  Base* r_subtract(const Base*)const override	{ untested();incomplete(); return NULL;}
  Base* divide(const Base* X)const override;  //{ untested();incomplete(); return NULL;}
  Base* r_divide(const Base*)const override   {incomplete(); return NULL;}	
  Base* modulo(const Base*)const override     {incomplete(); return NULL;}	

  Base* multiply(const Float*)const override	{ untested();unreachable(); return NULL;}
  Base* subtract(const Float*)const override	{ untested();unreachable(); return NULL;}
  Base* r_subtract(const Float*)const override	{ untested();unreachable(); return NULL;}
  Base* divide(const Float*)const override	{ untested();unreachable(); return NULL;}
  Base* r_divide(const Float*)const override    {unreachable(); return NULL;}	
  Base* modulo(const Float*)const override      {unreachable(); return NULL;}	

  Base* multiply(const String*)const override	{ untested();unreachable(); return NULL;}
  Base* subtract(const String*)const override	{ untested();unreachable(); return NULL;}
  Base* r_subtract(const String*)const override	{ untested();unreachable(); return NULL;}
  Base* divide(const String*)const override	{ untested();unreachable(); return NULL;}
  Base* r_divide(const String*)const override   {unreachable(); return NULL;}	
  Base* modulo(const String*)const override     {unreachable(); return NULL;}	
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
