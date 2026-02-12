/*                       -*- C++ -*-
 * Copyright (C) 2025 Felix Salfelder
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
/*--------------------------------------------------------------------------*/
#include "mg_storage.h"
#ifndef NDEBUG
#include "mg_module.h"
#endif
/*--------------------------------------------------------------------------*/
void Variable_Access::push_init(Token_VAR_REF* v)
{ untested();
  assert(v);
  trace1("XS push init", v->name());
  unreachable();
  //_list.push_back(xs(v, xs_init, true));
}
/*--------------------------------------------------------------------------*/
void Variable_Access::push_assign(Token_VAR_REF*, bool, bool)
{
  unreachable();
}
#if 0
{ untested();
  assert(v);
  assert(v->item());
  trace2("XS push assign", v->name(), is_const);
  auto A = prechecked_cast<Assignment*>(v->mutable_item());
  assert(A);

  // _list.push_back(xs(A));
  assert(v);
  xs x(v, is_const?xs_const_assign:xs_assign, always);
  assert(x.is_constant() == is_const);
  _list.push_back(x);
  assert(_list.back().is_constant() == is_const);
}
#endif
/*--------------------------------------------------------------------------*/
void Variable_Access::assign_variable(Token_VAR_REF const* v,
    bool is_const, bool)
{
  if(!v){
    // af??
  }else{
    if(is_const){
      _map[v].assign_const();
    }else{
      _map[v].assign();
    }
    trace3("VAC assign", is_const, v->name(), _map[v]);
  }
}
/*--------------------------------------------------------------------------*/
void Variable_Access::init_variable(Token_VAR_REF const* v)
{
  trace1("VAC init", v->name());
  _map[v].init();
}
/*--------------------------------------------------------------------------*/
void Variable_Access::event_variable(Token_VAR_REF const* v)
{
  trace1("VAC init", v->name());
  _map[v].event();
}
/*--------------------------------------------------------------------------*/
void Variable_Access::use_variable(Token const* v)
{
  trace1("VAC use", v->name());
  _map[v].use();
}
/*--------------------------------------------------------------------------*/
Block const* Variable_Access::xs::scope() const
{ untested();
  assert(_v);
  return _v->scope();
}
/*--------------------------------------------------------------------------*/
Block const* Variable_Access::xs::var_scope() const
{ untested();
  assert(_v);
  if(is_assign()){ untested();
    if(auto b = dynamic_cast<Variable_Decl const*>(_v->item())){ untested();
      // AF output arg hack
      return b->scope();
    }else{ untested();
    }

    auto a = prechecked_cast<Assignment const*>(_v->item());
    assert(a);
    return a->lhs_scope();

  }else{ untested();
    assert(_v);
    return _v->scope();
  }
}
/*--------------------------------------------------------------------------*/
Variable_Access::xs::xs(Token_VAR_REF* v, Variable_Access::mode_t mode,
    bool always) : _v(v), _mode(mode), _always(always)
{ untested();
}
/*--------------------------------------------------------------------------*/
void Variable_Access::submit(assignment const& p) const
{
  auto tt = prechecked_cast<Token_VAR_REF const*>(p.first);
  assert(tt);
  trace1("VAC submitting", tt->name());
  p.second.apply(p.first);
}
/*--------------------------------------------------------------------------*/
// called from AnalogSeqBlock::parse
void Variable_Access::collect(SeqBlock const* Scope)
{
  trace1("VAC collect", Scope->identifier());
  for(Base const* b : *Scope) {
    auto s = prechecked_cast<Statement const*>(b);
    assert(s);
    trace1("VAC", s->val_string());

    s->submit_variable_access(*this);
  }

  if(Scope->is_ctx_initial()) {
    prune_dynamic();
  }else{
  }

  sift_locals(Scope);
}
/*--------------------------------------------------------------------------*/
void Variable_Access::prune_dynamic()
{
  for(auto i = _map.begin(); i != _map.end(); ++i) {
    STORAGE_TYPE& s = i->second;
    s.prune_dynamic();
  }
}
/*--------------------------------------------------------------------------*/
void Variable_Access::sift_locals(Block const* Scope)
{
//   bool is_initial_ctx = false;
//   if(auto sb = dynamic_cast<SeqBlock const*>(Scope)) {
//     is_initial_ctx =  sb->is_ctx_initial();
//   }else{ untested();
//   }
#ifndef NDEBUG
  if(dynamic_cast<Module const*>(Scope)) {
    trace0("VAC sift module");
  }else{
    trace0("VAC sift no module");
  }
#endif

  for(auto i = _map.begin(); i != _map.end(); ) {
    auto tt = dynamic_cast<Token_VAR_REF const*>(i->first);
    if(!tt){ untested();
      incomplete(); // af??
      ++i;
      continue;
    }else{
    }
    bool local = tt->scope() == Scope;
    if(local){
      submit(*i);
      auto j = i;
      ++j;
      _map.erase(i);
      i = j;

    }else{
      ++i;
    }
  }
}
/*--------------------------------------------------------------------------*/
Variable_Access& Variable_Access::operator&=(Variable_Access const& x)
{
  for(auto i : x._map){
    _map[i.first] &= i.second;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
Variable_Access Variable_Access::operator|(Variable_Access const& other) const
{
  Variable_Access ret;

  map::const_iterator i = _map.begin();
  map::const_iterator j = other._map.begin();

  while(i!=_map.end() && j!=other._map.end()){
    if(i->first == j->first){
      ret.push(i->first, i->second | j->second);
      ++i;
      ++j;
    }else if(i->first < j->first){
      ret.push(maybe(*i++));
    }else{
      ret.push(maybe(*j++));
    }
  }

  while(i!=_map.end()) {
    ret.push(maybe(*i++));
  }

  while(j!=other._map.end()){
    ret.push(maybe(*j++));
  }

  return ret;
}
/*--------------------------------------------------------------------------*/
STORAGE_TYPE::set_t STORAGE_TYPE::set_t::maybe() const
{
      // s_unset = 0,
      // s_maybe = 1,  // maybe set
      // s_maybe_init, // (maybe) set, in init
      // s_event,      // set during event
      // s_const,      // certainly set. constant
      // s_set         // certainly set
  if(_s >= s_const){
    set_t r;
    r._s = s_maybe;
    return r;
  }else{
    return *this;
  }
}
/*--------------------------------------------------------------------------*/
STORAGE_TYPE STORAGE_TYPE::maybe() const
{
  set_t s = _set.maybe();
  use_t u = _use;

  STORAGE_TYPE r(s, u);
  trace2("STT maybe", *this, r);
  return r;
}
/*--------------------------------------------------------------------------*/
STORAGE_TYPE::set_t STORAGE_TYPE::set_t::operator|(STORAGE_TYPE::set_t const& o) const
{
  int s = _s;
  int os = o._s;
  if(s == s_event){
    return *this;
  }else if(os == s_event) {
    return o;
  }else if(s > os){
    std::swap(s, os);
  }else{
  }

//  STT|  *this=(maybe:not_used)  o=(const:not_used)  ret=(const:not_used)

  set_t r;
  if(s == s_maybe && os == s_const) {
    r._s = s_maybe;
  }else if(s == s_maybe_init && os == s_const) { untested();
    unreachable();
  }else if(s >= s_const){
    r._s = s_set;
  }else if(!os){
  }else if(s){
    r._s = d(os);
  }else if(!s && os == s_maybe){ itested();
    r._s = s_maybe;
  }else if(!s && os == s_set){ itested();
    r._s = s_maybe;
  }else if(!s && os == s_const){ itested();
    r._s = s_maybe;
  }else{
    error(bDANGER, "internal error %d %d\n", s, os);
    incomplete();
  }

  return r;

}
/*--------------------------------------------------------------------------*/
STORAGE_TYPE STORAGE_TYPE::operator|(STORAGE_TYPE const& o) const
{
  use_t u = _use;
  use_t ou = o._use;

    // u_unused = 0,
    // u_used,
    // u_init,
    // u_unset

//   STT|  *this=(maybe:not_used)  o=(const:not_used)  ret=(const:not_used)

  STORAGE_TYPE ret(_set | o._set, std::max(u, ou));
  trace3("STT|", *this, o, ret);
  return ret;
}
/*--------------------------------------------------------------------------*/
STORAGE_TYPE::set_t& STORAGE_TYPE::set_t::operator&=(STORAGE_TYPE::set_t const& o)
{
//      s_unset = 0,
//      s_maybe = 1,  // maybe set
//      s_maybe_init, // (maybe) set, in init
//      s_event,      // set during event
//      s_const,      // certainly set. constant
//      s_set         // certainly set
// @#@STT in  *this=(const:not_used)  o=(evt:not_used)  o.is_state()=0
// @#@STT out3  *this=(evt:not_used)
  d new_set = _s;
  switch(_s){
  case s_unset:
    new_set = o._s;
    break;
  case s_maybe:
    switch(o._s){
    case s_unset:
      new_set = s_maybe;
      break;
    case s_maybe:itested();
      new_set = s_maybe;
      break;
    case s_maybe_init: untested();
      new_set = s_maybe;
      break;
    case s_event: untested();
      new_set = o._s;
      break;
    case s_const:
      new_set = o._s;
      break;
    case s_set:
      new_set = o._s;
      break;
    }
    break;
  case s_maybe_init:
    switch(o._s){
    case s_unset:
      break;
    case s_maybe:
      break;
    case s_maybe_init:
      break;
    case s_event:
      new_set = o._s;
      break;
    case s_const:
      new_set = o._s;
      break;
    case s_set:
      new_set = o._s;
      break;
    }
    break;
  case s_event:
    break;
  case s_const:
    switch(o._s){
    case s_unset:
      break;
    case s_maybe:
      new_set = s_set;
      break;
    case s_maybe_init: untested();
      unreachable();
      break;
    case s_event:
      new_set = s_set;
      break;
    case s_const:
      new_set = s_set;
      break;
    case s_set:
      new_set = s_set;
      break;
    }
    break;
  case s_set:
    switch(o._s){
    case s_unset:
      break;
    case s_maybe:
      new_set = s_set;
      break;
    case s_maybe_init: untested();
      unreachable();
      break;
    case s_event:
      new_set = s_event;
      break;
    case s_const:
      new_set = s_set;
      break;
    case s_set:
      new_set = s_set;
      break;
    }
    break;
  }

  _s = new_set;
  return *this;
}
/*--------------------------------------------------------------------------*/
STORAGE_TYPE& STORAGE_TYPE::operator&=(STORAGE_TYPE const& o)
{
  trace3("STT in", *this, o, o.is_state());
  auto new_use = _use;
  if(is_unset() && !_use){
    *this = o;
    trace1("STT out0", *this);
  }else if(!_set.is_set() && o.is_state()){
    _set &= o._set;
    _use = u_unset;
    trace1("STT out1", *this);
  }else if(_set.is_init() && !_use && o.is_unset() && o._use == u_unset){
    _use = u_used;
    trace1("STT out1b", *this);
  }else if(!_set.is_set() && !o._set.is_set() && o.is_used()){
    _set &= o._set;
    _use = u_unset;
    trace1("STT out2", *this);
  }else{
    _set &= o._set;
    if(_use == u_init && o._use == u_init) { untested();
    }else if(_use == u_unset) {
    }else if(_use || o._use){
      new_use = u_used;
    }else{
    }
    _use = new_use;
    trace1("STT out3", *this);


  }

  return *this;
}
/*--------------------------------------------------------------------------*/
void STORAGE_TYPE::event()
{
  _set.set_event();
}
/*--------------------------------------------------------------------------*/
void STORAGE_TYPE::init()
{
  _set.set_init();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void STORAGE_TYPE::prune_dynamic()
{
  trace1("STORAGE_TYPE::prune_dynamic", *this);
  if(_use == u_init){
    // this wasn't necessary if initial use was recorded properly
  }else if(_use){
    _use = u_init;
  }else{
  }
  trace1("STORAGE_TYPE::prune_dynamic1", *this);

  assert(!is_state());
}
/*--------------------------------------------------------------------------*/
void STORAGE_TYPE::apply(Token const* t) const
{
  auto tt = prechecked_cast<Token_VAR_REF const*>(t);
  if(!tt){ untested();
    incomplete();
    return; // // AF?
  }else{
  }

  auto A = dynamic_cast<Variable_Decl*>(tt->mutable_item());

  if(!A){ untested();
    incomplete();
    // AF??
  }else if(is_state()){
    trace2("STORAGE_TYPE::apply state", tt->name(), *this);
    assert(!is_common());
    A->set_state_var();
  }else if(is_common()){
    assert(!is_state());
    trace2("STORAGE_TYPE::apply common", tt->name(), *this);
    A->set_common_var();
  }else if(is_temporary()){
    trace2("STORAGE_TYPE::apply tmp", tt->name(), *this);
    A->set_temporary_var();
  }else{
    trace2("STORAGE_TYPE::apply unused", tt->name(), *this);
    assert(is_unused());
  }
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
