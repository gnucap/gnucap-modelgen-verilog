/*                                      -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
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
#include "mg_out.h"
#include "mg_func.h"
#include "m_tokens.h"
#include <stack>
/*--------------------------------------------------------------------------*/
#if 0
/*--------------------------------------------------------------------------*/
static int is_function(std::string const& n)
{ untested();
  incomplete();
  // stub, need sth.
  if (n == "exp"
   || n == "log"
   || n == "cos"
   || n == "sin") { untested();
    return 1;
  }else if (n == "pow"){ untested();
    return 2;
  }else{ untested();
    return 0;
  }
}
#endif
/*--------------------------------------------------------------------------*/
static String_Arg const& potential_abstol(Branch const& b)
{
  static String_Arg def("OPT::abstol");
  if(!b.discipline()){ untested();
    // incomplete(); TODO: what's the default?
    return def;
  }else if(!b.discipline()->potential()){ untested();
    incomplete();
    return def;
  }else{
    return b.discipline()->potential()->abstol();
  }
}
/*--------------------------------------------------------------------------*/
static String_Arg const& flow_abstol(Branch const& b)
{
  static String_Arg def("OPT::abstol");
  if(!b.discipline()){ untested();
    incomplete();
    return def;
  }else if(!b.discipline()->flow()){ untested();
    incomplete();
    return def;
  }else{
    return b.discipline()->flow()->abstol();
  }
}
/*--------------------------------------------------------------------------*/
static void make_module_is_valid(std::ostream& o, const Module& m)
{
  o << "bool MOD_" << m.identifier() << "::is_valid()const\n{\n";
  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "return c->is_valid();\n";

  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_node_ref(std::ostream& o, const Node& n, bool used=true)
{
  if(n.is_ground()) {
    o << "gnd";
  }else if(!used){
    o << "gnd";
  }else{
    o << "_n[" << n.code_name() << "]";
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_branch_output(std::ostream& o, Branch const* br)
{
  Branch const* out = br->output();
  make_node_ref(o, *out->p(), br->is_used());
  o << ", ";
  make_node_ref(o, *out->n(), br->is_used());
}
/*--------------------------------------------------------------------------*/
// TODO: mg_out_analog.cc
void make_cc_branch_ctrl(std::ostream& o, Branch const* br);
void make_cc_current_ports(std::ostream& o, Branch const* br, Element_2 const&);
/*--------------------------------------------------------------------------*/
static void make_tr_needs_eval(std::ostream& o, const Module& m)
{
  o << "bool MOD_" << m.identifier() << "::tr_needs_eval()const\n{\n";
  o__ "node_t gnd(&ground_node);\n";
  o__ "if (is_q_for_eval()) { untested();\n";
  o____ "return false;\n";
  o__ "}else if (!converged()) {\n";
  o____ "return true;\n";
  o__ "}else ";

  if( m.has_filters()) {
    o__ "if(_sim->is_first_iteration()){\n";
    o____ "return true;\n";
    o__ "}else";
  }else{
  }

  // BUG this is incomplete.
  for(auto br : m.branches()) {
    assert(br);
    if(br->is_short()) {
    }else if(br->is_filter()) {
    }else{
      if(br->has_pot_probe()) {
	o << " if( !conchk(_potential"<< br->code_name() << ", ";
	make_node_ref(o, *br->p());
	o << ".v0() - ";
	make_node_ref(o, *br->n());
	o << ".v0(), ";
	o << potential_abstol(*br) << ")){\n";
	o____ "return true;\n" <<ind<<"}else";
      }else if(br->has_flow_probe()) {
	o << " if( " << br->code_name() << " && !conchk(_flow"<< br->code_name() << ", "
	  <<  br->code_name() << "->tr_amps(), ";
	o << flow_abstol(*br) << ")) {\n";
	o____ "return true;\n" <<ind<<"}else";
      }else{
      }
    }
  }
  o << "{\n";
  o____ "return false;\n";
  o__ "}\n";
  o << "};\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_probe_num(std::ostream& o, const Module& m)
{
  o << "double MOD_" << m.identifier() << "::tr_probe_num(std::string const& n) const\n{\n";
  for(auto const& vl : m.variables()){
    for (Variable_List::const_iterator p=vl->begin(); p!=vl->end(); ++p) {
      if((*p)->has_attributes()){
	// todo: only desc or unit
	o__ "if(n == \"" << (*p)->name() << "\"){\n";
	o__ ind << "return _v_" << (*p)->name() << ";\n";
	o__ "}\n";
      }else{
      }
    }
  }
  o__ "if(n == \"conv\") {\n";
  o____ "return converged();\n";
  o__ "}\n";
  o__ "return " <<  baseclass(m) << "::tr_probe_num(n)\n;";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_set_parameters(std::ostream& o, const Element_2& e)
{
  make_tag();
  std::string cn = e.code_name();
  if(dynamic_cast<Filter const*>(&e)){
//    cn = f->branch_code_name();
  }else{
  }
  o______ cn << "->set_parameters(\"" << e.short_label() << "\", this, ";
  if (e.discipline()) {
    // incomplete(); need NODE commons (or so)
    o << "&_C_V_" << e.discipline()->identifier();
  }else{
    o << cn << "->mutable_common()";
  }
  o << ", 0."; // value
  if (e.state() != "") {
    o << ", /*states:*/" << e.num_states() << ", " << e.state();
  }else{ untested();
    o << ", 0, NULL";
  }
  o << ", " << e.num_nodes() << ", nodes);\n";
}
/*--------------------------------------------------------------------------*/
static void make_renew_sckt(std::ostream& o, Module const& m)
{
  if(m.element_list().size()){
    o__ "renew_subckt(_parent, &(c->_netlist_params));\n";
  }else{ untested();
    // o__ "renew_subckt(_parent, NULL);\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_set_subdevice_parameters(std::ostream& o, const Element_2& e)
{
  for(auto p : e.list_of_parameter_assignments()){
    o____ e.code_name() << "->set_param_by_name(\"" << p->name() << "\", "
       << "\"[" << e.short_label() << "]" << p->name() << "\");\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_module_construct_stub(std::ostream& o, const Element_2& e, Module const&)
{
  make_tag();
  std::string dev_type = "device_stub";
  assert(!dynamic_cast<Branch const*>(&e));

  o__ "const CARD* p = device_dispatcher[\"" << dev_type << "\"]; // " << e.dev_type() << "\n";
  o__ "if(!p){\n";
  o____ "throw Exception(" << "\"Cannot find " << dev_type << ". Load module?\");\n";
  o__ "}else{\n";
  o__ "}\n";
  o__ "COMPONENT* " << e.code_name() << " = dynamic_cast<COMPONENT*>(p->clone());\n";
  o__ e.code_name() << "->set_dev_type(\"" << e.dev_type() << "\");\n";
  o__ e.code_name() << "->set_label(\"" << e.short_label() << "\");\n";
  o__ e.code_name() << "->set_owner(this);\n";
  o__ "if(!" << e.code_name() << "){\n";
  o____ "throw Exception(" << "\"Cannot use " << dev_type << ": wrong type\"" << ");\n";
  o__ "}else{\n";
  o__ "}\n";
  o__ "assert(subckt());\n";
  o__ "subckt()->push_front(" << e.code_name() << ");\n";

  o__ "{\n";
  o____ "std::string tmp;\n";
  
  assert(!dynamic_cast<Branch const*>(&e));
  Port_3_List_2::const_iterator p = e.ports().begin();
  if(e.ports().has_names()){
    for (;p != e.ports().end(); ++p) {
      o____ "tmp = \"" << (*p)->name()<<"\";\n";
      o____ "{\n";
      //o____ "std::string val = \"" << (*p)->value()<<"\";\n";
      o____ "assert(_n[n_" << (*p)->value() << "].short_label() == \"" << (*p)->value() << "\");\n";
      o____ e.code_name() << "->set_port_by_name(tmp,\n";
      o____ "   const_cast<std::string&>(_n[n_" << (*p)->value() << "].n_()->short_label()));\n";
      o____ "}\n";
    }
  }else{
    int i = 0;
    for (;p != e.ports().end(); ++p) {
      o____ "tmp = \"" << (*p)->name()<<"\";\n";
      o____ e.code_name() << "->set_port_by_index(" << i << ", tmp);\n";
      ++i;
    }
  }
#if 0
  o____ "node_t nodes[] = {";
  { untested();
    Port_3_List_2::const_iterator p = e.ports().begin();
    if (p != e.ports().end()) { untested();
      assert(*p);
      o << "_n[n_" << (**p).name() << "]";
      while (++p != e.ports().end()) { untested();
	o << ", _n[n_" << (**p).name() << "]";
      }
    }else{ untested();
    }
    o << "}; // nodes\n";
  }
#endif

//    make_set_parameters(o, e);
    make_set_subdevice_parameters(o, e);
  
  o____ "\n";
  o__ "}\n";
} // construct_stub
/*--------------------------------------------------------------------------*/
void make_module_copy_constructor(std::ostream& o, const Module& m)
{
  o << "MOD_" << m.identifier() << "::MOD_" << m.identifier() << "(MOD_" << m.identifier() << " const&p) : "
    <<  baseclass(m) << "(p)";
  if(m.has_submodule()){
    o << ", _parent(p._parent)";
  }
  o << "\n{\n";
  o__ "_n = _nodes;\n";

  o__ "for (int ii = 0; ii < max_nodes(); ++ii) {\n";
  o__ ind << "_n[ii] = p._n[ii];\n";
  o__ ind << "}\n";
  o << "}\n";
  o <<
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_build_netlist(std::ostream& o, const Module& m)
{
  o__ "// build netlist\n";
  if(m.element_list().size()){
    o__ "new_subckt();\n";
    auto nn = m.nodes().begin();
    for (++nn; nn!=m.nodes().end(); ++nn){
      std::string ext_name = (*nn)->name();
      int ii = (*nn)->number();

      if(!ii){
      }else if(ii <= int(m.ports().size())){
//	o__ "_n[" << ii << "].new_node(\"" << ext_name << "\", this); // port\n";
	o__ "{\n";
	o____ "std::string tmp = \"" << ext_name << "\";\n";
	o____ "set_port_by_index(" << ii-1 << ", tmp);\n";
	o__ "}\n";
      }else{
	o__ "_n[" << ii-1 << "].new_node(\"" << ext_name << "\", this);\n";
      }
    }
    o__ "assert(" << m.nodes().size() << " == subckt()->nodes()->how_many());\n";
    for (Element_2_List::const_iterator
	e = m.element_list().begin();
	e != m.element_list().end();
	++e) {
      o__ "{\n";
      {
	indent x;
	make_module_construct_stub(o, **e, m);
      }
      o__ "}\n";
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void make_module_default_constructor(std::ostream& o, const Module& m)
{
  make_tag();
  o << "MOD_" << m.identifier() << "::MOD_" << m.identifier() << "()\n";
  o << "    :" << baseclass(m) << "()";
  o << "\n{\n"
    "  _n = _nodes;\n"
    "  attach_common(&Default_" << m.identifier() << ");\n"
    "  ++_count;\n";

  make_build_netlist(o, m);

  o << "  // overrides\n";
//  for (Parameter_1_List::const_iterator
//       p = m.device().override().begin();
//       p != m.device().override().end();
//       ++p) {untested();
//    if (!((**p).final_default().empty())) {untested();
//      o << "  " << (**p).code_name() << " = NA;\n";
//    }else{untested();
//    }
//    if (!((**p).default_val().empty())) {untested();
//      o << "  " << (**p).code_name() << " = " << (**p).default_val() <<";\n";
//    }else{untested();
//    }
//  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_do_tr(std::ostream& o, const Module& m)
{
  o << "bool MOD_" << m.identifier() << "::do_tr()\n{\n";
  o__ "trace2(\"" << m.identifier() <<"::do_tr\", this, long_label());\n";
  o__ "clear_branch_contributions();\n";
  if(m.num_evt_slots()){
    o__ "_evt_seek = 0;\n";
  }else{
  }
  o__ "read_probes();\n";
  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "assert(c);\n";
  o__ "set_converged();\n";
  o__ "c->tr_eval_analog(this);\n";
  // if analog?
  o__ "set_branch_contributions();\n";
  o__ "assert(subckt());\n";
  o__ "set_converged(subckt()->do_tr() && converged());\n";
  o__ "return converged();\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_advance(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::tr_advance()\n{\n";
  if(m.has_analysis()) {
    o__ "COMMON_" << m.identifier() << " const* c = "
      "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
    o__ "assert(c);\n";
    o__ "if(_sim->_last_time == 0.){\n";
    o__ "c->tr_eval_analog(this);\n";
    o__ "}else{\n";
    o__ "}\n";
  }else{
  }
  o__ "set_not_converged();\n";
  o__ "return " << baseclass(m) << "::tr_advance();\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_review(std::ostream& o, const Module& m)
{
  o << "inline TIME_PAIR MOD_" << m.identifier() << "::tr_review()\n{\n";
  if(m.num_evt_slots()){
    o__ "if(_evt_seek){\n";
    o____ "q_accept();\n";
    o__ "}else ";
  }else{
    o__ "";
  }
#if 0
  if(m.has_analysis()){
    o << "if(_sim->analysis_is_tran_static()){ untested();\n";
    o____ "q_accept();\n";
    o__ "}else ";
  }else{
    o__ "";
  }
#endif
  o << "{itested();\n";
  o__ "}\n";
  o__ "return " << baseclass(m) << "::tr_review();\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_accept(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::tr_accept()\n{\n";
  if(m.num_evt_slots()){
  o__ "for(unsigned i=0; i<_evt_seek; ++i){\n";
  o____ "(*_evt[i])();\n";
  // o____ "(this->*_evt[i])();\n";
  o__ "}\n";
  o__ "_evt_seek = 0;\n"; // needed?
  }else{
  }
#if 0
  if(m.has_analysis()) {
    o__ "COMMON_" << m.identifier() << " const* c = "
      "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
    o__ "assert(c);\n";
    o__ "c->tr_eval_analog(this);\n";
  }else{
  }
#endif
  o__ "return " << baseclass(m) << "::tr_accept();\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_read_probes(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::read_probes()\n{\n";
  o__ "trace0(\"" << m.identifier() <<"::read_probes\");\n";
  // o__ "node_t gnd;\n";
  // o__ "gnd.set_to_ground(this);\n";
  // o__ "(void) gnd;\n";
  o__ "node_t gnd(&ground_node);\n";
  for(auto x : m.branches()){
    Branch const* b = x;
    assert(b);
    if(b->is_filter()){
      o__ "// filter " <<  b->code_name() << "\n";
    }else{
      if(b->is_short()){
      }else if(b->has_pot_probe()){
	o__ "_potential" << b->code_name() << " = volts_limited(";
	make_node_ref(o, *b->p());
	o << ", ";
	assert(b->n());
	make_node_ref(o, *b->n());
	o << ");\n";

	// o__ "trace2(\"potential\", _potential" << b->code_name() << ", _sim->_time0);\n";
      }else{
      }

      if(b->is_short()){
      }else if(b->has_flow_probe()){
	o__ "if(" << b->code_name() << "){\n";
	o____ "_flow" << b->code_name() << " = " << b->code_name() << "->tr_amps();\n";
	o__ "}else{\n";
	o____ "_flow" << b->code_name() << " = 0.;\n";
	o__ "}\n";
      }else if(b->has_pot_probe()){
      }else{
	trace1("no probe?", b->code_name());
      }
    }
  }

//  for(auto x : m.probes()){ untested();
//    Probe const* p = x;
//    assert(p);
//    o << ind << "_" << p->name() << " = volts_limited(_n[n_"<< p->pname() <<"], _n[n_"<< p->nname() <<"]);\n";
//    // o << "trace1(\"\"," << "_" << p->name() << ");\n";
//  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_clone(std::ostream& o, Module const& m)
{
  o << "CARD* MOD_" << m.identifier() << "::clone()const\n{\n";
  o__ "MOD_" << m.identifier() << "* new_instance = new MOD_" << m.identifier() << "(*this);\n";
  o__ "assert(!new_instance->subckt());\n";

  if(m.element_list().size()){
    o__ "if(_parent){\n";
    o__ "  new_instance->_parent = _parent;\n";
    o__ "  assert(new_instance->is_device());\n";
    o__ "}else{\n";
    o__ "  new_instance->_parent = this;\n";
    o__ "  assert(new_instance->is_device());\n";
    o__ "}\n";
  }else{
  }

  o__ "return new_instance;\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_class(std::ostream& o, Module const& m)
{
  make_tr_probe_num(o, m);
  make_read_probes(o, m);
  make_module_default_constructor(o, m);

  o << "// seq blocks\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  if(m.has_analog_block()){
    make_tr_needs_eval(o, m);
    make_do_tr(o, m);
    make_cc_analog(o, m);
  }else{
  }
  if(m.has_events()){
    make_tr_review(o, m);
    make_tr_accept(o, m);
  }else if(m.has_analysis()){
    make_tr_review(o, m);
    make_tr_accept(o, m);
  }else{
  }

  if(m.has_analog_block()){
    make_tr_advance(o, m);
  }else{
    assert(!m.has_analysis());
  }

  // TODO: set_port_by_name
  o__ "std::string MOD_" << m.identifier() << "::port_name(int i)const\n{\n";
  o____ "assert(i >= 0);\n";
  o____ "assert(i < max_nodes());\n";
  o____ "static std::string names[] = {";
  std::string comma = "";
  for (auto nn : m.ports()){ // BUG: array?
    o << comma << '"' << nn->name() << '"';
    comma = ", ";
  }
  o____ "};\n";
  o____ "return names[i];\n";
  o__ "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
} // make_module_class
/*--------------------------------------------------------------------------*/
static void make_module_allocate_local_node(std::ostream& o, const Node& p)
{
#if 1
  make_tag();
  o__ "// node " << p.name() << " " << p.number() << "\n";
 // if (p.short_if().empty()) { untested();
 //   o <<
 //     "    if (!(_n[n_" << p.name() << "].n_())) {\n"
 //     "      _n[n_" << p.name() << "] = _n[n_" << p.short_to() << "];\n"
 //     "    }else{\n"
 //     "    }\n";
 //   //BUG// generates bad code if no short_to
 // }else
  {
    o <<
      "    //assert(!(_n[n_" << p.name() << "].n_()));\n"
      "    //BUG// this assert fails on a repeat elaboration after a change.\n"
      "    //not sure of consequences when new_model_node called twice.\n"
      "    if (!(_n[n_" << p.name() << "].n_())) {\n";
    if(p.short_to()){
      assert(!p.short_if().empty());
      o____ "if (" << p.short_if() << ") {\n";
      o______ "_n[n_" << p.name() << "] = _n[n_" << p.short_to()->name() << "];\n";
      o____ "}else";
    }else{
      o____ "";
    }
    o << "{\n";
    o______ "_n[n_" << p.name() << "].new_model_node(\".\" + long_label() + \"." << p.name() 
			   << "\", this);\n";
    o______ "}\n";
    o____ "}else{\n";

      // "      if (" << p.short_if() << ") {\n"
      // "        assert(_n[n_" << p.name() << "] == _n[n_" << p.short_to() << "]);\n"
      // "      }else"
    o____ "{\n";
    o______ " //_n[n_" << p.name() << "].new_model_node(\"" << p.name()
		 << ".\" + long_label(), this);\n";
    o____ "}\n";
    o__ "}\n";
  }
#endif
}
/*--------------------------------------------------------------------------*/
static void make_module_allocate_local_nodes(std::ostream& o, Module const& m)
{
  if(m.element_list().size()){
    // nodes come from sckt proto
  }else{
    //for (auto n=m.nodes().rbegin(); n != m.nodes().rend(); ++ n)
    int n = 1;
    for (; n <= int(m.nodes().size()); ++n) {
      Node const* nn = m.nodes()[n];
      assert(nn);
      if(nn->number() == 0) {
	o__ "// ground\n";
	o__ "_n[n_" << nn->name() << "].set_to_ground(this);\n";
      }else if(nn->number() < n){
	o__ "_n[" << n - 1 << "] = _n[" << nn->number() - 1 << "];\n";
      }else if(n <= int(m.ports().size())){
	o__ "// port " << nn->name() << " " << nn->number() << "\n";
      }else if(nn->is_used()){
	o__ "// internal " << nn->name() << " : " << nn->number() << "\n";
	make_module_allocate_local_node(o, *nn);
      }else{
	o__ "// unused " << nn->name() << " : " << nn->number() << "\n";
	o__ "_n[n_" << nn->name() << "].set_to_ground(this);\n"; // for now.
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_module_expand_one_branch(std::ostream& o, const Element_2& e, Module const&)
{
  make_tag();
  auto br = dynamic_cast<Branch const*>(&e);
  if(br){
  }else{
  }

  if (br && br->is_short()) { untested();
    o__ "if(0){ // short\n";
  }else if (!(e.omit().empty())) { untested();
    o__ "if (" << e.omit() << ") {\n";
    o____ "if (" << e.code_name() << ") {\n";
    o______ "subckt()->erase(" << e.code_name() << ");\n";
    o______ e.code_name() << " = NULL;\n";
    o____ "}else{\n";
    o____ "}\n";
    o__ "}else{\n";
  }else{
    o__ "{\n";
  }

  std::string dev_type = e.dev_type();

  o__ "if (!" << e.code_name() << ") {\n";
  o____ "const CARD* p = device_dispatcher[\"" << dev_type << "\"]; // " << e.dev_type() << "\n";
  o____ "if(!p){\n";
  o______ "throw Exception(" << "\"Cannot find " << dev_type << ". Load module?\");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ e.code_name() << " = dynamic_cast<ELEMENT*>(p->clone());\n";
  o____ "if(!" << e.code_name() << "){\n";
  o______ "throw Exception(" << "\"Cannot use " << dev_type << ": wrong type\"" << ");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ "subckt()->push_front(" << e.code_name() << ");\n";
//o____ e.code_name() << "->set_dev_type(\"" << e.dev_type() << "\");\n";
  o__ "}else{\n";
  o__ "}\n";

#if 1
  o____ "{\n";
  
  o______ "node_t nodes[] = {";
  
  if(e.num_nodes()){
    make_cc_branch_output(o, br);
    make_cc_branch_ctrl(o, br);

    o << "}; // nodes\n";

//    Port_1_List::const_iterator p = e.current_ports().begin();
    make_set_parameters(o, e);
    if(br == br->output()){
    }else{
      o______ e.code_name() << "->_loss0 = 0.;\n";
      o______ e.code_name() << "->_loss1 = 0.;\n";
    }

    make_cc_current_ports(o, br, e);
  }else{
    o << "gnd, gnd"; // filt subs hack.
    o << "}; // nodes\n";
    make_set_parameters(o, e);
//    Port_3_List_2::const_iterator p = e.ports().begin();
//    if (p != e.ports().end()) { untested();
//      assert(*p);
//      o << "_n[n_" << (**p).name() << "]";
//      while (++p != e.ports().end()) { untested();
//	o << ", _n[n_" << (**p).name() << "]";
//      }
//    }else{ untested();
//    }
//    o << "}; // nodes\n";
//    make_set_parameters(o, e);
//    make_set_subdevice_parameters(o, e);
  }
  
  o << "      }\n";
#endif
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_precalc_first(std::ostream& o, Module const& m)
{
  make_tag();
  String_Arg const& mid = m.identifier();
  o << "void MOD_" << mid << "::precalc_first()\n{\n";
  o__ baseclass(m) << "::precalc_first();\n";

  o__ "auto c = static_cast<COMMON_" << mid << "*>(mutable_common());\n";
  o__ "assert(c);\n";
  o__ "auto cc = c->clone();\n";

  o__ "if(subckt()){\n";
  if(m.element_list().size()){
    o__ "subckt()->attach_params(&(c->_netlist_params), scope());\n";
  }else{
  }
  o__ "attach_common(NULL);\n";
  o__ "attach_common(cc);\n";
  o____ "subckt()->precalc_first();\n";
  o__ "}else{\n";
  o__ "}\n";

  o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";

 //  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
static void make_module_precalc_last(std::ostream& o, Module const& m)
{
  make_tag();
  String_Arg const& mid = m.identifier();
  o << "void MOD_" << mid << "::precalc_last()\n{\n";
  o__ baseclass(m) << "::precalc_last();\n";

  o__ "auto c = static_cast<COMMON_" << mid << "*>(mutable_common());\n";
  o__ "assert(c);\n";
  o__ "(void)c;\n";

  if(m.element_list().size()){
    o__ "assert(subckt());\n";
    o__ "subckt()->attach_params(&(c->_netlist_params), scope());\n";
  }else{
  }

  if(m.has_analog_block()){
    o__ "zero_filter_readout();\n";
  }else{
  }

  if(m.has_analog_block()){
    // incomplete();
    o__ "c->precalc_analog(this);\n"; // call through COMPONENT::precalc?
  }else{
  }

  o__ "if(subckt()){\n";
  o____ "subckt()->precalc_last();\n";
  o__ "}else{untested();\n";
  o__ "}\n";

  o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";

 //  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
static void make_module_expand(std::ostream& o, Module const& m)
{
  make_tag();
  String_Arg const& mid = m.identifier();
  o << "void MOD_" << mid << "::expand()\n{\n";
  o__ baseclass(m) << "::expand();\n";
  o__ "assert(_n);\n";
  o__ "assert(common());\n";
  o__ "auto c = static_cast</*const*/ COMMON_" << mid << "*>(mutable_common());\n"; // const?!
  o__ "assert(c);\n";
  o__ "(void)c;\n";
  o__ "if (!subckt()) {\n"
    "    new_subckt();\n"
    "  }else{\n"
    "  }\n"
    "\n";
  o__ "node_t gnd;\n";
  o__ "gnd.set_to_ground(this);\n";
  if(m.element_list().size()){
    o__ "assert(_parent);\n";
    o__ "assert(_parent->subckt());\n";
    o__ "assert(_parent->subckt()->nodes());\n";
    o__ "// trace2(\"\",  _parent->net_nodes(),  _parent->subckt()->nodes()->how_many());\n";
    o__ "assert(_parent->net_nodes() <= _parent->subckt()->nodes()->how_many());\n";
    o__ "// assert(_parent->subckt()->params());\n";
    o__ "PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());\n";
    o__ "assert(pl);\n";

    // o__ "c->_params.set_try_again(pl);\n";
    make_renew_sckt(o, m);
  }else{
  }
    o__ "if (_sim->is_first_expand()) {\n";
    o____ "precalc_first();\n";
//    "    // optional nodes\n";
//  for (Port_1_List::const_iterator
//       p = d.circuit().opt_nodes().begin();
//       p != d.circuit().opt_nodes().end();
//       ++p) { untested();
//    make_dev_allocate_local_nodes(out, **p);
//  }
  o << ind << "    // local nodes\n";
  make_module_allocate_local_nodes(o, m);
  o << "\n";
  o__ "// clone branches\n";
  for(auto i: m.branches()){
    if(i->has_element()) {
      o__ "// branch " << i->name() << "\n";
      indent x;
      make_module_expand_one_branch(o, *i, m);
    }else if(i->is_filter()) {
      o__ "// filter " << i->name() << "\n";
      make_module_expand_one_branch(o, *i, m);
    }else{
      o__ "// branch no elt " << i->name() << "\n";
    }
  }

  o << "\n";
  o__ "// clone filters\n";
#if 1
  for (auto i: m.filters()){
    if(i->has_branch()){
      unreachable();
    }else{
      // TODO incomplete();
      o__ "// no branch? " << i->name() << "\n";
      make_module_expand_one_branch(o, *i, m);
      // make_module_expand_one_filter(o, *i);
      o__ "// =====/no branch===== // \n";
    }
  }
#endif

  o << "\n";
  o__ "}else{\n";
  o____ "  //precalc();\n";
  o__ "}\n";
  o__ "//precalc();\n";

  // TODO: deflate
  o__ "subckt()->expand();\n";
  o__ "//subckt()->precalc();\n";
  o__ "assert(!is_constant());\n";
  if (m.sync()) {
    o << "  subckt()->set_slave();\n";
  }else{
  }
  o__ "for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ++i){\n";
  o__ "  CARD* d = (*i)->deflate();\n";

  o__ "  if(d == (*i)){\n";
  o__ "  }else{\n";
  o__ "    assert(d->owner() == this);\n";
  o__ "    delete *i;\n";
  o__ "    *i = d;\n";
  o__ "  }\n";
  o__ "}\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_dispatcher(std::ostream& o, Module const& m)
{
  o << "MOD_" << m.identifier() << " m_" << m.identifier() << ";\n";
    o << "DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, \""
      << m.identifier() << "\", &m_" << m.identifier() << ");\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_func(std::ostream& o, const Module& m)
{
  for(auto f : m.funcs()){
    f->make_cc_impl(o);
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_module(std::ostream& o, const Module& m)
{
  make_tag();

  make_cc_decl(o, m);
  make_cc_common(o, m);
  o <<
      "int COMMON_" << m.identifier() << "::_count = -1;\n"
      "static COMMON_" << m.identifier() << " Default_" << m.identifier()
	<< "(CC_STATIC);\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  o <<
      "/*--------------------------------------"
      "------------------------------------*/\n";
  //make_precalc_class(o, m);
  o <<
      "/*--------------------------------------"
      "------------------------------------*/\n";
  make_module_class(o, m);
    o << "int MOD_" << m.identifier() << "::_count = -1;\n";
  make_module_dispatcher(o, m);
  make_module_clone(o, m);
//  make_module_evals(o, m);
//  make_module_default_constructor(o, m);
  make_module_copy_constructor(o, m);
  make_module_precalc_first(o, m);
  make_module_is_valid(o, m);
  make_module_expand(o, m);
  make_module_precalc_last(o, m);
  make_cc_func(o, m);
//  make_module_probe(o, m);
//  make_module_aux(o, m);
  if(m.element_list().size()){
    o << "CARD_LIST* MOD_" << m.identifier() << "::scope()\n{\n";
    o__ "if(_parent){\n";
    o__ "  return COMPONENT::scope();\n";
    o__ "}else{\n";
    o__ "  return subckt();\n";
    o__ "}\n";
    o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
