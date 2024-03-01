/*$Id: mg_out_h.cc,v 26.134 2009/11/29 03:44:57 al Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023 Felix Salfelder
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
#include "mg_out.h"
#include "mg_func.h"
#include "mg_options.h"
#include "mg_analog.h" // BUG. Probe
#include "m_tokens.h" // Deps
/*--------------------------------------------------------------------------*/
static void declare_deriv_enum(std::ostream& o, const Module& m)
{
  std::string comma = "";

  o << ind << "enum {\n";
  // for (auto nn : m.probes()){ untested();
  //   o << comma << "d_" << nn.second->name();
  // }
  assert(m.circuit());
  for(auto x : m.circuit()->branches()){
    assert(x);
    Branch const* b = x;
    if(b->is_short()){
      // !has_element?
    }else{
      if(b->has_flow_probe()){
	o << "    d_flow" << b->code_name() << ",\n";
      }else{
      }

      if(b->has_name()){
	o << " // named:   d_potential" << b->code_name() << ",\n";
      }else if(b->has_pot_probe()){
	o << "    d_potential" << b->code_name() << ",\n";
      }else{
      }
    }
  }
  for(auto x : m.circuit()->branches()){
    assert(x);
    Branch const* b = x;
    if(b->is_short()){
      // !has_element?
	o << "// short " << b->code_name() << "\n";
    }else{
      if(!b->has_name()){
	o << "// not named: " << b->code_name() << "\n";
      }else if(b->has_pot_probe()){
	auto nb = prechecked_cast<Named_Branch const*>(b);
	assert(nb);
	o << "    d_potential" << nb->code_name()
	  << " = d_potential" << nb->base()->code_name() << ",\n";
      }else{
      }
    }
  }
  o << "     num_branches__";
  o__ "};\n";
}
/*--------------------------------------------------------------------------*/
static void declare_ddouble(std::ostream& o, Module const& m)
{
  size_t np = 0;
  for(auto x : m.circuit()->branches()){
    assert(x);
    Branch const* b = x;
    if(b->has_flow_probe()){
      ++np;
    }else{
    }
    if(b->has_name()){
    }else if(b->has_pot_probe()){
      // BUG
      ++np;
    }else{
    }
  }
  o << ind << "typedef ddouble_<"<<np<<"> ddouble;\n";
  declare_deriv_enum(o, m);
}
/*--------------------------------------------------------------------------*/
static void make_func_dev(std::ostream& o, pSet<FUNCTION_ const> const& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    if(dynamic_cast<MGVAMS_TASK const*>(*q)){
      o<<"//task " << (*q)->label() << "\n";
    }else if(dynamic_cast<MGVAMS_FUNCTION const*>(*q)) {
      o<<"//func " << (*q)->label() << "\n";
    }else if(dynamic_cast<Probe const*>(*q)) {
      o<<"//probe " << (*q)->label() << "\n";
    }else if(dynamic_cast<MGVAMS_FILTER const*>(*q)) {
      o<<"//filt " << (*q)->label() << "\n";
    }else{
      o<<"//other: " << (*q)->label() << "\n";
    }
    (*q)->make_cc_dev(o);
  }
}
/*--------------------------------------------------------------------------*/
static void make_funcs_common(std::ostream& o, pSet<FUNCTION_ const> const& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    if(!*q){ untested();
      o<<"// NULL\n";
    }else if( (*q)->has_refs() ){
      (*q)->make_cc_common(o);
    }else if(dynamic_cast<MGVAMS_FUNCTION const*>(*q)){ untested();
      o<<"// FUNCTION no refs: " << (*q)->label() << "\n";
    }else if(dynamic_cast<MGVAMS_TASK const*>(*q)){ untested();
      o<<"// TASK no refs: " << (*q)->label() << "\n";
    }else if(dynamic_cast<MGVAMS_FILTER const*>(*q)){ untested();
      o<<"// FILTER no refs: " << (*q)->label() << "\n";
    }else if(dynamic_cast<VAMS_ACCESS const*>(*q)){ untested();
      o<<"// XS no refs: " << (*q)->label() << "\n";
    }else if(dynamic_cast<Probe const*>(*q)){
      o<<"// Probe no refs: " << (*q)->label() << "\n";
    }else{ untested();
      unreachable();
      o<<"// func no refs " << (*q)->label() << "\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_parameter_decl(std::ostream& o, const Parameter_List_Collection& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    if((*q)->is_local()){
      o__ (**q).type();
    }else{
      o__ "PARAMETER<" << (**q).type() << ">";
    }
    std::string comma = " ";
    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      o << comma << (**p).code_name()
	  << " /* " << (**p).comment() << " */";
      comma = ", ";
    }
    o << ";\n";

    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      if( (*p)->aliases().size() ) {
	o__  "int _s" << (**p).code_name() << "{0};\n";
      }else{
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
// lib?
void make_one_variable_decl(std::ostream& o, const Variable_Decl& V)
{
  if(V.type().is_real()) {
      o__ "double ";
      o << " _v_" << V.name() << "{0.}";
   if(!V.has_attributes()){
#if 0
   }else if(options().optimize_deriv()) { untested();
      o__ "struct _V_" << V.name() << " : ddouble {\n";
      o____ "typedef ddouble base;\n";
      o____ "typedef va::ddouble_tag base_tag;\n";
      o____ "_V_" << V.name() << "(ddouble x) : ddouble(x){}\n";
      o____ "template<class A>\n";
      o____ "explicit _V_" << V.name() << "(A x) : ddouble(x){}\n";
      o____ "_V_" << V.name() << "(){ zeroderiv(); }\n";
      o____ "void zeroderiv(){\n";
      for(auto d : V.deps()){itested();
	o______ "set_no_deps();\n";
	o______ "_data[1+d" << d->code_name() << "] = 0.; // " << d.order() << "\n";
      }
      o____ "}\n";
      o____ "ddouble& operator=(ddouble t){\n";
      o______ "ddouble::operator=(t);\n";
      o______ "return *this;\n";
      o____ "}\n";
      o____ "ddouble& operator=(double t){\n";
      o______ "ddouble::operator=(t);\n";
      o______ "return *this;\n";
      o____ "}\n";
      o__ "}";
#endif
    }else{
//      o__ "ddouble ";
    }
  }else if(V.type().is_int()) {
    o__ "int";
    o << " _v_" << V.name() << "{0}";
  }else{ untested();
    incomplete();
    o__ "unknown";
  }
  o << ";\n";
}
/*--------------------------------------------------------------------------*/
static void make_variable_decl(std::ostream& o, const Variable_List_Collection& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      Variable_Decl const* V = *p;
      assert(V);
      //if(V->has_attributes()) { untested();
	make_one_variable_decl(o, *V);
      //}else{ untested();
//	tr_eval_analog local
      //}
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_common(std::ostream& o, const Module& m)
{
  assert(m.circuit());
  std::string class_name = "COMMON_" + m.identifier().to_string();
  std::string base_class_name;
  base_class_name = "COMMON_COMPONENT";
  // if(m.has_submodule()){ untested();
  //   base_class_name = "COMMON_PARAMLIST";
  // }else{ untested();
  //   base_class_name = "COMMON_COMPONENT";
  // }
  o << "class MOD_" << m.identifier() << ";\n";
  o << "class " << class_name << " :public " << base_class_name << "{\n";
  o__ "typedef MOD_" << m.identifier() << " MOD;\n";
  if(m.circuit()->element_list().size()){
  o << "public:\n";
    o__ "PARAM_LIST _netlist_params;\n";
  }else{
  }
  declare_ddouble(o, m);
  o << "public:\n";
  o__ "explicit " << class_name << "(const " << class_name << "& p);\n"
    "  explicit " << class_name << "(int c=0);\n"
    "           ~" << class_name << "();\n"
    "  bool     operator==(const COMMON_COMPONENT&)const override;\n"
    "  COMMON_COMPONENT* clone()const override {return new "<<class_name<<"(*this);}\n"
    "  void     set_param_by_index(int, std::string&, int)override;\n"
    "  aidx     set_param_by_name(std::string, std::string)override;\n"
    "  bool     is_valid()const;\n"
    "  bool     param_is_printable(int)const override;\n"
    "  std::string param_name(int)const override;\n"
    "  std::string param_name(int,int)const override;\n"
    "  std::string param_value(int)const override;\n"
    "  int param_count()const override {return ("
	     << m.parameters().count_nonlocal()
	     << " + " << base_class_name << "::param_count());}\n"
    "  void precalc_first(const CARD_LIST*)override;\n"
    "  void expand(const COMPONENT*)override;\n"
    "  void precalc_last(const CARD_LIST*)override;\n"
    // if has_analog?
    "  void tr_eval_analog(MOD_" << m.identifier() << "*)const;\n"
    "  void precalc_analog(MOD_" << m.identifier() << "*)const;\n"
    "  std::string name()const override {itested();return \"" << m.identifier() << "\";}\n"
//    "  const SDP_CARD* sdp()const {return _sdp;}\n"
//    "  bool     has_sdp()const {untested();return _sdp;}\n"
    "  static int  count() {return _count;}\n"
    "private: // strictly internal\n"
    "  static int _count;\n"
    "public: // input parameters\n";
  make_parameter_decl(o, m.parameters());
//  out <<
//    "public: // calculated parameters\n"
//    "  SDP_CARD* _sdp;\n";
//  for (Parameter_1_List::const_iterator
//       p = d.common().calculated().begin();
//       p != d.common().calculated().end();
//       ++p) { untested();
//    out << "  " << (**p).type() << " " << (**p).code_name()
//	<< ";\t// " << (**p).comment() << '\n';
//  }
//  out << "public: // attached commons\n";
//  for (Args_List::const_iterator
//       p = d.circuit().args_list().begin();
//       p != d.circuit().args_list().end();
//       ++p) { untested();
//    out << "  COMMON_COMPONENT* _" << (**p).name() << ";\n";
//  }
  o << "private: // funcs\n";
  make_funcs_common(o, m.funcs());

  o << "}; //" << class_name << "\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
} // make_common
/*--------------------------------------------------------------------------*/
static void make_module_one_branch_state(std::ostream& o, Element_2 const& elt)
{
  Branch const* bb;
  if((bb = dynamic_cast<Branch const*>(&elt))){
  }else{ untested();
    o__ "// not a branch...\n";
    return;
  }
  Branch const& br = *bb;
  trace2("states", br.code_name(), br.deps().ddeps().size());
  o << "public: // states, " << br.code_name() << ";\n"; //  << br.deps().size()<<";\n";
  if(br.has_pot_source()){
    o__ "bool _pot" << br.code_name() << ";\n";
//    for(auto n : br.names()){ untested();
//      o__ "bool _pot_br_" << n << ";\n";
//    }
  }else{
  }
  o__ "double _value" << br.code_name() << ";\n";
  o__ "double _st" << br.code_name();
  size_t k = br.num_states();
  o__ "[" << k << "]; // (s)\n";

//  for(auto n : br.names()){ untested();
//    o__ "double _value_br_" << n << ";\n";
//    o__ "double _st_br_" << n;
//    o__ "[" << k << "];\n";
//  }

  o__ "struct _st" << br.code_name() << "_ {\n";
  o____ "enum { ";
  std::string comma = "";
  o____ "VALUE, SELF";
  for(auto d : br.ddeps()){
//      o << "/* found " << d->code_name() << "*/";
    Branch const* bbb = d->branch();
    assert(bbb);
    if(bbb->is_short()){ untested();
    }else if(bbb == &br){
    }else if(bbb->has_flow_probe()){
    }else{
      assert(d);
      o << ", dep" << d->code_name();
    }
  }
  o << "/* : */\n";
  for(auto d : br.ddeps()){
    Branch const* bbb = d->branch();
    assert(bbb);
    if(bbb->is_short()){ untested();
    }else if(bbb == &br){
    }else if(!bbb->has_flow_probe()){
    }else{
      assert(d);
      o << ", dep" << d->code_name();
    }
  }
  o____ "};\n";
  o__ "} _dep" << br.code_name() << ";\n";

}
/*--------------------------------------------------------------------------*/
static void make_node_decl(std::ostream& o, const Module& m)
{
  std::string comma = "";
  o__ "enum {\n";
  int n = 1;
  for (; n <= int(m.circuit()->nodes().size()); ++n) {
    Node const* nn = m.circuit()->nodes()[n];
    // TODO: node aliases, shorts etc.
    if(nn->number() == int(1+m.circuit()->ports().size())){
      o << "\n    /* ---- */";
    }else{
    }
    o << comma << "    n_" << nn->name() << " /* used: " << nn->is_used() << " */";
    comma = ",\n";
  }
  o << "\n";
  o__ "};\n";
}
/*--------------------------------------------------------------------------*/
static void make_branch_states(std::ostream& o, const Module& m)
{
  for(auto x : m.circuit()->branches()){
    assert(x);
    if(x->has_element()){
      make_module_one_branch_state(o, *x);
    }else if (x->is_filter()) {
      make_module_one_branch_state(o, *x);
    }else{
      o__ "// branch no elt: " << x->code_name() << "\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_precalc(std::ostream& o, Module const& m)
{
  std::string class_name = "PRECALC_" + m.identifier().to_string();
  o << "class " << class_name << "{\n";
  o__ "MOD_" <<  m.identifier() << "* _d{NULL};\n";
  o << "public:\n";
  declare_ddouble(o, m);
  o << "public:\n";
  o__ "explicit " << class_name << "(MOD_" <<  m.identifier() << "*d) : _d(d) {}\n";
  // ... functions->make_cc_precalc?
  for(auto const& i : m.funcs()) {
    // indent x;
    i->make_cc_precalc(o);
  }

  o << "};\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_elements(std::ostream& o, Element_2_List const& L)
{
  o__ "COMPONENT const* _parent{NULL};\n";
  for (Element_2_List::const_iterator e = L.begin(); e != L.end();
      ++e) {
    {
      assert(*e);
      o__ "COMPONENT const* " << (*e)->code_name() << "{NULL};\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_module(std::ostream& o, const Module& m)
{
  std::string class_name = "MOD_" + m.identifier().to_string();
  std::string base_name = baseclass(m);
  std::string common_name = "COMMON_" + m.identifier().to_string();
  std::string precalc_name = "PRECALC_" + m.identifier().to_string();
  o << "class " << class_name << " : public " << base_name << " {\n";
  o << "private:\n";
  o__ "static int _count;\n";
  o << "public:\n";
  declare_ddouble(o, m);
  o << "private: // data\n";
  size_t total_nodes = m.circuit()->nodes().size();
  o__ "node_t _nodes[" << total_nodes << "];\n";
  o << "public: // netlist\n";
  if(m.circuit()->element_list().size()){
    make_cc_elements(o, m.circuit()->element_list());
  }else{
  }
  for (auto br : m.circuit()->branches()){
    if(br->has_element()){
      o__ "ELEMENT* " << br->code_name() << "{NULL}; // branch\n";
    }else if(br->is_short()){
      o__ "// short : " << br->code_name() << "\n";
    }else if(br->is_filter()){
      o__ "ELEMENT* " << br->code_name() << "{NULL}; // filter\n";
    }else{
      o__ "// ELEMENT* " << br->code_name() << "{NULL}; // no element (BUG)?\n";
    }
  }
  o << "private: // func decl\n";
  make_func_dev(o, m.funcs());
  if(m.has_events()){
    o << "private: // evt, tasks\n";
    o__ "unsigned _evt_seek{0};\n";
  //  o__ "void("<<class_name<<"::*_evt[" << m.num_evt_slots() << "])();\n";
    o__ "va::EVT const* _evt[" << m.num_evt_slots() << "];\n";
    o__ "void q_evt(va::EVT const* c){\n";
  //  o__ "void q_evt(void ("<< class_name <<"::*c)()) {\n";
    o____ "assert(_evt_seek<" << m.num_evt_slots() << ");\n";
    o____ "_evt[_evt_seek++] = c;\n";
    o__ "}\n";
  }else{
  }
  o << "private: // construct\n";
  o__ "explicit MOD_" << m.identifier() << "(MOD_" << m.identifier() << " const&);\n";
  o << "public:\n";
  o__ "explicit MOD_" << m.identifier() << "(); // : "<< base_name <<"() { _n = _nodes; }\n";
  o__ "CARD* clone()const override;\n";
  o << "private: // overrides\n";
  if(m.circuit()->element_list().size()){
    o__ "bool is_device() const override{return _parent;}\n";
    o__ "CARD_LIST* scope() override;\n";
    o__ "const CARD_LIST* scope()const override " <<
	"{ return const_cast<MOD_" << m.identifier() << "*>(this)->scope();}\n";
  }else{
  }
/*--------------------------------------------------------------------------*/
  o__ "bool is_valid()const override;\n";
  o__ "void precalc_first()override;\n";
  o__ "void expand()override;\n";
  o__ "void precalc_last()override;\n";
  o__ "void zero_filter_readout();\n";
  o__ "//void    map_nodes();         //BASE_SUBCKT\n";
  o__ "//void    tr_begin();          //BASE_SUBCKT\n";
  o__ "//void    tr_restore();        //BASE_SUBCKT\n";
  o__ "void    tr_load()override{ trace1(\"tr_load\", long_label());BASE_SUBCKT::tr_load();}\n";
  if(m.num_evt_slots() || m.has_analysis() ){
    o__ "TIME_PAIR  tr_review()override;\n";
    o__ "void    tr_accept()override;\n";
  }else if(m.has_tr_review()){
    o__ "TIME_PAIR  tr_review()override;\n";
  }else{
  }
  if(m.has_analysis()){
    // o__ "void    tr_advance()override;         //BASE_SUBCKT\n";
  }else{
    o__ "//TIME_PAIR  tr_review()override;         //BASE_SUBCKT\n";
    o__ "//void    tr_accept()override;         //BASE_SUBCKT\n";
  }
  o__ "//void    tr_unload();         //BASE_SUBCKT\n";
  if (!m.has_analog_block()) {
    o__ "//void    dc_advance();        //BASE_SUBCKT\n";
    o__ "//void    tr_advance();        //BASE_SUBCKT\n";
    o__ "//void    tr_regress();        //BASE_SUBCKT\n";
    o__ "//bool    tr_needs_eval()const;//BASE_SUBCKT\n";
    o__ "//void    tr_queue_eval();     //BASE_SUBCKT\n";
    o__ "//bool    do_tr();             //BASE_SUBCKT\n";
  }else{
    o__ "void      dc_advance()override {set_not_converged(); BASE_SUBCKT::dc_advance();}\n";
    o__ "void      tr_advance()override;\n";
    o__ "void      tr_regress()override {set_not_converged(); BASE_SUBCKT::tr_regress();}\n";
    o__ "bool      tr_needs_eval()const override;\n";
    o__ "void      tr_queue_eval()override {if(tr_needs_eval()){q_eval();}else{} }\n";
    o__ "bool      do_tr() override;\n";
//    o__ "void      ac_begin() override;\n";
//    o__ " void    do_ac();\n";
  }
  o__ "double tr_probe_num(std::string const&)const override;\n";
  o__ "  //void    ac_load();           //BASE_SUBCKT\n";
  o__ "  //XPROBE  ac_probe_ext(CS&)const;//CKT_BASE/nothing\n";
//  o << ind << "std::string dev_type()const override {return \"demo\";}\n";
  o__ "int max_nodes()const override {return "<< m.circuit()->ports().size() <<";}\n";
  o__ "int min_nodes()const override {return "<< m.circuit()->ports().size() <<";}\n";
  o__ "int int_nodes()const override    {return "
      << m.circuit()->nodes().size() - m.circuit()->ports().size() << ";}\n";
  o__ "std::string value_name()const override {itested(); return \"\";}\n";
  o__ "bool print_type_in_spice()const override {itested(); return false;}\n";
  o__ "std::string port_name(int i)const override;\n";
  o << "private: // impl\n";
  o << "/* ========== */\n";

  o << "public: // instance vars\n";
  make_variable_decl(o, m.variables());
  o << "private: // branch state\n";
  make_branch_states(o, m);
  o << "private: // node list\n";
  make_node_decl(o, m);
  o << "private: // probe values\n";
  for(auto x : m.circuit()->branches()){
    assert(x);
    Branch const* b = x;
    if(b->has_flow_probe()){
      o << ind << "double _flow" << b->code_name() << ";\n";
    }else{
    }
    if(b->has_pot_probe()){
      o << ind << "double _potential" << b->code_name() << ";\n";
    }else{
    }
  }

//  o << ind << "bool do_tr() override; // AnalogBlock\n";
  o << "private: // impl\n";
  o << ind << "void read_probes();\n";
  o << ind << "void set_branch_contributions();\n";
  o << ind << "void clear_branch_contributions();\n";
  o << ind << "friend class " << common_name << ";\n";
  o << ind << "friend class " << precalc_name << ";\n";

  o << "}; // m_" << m.identifier() << ";\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_decl(std::ostream& out, const Module& d)
{
  make_common(out, d);
  make_precalc(out, d);
  make_module(out, d);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
