/*                -*- C++ -*-
 * Copyright (C) 2025-26 Felix Salfelder
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
 * "discipline"
 */
#include "u_lang.h"
#include "globals.h"
#include "c_comand.h"
#include "e_node_type.h"
#include "e_cardlist.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DISCIPLINE : public NODE_TYPE {
  std::string _potential;
  std::string _flow;
public:
  explicit DISCIPLINE(std::string const&s) : NODE_TYPE(s) {}
  explicit DISCIPLINE(DISCIPLINE const&s) : NODE_TYPE(s) {}
  ~DISCIPLINE() {}

  NODE* clone()const override { return new DISCIPLINE(*this);}
  NODE* clone_instance()const override {incomplete(); return new DISCIPLINE(*this);}
//  std::string dev_type()const override {return short_label();}

public:
  void set_continuous() {set_analog();}
  void set_discrete()   {set_digital();}
//  bool is_continuous()const override {return _domain == dom_continuous;}
//  bool is_mixed()const override      {return _domain == dom_mixed;}
//  bool is_discrete()const override   {untested(); return _domain == dom_discrete;}

public:
  int param_count()const override { return 3; }
  using NODE::param_name;
  std::string param_name(int i)const override {
    switch(i){
    case 0: return "domain";
    case 1: return "potential";
    case 2: return "flow";
    default: unreachable(); return "???";
    }
  }
  std::string param_value(int i)const override {
    static std::string d[4] = {"default", "continuous", "discrete", "hybrid"};
    switch(i){
    case 0: return d[domain()];
    case 1: return _potential;
    case 2: return _flow;
    default: unreachable(); return "???";
    }
  }
  bool param_is_printable(int i)const override {
    switch(i){
    case 0: return domain();
    case 1: return _potential.size();
    case 2: return _flow.size();
    default: unreachable(); return false;
    }
  }
  int set_param_by_name(std::string n, std::string v)override {
    if(n=="domain"){
      if(v=="discrete"){
	set_digital();
      }else if(v=="continuous"){
	set_analog();
      }else{ untested();
	error(bWARNING, "invalid domain " + v);
	unset_domain();
      }
      return 0;
    }else if(n=="potential"){
      _potential = v;
      return 1;
    }else if(n=="flow"){
      _flow = v;
      return 2;
    }else{
      return NODE::set_param_by_name(n, v) + 3;
    }
  }
  void clear();
  NODE* deflate()override { return new LOGIC_NODE; }
private:
  int user_number()const override   { return INVALID_NODE;}
private:
  int flat_number()const override   {return INVALID_NODE;}
  int matrix_number()const override {unreachable(); return INVALID_NODE;}
};
/*--------------------------------------------------------------------------*/
class CMD_DISCIPLINE : public CMD {
  mutable node_t _node;
  NODE* _nn{nullptr};
public:
  CMD_DISCIPLINE() : CMD() {}
  CMD_DISCIPLINE(CMD_DISCIPLINE const& p) : CMD(p) {
  }
  ~CMD_DISCIPLINE(){
    delete _nn;
    _node = _nn = nullptr;
  }
  CARD* clone()const override { return new CMD_DISCIPLINE(*this); }
  CARD* clone_instance()const override {
    if(_node) {
      return _node->clone();
    }else{
      return clone();
    }
  }
public:
  node_t& n_(int)const override {return _node;}
  void do_it(CS& cmd, CARD_LIST*)override {
    set("");
    std::string label;
    cmd >> label;
    trace1("do_it", label);
    _node = _nn = new DISCIPLINE(label);
    assert(node_dispatcher[label]);
    assert(node_dispatcher[label] == _nn);

    if(cmd >> ';'){
    }else{
      cmd.warn(bDANGER, "expecting ';'");
    }

    try{
      parse(cmd, _nn);
    }catch(Exception const& e){ untested();
      delete _nn;
      _node = _nn = nullptr;
      throw e;
    }
    set_label(label);
  }
  void parse(CS& cmd, NODE* nn) {
    while(1){
      if(!cmd.more()){
	cmd.getline("discipline>");
      }else{
      }
      if(cmd >> "enddiscipline"){
	break;
      }else{
	std::string n, v;
	cmd >> n;
	cmd >> v;

	trace2("disc::spbn", n, v);
	nn->set_param_by_name(n, v);
	if(cmd >> ';'){
	}else{
	  cmd.warn(bDANGER, "expecting ';'");
	}
      }
    }
  }
  std::string dev_type()const override {return "discipline";}
} p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "discipline", &p1);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class CMD_DEFAULT_DISC : public CMD {
  CMD_DEFAULT_DISC(CMD_DEFAULT_DISC const&p)  : CMD(p) {}
public:
  explicit CMD_DEFAULT_DISC() {}
  CMD* clone()const override {return new CMD_DEFAULT_DISC(*this);}

  void do_it(CS& cmd, CARD_LIST*)override {
    trace1("do_it", cmd.tail());
    size_t here = cmd.cursor();
    if(!cmd.more()){
      if(OPT::default_logic){
	IO::mstdout << OPT::default_logic->short_label();
      }else{
	IO::mstdout << "(null)";
      }
      IO::mstdout << '\n';
    }else if(NODE* n = node_dispatcher[cmd]) {
      if(n->param_value(0) != "discrete"){
	trace1("", OPT::default_logic->param_value(3));
	cmd.warn(bWARNING, here, "not a discrete discipline. nonstandard\n");
      }else{
      }
      OPT::default_logic = n;
    }else{
      std::string choices;
      std::string sep;
      for(DISPATCHER<NODE>::const_iterator
	  i = node_dispatcher.begin(); i != node_dispatcher.end(); ++i) {
	auto t = dynamic_cast<NODE_TYPE const*>(i->second);
	if (!t){
	}else if (t && t->domain()) {
	  choices += sep + i->first;
	  sep = " ";
	}else{
	}
      }
      cmd.warn(bWARNING, "need a node (" + choices + ")");
      cmd >> sep;
    }
}
} p2;
DISPATCHER<CMD>::INSTALL d2(&command_dispatcher, "`default_discipline", &p2);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
