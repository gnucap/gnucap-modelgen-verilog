/*                       -*- C++ -*-
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
 */
#include "u_nodemap.h"
#include "u_lang.h"
#include "u_opt.h"
#include "globals.h"
#include "c_comand.h"
#include "e_subckt.h"
#include "e_node_type.h"
#include "e_model.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
/*
  connectrules_declaration ::=
  connectrules connectrules_identifier ;
    { untested(); connectrules_item }
  endconnectrules
  connectrules_item ::=
    connect_insertion
  | connect_resolution
  connect_resolution ::= connect discipline_identifier { untested(); , discipline_identifier }
                              resolveto discipline_identifier_or_exclude ;
  discipline_identifier_or_exclude ::= discipline_identifier | exclude
*/
/*--------------------------------------------------------------------------*/
class CMD_CONNECTRULES : public CMD {
  class CR_ENV : public MODEL_SUBCKT {
  public:
    explicit CR_ENV(COMPONENT* c) : MODEL_SUBCKT(c) {}
  };

  void do_it(CS& cmd, CARD_LIST* Scope)override {
    BASE_SUBCKT* new_cr = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("connectrules"));
    assert(new_cr);
    new_cr->set_owner(nullptr);
    assert(new_cr->subckt());
    assert(new_cr->subckt()->is_empty());
    assert(!new_cr->is_device());
    try {
      parse_connectrules(cmd, new_cr);
      Scope->push_back(new CR_ENV(new_cr));
      OPT::connect_rules = new_cr;
    }catch(Exception const& e) {
      cmd.warn(bDANGER, e.message());
      for (;;) {
	cmd.getline("verilog-connect>");

	if (cmd >> "endconnectrules ") {
	  break;
	}else{
	}
      }
      delete new_cr;
    //  cmd.warn(bDANGER, e.message());
    }
  }
  void parse_connectrules(CS&, BASE_SUBCKT*)const;
} p3;
DISPATCHER<CMD>::INSTALL d3(&command_dispatcher, "connectrules", &p3);
/*--------------------------------------------------------------------------*/
/*
  connect_insertion ::= connect connectmodule_identifier [ connect_mode ]
    [ parameter_value_assignment ] [ connect_port_overrides ] ;
  connect_mode ::= merged | split
  connect_port_overrides ::=
  discipline_identifier , discipline_identifier
  | input discipline_identifier , output discipline_identifier
  | output discipline_identifier , input discipline_identifier
  | inout discipline_identifier , inout discipline_identifier
*/
/*--------------------------------------------------------------------------*/
class CONNECT_INSERT : public COMPONENT {
  mutable node_t _nodes[2];
  std::string _mode;
public:
  CONNECT_INSERT* clone()const override {return new CONNECT_INSERT(*this);}
public:
 // void set_dev_type(std::string const&s)override{ untested();
 //   _type = s;
 // }
  std::string dev_type()const override { return "connect";}
  int param_count()const override { return 1; }
  bool param_is_printable(int)const override { return _mode!="";}
  std::string param_name(int i, int)const override { untested();
    return param_name(i);
  }
  std::string param_name(int i)const override { untested();
    switch(i){
    case 0: return "mode";
    default: return COMPONENT::param_name(i-1);
    }
  }
  void set_param_by_index(int i, std::string& s, int offset)override {
    if(i==0){
      _mode=s;
    }else{ untested();
      COMPONENT::set_param_by_index(i-1, s, i+offset);
    }
  }
  std::string param_value(int i)const override {
    switch(i){
    case 0: return _mode;
    default: return COMPONENT::param_value(i-1);
    }
  }
  std::string port_name(int)const override { untested();return "";}

  int net_nodes()const override {return 2;}
  node_t& n_(int i)const override { untested();return _nodes[i];}
  bool print_type_in_spice()const override { untested();return false;}
}ci;
/*--------------------------------------------------------------------------*/
static CARD* parse_connect_insert(CS& cmd, std::string const& what)
{
  COMPONENT* connect = ci.clone();
  connect->set_label(what);
  trace1("insert", cmd.tail());
  if(cmd >> "split"){
    std::string v = "split";
    connect->set_param_by_index(0, v, 0);
  }else if(cmd >> "merged"){
    std::string w = "merged";
    connect->set_param_by_index(0, w, 0);
  }else{
  }

  return connect;
}
/*--------------------------------------------------------------------------*/
static DEV_DOT* parse_resolveto(CS& cmd, DEV_DOT* d)
{
  trace1("DOT?", cmd.fullstring());
  if(node_dispatcher[cmd]){
  }else{ untested();
    cmd.warn(bDANGER, "expecting discipline");
    delete d;
    return nullptr;
  }
  while(cmd.more()){
    if(node_dispatcher[cmd]){ untested();
    }else{
      break;
    }
  }
  if(cmd >> "resolveto"){
  }else{ untested();
    cmd.warn(bDANGER, "expecting discipline or resolveto");
    delete d;
    return nullptr;
  }
  if(node_dispatcher[cmd]){
    d->set(cmd.fullstring());
    return d;
  }else{ untested();
    cmd.warn(bDANGER, "expecting discipline");
    delete d;
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
void CMD_CONNECTRULES::parse_connectrules(CS& cmd, BASE_SUBCKT* cr) const
{
  std::string label;
  cmd >> label;
  cr->set_label(label);
  if(cmd >> ';'){
  }else{ untested();
    throw Exception_CS("expecting ';'", cmd);
  }

  cmd.getline("cr>");
  while(cmd >> "connect"){
    std::string what;
    size_t here = cmd.cursor();
    cmd >> what;
    assert(OPT::language);
    trace1("disc parse", what);
    if(node_dispatcher[what]) {
      DEV_DOT* d = new DEV_DOT();
      if((d = parse_resolveto(cmd, d))) {
	cr->subckt()->push_back(d);
      }else{ untested();
      }
    }else if(CARD const* c = OPT::language->find_proto(what, nullptr)){
      CARD* cm = c->clone_instance();
      assert(cm);
      auto cc = dynamic_cast<COMPONENT*>(cm);
      if(!cc){ untested();
	delete cm;
	cm = nullptr;
	error(bDANGER, "not suitable: " + what + "\n");
      }else{
      }

      if(cm){
	cm = parse_connect_insert(cmd, what);
      }else{ untested();
      }

      if(cm){
	cr->subckt()->push_back(cm);
      }else{ untested();
      }
    }else{ untested();
      cmd.warn(bDANGER, here, "expecting discipline or connectmodule");
      incomplete();
      // DEV_DOT* d = new DEV_DOT();
      // trace1("DOT?", cmd.fullstring());
      // d->set(cmd.fullstring());
      // cr->subckt()->push_back(d);
    }
    cmd.getline("cr>");
  }
  if(cmd >> "endconnectrules "){
  }else{
    throw Exception_CS("expecting endconnectrules", cmd);
  }
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
