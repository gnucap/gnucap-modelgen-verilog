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
#ifndef MG_STORAGE_H
#define MG_STORAGE_H
#include "mg_code.h"
/*--------------------------------------------------------------------------*/
// keep track of storage type..
// a variable that is used before it is set is a state variable.
// a variable that is not a state variable that is set after it has
// been used is a temporary
class STORAGE_TYPE {
public:
  struct set_t {
    enum d{
      s_unset = 0,
      s_maybe = 1,  // maybe set
      s_maybe_init, // (maybe) set, in init
      s_event,      // set during event
      s_const,      // certainly set. constant
      s_set         // certainly set
    }_s{s_unset};
    explicit set_t() = default;
    set_t(set_t const&) = default;
#ifdef DO_TRACE
    void dump(std::ostream&)const;
#endif
//    bool is_init()const {return _s == s_maybe_init;}
    bool is_unset()const {return _s == s_unset;}
    bool is_set()const {return _s >= s_const;}
    bool is_event()const {return _s == s_event;}
    bool is_const()const {return _s == s_const;}
    bool is_init()const {return _s == s_const || _s == s_maybe_init;}
    bool is_common()const {return _s == s_const || _s == s_maybe_init;}
    bool is_maybe()const {return _s == s_maybe;}
    bool is_maybe_init()const {return _s == s_maybe_init;}
    set_t maybe()const;
    void set_event() {_s = s_event;}
    void set_init() {_s = s_maybe_init;}
    void set_const() {_s = s_const;}
    void set() {_s = s_set;}
    set_t& operator&=(set_t const&);
    set_t operator|(set_t const&) const;
//    s_state,      // possibly used but maybe not set
  }_set;
  typedef enum {
    u_unused = 0,
    u_used,
    u_init,       // used in init (<= used & const?)
    u_unset       // used while (maybe) not set
  } use_t;
private:
#ifdef DO_TRACE
public:
#endif
  use_t _use{u_unused};
private:
  explicit STORAGE_TYPE(set_t const& s, use_t const& u): _set(s), _use(u) {}
public:
  explicit STORAGE_TYPE() {}
  STORAGE_TYPE(STORAGE_TYPE const& o) = default;
   //  : _st(o._st), _set(o._set), _use(o._use) {
  //  }
public:
  STORAGE_TYPE operator|(STORAGE_TYPE const&)const;
  STORAGE_TYPE maybe()const;
  STORAGE_TYPE& operator&=(STORAGE_TYPE const&);
  STORAGE_TYPE& operator=(STORAGE_TYPE const& o) = default;
  void init();
  void assign();
  void assign_const();
  void use();
  void event();
  void prune_dynamic();

private:
  bool is_unset()const{ return _set.is_unset(); }
  bool is_used()const{ return _use; }

public: // query
  bool is_common()const { return _use && _use!=u_unset && _use!=u_init && _set.is_common(); }
  bool is_temporary()const { return (_use && _set.is_set() && !_set.is_const())
                                 || (_use==u_init && _set.is_common())
                                 || (_use && is_unset())
                                 || (_use && _use!=u_unset && _set.is_maybe())
                                 || (_use==u_init); }
  bool is_state()const {
      return ( _use == u_unset && !is_unset() )
          || (_use && _set.is_event() );
  }
  bool is_unused()const { return _use == u_unused; }
  void apply(Token const*)const;
}; // STORAGE_TYPE
/*--------------------------------------------------------------------------*/
inline void STORAGE_TYPE::assign()
{
  STORAGE_TYPE s;
  s._set.set();
  *this &= s;
}
/*--------------------------------------------------------------------------*/
inline void STORAGE_TYPE::use()
{
  STORAGE_TYPE s;
  s._use = u_unset; // "unset use".
  *this &= s;
}
/*--------------------------------------------------------------------------*/
inline void STORAGE_TYPE::assign_const()
{
  STORAGE_TYPE s;
  s._set.set_const();
  *this &= s;
}
/*--------------------------------------------------------------------------*/
#ifdef DO_TRACE
inline std::ostream& operator<<(std::ostream& o, STORAGE_TYPE::set_t const& s)
{
  s.dump(o);
  return o;
}
/*--------------------------------------------------------------------------*/
inline std::ostream& operator<<(std::ostream& o, STORAGE_TYPE const& s)
{
  o << "(" << s._set << ":";
  static std::string str[11] = {
    "not_used",
    "used",
    "init",
    "used_unset"
  };
  o << str[s._use] << ")";
  return o;
}
/*--------------------------------------------------------------------------*/
inline void STORAGE_TYPE::set_t::dump(std::ostream& o) const
{
  static std::string str[11] = {
    "not_set",
    "maybe",
    "maybe_i",
    "evt",
    "const",
    "set"
  };
  o << str[_s];
}
#endif
/*--------------------------------------------------------------------------*/
class SeqBlock;
class Variable_Access {
  typedef std::map<Token const*, STORAGE_TYPE> map;
  typedef std::pair<Token const*, STORAGE_TYPE> assignment;
  typedef enum{
    xs_assign,
    xs_const_assign,
    xs_use,
    xs_init
  }mode_t;
  struct xs{
    Token_VAR_REF* _v{nullptr};
    mode_t _mode;
    bool _always{false};

    explicit xs(xs const& x) = default;
    xs(Token_VAR_REF* v, mode_t mode, bool always);
    bool is_use()const {return _mode == xs_use;}
    bool is_assign()const {return _mode == xs_assign || _mode == xs_const_assign
                                 || _mode == xs_init;}
    bool is_init()const {untested(); return _mode == xs_init;}
    bool is_always()const {return _always;}
    bool is_constant()const {return _mode == xs_const_assign;}

    Block const* scope()const;
    Block const* var_scope()const;
  };
//  std::list<xs> _list;
  map _map;
public:
  Variable_Access operator|(Variable_Access const& b) const;

public:
  void clear() { _map.clear(); }
  size_t size()const {return _map.size(); }
  void push_init(Token_VAR_REF* v);
  void init_variable(Token_VAR_REF const* v);
  void event_variable(Token_VAR_REF const* v);
  void push_assign(Token_VAR_REF* a, bool is_const, bool always);
  void assign_variable(Token_VAR_REF const* a, bool is_const, bool always);
  void use_variable(Token const* v);
  void collect(SeqBlock const* scope);
  Variable_Access& operator&=(Variable_Access const& x);
//private: // BUG? used in Analog::setup
  void sift_locals(Block const* scope);
  void prune_dynamic();
private:
//  void push(xs const& x) {_list.push_back(xs(x));}
  void push(Token const* k, STORAGE_TYPE const& t) { _map[k] = t; }
  assignment maybe(assignment const& a)const {
    return std::make_pair(a.first, a.second.maybe());
  }
  void push(assignment const& a) { _map[a.first] = a.second; }
  void submit(assignment const& p)const;
}; // Variable_Access
/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
