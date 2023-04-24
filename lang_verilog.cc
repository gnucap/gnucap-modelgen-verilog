/*$Id: lang_verilog.cc $ -*- C++ -*-
 * Copyright (C) 2007 Albert Davis
 *               2022 Felix Salfelder
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
#include <gnucap/u_nodemap.h>
#include <gnucap/globals.h>
#include <gnucap/c_comand.h>
#include <gnucap/d_dot.h>
#include <gnucap/d_coment.h>
#include <gnucap/e_subckt.h>
#include <gnucap/e_model.h>
#include <gnucap/u_lang.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class LANG_VERILOG : public LANGUAGE {
public:
  typedef enum MODE {mDEFAULT, mPARAMSET} mode;
private:
  mode _mode;
  mutable int arg_count;
  enum {INACTIVE = -1};
public:
  LANG_VERILOG() : arg_count(INACTIVE) {}
  ~LANG_VERILOG() {}
  std::string name()const override {return "verilog";}
  bool case_insensitive()const override {return false;}
  UNITS units()const override {return uSI;}

public: // override virtual, used by callback
  std::string arg_front()const override {itested();
    switch (_mode) {
    case mPARAMSET:untested(); return " .";			    break;
    case mDEFAULT:itested();  return (arg_count++ > 0) ? ", ." : "."; break;
    }
    unreachable();
    return "";
  }
  std::string arg_mid()const override {itested();
    switch (_mode) {
    case mPARAMSET:untested(); return "="; break;
    case mDEFAULT:itested();  return "("; break;
    }
    unreachable();
    return "";
  }
  std::string arg_back()const override {itested();
    switch (_mode) {
    case mPARAMSET:untested(); return ";"; break;
    case mDEFAULT:itested();  return ")"; break;
    }
    unreachable();
    return "";
  }

public: // override virtual, called by commands
  void		parse_top_item(CS&, CARD_LIST*)override;
  DEV_COMMENT*	parse_comment(CS&, DEV_COMMENT*)override;
  DEV_DOT*	parse_command(CS&, DEV_DOT*)override;
  CARD*		parse_paramset(CS&, CARD*) /* override */;
  COMPONENT*	parse_paramset_(CS&, BASE_SUBCKT*);
  CARD*		obsolete_parse_modelcard(CS&, MODEL_CARD*);
  BASE_SUBCKT*  parse_module(CS&, BASE_SUBCKT*)override;
  COMPONENT*	parse_instance(CS&, COMPONENT*)override;
  std::string	find_type_in_string(CS&)override;

private: // TODO, transition, stale pure virtuals...
  MODEL_CARD*	parse_paramset(CS&, MODEL_CARD*)
  { incomplete(); unreachable(); return NULL; }
  void print_paramset(OMSTREAM&, const MODEL_CARD*)
  { incomplete(); unreachable(); }

private: // override virtual, called by print_item
  void print_paramset(OMSTREAM&, const CARD*) /* override */;
  void print_module(OMSTREAM&, const BASE_SUBCKT*)override;
  void print_instance(OMSTREAM&, const COMPONENT*)override;
  void print_comment(OMSTREAM&, const DEV_COMMENT*)override;
  void print_command(OMSTREAM& o, const DEV_DOT* c)override;
private: // local
  void print_args(OMSTREAM&, const MODEL_CARD*);
  void print_args(OMSTREAM&, const COMPONENT*);
  template<class T>
  void print_args_paramset(OMSTREAM&, const T*);
  void print_items_sckt(OMSTREAM&, const BASE_SUBCKT*);
} lang_verilog;

DISPATCHER<LANGUAGE>::INSTALL
	d(&language_dispatcher, lang_verilog.name(), &lang_verilog);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void parse_type(CS& cmd, CARD* x)
{
  assert(x);
  std::string new_type;
  cmd >> new_type;
  x->set_dev_type(new_type);
}
/*--------------------------------------------------------------------------*/
static void parse_args_paramset(CS& cmd, CARD* x)
{
  assert(x);

  while (cmd >> '.') {
    size_t here = cmd.cursor();
    std::string name, value;
    try{
      cmd >> name >> '=' >> value >> ';';
      x->set_param_by_name(name, value);
    }catch (Exception_No_Match&) {untested();
      cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + name + " ignored");
    }
  }
}
/*--------------------------------------------------------------------------*/
static void parse_args_instance(CS& cmd, CARD* x)
{
  assert(x);

  if (cmd >> "#(") {
    if (cmd.match1('.')) {
      // by name
      while (cmd >> '.') {
	size_t here = cmd.cursor();
	std::string name  = cmd.ctos("(", "", "");
	std::string value = cmd.ctos(",)", "(", ")");
	cmd >> ',';
	try{
	  x->set_param_by_name(name, value);
	}catch (Exception_No_Match&) {
	  cmd.warn(bDANGER, here, x->long_label() + ": bad parameter " + name + " ignored");
	}
      }
    }else{
      // by order
      int index = 1;
      while (cmd.is_alnum() || cmd.match1("+-.")) {
	size_t here = cmd.cursor();
	try{
	  std::string value = cmd.ctos(",)", "", "");
	  x->set_param_by_index(x->param_count() - index++, value, 0/*offset*/);
	}catch (Exception_Too_Many& e) {untested();
	  cmd.warn(bDANGER, here, e.message());
	}
      }
    }
    cmd >> ')';
  }else{
    // no args
  }
}
/*--------------------------------------------------------------------------*/
static void parse_label(CS& cmd, CARD* x)
{
  assert(x);
  std::string my_name;
  if (cmd >> my_name) {
    x->set_label(my_name);
  }else{
    x->set_label(x->id_letter() + std::string("_unnamed")); //BUG// not unique
    cmd.warn(bDANGER, "label required");
  }
}
/*--------------------------------------------------------------------------*/
static void parse_ports(CS& cmd, COMPONENT* x, bool all_new)
{
  assert(x);

  if (cmd >> '(') {
    if (cmd.is_alnum()) {
      // by order
      int index = 0;
      while (cmd.is_alnum()) {
	size_t here = cmd.cursor();
	try{
	  std::string value;
	  cmd >> value;
	  x->set_port_by_index(index, value);
	  if (all_new) {
	    if (x->node_is_grounded(index)) {
	      cmd.warn(bDANGER, here, "node 0 not allowed here");
	    }else if (x->subckt() && x->subckt()->nodes()->how_many() != index+1) {
	      cmd.warn(bDANGER, here, "duplicate port name, skipping");
	    }else{
	      ++index;
	    }
	  }else{
	    ++index;
	  }
	}catch (Exception_Too_Many& e) { untested();
	  cmd.warn(bDANGER, here, e.message());
	}
      }
      if (index < x->min_nodes()) {
	cmd.warn(bDANGER, "need " + to_string(x->min_nodes()-index) +" more nodes, grounding");
	for (int iii = index;  iii < x->min_nodes();  ++iii) {
	  x->set_port_to_ground(iii);
	}
      }else{
      }
    }else{
      // by name
      while (cmd >> '.') {
	size_t here = cmd.cursor();
	try{
	  std::string name, value;
	  cmd >> name >> '(' >> value >> ')' >> ',';
	  x->set_port_by_name(name, value);
	}catch (Exception_No_Match&) {itested();
	  cmd.warn(bDANGER, here, "mismatch, ignored");
	}
      }
      for (int iii = 0;  iii < x->min_nodes();  ++iii) { untested();
	if (!(x->node_is_connected(iii))) {untested();
	  cmd.warn(bDANGER, x->port_name(iii) + ": port unconnected, grounding");
	  x->set_port_to_ground(iii);
	}else{ untested();
	}
      }
    }
    cmd >> ')';
  }else{
    cmd.warn(bDANGER, "'(' required (parse ports) (grounding)");
    for (int iii = 0;  iii < x->min_nodes();  ++iii) {
      if (!(x->node_is_connected(iii))) {
	cmd.warn(bDANGER, x->port_name(iii) + ": port unconnected, grounding");
	x->set_port_to_ground(iii);
      }else{ untested();
	unreachable();
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_COMMENT* LANG_VERILOG::parse_comment(CS& cmd, DEV_COMMENT* x)
{
  assert(x);
  x->set(cmd.fullstring());
  return x;
}
/*--------------------------------------------------------------------------*/
DEV_DOT* LANG_VERILOG::parse_command(CS& cmd, DEV_DOT* x)
{
  assert(x);
  x->set(cmd.fullstring());
  CARD_LIST* scope = (x->owner()) ? x->owner()->subckt() : &CARD_LIST::card_list;

  cmd.reset();
  CMD::cmdproc(cmd, scope);
  delete x;
  return NULL;
}
/*--------------------------------------------------------------------------*/
/* "paramset" <my_name> <base_name> ";"
 *    <paramset_item_declaration>*
 *    <paramset_statement>*
 *  "endparamset"
 */
CARD* LANG_VERILOG::parse_paramset(CS& cmd, CARD* x)
{
  if(auto c = dynamic_cast<BASE_SUBCKT*>(x)) {
    return parse_paramset_(cmd, c);
  }else if(auto m = dynamic_cast<MODEL_CARD*>(x)) {
    //BUG// no paramset_item_declaration, falls back to spice mode
    return obsolete_parse_modelcard(cmd, m);
  }else{ untested();
    unreachable();
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
CARD* LANG_VERILOG::obsolete_parse_modelcard(CS& cmd, MODEL_CARD* x)
{
  assert(x);
  cmd.reset();
  cmd >> "paramset ";
  parse_label(cmd, x);
  parse_type(cmd, x);
  cmd >> ';';

  for (;;) {
    parse_args_paramset(cmd, x);
    if (cmd >> "endparamset ") {
      break;
    }else if (!cmd.more()) {
      cmd.get_line("verilog-paramset>");
    }else{
      cmd.check(bWARNING, "what's this?");
      break;
    }
  }
  return x;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_VERILOG::parse_paramset_(CS& cmd, BASE_SUBCKT* x)
{
  assert(x);

  cmd.reset();
  cmd >> "paramset ";
  parse_label(cmd, x);
  parse_type(cmd, x);
  cmd >> ';';

  for (;;) {
    size_t here = cmd.cursor();
    if (cmd >> "parameter ") {
      cmd.reset(here);
      CMD::cmdproc(cmd, x->subckt());
    }else if (cmd >> "// ") { untested();
      cmd.reset(here);
      // new__instance(cmd, x, x->subckt()); // BUG
      cmd.get_line("verilog-paramset>");
    }else if (!cmd.more()) {
      cmd.get_line("verilog-paramset>");
    }else{
      break;
    }
  }

  for (;;) {
    parse_args_paramset(cmd, x);
    size_t here = cmd.cursor();
    if (cmd >> "endparamset ") {
      break;
    }else if (cmd >> "// ") {
      cmd.reset(here);
      // new__instance(cmd, x, x->subckt()); // BUG
      cmd.get_line("verilog-paramset>");
    }else if (!cmd.more()) {
      cmd.get_line("verilog-paramset>");
    }else{untested();
      cmd.check(bWARNING, "what's this?");
      break;
    }
  }
  trace2("LANG_VERILOG::parse_paramset_ done", x->long_label(), ((CARD*)x)->dev_type());
  return x;
}
/*--------------------------------------------------------------------------*/
/* "module" <name> "(" <ports> ")" ";"
 *    <declarations>
 *    <netlist>
 * "endmodule"
 */
//BUG// strictly one device per line

BASE_SUBCKT* LANG_VERILOG::parse_module(CS& cmd, BASE_SUBCKT* x)
{
  assert(x);

  // header
  cmd.reset();
  (cmd >> "module |macromodule ");
  parse_label(cmd, x);
  parse_ports(cmd, x, true/*all new*/);
  cmd >> ';';

  bool have_instance = false;

  // body
  for (;;) {

    cmd.get_line("verilog-module>");
    if (cmd >> "endmodule ") {
      break;
    }else if (!have_instance && (cmd >> "parameter ")) {
      cmd.reset();
      CMD::cmdproc(cmd, x->subckt());
    }else if (cmd >> "// ") {
      cmd.reset();
      new__instance(cmd, x, x->subckt());
    }else if (cmd >> "paramset ") {
      cmd.reset();
      cmd.check(bDANGER, "ERROR: This will not work. Need top level.");
      new__instance(cmd, x, x->subckt());
    }else{
      have_instance = true;
      BASE_SUBCKT* new_instance = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("instance"));
      assert(new_instance);
      CARD_LIST* Scope = x->subckt();
      trace3("parse_module instance", cmd.fullstring(), Scope, Scope->nodes());
      assert(Scope);

      new_instance->set_owner(x);
      parse_instance(cmd, new_instance);

      Scope->push_back(new_instance);
    }
  }
  return x;
}
/*--------------------------------------------------------------------------*/
COMPONENT* LANG_VERILOG::parse_instance(CS& cmd, COMPONENT* x)
{
  assert(x);
  cmd.reset();
  parse_type(cmd, x);
  parse_args_instance(cmd, x);
  parse_label(cmd, x);
  parse_ports(cmd, x, false/*allow dups*/);
  cmd >> ';';
  cmd.check(bWARNING, "what's this?");
  return x;
}
/*--------------------------------------------------------------------------*/
std::string LANG_VERILOG::find_type_in_string(CS& cmd)
{
  size_t here = cmd.cursor();
  std::string type;
  if ((cmd >> "//")) {
    assert(here == 0);
    type = "dev_comment";
  }else{
    cmd >> type;
  }
  cmd.reset(here);
  trace2("found type", cmd.tail(), type);
  return type;
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::parse_top_item(CS& cmd, CARD_LIST* Scope)
{
  cmd.get_line("gnucap-verilog>");
  new__instance(cmd, NULL, Scope);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_args(OMSTREAM& o, const MODEL_CARD* x)
{
  print_args_paramset(o, x);
}
/*--------------------------------------------------------------------------*/
template<class T>
void LANG_VERILOG::print_args_paramset(OMSTREAM& o, const T* x)
{
  assert(x);
  if (x->use_obsolete_callback_print()) {untested();
    unreachable();
    // x->print_args_obsolete_callback(o, this);  //BUG//callback//
  }else{
    if(0 && x->subckt()){
      for(auto p : *x->subckt()->params()){
	std::string const& f = p.first;
	if(!p.second.has_hard_value()){
	  o << "  parameter " << p.first << ";\n";
	}else if(f.size() < 2 || f[0] != '_' || f[1] != '.'){
	  o << "  parameter " << p.first << " = " << p.second.string() << ";\n";
	}else{
	  // hack: hide internal parameter.
	  // move to instance?
	}
      }
    }else{
    	//  DEV_DOT for now.
    }


    { // slow? use common->_params..?
      for (int ii = x->param_count() - 1;  ii >= 0;  --ii) {
	if (x->param_is_printable(ii)) {
	  o << " ." << x->param_name(ii) << "=" << x->param_value(ii) << ";";
//	  o << arg;
	}else{
	}
      }
//      o << "\\\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_args(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  o << " #(";
  if (x->use_obsolete_callback_print()) {
    arg_count = 0;
    x->print_args_obsolete_callback(o, this);  //BUG//callback//
    arg_count = INACTIVE;
  }else{
    std::string sep = "";
    for (int ii = x->param_count() - 1;  ii >= 0;  --ii) {
      if (x->param_is_printable(ii)) {
	o << sep;
	std::string pn = x->param_name(ii);
	if(pn==""){
	  o << x->param_value(ii);
	}else{
	  o << "." << pn << "(" << x->param_value(ii) << ")";
	}
	sep = ",";
      }else{
      }
    }
  }
  o << ") ";
}
/*--------------------------------------------------------------------------*/
static void print_type(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  o << x->dev_type();
}
/*--------------------------------------------------------------------------*/
static void print_label(OMSTREAM& o, const COMPONENT* x)
{
  assert(x);
  o << x->short_label();
}
/*--------------------------------------------------------------------------*/
static void print_ports_long(OMSTREAM& o, const COMPONENT* x)
{
  // print in long form ...    .name(value)
  assert(x);

  o << " (";
  std::string sep = "";
  for (int ii = 0;  x->port_exists(ii);  ++ii) {
    o << sep;
    if(x->port_name(ii) != ""){
      o << "." << x->port_name(ii) << '(' << x->port_value(ii) << ')';
    }else{
      o << x->port_value(ii);
    }
    sep = ",";
  }
  for (int ii = 0;  x->current_port_exists(ii);  ++ii) {untested();
    o << sep << x->current_port_name(ii) << '(' << x->current_port_value(ii) << ')';
    sep = ",.";
  }
  o << ")";
}
/*--------------------------------------------------------------------------*/
static void print_ports_short(OMSTREAM& o, const COMPONENT* x)
{
  // print in short form ...   value only
  assert(x);

  o << " (";
  std::string sep = "";
  for (int ii = 0;  x->port_exists(ii);  ++ii) {
    o << sep << x->port_value(ii);
    sep = ",";
  }
  for (int ii = 0;  x->current_port_exists(ii);  ++ii) {untested();
    o << sep << x->current_port_value(ii);
    sep = ",";
  }
  o << ")";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_items_sckt(OMSTREAM& o, const BASE_SUBCKT* x)
{
  for (CARD_LIST::const_iterator
      ci = x->subckt()->begin(); ci != x->subckt()->end(); ++ci) {
    o << "  ";
    print_item(o, *ci);
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_paramset(OMSTREAM& o, const CARD* c)
{
  if(auto x = prechecked_cast<MODEL_CARD const*>(c)){
    _mode = mPARAMSET;
    o << "paramset " << x->short_label() << ' ' << x->dev_type() << ";\n";
    print_args(o, x);
    o << "\n"
      "endparamset\n\n";
    _mode = mDEFAULT;
  }else if(auto x = prechecked_cast<BASE_SUBCKT const*>(c)){
    o << "paramset " << x->short_label() << ' ' << c->dev_type() << ";\n";
    print_items_sckt(o, x);
    print_args_paramset(o, x);
    o << "\nendparamset\n\n";
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_module(OMSTREAM& o, const BASE_SUBCKT* x)
{

  if(((CARD const*)x)->dev_type()!=""){
    // tmp hack. module type is the label, so dev_type is blank.
    return print_paramset(o, x);
  }else{
  }
  assert(x);
  assert(x->subckt());

  o << "module " << x->short_label();
  print_ports_short(o, x);
  o << ";\n";
  print_items_sckt(o, x);
  o << "endmodule // " << x->short_label() << "\n\n";
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_instance(OMSTREAM& o, const COMPONENT* x)
{
  if(x->is_device()){
    print_type(o, x);
    print_args(o, x);
    print_label(o, x);
    print_ports_long(o, x);
    o << ";\n";
  }else{ untested();
    _mode = mPARAMSET;
    print_paramset(o, x);
    _mode = mDEFAULT;
  }
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_comment(OMSTREAM& o, const DEV_COMMENT* x)
{
  assert(x);
  if ((x->comment().compare(0, 2, "//")) != 0) {itested();
    o << "//";
  }else{
  }
  o << x->comment() << '\n';
}
/*--------------------------------------------------------------------------*/
void LANG_VERILOG::print_command(OMSTREAM& o, const DEV_DOT* x)
{untested();
  assert(x);
  o << x->s() << '\n';
}
/*--------------------------------------------------------------------------*/
class CMD_PARAMSET : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope) {
    if(Scope == &CARD_LIST::card_list){
    }else{
      // allowed?
    }
    // already got "paramset"
    std::string my_name, base_name;
    cmd >> my_name >> base_name;
    // cmd >> model_name;

    assert(OPT::language);
    const CARD* proto = OPT::language->find_proto(base_name, NULL);
    CARD* paramset = NULL;

    if(auto model=dynamic_cast<MODEL_CARD const*>(proto)){
      trace1("CMD_PARAMSET::do_it MODEL_CARD", base_name);
      // Spice compatibility mode. fix later.
      // // BUG also hits paramset //

      paramset = model->clone();
      auto mc = prechecked_cast<MODEL_CARD*>(paramset);
      assert(mc);
      OPT::language->parse_paramset(cmd, mc);
    }else{
      paramset = device_dispatcher.clone("paramset");
      assert(paramset);
      auto dev = prechecked_cast<BASE_SUBCKT*>(paramset);
      assert(dev);
      //assert(dev->scope());

      // OPT::language->parse_paramset(cmd, dev);
      lang_verilog.parse_paramset(cmd, dev);
    }
    trace3("CMD_PARAMSET", paramset->long_label(), paramset->dev_type(), paramset);
    Scope->push_back(paramset);
  }
} p1;
DISPATCHER<CMD>::INSTALL d1(&command_dispatcher, "paramset", &p1);
class CMD_MODULE : public CMD {
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    BASE_SUBCKT* new_module = dynamic_cast<BASE_SUBCKT*>(device_dispatcher.clone("module"));
    assert(new_module);
    assert(!new_module->owner());
    assert(new_module->subckt());
    assert(new_module->subckt()->is_empty());
    assert(!new_module->is_device());
    lang_verilog.parse_module(cmd, new_module);
    Scope->push_back(new_module);
  }
} p2;
DISPATCHER<CMD>::INSTALL d2(&command_dispatcher, "module|macromodule", &p2);
/*--------------------------------------------------------------------------*/
class CMD_VERILOG : public CMD {
public:
  void do_it(CS&, CARD_LIST* Scope)override {
    command("options lang=verilog", Scope);
  }
} p8;
DISPATCHER<CMD>::INSTALL d8(&command_dispatcher, "verilog", &p8);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
