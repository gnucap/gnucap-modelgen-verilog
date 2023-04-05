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
#include "m_tokens.h"
#include <stack>
/*--------------------------------------------------------------------------*/
#if 0
bool Module::sync() const
{
  // need getattr<bool>?
  if(!has_attr("sync")){
    true, false? automatic? how
  }else if(attr("sync") == "1"
        || attr("sync") == "yes"){
    return true;
  }else if(attr("sync") == "0"
        || attr("sync") == "no"){
  }else{
    unsupported.
  }
}
#endif
/*--------------------------------------------------------------------------*/
static int is_function(std::string const& n)
{
  incomplete();
  // stub, need sth.
  if (n == "exp"
   || n == "log"
   || n == "cos"
   || n == "sin") {
    return 1;
  }else if (n == "pow"){
    return 2;
  }else{
    return 0;
  }
}
/*--------------------------------------------------------------------------*/
static String_Arg const& potential_abstol(Branch const& b)
{
  static String_Arg def("OPT::abstol");
  if(!b.discipline()){
    incomplete();
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
  if(!b.discipline()){
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
static void make_tr_needs_eval(std::ostream& o, const Module& m)
{
  o << "bool MOD_" << m.identifier() << "::tr_needs_eval()const\n{\n";
  o__ "if (is_q_for_eval()) { untested();\n";
  o____ "return false;\n";
  o__ "}else if (!converged()) {\n";
  o____ "return true;\n";
  o__ "}else";
  for(auto x : m.probes()){
    Probe const* p = x.second;
    assert(p);
    if(p->is_reversed()){
      // only check once.
    }else if(p->is_pot_probe()){
      o << " if(!conchk("<< p->code_name() << ", "
	  "_n[n_"<< p->pname() <<"].v0() - _n[n_"<< p->nname() <<"].v0(), ";
      o << potential_abstol(*p->branch()) << ")){\n";
      o____ "trace3(\"need eval\", "<< p->code_name() << ", "
          "_n[n_"<< p->pname() <<"].v0(), _n[n_"<< p->nname() <<"].v0());\n";
      o____ "return true;\n" <<ind<<"}else";

      if(p->nature())
      o__ "/* Nature: " << p->nature()->identifier() << "*/";
    }else if(p->is_flow_probe()){
      o << " if(!conchk("<< p->code_name() << ", "
	<<  p->branch()->code_name() << "->tr_amps(), ";
      o << flow_abstol(*p->branch()) << ")){\n";
      o____ "trace2(\"need eval\", "<< p->code_name() << ", " << p->branch()->code_name() << "->tr_amps());\n";
      o____ "return true;\n" <<ind<<"}else";

      if(p->nature())
      o__ "/* Nature: " << p->nature()->identifier() << "*/";
    }else{
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
  // no range check whatsoever. debugging/testing, remove later.
  for(int i=0; i<4; ++i){
    o << ind << "if(n == \"v" << i << "\") return _n[" << i << "].v0();\n";
  }
  size_t i = 0;
  for(auto b : m.branches()){
    if(b.second->has_element()){
      o << ind << "if(n == \"i" << i << "\") return (_value" << b.second->code_name() << ");\n";
      for(int j=0; j<4; ++j){
	o << ind << "if(n == \"i" << i << "_d"<<j<<"\") return ("
		  << b.second->state() << "["<<j<<"]);\n";
      }
      ++i;
    }else{
    }
  }
  o << ind << "return NOT_VALID;\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_module_default_constructor(std::ostream& out, const Module& d)
{
  make_tag();
  out << "MOD_" << d.identifier() << "::MOD_" << d.identifier() << "() :" << baseclass(d) << "()";

//  out << ",\n   // input parameters";
//  make_construct_parameter_list(out, d.parameters());

//  out << ",\n   // calculated parameters";
//  make_construct_parameter_list(out, d.device().calculated());

//  out << ",\n   // netlist";
//  for (Element_1_List::const_iterator
//       p = d.elements().begin();
//       p != d..elements().end();
//       ++p) {
//    out << ",\n   _" << (**p).name() << "(0)";
//  }

  out << "\n{\n"
    "  _n = _nodes;\n"
    "  attach_common(&Default_" << d.identifier() << ");\n"
    "  ++_count;\n";

  out << "  // overrides\n";
//  for (Parameter_1_List::const_iterator
//       p = d.device().override().begin();
//       p != d.device().override().end();
//       ++p) {untested();
//    if (!((**p).final_default().empty())) {untested();
//      out << "  " << (**p).code_name() << " = NA;\n";
//    }else{untested();
//    }
//    if (!((**p).default_val().empty())) {untested();
//      out << "  " << (**p).code_name() << " = " << (**p).default_val() <<";\n";
//    }else{untested();
//    }
//  }
  out << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_do_tr(std::ostream& o, const Module& m)
{
    o << "bool MOD_" << m.identifier() << "::do_tr()\n{\n";
    o__ "clear_branch_contributions();\n";
    o__ "read_probes();\n";
    o__ "COMMON_" << m.identifier() << " const* c = "
        "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
    o__ "assert(c);\n";
    o__ "c->tr_eval_analog(this);\n";
    o__ "set_branch_contributions();\n";
    o__ "assert(subckt());\n";
    o__ "set_converged(subckt()->do_tr());\n";
    o__ "return converged();\n";
    o << "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_read_probes(std::ostream& o, const Module& m)
{
  o << "void MOD_" << m.identifier() << "::read_probes()\n{\n";
  for(auto x : m.branches()){
    Branch const* b = x.second;
    assert(b);
    if(b->has_pot_probe()){
      o__ "_potential" << b->code_name() << " = volts_limited(_n[n_"<< b->p()->name() <<"], _n[n_"<< b->n()->name() <<"]);\n";
    }else if(b->has_flow_probe()){
      o__ "assert(" << b->code_name() << ");\n";
      o__ "_flow" << b->code_name() << " = " << b->code_name() << "->tr_amps();\n";
    }else{
      trace1("no probe?", b->code_name());
    }
  }

  for(auto x : m.filters()){
    o__ "// filter " << x->code_name() << "\n";
  }
//  for(auto x : m.probes()){
//    Probe const* p = x.second;
//    assert(p);
//    o << ind << "_" << p->name() << " = volts_limited(_n[n_"<< p->pname() <<"], _n[n_"<< p->nname() <<"]);\n";
//    // o << "trace1(\"\"," << "_" << p->name() << ");\n";
//  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_one_branch_contribution(std::ostream& o, Module const& m, const Branch& br)
{
  Branch const* b = &br;
  size_t k = 1;
  o__ "assert(_value" << b->code_name() << " == _value" << b->code_name() << ");\n";
  o__ b->state() << "[0] = _value" << b->code_name() << ";\n";
  for(auto d : b->deps()){
    if(d->branch() == b){
      if(b->has_pot_source() && b->has_flow_probe()){ untested();
	throw Exception("cannot use self as current probe.. yet\n");
      }else{ untested();
      }
      o__ "trace2(\"" <<  b->state() << "self\", " << b->state() << "["<<k<<"], "<<  d->code_name() <<");\n";
      o__ b->state() << "[0] -= " << b->state() << "["<<k<<"] * "<< d->code_name() << ";\n";
      break;
    }else{
    }
  }
  k = 2;
  for(auto i : m.branches()){
    o__ "trace1(\"" <<  b->state() << "\", _value" << b->code_name() << ");\n";
    for(auto d : b->deps()){
      if(d->branch() == b){
      }else if(d->branch() != i.second){
      }else if(d->is_pot_probe()){
	o__ "trace2(\" pot " <<  b->state() << "\", " << b->state() << "["<<k<<"], "<<  d->code_name() <<");\n";
	o__ b->state() << "[0] -= " << b->state() << "["<<k<<"] * "<< d->code_name() << ";\n";
	++k;
	break;
      }else{
      }
    }
  }
  for(auto i : m.branches()){
    o__ "trace1(\"" <<  b->state() << "\", _value" << b->code_name() << ");\n";
    for(auto d : b->deps()){
      if(d->branch() == b){
      }else if(d->branch() != i.second){
      }else if(d->is_flow_probe()){
	o__ "trace2(\"flow " <<  b->state() << "\", " << b->state() << "["<<k<<"], "<<  d->code_name() <<");\n";
	o__ b->state() << "[0] -= " << b->state() << "["<<k<<"] * "<< d->code_name() << ";\n";
	// BUG? scaling before convcheck?
	o__ b->state() << "["<<k<<"] *= " << d->branch()->code_name() <<"->_loss0; // BUG?\n";
	++k;
	break;
      }else{
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_set_branch_contributions(std::ostream& o, const Module& m)
{
  o << "void MOD_" << m.identifier() << "::set_branch_contributions()\n{\n";
  for(auto i : m.branches()){
    Branch const* b = i.second;
    if(b->has_pot_source()) {
      assert(!b->has_flow_source());
      o__ "// pot src " << b->name() << "\n";
    }else if(b->has_flow_source()) {
      assert(!b->has_pot_source());
      o__ "// flow src " << b->name() << "\n";
    }else{
    }

    if(b->has_flow_source() || b->has_pot_source()) {
      make_one_branch_contribution(o, m, *b);
    }else{
    }
  }

  o__ "// filters\n";
  for(auto x : m.filters()){
    o__ "// filter " << x->code_name() << "\n";
  }

  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_class(std::ostream& o, Module const& m)
{
  o << "MOD_" << m.identifier() << "::MOD_" << m.identifier() << "(MOD_" << m.identifier() << " const&p) : "
    <<  baseclass(m) << "(p)\n{\n"
    << ind << "_n = _nodes;\n"
    << ind << "for (int ii = 0; ii < max_nodes() + int_nodes(); ++ii) {\n"
    << ind << ind << "_n[ii] = p._n[ii];\n"
    << ind << "}\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  make_tr_probe_num(o, m);
  make_set_branch_contributions(o, m);
  make_read_probes(o, m);
  make_module_default_constructor(o, m);
  o << "// seq blocks\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  if(m.has_analog_block()){
    make_tr_needs_eval(o, m);
    make_do_tr(o, m);
  }else{
  }
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
  o__ "}\n";
} // make_module_class
/*--------------------------------------------------------------------------*/
static void make_module_allocate_local_node(std::ostream& out, const Port_1& p)
{
#if 1
  make_tag();
  if (p.short_if().empty()) {
    out <<
      "    if (!(_n[n_" << p.name() << "].n_())) {\n"
      "      _n[n_" << p.name() << "] = _n[n_" << p.short_to() << "];\n"
      "    }else{\n"
      "    }\n";
    //BUG// generates bad code if no short_to
  }else{
    out <<
      "    //assert(!(_n[n_" << p.name() << "].n_()));\n"
      "    //BUG// this assert fails on a repeat elaboration after a change.\n"
      "    //not sure of consequences when new_model_node called twice.\n"
      "    if (!(_n[n_" << p.name() << "].n_())) {\n"
      "      if (" << p.short_if() << ") {\n"
      "        _n[n_" << p.name() << "] = _n[n_" << p.short_to() << "];\n"
      "      }else{\n"
      "        _n[n_" << p.name() << "].new_model_node(\".\" + long_label() + \"." << p.name() 
			   << "\", this);\n"
      "      }\n"
      "    }else{\n"
      "      if (" << p.short_if() << ") {\n"
      "        assert(_n[n_" << p.name() << "] == _n[n_" << p.short_to() << "]);\n"
      "      }else{\n"
      "        //_n[n_" << p.name() << "].new_model_node(\"" << p.name() 
		 << ".\" + long_label(), this);\n"
      "      }\n"
      "    }\n";
  }
#endif
}
/*--------------------------------------------------------------------------*/
static void make_set_parameters(std::ostream& o, const Element_2& e)
{
  make_tag();
  o______ e.code_name() << "->set_parameters(\"" << e.code_name() << "\", this, ";
  if (e.discipline()) {
    // incomplete(); need NODE commons (or so)
    o << "&_C_V_" << e.discipline()->identifier();
  }else{ untested();
    o << "NULL";
  }
  o << ", 0."; // value
  if (e.state() != "") {
    o << ", " << e.num_states() << ", " << e.state();
  }else{
    o << ", 0, NULL";
  }
  o << ", " << e.num_nodes() << ", nodes);\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_expand_one_element(std::ostream& o, const Element_2& e, Module const& m)
{
  make_tag();
  if (!(e.omit().empty())) {
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

  o__ "if (!" << e.code_name() << ") {\n";
  o____ "const CARD* p = device_dispatcher[\"" << e.dev_type() << "\"];\n";
  o____ "if(!p){\n;";
  o______ "throw Exception(" << "\"Cannot find " << e.dev_type() << ". Load module?\");\n";
  o____ "}else{\n";
  o____ "}\n";
  if(auto br = dynamic_cast<Branch const*>(&e)){
    o____ e.code_name() << " = dynamic_cast<ELEMENT*>(p->clone());\n";
  }else{
    o____ e.code_name() << " = dynamic_cast<COMPONENT*>(p->clone());\n";
  }
  o____ "if(!" << e.code_name() << "){\n";
  o______ "throw Exception(" << "\"Cannot use " << e.dev_type() << ": wrong type\"" << ");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ "subckt()->push_front(" << e.code_name() << ");\n";
  o__ "}else{\n";
  o__ "}\n";

#if 1
  o____ "{\n";
  
  o______ "node_t nodes[] = {";
  
  if(auto br = dynamic_cast<Branch const*>(&e)) {
    o << "_n[n_" << br->p()->name() << "],";
    o << "_n[n_" << br->n()->name() << "]";
    for(auto x : m.branches()){
      Branch const* b = x.second;
      for(auto i : br->deps()){
	if(i->branch() != b){
	}else if(i->branch() == br){
	  o << "/* self conductance */" << "";
	}else if(i->is_pot_probe()){
	  assert(i->branch());
	  o << ",_n[n_" << i->branch()->p()->name() << "]";
	  o << ",_n[n_" << i->branch()->n()->name() << "]";
	  break;
	}else{
	}
      }
    }
    o << "}; // nodes\n";

    Port_1_List::const_iterator p = e.current_ports().begin();
    make_set_parameters(o, e);

    int kk = 0;
    // BUG iterate branches?
#if 0
    for(auto i : br->deps()){
      if(i->branch() == br){
      }else if(i->is_flow_probe()){
	assert(i->branch());
	o______ e.code_name() << "->set_current_port_by_index( "<< kk << ", \"" << i->branch()->code_name() << "\");\n";
      }else{
      }
    }
#else
    for(auto x : m.branches()){
      Branch const* b = x.second;
      for(auto i : br->deps()){
	if(i->branch() != b){
	}else if(i->branch() == br){
	}else if(i->is_flow_probe()){
	  assert(i->branch());
	  o______ e.code_name() << "->set_current_port_by_index( "<< kk << ", \"" << i->branch()->code_name() << "\");\n";
	  break;
	}else{
	}
      }
    }
#endif
  }else{
    Port_3_List_2::const_iterator p = e.ports().begin();
    if (p != e.ports().end()) {
      assert(*p);
      o << "_n[n_" << (**p).name() << "]";
      while (++p != e.ports().end()) {
	o << ", _n[n_" << (**p).name() << "]";
      }
    }else{
    }
    o << "}; // nodes\n";
    make_set_parameters(o, e);
  }
  
  o << "      }\n";
#endif
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_expand_one_filter(std::ostream& o, const Filter& e)
{
  make_tag();
  if (!(e.omit().empty())) {
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

  // BUG: duplicate
  o__ "if (!" << e.code_name() << ") {\n";
  o____ "const CARD* p = device_dispatcher[\"" << e.dev_type() << "\"];\n";
  o____ "if(!p){\n;";
  o______ "throw Exception(" << "\"Cannot find " << e.dev_type() << ". Load module?\");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ e.code_name() << " = dynamic_cast<COMPONENT*>(p->clone());\n";
  o____ "if(!" << e.code_name() << "){\n";
  o______ "throw Exception(" << "\"Cannot use " << e.dev_type() << ": wrong type\"" << ");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ "subckt()->push_front(" << e.code_name() << ");\n";
  o__ "}else{\n";
  o__ "}\n";

#if 1
  o____ "{\n";
  
  o______ "node_t nodes[] = {";
  
  if(auto br = dynamic_cast<Filter const*>(&e)) {
    o << "_n[n_" << br->name() << "],";
    o << "_n[0]"; // eek
    for(auto i : br->deps()){
      o << ", _n[" << i->branch()->p()->code_name() << "]";
      o << ", _n[" << i->branch()->n()->code_name() << "]";
      //o << ",_n[n_" << i->branch()->p()->name() << "]";
      //o << ",_n[n_" << i->branch()->n()->name() << "]";
    }
  }else{
  }
  
  o << "};\n";
//  make_set_parameters(o, e);
  o << "      }\n";
#endif
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_expand(std::ostream& o, Module const& m)
{
  make_tag();
  String_Arg const& mid = m.identifier();
  o << "void MOD_" << mid << "::expand()\n{\n";
  o << ind << baseclass(m) << "::expand();"
    << ind << "assert(_n);\n"
    << ind << "assert(common());\n"
    "  const COMMON_" << mid << "* c = static_cast<const COMMON_"
      << mid << "*>(common());\n"
    "  assert(c);\n"
    "  if (!subckt()) {\n"
    "    new_subckt();\n"
    "  }else{\n"
    "  }\n"
    "\n"
    "  if (_sim->is_first_expand()) {\n"
    "    precalc_first();\n"
    "    precalc_last();\n";
//    "    // optional nodes\n";
//  for (Port_1_List::const_iterator
//       p = d.circuit().opt_nodes().begin();
//       p != d.circuit().opt_nodes().end();
//       ++p) {
//    make_dev_allocate_local_nodes(out, **p);
//  }
  o << ind << "    // local nodes\n";
  for (Port_1_List::const_iterator
       p = m.local_nodes().begin();
       p != m.local_nodes().end();
       ++p) {
    make_module_allocate_local_node(o, **p);
  }
  o << "\n";
  o__ "// clone branches\n";
  for(auto i: m.branches()){
    if(i.second->has_element()) {
      o__ "// branch " << i.second->name() << "\n";
      indent x;
      make_module_expand_one_element(o, *i.second, m);
    }else{
    }
  }

  o << "\n";
  o__ "// clone filters\n";
  for (auto i: m.filters()){
    o__ "// " << i->name() << "\n";
    indent x;
    make_module_expand_one_filter(o, *i);
  }

  o << "\n";
  o__ "// clone subckt elements\n";
  for (Element_2_List::const_iterator
       e = m.element_list().begin();
       e != m.element_list().end();
       ++e) {
    indent x;
    make_module_expand_one_element(o, **e, m);
  }
  o__ "}else{\n";
  o____ "  //precalc();\n";
  o__ "}\n";
  o__ "//precalc();\n";
  o__ "subckt()->expand();\n";
  o__ "//subckt()->precalc();\n";
  o__ "assert(!is_constant());\n";
//  if (m.sync()) {
//    o << "  subckt()->set_slave();\n";
//  }else{
//  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_module_dispatcher(std::ostream& o, Module const& m)
{
  o << "MOD_" << m.identifier() << " m_" << m.identifier() << ";\n";
    o << "DISPATCHER<CMD>::INSTALL d0(&device_dispatcher, \""
      << m.identifier() << "\", &m_" << m.identifier() << ");\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
#if 0
static void make_module_probe_num(std::ostream& out, const Module& d)
{
  make_tag();
  out << "double DEV_" << d.name() << "::tr_probe_num(const std::string& x)const\n"
    "{\n"
    "  assert(_n);\n"
    "  const COMMON_" << d.name() << "* c = prechecked_cast<const COMMON_"
      << d.name() << "*>(common());\n"
    "  assert(c);\n"
    "  const MODEL_" << d.model_type() << "* m = prechecked_cast<const MODEL_"
      << d.model_type() << "*>(c->model());\n"
    "  assert(m);\n"
    "  const SDP_" << d.model_type() << "* s = prechecked_cast<const SDP_"
      << d.model_type() << "*>(c->sdp());\n"
    "  assert(s);\n"
    "\n"
    "  ";
//   for (Probe_List::const_iterator
//        p = d.probes().begin();
//        p != d.probes().end();
//        ++p) {
//     assert(*p);
//     out << "if (Umatch(x, \"" << (**p).name() << " \")) {\n"
//       "    return " << fix_expression((**p).expression()) << ";\n"
//       "  }else ";
//   }
  make_probe_parameter_list(out, d.device().calculated());
  out << "{\n"
    "    return BASE_SUBCKT::tr_probe_num(x);\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
#endif
/*--------------------------------------------------------------------------*/
void make_cc_module(std::ostream& out, const Module& m)
{
  make_tag();

  make_cc_decl(out, m);
  make_cc_common(out, m);
  out <<
      "int COMMON_" << m.identifier() << "::_count = -1;\n"
      "static COMMON_" << m.identifier() << " Default_" << m.identifier()
	<< "(CC_STATIC);\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  out <<
      "/*--------------------------------------"
      "------------------------------------*/\n";
  make_module_class(out, m);
    out << "int MOD_" << m.identifier() << "::_count = -1;\n";
  make_module_dispatcher(out, m);
//  make_module_evals(out, m);
//  make_module_default_constructor(out, m);
//  make_module_copy_constructor(out, m);
  make_module_expand(out, m);
//  make_module_probe(out, m);
//  make_module_aux(out, m);
  out << "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
#if 0
void make_module_default_constructor(std::ostream& out, const Module& m)
{
  make_tag();
  out << "MOD_" << m.identifier() << "::MOD_" << m.identifier() << "()" << baseclass(m) << "()";

  out << ",\n   // input parameters";
  make_construct_parameter_list(out, m.parameters());

//   out << ",\n   // calculated parameters";
//   make_construct_parameter_list(out, m.device().calculated());

  out << ",\n   // netlist";
//   for (Element_1_List::const_iterator
//        p = m.circuit().elements().begin();
//        p != m.circuit().elements().end();
//        ++p) {
//     out << ",\n   _" << (**p).name() << "(0)";
//   }

  out << "\n{\n"
    "  _n = _nodes;\n"
    "  attach_common(&Default_" << d.name() << ");\n"
    "  ++_count;\n";

  out << "  // overrides\n";
  for (Parameter_1_List::const_iterator
       p = d.device().override().begin();
       p != d.device().override().end();
       ++p) {untested();
    if (!((**p).final_default().empty())) {untested();
      out << "  " << (**p).code_name() << " = NA;\n";
    }else{untested();
    }
    if (!((**p).default_val().empty())) {untested();
      out << "  " << (**p).code_name() << " = " << (**p).default_val() <<";\n";
    }else{untested();
    }
  }
  out << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
