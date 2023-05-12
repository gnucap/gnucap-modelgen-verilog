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
#endif
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
  for(auto const& vl : m.variables()){
    // todo: only those with desc or unit attribute
    for (Variable_List::const_iterator p=vl->begin(); p!=vl->end(); ++p) {
      o__ "if(n == \"" << (*p)->name() << "\"){\n";
      o__ ind << "return _v_" << (*p)->name() << ".value();\n";
      o__ "}\n";
    }
  }
  o << ind << "return NOT_VALID;\n";
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
  if(auto f=dynamic_cast<Filter const*>(&e)){
    cn = f->branch_code_name();
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
  }else{
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
  {
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
  }
#endif

//    make_set_parameters(o, e);
    make_set_subdevice_parameters(o, e);
  
  o____ "\n";
  o__ "}\n";
}
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
    for (auto nn : m.nodes()){
      std::string ext_name = nn->name();
      int ii = nn->number();

      if(ii < int(m.ports().size())){
//	o__ "_n[" << ii << "].new_node(\"" << ext_name << "\", this); // port\n";
	o__ "{\n";
	o____ "std::string tmp = \"" << ext_name << "\";\n";
	o____ "set_port_by_index(" << ii << ", tmp);\n";
	o__ "}\n";
      }else{
	o__ "_n[" << ii << "].new_node(\"" << ext_name << "\", this);\n";
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
  o << "MOD_" << m.identifier() << "::MOD_" << m.identifier() << "() :" << baseclass(m) << "()";

//  o << ",\n   // input parameters";
//  make_construct_parameter_list(o, m.parameters());

//  o << ",\n   // calculated parameters";
//  make_construct_parameter_list(o, m.device().calculated());

//  o << ",\n   // netlist";
//  for (Element_1_List::const_iterator
//       p = m.elements().begin();
//       p != m..elements().end();
//       ++p) {
//    o << ",\n   _" << (**p).name() << "(0)";
//  }
//
  

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
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_read_probes(std::ostream& o, const Module& m)
{
  o << "void MOD_" << m.identifier() << "::read_probes()\n{\n";
  for(auto x : m.branches()){
    Branch const* b = x.second;
    assert(b);
    if(b->is_filter()){
      o__ "// filter " <<  b->code_name() << "\n";
    }else if(b->has_pot_probe()){
      o__ "_potential" << b->code_name() << " = volts_limited(_n[n_"<< b->p()->name() <<"], _n[n_"<< b->n()->name() <<"]);\n";
      o__ "trace2(\"potential\", _potential" << b->code_name() << ", _sim->_time0);\n";
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
      if(b->has_pot_source() && b->has_flow_probe()){
	throw Exception("cannot use self as current probe.. yet\n");
      }else{
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
      }else if(d->is_filter_probe()){
	o__ "// trace2(\" filter " <<  b->state() << "\", " << b->state() << "["<<k<<"], "<<  d->code_name() <<");\n";
	//o__ b->state() << "[0] -= " << b->state() << "["<<k<<"] * _st" << d->branch()->code_name() << "[0];\n";
	o__ b->state() << "[0] -= " << b->state() << "["<<k<<"] * " << d->branch()->code_name() << "->tr_amps();\n";
	++k;
	break;
      }else if(d->is_flow_probe()){
	// nothing
      }else{ untested();
	o__ "// bogus probe " << b->state() << " : " << d->code_name() << "\n";
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
      }else if(d->is_pot_probe()){
	// nothing
      }else if(d->is_filter_probe()){
	// nothing
      }else{ untested();
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
      for(auto D : b->deps()){
	o__ "// DEP " << D->code_name() << "\n";
      }
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
 // if (p.short_if().empty()) {
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
//      "      if (" << p.short_if() << ") {\n"
//      "        _n[n_" << p.name() << "] = _n[n_" << p.short_to() << "];\n"
//      "      }else:
    o__  "{\n"
      "        _n[n_" << p.name() << "].new_model_node(\".\" + long_label() + \"." << p.name() 
			   << "\", this);\n"
      "      }\n"
      "    }else{\n";

      // "      if (" << p.short_if() << ") {\n"
      // "        assert(_n[n_" << p.name() << "] == _n[n_" << p.short_to() << "]);\n"
      // "      }else"
    o__  "{\n"
      "        //_n[n_" << p.name() << "].new_model_node(\"" << p.name() 
		 << ".\" + long_label(), this);\n"
      "      }\n"
      "    }\n";
  }
#endif
}
/*--------------------------------------------------------------------------*/
static void make_module_allocate_local_nodes(std::ostream& o, Module const& m)
{
  if(m.element_list().size()){
    // nodes come from sckt proto
  }else{
    for (auto nn : m.nodes()){
      assert(nn);
      // TODO: node aliases, shorts etc.
      if(nn->number() < int(m.ports().size())){
      }else{
	make_module_allocate_local_node(o, *nn);
      }
    }
  }
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

  std::string dev_type;
//  if(dynamic_cast<Branch const*>(&e)){
    dev_type = e.dev_type();
//  }else{
//    dev_type = "device_stub";
//  }

  o__ "if (!" << e.code_name() << ") {\n";
  o____ "const CARD* p = device_dispatcher[\"" << dev_type << "\"]; // " << e.dev_type() << "\n";
  o____ "if(!p){\n;";
  o______ "throw Exception(" << "\"Cannot find " << dev_type << ". Load module?\");\n";
  o____ "}else{\n";
  o____ "}\n";
//  if(dynamic_cast<Branch const*>(&e)){
    o____ e.code_name() << " = dynamic_cast<ELEMENT*>(p->clone());\n";
//  }else{
//    o____ e.code_name() << " = dynamic_cast<COMPONENT*>(p->clone());\n";
//    o____ e.code_name() << "->set_dev_type(\"" << e.dev_type() << "\");\n";
//  }
  o____ "if(!" << e.code_name() << "){\n";
  o______ "throw Exception(" << "\"Cannot use " << dev_type << ": wrong type\"" << ");\n";
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
	}else if(i->is_filter_probe()){
	  assert(i->branch());
	  o << ",gnd";
	  o << ",_n[n_" << i->branch()->p()->name() << "]";
	  break;
	}else if(i->is_pot_probe()){
	  assert(i->branch());
	  o << ",_n[n_" << i->branch()->p()->name() << "]";
	  o << ",_n[n_" << i->branch()->n()->name() << "]";
	  break;
	}else{
	  o << "/* nothing " << i->code_name() << " */";
	}
      }
    }
    o << "}; // nodes\n";

//    Port_1_List::const_iterator p = e.current_ports().begin();
    make_set_parameters(o, e);

    int kk = 0;
    // BUG iterate branches?

    // set_current ports.
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
  }else{
    unreachable();
//    Port_3_List_2::const_iterator p = e.ports().begin();
//    if (p != e.ports().end()) {
//      assert(*p);
//      o << "_n[n_" << (**p).name() << "]";
//      while (++p != e.ports().end()) {
//	o << ", _n[n_" << (**p).name() << "]";
//      }
//    }else{
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
static void make_node_ref(std::ostream& o, const Node& n)
{
  if(&n == &mg_ground_node){
    o << "gnd";
  }else{
    o << "_n[" << n.code_name() << "]";
  }
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
    o__ "}else";
  }else{
    o__ "";
  }
  o << "{ // filter expand\n";

  // BUG: duplicate
  o__ "if (!" << e.branch_code_name() << ") {\n";
  o____ "const CARD* p = device_dispatcher[\"" << e.dev_type() << "\"];\n";
  o____ "if(!p){\n;";
  o______ "throw Exception(" << "\"Cannot find " << e.dev_type() << ". Load module?\");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ e.branch_code_name() << " = dynamic_cast<ELEMENT*>(p->clone());\n";
  o____ "if(!" << e.branch_code_name() << "){\n";
  o______ "throw Exception(" << "\"Cannot use " << e.dev_type() << ": wrong type\"" << ");\n";
  o____ "}else{\n";
  o____ "}\n";
  o____ "subckt()->push_front(" << e.branch_code_name() << ");\n";
  o__ "}else{\n";
  o__ "}\n";

#if 1
  o____ "{\n";
  
  o______ "node_t nodes[] = {";
  
  if(auto br = dynamic_cast<Filter const*>(&e)) {
    if(1) {
//      o << "gnd, _n[n_" << br->name() << "]";
      o << "_n[n_" << br->name() << "], gnd";
    }else{
      o << "gnd, gnd";
    }
    // o << "_n[0]"; // eek
    for(auto i : br->deps()){
      o << ", ";
      make_node_ref(o, *i->branch()->p());
      o << ", ";
      make_node_ref(o, *i->branch()->n());
    //   o << ", _n[" << i->branch()->p()->code_name() << "]";
    //   o << ", _n[" << i->branch()->n()->code_name() << "]";
    }
  }else{
  }
  
  o << "};\n";
  make_set_parameters(o, e);
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

  o__ "if(subckt()){\n";
  if(m.element_list().size()){
    o__ "subckt()->attach_params(&(c->_netlist_params), scope());\n";
  }else{
  }
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

  if(m.element_list().size()){
    o__ "assert(subckt());\n";
    o__ "subckt()->attach_params(&(c->_netlist_params), scope());\n";
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
  o__ "if (!subckt()) {\n"
    "    new_subckt();\n"
    "  }else{\n"
    "  }\n"
    "\n";
     o << " node_t gnd;\n";
   o << "gnd.set_to_ground(this);\n";
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
    o<<
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
  make_module_allocate_local_nodes(o, m);
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
  make_module_class(o, m);
    o << "int MOD_" << m.identifier() << "::_count = -1;\n";
  make_module_dispatcher(o, m);
  make_module_clone(o, m);
//  make_module_evals(o, m);
//  make_module_default_constructor(o, m);
  make_module_copy_constructor(o, m);
  make_module_precalc_first(o, m);
  make_module_expand(o, m);
  make_module_precalc_last(o, m);
  make_cc_filter(o, m);
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
