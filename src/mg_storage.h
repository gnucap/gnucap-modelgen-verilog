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
class SeqBlock;
class Variable_Access {
  typedef enum{
    xs_assign,
    xs_const_assign,
    xs_use
  }mode_t;
  struct xs{
    Token_VAR_REF* _v{nullptr};
    mode_t _mode;

    explicit xs(xs const& x) = default;
    xs(Token_VAR_REF* v, mode_t mode);
    bool is_use()const {return _mode == xs_use;}
    bool is_assign()const {return _mode == xs_assign || _mode == xs_const_assign;}
    bool is_constant()const {return _mode == xs_const_assign;}

    Block const* scope()const;
    Block const* var_scope()const;
  };
  std::list<xs> _list;
  std::map<Token_VAR_REF*, STORAGE_TYPE> _map;
public:
  void clear() { _list.clear(); }
  void push(xs const& x) { _list.push_back(xs(x)); }
  void push_assign(Token_VAR_REF* a);
  void push_use(Token_VAR_REF* v);
  void propagate(SeqBlock const* scope);
}; // Variable_Access
/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
