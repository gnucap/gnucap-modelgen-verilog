/*                       -*- C++ -*-
 * Copyright (C) 2026 Felix Salfelder
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
#ifndef MG_ASSIGN_H
#define MG_ASSIGN_H
/*--------------------------------------------------------------------------*/
#include "mg_code.h"
// #include "mg_circuit.h" // BUG: mg_in
/*--------------------------------------------------------------------------*/
class ContinuousAssign;
typedef Collection<ContinuousAssign> AssignList;
/*--------------------------------------------------------------------------*/
class Assign : public Owned_Base {
  AssignList _list;
public:
  explicit Assign();
  ~Assign();
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;

  bool has_block() const;
  AssignList const& list()const { return _list; }
  bool is_empty()const {return _list.is_empty(); }
  void push_back(Base*);
};
/*--------------------------------------------------------------------------*/
class NetAssignment : public Assignment {
  const int _seq;
public:
  explicit NetAssignment(CS& f, Base* o);
//   explicit NetAssignment() {}
  ~NetAssignment() {}
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  std::string code_name()const {
    return "_na" + to_string(_seq) + Assignment::code_name();
  }
  int count_variables()const;
  tokenlist vars()const {return Expression_::used_variables();}
};
/*--------------------------------------------------------------------------*/
class ContinuousAssign : public Block {
  int _delay{-1};
public:
  ContinuousAssign(){
  }
  ~ContinuousAssign(){
  }
private:
//  void new_block();
//  Block const* block_or_null() const{ untested(); return _block; }
//  Block* block(){ untested(); return _block; }

public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  void push_back(NetAssignment*);
  void push_back(Statement*);
};
/*--------------------------------------------------------------------------*/
class Module;
void setup_assign(Module& m);
void make_assign_elements(std::ostream& o, Module const& m);
void make_assign_common(std::ostream& o, Module const& m);
void make_assign_eval(std::ostream& o, Module const& m);
void make_assign_expand(std::ostream& o, Module const& m);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
