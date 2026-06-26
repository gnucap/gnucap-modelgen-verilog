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
#include "mg_module.h"
#include "mg_.h" // BUG
#include "mg_circuit.h"
/*--------------------------------------------------------------------------*/
class AssignLogic : public Element_2 {
  NetAssignment const* _assign{nullptr};
  std::string _arg;
  New_Port_List _ports;
public:
  explicit AssignLogic(NetAssignment const* a, std::string arg)
    : Element_2(), _assign(a), _arg(arg) {
    set_label(a->code_name());
    set_eval(a->code_name());
    set_value(arg);
    set_dev_type("__assign");

//    for(int i = 0; i < net_nodes(); ++i){
//      ports().push_back(port_name(i));
//    }
  }
  virtual New_Port_List const& ports()const {
    return _ports;
  }
  std::string const& port_name(int i)const override {
    assert(i >= 0);
    assert(i < 10);
    static std::string names[10] = {"out",
			"in1", "in2", "in3", "in4", "in5", "in6", "in7", "in8", "in9"};
    return names[i];
  }
  void parse(CS&)override {unreachable();}
//  std::string const/*&*/ eval()const { untested(); return "eval_" + short_label(); }
  std::string state()const override { return ""; }
  int num_states()const override { return 0; }
  int net_nodes()const override { return _assign->num_used_variables()+1; }
  std::string const& port_value(int i)const override {
    trace4("port_value", i, _arg, _assign->vars().size(), _assign->num_used_variables());
    assert(_assign);
    static std::string stringhack;
    if(i==0){
      stringhack = _assign->lhs().name();
    }else{
      --i;
      assert(i < net_nodes());
      auto b = _assign->vars().begin();
      trace1("port_value adv", i);
      std::advance(b, i);
      assert(b!=_assign->vars().end());
      stringhack = (*b)->name();
    }
    trace2("port_value", i, stringhack);
    return stringhack;
  }
};
/*--------------------------------------------------------------------------*/
int NetAssignment::count_variables() const
{
  return Expression_::num_used_variables();
}
/*--------------------------------------------------------------------------*/
void setup_assign(Module& m)
{
  auto a = prechecked_cast<Assign const*>(&m.assigns());
  assert(a);
  for(auto i : a->list()) {
    auto c = prechecked_cast<ContinuousAssign const*>(i);
    assert(c);

    std::string value = "";
    if(c->has_delay()){
      value = c->delay_string();
    }else{
    }

    for(Base const* j : *i) {
      auto na = prechecked_cast<NetAssignment const*>(j);
      assert(na);
      auto p = new AssignLogic(na, value);
      m.push_back(p);
      assert(m.has_submodule());
    }
  }
}
/*--------------------------------------------------------------------------*/
void ContinuousAssign::parse(CS& f)
{
  if(f >> '#'){
    f >> _delay;
  }else{
  }
  auto n = new NetAssignment(f, owner());
  push_back(n);
  while(f >> ','){
    f.skipbl();
    auto a = new NetAssignment(f, owner());
    push_back(a);
  }
  if(f >> ';'){
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
bool ContinuousAssign::has_delay() const
{
  return _delay != -1;
}
/*--------------------------------------------------------------------------*/
std::string ContinuousAssign::delay_string() const
{
  return to_string(_delay);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
