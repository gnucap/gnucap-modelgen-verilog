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
#include "mg_circuit.h"
#include "mg_token.h"
#include <stack>
#include <numeric> // iota
#include "mg_.h" // TODO
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
  }else if(!used){ untested();
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
  o__ "trace3(\"" << m.identifier() <<"::needs_eval?\", long_label(), _sim->_time0, has_probes());\n";
  o__ "node_t gnd(&ground_node);\n";
  o__ "if (is_q_for_eval()) {\n";
  o____ "return false;\n";
  o__ "}else if (!converged()) {\n";
  o____ "return true;\n";
  o__ "}else if (has_probes()) {\n";
  o____ "return true;\n";
  o__ "}else ";

  if( m.circuit()->has_filters()) {
    o__ "if(_sim->is_first_iteration()){\n";
    o____ "return true;\n";
    o__ "}else";
  }else{
  }

  // BUG this is incomplete.
  for(auto br : m.circuit()->branches()) {
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
// DUP in code.cc
static bool is_output_var(tag_t t)
{
  ATTRIB_LIST_p const& a = attr.attributes(t);
  if(!a) {
  }else if(a->operator[](std::string("desc")) != "0"
         ||a->operator[](std::string("units")) != "0") {
    return true;
  }else{
  }
  return false;
}
/*--------------------------------------------------------------------------*/
static void make_tr_probe_num(std::ostream& o, const Module& m)
{
  o << "double MOD_" << m.identifier() << "::tr_probe_num(std::string const& n) const\n{\n";
  for(auto const& v : m.var_refs()) {
    if(auto p=dynamic_cast<Token_VAR_REF const*>(v.second)){
      o__ "//" << v.first << ":" << p->name() << "\n";
      std::string name = p->name();

      size_t L = strlen(PS_MANGLE_PREFIX);
      if(v.first.substr(0, L) == PS_MANGLE_PREFIX){
	// not a probe.
      }else if(is_output_var(tag_t(p))) {
	o__ "if(n == \"" << v.first << "\"){\n";
	o____ "return _v_._" << p->name() << ";\n";
	o__ "}\n";
      }else{
      }

    }else{
    }
  }
  o__ "if(n == \"conv\") {\n";
  o____ "return converged();\n";
  o__ "}\n";
  o__ "return " <<  baseclass(m) << "::tr_probe_num(n);\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_set_parameters(std::ostream& o, const Element_2& e, std::string cn)
{
  make_tag(o);
  if(cn==""){ untested();
    cn = e.code_name();
  }else{
  }
  o______ cn << "->set_parameters(\"" << e.short_label() << "\", this, ";
  if (e.eval() != "") {
    o << "&" << e.eval();
  }else if (e.discipline()) {
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
static void map_subdev_nodes(std::ostream& o, const Element_2& e)
{
  o____ "node_t nodes[] = {";
  std::string comma;
  // yikes, name vs. value
  if(e.ports().has_names()){
    Port_3_List_2::const_iterator p = e.ports().begin();
    for (;p != e.ports().end(); ++p) {
      o << comma << "_n[n_" << (**p).value() << "]";
      comma = ",";
    }
  }else{
    Port_3_List_2::const_iterator p = e.ports().begin();
    for (;p != e.ports().end(); ++p) {
      o << comma << "_n[n_" << (**p).name() << "]";
      comma = ",";
    }
  }
  {
    o << "};\n";
    o____ "c->set_parameters(c->short_label(), this";
    o << ", c->mutable_common()";
    o << ", 0."; // value
    o << ", 0, NULL";
    o << ", " << e.num_nodes() << ", nodes);\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_renew_sckt(std::ostream& o, Module const& m)
{
 //    o__ "renew_subckt(_parent, &(c->_netlist_params));\n";
  o__ "auto pp = prechecked_cast<MOD_" << m.identifier() << " const*>(_parent);\n";
  o__ "assert(pp);\n";
  o__ "assert(subckt());\n";
  for (auto const& e : m.circuit()->element_list()) {
    assert(e);
    o__ "assert(!" << e->code_name() << ");\n";
    o__ "assert(pp->" << e->code_name() << ");\n";
    o__ "{\n";
    o____ "auto c = prechecked_cast<COMPONENT*>(pp->" << e->code_name() << "->clone());\n";
    o____ "assert(c);\n";
    o____ "subckt()->push_back(c);\n";
    o____ "c->set_owner(this);\n";
    o____ e->code_name() << " = c;\n";
    o____ "trace1(\"renew\", " << e->code_name() << "->long_label());\n";
    map_subdev_nodes(o, *e);
    o__ "}\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_set_subdevice_parameters(std::ostream& o, const Element_2& e)
{
  for(auto p : e.list_of_parameter_assignments()){
    o____ "compon->set_param_by_name(\"" << p->name() << "\", "
       << "\"[" << e.short_label() << "]" << p->name() << "\");\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_module_construct_stub(std::ostream& o, const Element_2& e, Module const&)
{
  make_tag(o);
  // std::string dev_type = "instance_proto";
  std::string dev_type = "instance";
  assert(!dynamic_cast<Branch const*>(&e));

  o__ "const CARD* p = device_dispatcher[\"" << dev_type << "\"]; // " << e.dev_type() << "\n";
  o__ "if(!p){\n";
  o____ "throw Exception(" << "\"Cannot find " << dev_type << ". Load module?\");\n";
  o__ "}else{\n";
  o__ "}\n";
  o__ "auto compon = dynamic_cast<COMPONENT*>(p->clone());\n";
  o__ "compon->set_dev_type(\"" << e.dev_type() << "\");\n";
  o__ "compon->set_label(\"" << e.short_label() << "\");\n";
  o__ "compon->set_owner(this);\n";
  o__ "if(!compon){\n";
  o____ "throw Exception(" << "\"Cannot use " << dev_type << ": wrong type\"" << ");\n";
  o__ "}else{\n";
  o__ "}\n";
  o__ "assert(subckt());\n";
  o__ "// subckt()->push_front(" << e.code_name() << ");\n";
  o__ e.code_name() << " = compon;\n";

  o__ "{\n";
  
  assert(!dynamic_cast<Branch const*>(&e));
  Port_3_List_2::const_iterator p = e.ports().begin();
  o____ "std::string tmp;\n";
  if(e.ports().has_names()){
    o____ "std::string name;\n";
    int i = 0;
    for (;p != e.ports().end(); ++p) {
      o____ "name = \"" << (*p)->name()<<"\";\n";
      o____ "tmp = \""<< ++i <<"\";\n";
      o____ "compon->set_port_by_name(name, tmp);\n";
    }
  }else{
    int i = 0;
    for (;p != e.ports().end(); ++p) {
      o____ "tmp = \""<< i <<"\";\n";
      o____ "trace1(\"spbi " << i << "\", tmp);\n";
      o____ "compon->set_port_by_index(" << i << ", tmp);\n";
      ++i;
    }
  }

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
  o__ "// ports:" << m.circuit()->ports().size() << "\n";
  if(m.circuit()->element_list().size()){
    o__ "new_subckt();\n";
  }else{
  }
  for (Element_2_List::const_iterator
      e = m.circuit()->element_list().begin();
      e != m.circuit()->element_list().end();
      ++e) {
    o__ "{\n";
    {
      indent x;
      make_module_construct_stub(o, **e, m);
    }
    o__ "}\n";
  }
}
/*--------------------------------------------------------------------------*/
void make_module_default_constructor(std::ostream& o, const Module& m)
{
  make_tag(o);
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
  o__ "trace3(\"" << m.identifier() <<"::do_tr\", this, long_label(), _sim->iteration_number());\n";
  o__ "clear_branch_contributions();\n";
  o__ "read_probes();\n";
  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "assert(c);\n";

  // if has_analog
  o__ "set_converged();\n";
  o__ "c->tr_eval_analog(this);\n";
  o__ "set_branch_contributions();\n";

  o__ "assert(subckt());\n";
  o__ "set_converged(subckt()->do_tr() && converged());\n";
  // o__ "return converged() && _sim->iteration_number() > 3;\n";
  o__ "return converged();\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_begin(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::tr_begin()\n{\n";
  o__ "BASE_SUBCKT::tr_begin();\n";
  if(m.times()){
    o__ "_time[0] = 0.;\n";
    o__ "for (int i=" << m.times()-1 << "; i>0; --i) {\n";
    o____ "_time[i] = 0;\n";
    o__ "}\n";
  }else{ untested();
  }
  if(m.has_tr_begin_analog()) { untested();
  o__ "c->tr_begin_analog(this);\n"; // call from COMMON::tr_begin?
  }else{
  }
  if(m.has_tr_begin_digital()) { untested();
  o__ "c->tr_begin_digital(this);\n"; // call from COMMON::tr_begin?
  }else{
  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_restore(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::tr_restore()\n{\n";
  o__ "BASE_SUBCKT::tr_restore();\n";
  if(m.times()){
    o__ "for (int i=" << m.times()-1 << "; i>0; --i) {\n";
    o____ "_time[i] = 0;\n";
    o__ "}\n";
  }else{ untested();
  }
  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "assert(c);\n";
  o__ "(void)c;\n";
  if(m.has_tr_begin_analog()) { untested();
  o__ "c->tr_begin_analog(this);\n"; // for now
  }else{
  }
  if(m.has_tr_begin_digital()) { untested();
  o__ "c->tr_begin_digital(this);\n"; // for now
  }else{
  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_advance(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::tr_advance()\n{\n";
  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "assert(c);\n";
  o__ "(void)c;\n";

  if(m.has_analysis()) { // BUG?
    o__ "if(_sim->_last_time == 0.){\n";
    o__ "incomplete();\n";
    o__ "c->tr_eval_analog(this);\n";
    o__ "}else{\n";
    o__ "}\n";
  }else{
  }
  if(m.times()){
    o__ "for (int i=" << m.times()-1 << "; i>0; --i) {\n";
    o____ "assert(_time[i] < _time[i-1] || _time[i] == 0.);\n";
    o____ "_time[i] = _time[i-1];\n";
    // _y[i] = _y[i-1];
    o__ "}\n";
    o__ "_time[0] = _sim->_time0;\n";
  }else{
  }
  o__ "set_not_converged();\n";

  if(m.has_tr_advance_analog()){
    o__ "c->tr_advance_analog(this);\n";
  }else{
  }
  for(auto f : m.funcs()){
    f->make_cc_tr_advance(o);
  }
  o__ "_v_1 = _v_;\n";
  o__ baseclass(m) << "::tr_advance();\n"; // upside down. cf mg2_an2
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_regress(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::tr_regress()\n{\n";
  o__ baseclass(m) << "::tr_regress();\n";
  if(m.times()>1){
    o__ "assert(_time[1] <= _sim->_time0);\n";
  }else{
  }
  if(m.times()){
    o__ "assert(_time[0] >= _sim->_time0); // moving backwards\n";
    o__ "for (int i=" << m.times()-1 << "; i>0; --i) {\n";
    o____ "assert(_time[i] < _time[i-1] || _time[i] == 0.);\n";
    o__ "}\n";
    o__ "_time[0] = _sim->_time0;\n";
  }else{
  }
    // _y[i] = _y[i-1];
  o__ "set_not_converged();\n";

  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "assert(c);\n";
  o__ "_v_ = _v_1;\n";
  o__ "c->tr_regress_analog(this);\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_review(std::ostream& o, const Module& m)
{
  o << "inline TIME_PAIR MOD_" << m.identifier() << "::tr_review()\n{\n";
#if 0
  if(m.has_analysis()){ untested();
    o << "if(_sim->analysis_is_tran_static()){ untested();\n";
    o____ "q_accept();\n";
    o__ "}else ";
  }else{ untested();
    o__ "";
  }
#endif
  o__ "_time_by = BASE_SUBCKT::tr_review();\n";
  if(m.has_tr_accept()){
    o__ "_accept = 0;\n";
  }else{
  }
  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "assert(c);\n";
  o__ "trace1(\"review0\", _time_by._event);\n";
  o__ "c->tr_review_analog(this);\n";
  for(auto f : m.funcs()){
    f->make_cc_tr_review(o);
  }
  if(m.has_tr_accept()){
    o__ "if(_accept){\n";
    o____ "COMPONENT::q_accept();\n";
    o____ "trace1(\"" << m.identifier() <<"::_accept\", _sim->_time0);\n";
    o__ "}else{\n";
    o__ "}\n";
  }

  o__ "trace3(\"review done\", long_label(), _sim->_time0, _time_by._event);\n";
  o__ "return _time_by;\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_accept(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::tr_accept()\n{\n";
  o__ "trace1(\"" << m.identifier() <<"::tr_accept\", _sim->_time0);\n";

  o__ "COMMON_" << m.identifier() << " const* c = "
    "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
  o__ "assert(c);\n";
  o__ "c->tr_accept_analog(this);\n"; // call from COMMON::tr_accept?
  o__ "return " << baseclass(m) << "::tr_accept();\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_analog_list(std::ostream& o, const Module& m, Branch const*
    src=NULL);
/*--------------------------------------------------------------------------*/
// move to analog.cc?
static void make_tr_eval_branch(std::ostream& o, Module const& m,
		Branch const& br)
{
  make_tag(o);
  std::string class_name = "EVAL_" + br.code_name() + '_';
  o << "static " << class_name << " Eval_" << br.name()
      << "(CC_STATIC);\n"
    "void " << class_name << "::tr_eval(ELEMENT* d)const\n{\n";
  o__ "assert(d);\n"
    "  auto* p = prechecked_cast<DEV_"
      << m.identifier() << "*>(d->owner());\n"
    "  assert(p);\n"
    "  const COMMON_" << m.identifier() << "* c = prechecked_cast<const COMMON_"
      << m.identifier() << "*>(p->common());\n"
    "  assert(c);\n";
    make_cc_analog_list(o, m, &br);
    o<< "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tr_eval_branches(std::ostream& o, const Module& m)
{
  o << "#if 0\n";
  for(auto br : m.circuit()->branches()) {
    o << "// tr_eval_branch " << br->code_name() << "\n";
    make_tr_eval_branch(o, m, *br);
  }
  o << "#endif\n";
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
  for(auto x : m.circuit()->branches()){
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

  if(m.circuit()->element_list().size()){
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
    make_tr_eval_branches(o, m);
    make_do_tr(o, m);
    make_cc_analog(o, m);
  }else{
  }

  if(m.has_tr_begin()) {
    make_tr_begin(o, m);
    make_tr_restore(o, m);
  }else{
  }

  if(m.has_analysis()){
    make_tr_review(o, m);
  }else if(m.has_tr_review()){
    make_tr_review(o, m);
  }else{
  }
  if(m.has_tr_accept()){
    make_tr_accept(o, m);
  }else{
  }
  if(m.has_tr_advance()){
    make_tr_advance(o, m);
    make_tr_regress(o, m);
  }else{
    assert(!m.has_analysis());
  }

  // TODO: set_port_by_name
  o__ "std::string MOD_" << m.identifier() << "::port_name(int i)const\n{\n";
  o____ "assert(i >= 0);\n";
  o____ "assert(i < max_nodes());\n";
  o____ "static std::string names[] = {";
  std::string comma = "";
  for (auto nn : m.circuit()->ports()){ // BUG: array?
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
  make_tag(o);
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
      o______ "_n[n_" << p.name() << "] = "; // _n[n_" << p.short_to()->name() << "];\n";
	make_node_ref(o, *p.short_to());
	o << ";\n";
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
}
/*--------------------------------------------------------------------------*/
static void make_module_allocate_local_nodes(std::ostream& o, Module const& m)
{
  for (int n=1; n<=int(m.circuit()->nodes().size()); ++n) {
    Node const* nn = m.circuit()->nodes()[n];
    assert(nn);
    if(nn->number() == 0) {
      o__ "// ground\n";
      o__ "_n[n_" << nn->name() << "].set_to_ground(this);\n";
    }else if(nn->number() < n){
    }else if(n <= int(m.circuit()->ports().size())){
      o__ "// port " << nn->name() << " " << nn->number() << "\n";
    }else if(nn->is_used()){
      o__ "// internal " << nn->name() << " : " << nn->number() << "\n";
      make_module_allocate_local_node(o, *nn);
    }else{
      o__ "// unused " << nn->name() << " : " << nn->number() << "\n";
      o__ "_n[n_" << nn->name() << "].set_to_ground(this);\n"; // for now.
    }
  }

  for (int n=1; n<=int(m.circuit()->nodes().size()); ++n) {
    Node const* nn = m.circuit()->nodes()[n];
    assert(nn);
    if(nn->number() == 0) {
    }else if(nn->number() < n){
      o__ "_n[" << n - 1 << "] = _n[" << nn->number() - 1 << "]; // (a)\n";
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
// out_analog??
static void make_module_expand_one_branch(std::ostream& o, const Element_2& e, Module const&, std::string cn_)
{
  std::string cn;
  if(cn_==""){
    cn = e.code_name();
  }else{ untested();
    cn = cn_;
  }

  make_tag(o);
  auto br = dynamic_cast<Branch const*>(&e);
  if(br){
  }else{
  }

  if (br && br->is_short() && !br->is_filter()) { untested();
    o__ "if(0){ // short\n";
  }else if (!(e.omit().empty())) { untested();
    o__ "if (" << e.omit() << ") {\n";
    o____ "if (" << cn << ") {\n";
    o______ "subckt()->erase(" << cn << ");\n";
    o______ cn << " = NULL;\n";
    o____ "}else{\n";
    o____ "}\n";
    o__ "}else{\n";
  }else{
    o__ "{\n";
  }

  std::string dev_type = e.dev_type();

  o__ "if (!" << cn << ") {\n";
  o____ "const CARD* p = device_dispatcher[\"" << dev_type << "\"]; // " << e.dev_type() << "\n";
  o____ "if(!p){\n";
  o______ "throw Exception(" << "\"Cannot find " << dev_type << ". Load module?\");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ cn << " = dynamic_cast<ELEMENT*>(p->clone()); // elt\n";
  o____ "if(!" << cn << "){\n";
  o______ "throw Exception(" << "\"Cannot use " << dev_type << ": wrong type\"" << ");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ "subckt()->push_front(" << cn << ");\n";
//o____ e.code_name() << "->set_dev_type(\"" << e.dev_type() << "\");\n";
  o__ "}else{\n";
  o__ "}\n";

#if 1
  o____ "{\n";
  
  o______ "node_t nodes[] = {";
  
#if 1
//  auto ee = Named_Branch(e, cn_); // TODO
  if(e.num_nodes()){
    make_cc_branch_output(o, br);
    make_cc_branch_ctrl(o, br);

    o << "}; // nodes\n";

//    Port_1_List::const_iterator p = e.current_ports().begin();
//    make_set_parameters(o, ee, cn);
    make_set_parameters(o, e, cn);
    if(br == br->output()){
      // internal node
    }else{
      o______ "if(auto e = dynamic_cast<ELEMENT*>(" << cn << ")){\n";
      o________ "e->_loss0 = 0.;\n";
      o________ "e->_loss1 = 0.;\n";
      o______ "}else{\n";
      o______ "}\n";
    }

    make_cc_current_ports(o, br, e);
  }else{
    o << "gnd, gnd"; // filt subs hack.
    o << "}; // nodes\n";
    make_set_parameters(o, e, cn);
  }
#endif
  
  o << "      }\n";
#endif
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_precalc_first(std::ostream& o, Module const& m)
{
  make_tag(o);
  String_Arg const& mid = m.identifier();
  o << "void MOD_" << mid << "::precalc_first()\n{\n";
  o__ baseclass(m) << "::precalc_first();\n";
  o__ "trace2(\"" << m.identifier() <<"::pf\", long_label(), mfactor());\n";

  o__ "auto c = static_cast<COMMON_" << mid << "*>(mutable_common());\n";
  o__ "assert(c);\n";
  o__ "auto cc = c->clone();\n";

  if(m.has_analog_block()){
    // o__ "c->precalc_analog(this);\n";
  }else{
  }

  o__ "if(subckt()){\n";
  if(m.circuit()->element_list().size()){
    o__ "subckt()->attach_params(&(c->_netlist_params), scope());\n";
  }else{
  }
  o____ "attach_common(NULL);\n";
  o____ "attach_common(cc);\n";
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
  make_tag(o);
  String_Arg const& mid = m.identifier();
  o << "void MOD_" << mid << "::precalc_last()\n{\n";
//    o__ baseclass(m) << "::precalc_last();\n";
  o__ "CARD::precalc_last();\n";

  o__ "try {\n";
  o____ "mutable_common()->precalc_last(scope());\n"; // for now.
  o__ "}catch (Exception_Precalc& e) { untested();\n";
  o____ "error(bWARNING, long_label() + \": \" + e.message());\n";
  o__ "}\n;";

  if(m.circuit()->element_list().size()) {
    o__ "assert(subckt());\n";
    o__ "subckt()->precalc_last();\n";
  }else{
  }

  // bug? using mutable_common.. again.
  o__ "auto c = static_cast<COMMON_" << mid << "*>(mutable_common());\n";
  o__ "assert(c);\n";
  o__ "(void)c;\n";

  if(m.circuit()->element_list().size()){
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
  make_tag(o);
  String_Arg const& mid = m.identifier();
  o << "void MOD_" << mid << "::expand()\n{\n";
  o__ "trace1(\"expand\", long_label());\n";

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
  make_module_allocate_local_nodes(o, m);
  if(m.circuit()->element_list().size()){
    o__ "assert(_parent);\n";
    o__ "assert(_parent->subckt());\n";
    o__ "assert(_parent->subckt()->nodes());\n";
    o__ "// trace2(\"\",  _parent->net_nodes(),  _parent->subckt()->nodes()->how_many());\n";
    o__ "assert(_parent->net_nodes() <= _parent->subckt()->nodes()->how_many());\n";
    o__ "// assert(_parent->subckt()->params());\n";
    o__ "PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());\n";
    o__ "assert(pl);\n";

    // o__ "c->_params.set_try_again(pl);\n";
  }else{
  }
  o__ "if (_sim->is_first_expand()) {\n";

    if(m.circuit()->element_list().size()){
      make_renew_sckt(o, m);
    }else{
    }
    o____ "precalc_first();\n";
//    "    // optional nodes\n";
//  for (Port_1_List::const_iterator
//       p = d.circuit().opt_nodes().begin();
//       p != d.circuit().opt_nodes().end();
//       ++p) { untested();
//    make_dev_allocate_local_nodes(out, **p);
//  }
  o << "\n";
  o__ "// clone branches\n";
  for(auto i: m.circuit()->branches()){
    if(i->is_filter()) {
      o__ "// filter " << i->name() << "\n";
      indent x;
      if(i->is_used()) {
	make_module_expand_one_branch(o, *i, m, "");
      }else{
	o__ "//unused filter\n";
      }
    }else if(!i->is_used()) {
      o__ "// unused " << i->name() << "\n";
    }else if(i->has_element()) {
      o__ "// branch " << i->name() << "\n";
      indent x;
      make_module_expand_one_branch(o, *i, m, "");
//      for(auto n : i->names()){ untested();
//	make_module_expand_one_branch(o, *i, m, "_br_" + n);
//      }
    }else{
      o__ "// branch no elt " << i->name() << "\n";
    }
  }

  o << "\n";
  o__ "// clone filters\n";
#if 1
  for (auto i: m.circuit()->filters()){
    if(i->has_branch()){ untested();
      unreachable();
    }else{
      // TODO incomplete();
      o__ "// no branch? " << i->name() << "\n";
      make_module_expand_one_branch(o, *i, m, "");
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
//    o << "  subckt()->set_slave();\n";
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
  if(1||m.size()) {
    o__ "inline MOD_" << m.identifier() << "* DEV(COMPONENT* d)\n{\n";
    o____ "auto m = prechecked_cast<MOD_" << m.identifier() << "*>(d);\n";
    o____ "assert(m);\n";
    o____ "return m;\n";
    o__ "};\n";
  }else{ untested();
  }
  for(FUNCTION_ const* f : m.funcs()){
    make_tag(o);
    if(f->has_probes()){
      f->make_cc_impl(o);
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_module_set_param_by_name(std::ostream& o, const Module& m)
{
  o << "aidx MOD_" << m.identifier() << "::set_param_by_name("
       "std::string Name, std::string Value)\n{\n";
  o__ "trace2(\"spbn " << m.identifier() << "\", Name, Value);\n";

  o__ "{\n";


  o__ "static std::string names[] = {";
  int cnt = 0;
  std::vector<std::string const*> names;
  std::vector<std::string const*> hs;
  std::vector<int> alias;

  for (auto n : m.aliasparam()){
    if(n->is_hs()){
      names.push_back(&n->name());
      hs.push_back(&n->param_name());
      alias.push_back(-1);
    }else{
    }
  }

  std::vector<int> idx(names.size());
  std::iota(idx.begin(), idx.end(), 0);
  std::sort(idx.begin(), idx.end(),
      [&](int A, int B) -> bool { untested();
      return *names[A] < *names[B];
      });


  std::string comma;
  for(auto n : idx){
      o << comma << "\"" << *names[n] << "\"";
      comma = ", ";
  }
  cnt = int(names.size());
  o__ "};\n";
  o__ "int lb = 0;\n";
  o__ "int ub = " << cnt << ";\n";
  o__ "while(lb < ub) {\n";
  o____ "int mid = (lb+ub)/2;\n";
  o____ "int c = std::strcmp(Name.c_str(), names[mid].c_str());\n";
  o____ "if(c<0){\n";
  o______ "ub = mid;\n";
  o____ "}else if(c>0){\n";
  o______ "lb = mid+1;\n";
  o____ "}else{\n";
  o______ "lb = mid;\n";
  o______ "ub = " << cnt << ";\n";
  o______ "assert(lb<ub);\n";
  o______ "break;\n";
  o____ "}\n";
  o__ "}\n";
  o__ "assert(lb<=ub);\n";

  o__ "switch(ub-lb){\n";
  for(auto n : idx){
    std::string cn = "_p_" + *names[n];
    std::string pn;
    o____ "case " << cnt << ":";
    if(alias[n] == -1){
      assert(hs[n]);
      o__ "return COMPONENT::set_param_by_name(\"" << *hs[n] << "\", Value);\n";
    }else{ untested();
      unreachable();
    }
    o____ "break; // " << alias[n] << "\n";
    --cnt;
  }
  o____ "case 0: return COMPONENT::set_param_by_name(Name, Value);\n";

  o__ "}\n";
  o__ "return lb;\n";
  o__ "}\n";

  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_module(std::ostream& o, const Module& m)
{
  make_tag(o);

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
  if(m.has_hsparam()) {
    make_module_set_param_by_name(o, m);
  }else{
  }
  make_module_precalc_first(o, m);
  make_module_is_valid(o, m);
  make_module_expand(o, m);
  make_module_precalc_last(o, m);
  make_cc_func(o, m);
//  make_module_probe(o, m);
//  make_module_aux(o, m);
  if(m.circuit()->element_list().size()){
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
std::string Named_Branch::code_name() const
{
  return "_nb_" + name();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
