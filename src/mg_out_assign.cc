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
#include "mg_assign.h"
#include "mg_logic.h"
#include "mg_module.h"
#include "mg_out.h"
/*--------------------------------------------------------------------------*/
void make_assign_common(std::ostream& o, Module const& m)
{
  auto a = prechecked_cast<Assign const*>(&m.assigns());
  assert(a);
  for(auto i : a->list()) {
    for(Base const* j : *i) {
      auto na = prechecked_cast<NetAssignment const*>(j);
      assert(na);
      o << "// ASSIGN " << na->code_name() << "\n";
//      AssignLogic p(na);
//      make_common_logic(o, p);
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_one_assign_eval(std::ostream& o, NetAssignment const& na)
{
  o << "// ASSIGN EVAL\n";
  o << "LOGICVAL COMMON_" << na.code_name()
    << "::logic_eval(node_l const* n, int i)const\n{\n";
  o__ "assert(i == " << na.count_variables() << ");\n";

  int ii = 0;
  for(Token const* i : na.vars()){
    if(auto n = dynamic_cast<Token_NODE const*>(i)){
      o__ "LOGICVAL _v_" << n->code_name() << " = n[" << ii++ << "]->lv();\n";
    }else{ untested();
      incomplete();
    }
  }

  std::string v = make_cc_expression(o, na, false, "logic");
  o__ "return " << v << ";\n";
  o << "}\n";
}
/*--------------------------------------------------------------------------*/
void make_assign_eval(std::ostream& o, Module const& m)
{
  auto a = prechecked_cast<Assign const*>(&m.assigns());
  assert(a);
  for(auto i : a->list()) {
    for(Base const* j : *i) {
      auto na = prechecked_cast<NetAssignment const*>(j);
      assert(na);
      make_one_assign_eval(o, *na);
    }
  }
}
/*--------------------------------------------------------------------------*/
void make_assign_elements(std::ostream&, Module const&)
{
  return; // circuit
#if 0
  auto a = prechecked_cast<Assign const*>(&m.assigns());
  assert(a);
  for(auto i : a->list()) {
    for(Base const* j : *i) {
      auto na = prechecked_cast<NetAssignment const*>(j);
      assert(na);
      o__ "COMPONENT const* " << na->code_name() << "{nullptr};\n";
    }
  }
#endif
}
/*--------------------------------------------------------------------------*/
void make_assign_expand(std::ostream& , Module const& m)
{
  auto a = prechecked_cast<Assign const*>(&m.assigns());
  assert(a);
  for(auto i : a->list()) {
    for(Base const* j : *i) {
      auto na = prechecked_cast<NetAssignment const*>(j);
      assert(na);
 //     AssignLogic p(na);
//      make_one_assign_expand(o, p);
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
