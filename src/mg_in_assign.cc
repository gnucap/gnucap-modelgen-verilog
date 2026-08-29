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
int ContinuousAssign::_count;
/*--------------------------------------------------------------------------*/
class AssignLogic : public Element_2 {
  NetAssignment const* _assign{nullptr};
//  ContinuousAssign const* _ctx{nullptr};
  std::string _arg;
  New_Port_List _ports;
public:
  explicit AssignLogic(NetAssignment const* a, Module& m)
    : Element_2(), _assign(a) {
    set_owner(&m);
    set_label(a->code_name());
    set_eval(a->code_name());
    // set_value(ctx->code_name());
    set_dev_type("__assign");
  //  set_state("_assign_" + a->code_name()); // BUG: used as value in set_params call?!

//    Expression_ arg(ctx->delay());
    // set_param("delay", arg);
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
//  std::string state()const override {untested();
//    return "???_st" + code_name();
//  }
  int num_states()const override { return !!state().size(); }
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
    auto c = prechecked_cast<ContinuousAssign*>(i);
    trace1("setup_assign", c->has_delay());
    assert(c);

    AssignLogic* first=nullptr;
    for(Base const* j : *i) {
      auto na = prechecked_cast<NetAssignment const*>(j);
      assert(na);
      auto p = new AssignLogic(na, m);
      if(!c->has_delay()){
      }else if(first){
	p->set_state(first->state()); // share
      }else{
	p->set_state("_assign_" + na->code_name()); // BUG: used as value in set_params call?!
	first = p;
      }
      c->set(p);
      m.push_back(p);
      assert(m.has_submodule());
    }
  }
}
/*--------------------------------------------------------------------------*/
void ContinuousAssign::parse(CS& f)
{
  _delay.set_owner(this);
  if(f >> '#'){
    Expression in(f);
    _delay.resolve_symbols(in);
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
  return _delay.size();
}
/*--------------------------------------------------------------------------*/
std::string ContinuousAssign::state() const
{
  auto e = prechecked_cast<Element_2 const*>(_one);
  assert(e);
  if(e->num_states()){
    return e->state();
  }else{
    return "";
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
