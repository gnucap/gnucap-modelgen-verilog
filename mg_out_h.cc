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
/*--------------------------------------------------------------------------*/
static void declare_deriv_enum(std::ostream& o, const Module& m)
{
  std::string comma = "";

  o << ind << "enum {\n";
  // for (auto nn : m.probes()){
  //   o << comma << "d_" << nn.second->name();
  // }
  for(auto x : m.branches()){
    assert(x);
    Branch const* b = x;
    if(b->is_short()){ untested();
      // !has_element?
    }else{
      if(b->has_flow_probe()){
	o << "    d_flow" << b->code_name() << ",\n";
      }else{
      }

      if(b->is_filter()){
	o << "    d__filter" << b->code_name() << ",\n";
      }else if(b->has_pot_probe()){
	o << "    d_potential" << b->code_name() << ",\n";
      }else{
	//      o << comma "// no pot probe?   d_potential" << b->code_name();
	//      comma = ",\n";
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
  for(auto x : m.branches()){
    assert(x);
    Branch const* b = x;
    if(b->has_flow_probe()){
      ++np;
    }else{
    }
    if(b->has_pot_probe()){
      ++np;
    }else if(b->is_filter()){
      ++np;
    }else{
    }
  }
  o << ind << "typedef ddouble_<"<<np<<"> ddouble;\n";
  declare_deriv_enum(o, m);
}
/*--------------------------------------------------------------------------*/
static void make_analogfunction_decl(std::ostream& o, const Analog_Function& F)
{
  o__ "ddouble " << F.code_name() << "(";

  // BUG? make_cc_af_args
  std::string sep = "";
  std::string qual = "";
  for (auto coll : F.args()){
      if(coll->is_output()){
	qual = "/*output*/ &";
      }else{
	qual = "";
      }
    for(auto i : *coll){
      o << sep << "ddouble " << qual;
      o << "/*" << i->identifier() << "*/";
      sep = ", ";
    }
  }
  o << ") const;\n";
}
/*--------------------------------------------------------------------------*/
static void make_analogfunctions_decl(std::ostream& o, const Analog_Functions& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    make_analogfunction_decl(o, **q);
  }
}
/*--------------------------------------------------------------------------*/
static void make_func_dev(std::ostream& o, std::set<FUNCTION_ const*> const& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    if(auto t = dynamic_cast<MGVAMS_TASK const*>(*q)){
      o<<"//task " << (*q)->label() << "\n";
      t->make_cc_dev(o);
    }else{
      o<<"//func " << (*q)->label() << "\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_funcs_common(std::ostream& o, std::set<FUNCTION_ const*> const& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    if(auto ff = dynamic_cast<MGVAMS_FUNCTION const*>(*q)){
      ff->make_cc_common(o);
    }else if(auto tt = dynamic_cast<MGVAMS_TASK const*>(*q)){
      tt->make_cc_common(o);
    }else if(auto ff = dynamic_cast<MGVAMS_FILTER const*>(*q)){
      ff->make_cc_common(o);
    }else{
      unreachable();
      o<<"//func " << (*q)->label() << "\n";
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
  }
}
/*--------------------------------------------------------------------------*/
static void make_variable_decl(std::ostream& o, const Variable_List_Collection& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    if(((**q).type()).to_string().substr(0,4) == "real"){
      o__ "ddouble";
    }else{
      o__ (**q).type();
    }
    std::string comma = " ";
    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      o << comma << "_v_" << (**p).name(); // code_name??
//	  << " /* " << (**p).comment() << " */";
      comma = ", ";
    }
    o << ";\n";
  }
}
/*--------------------------------------------------------------------------*/
//static void make_parameter_decl(std::ostream& o, const Localparam_List_Collection& P)
//{
//  for (auto q = P.begin(); q != P.end(); ++q) {
//    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
//      o__ (**p).type() << " " << (**p).code_name()
//	  << "{0.};\t// " << (**p).comment() << '\n';
//    }
//  }
//}
/*--------------------------------------------------------------------------*/
static void make_common(std::ostream& o, const Module& m)
{
  std::string class_name = "COMMON_" + m.identifier().to_string();
  std::string base_class_name;
  base_class_name = "COMMON_COMPONENT";
  // if(m.has_submodule()){
  //   base_class_name = "COMMON_PARAMLIST";
  // }else{
  //   base_class_name = "COMMON_COMPONENT";
  // }
  o << "class MOD_" << m.identifier() << ";\n";
  o << "class " << class_name << " :public " << base_class_name << "{\n";
  o__ "typedef MOD_" << m.identifier() << " MOD;\n";
  if(m.element_list().size()){
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
    "  COMMON_COMPONENT* clone()const override{return new "<<class_name<<"(*this);}\n"
    "  void     set_param_by_index(int, std::string&, int)override;\n"
    "  bool     is_valid()const;\n"
    "  bool     param_is_printable(int)const override;\n"
    "  std::string param_name(int)const override;\n"
    "  std::string param_name(int,int)const override;\n"
    "  std::string param_value(int)const override;\n"
    "  int param_count()const override {return (" 
	     << m.parameters().size()
	     << " + " << base_class_name << "::param_count());}\n"
    "  void precalc_first(const CARD_LIST*)override;\n"
    "  void expand(const COMPONENT*)override;\n"
    "  void precalc_last(const CARD_LIST*)override;\n"
    "  void tr_eval_analog(MOD_" << m.identifier() << "*)const;\n"
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
//       ++p) {
//    out << "  " << (**p).type() << " " << (**p).code_name()
//	<< ";\t// " << (**p).comment() << '\n';
//  }
//  out << "public: // attached commons\n";
//  for (Args_List::const_iterator
//       p = d.circuit().args_list().begin();
//       p != d.circuit().args_list().end();
//       ++p) {
//    out << "  COMMON_COMPONENT* _" << (**p).name() << ";\n";
//  }
  o << "private: // analog functions\n";
  make_analogfunctions_decl(o, m.analog_functions());
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
  // if(auto f = dynamic_cast<Filter const*>(&elt)){
  //   unreachable();
  //   // make_module_one_filter_state(o, *f);
  // }else
  if((bb = dynamic_cast<Branch const*>(&elt))){
  }else{
    unreachable();
    return;
  }
  Branch const& br = *bb;
  o << "public: // states, " << br.code_name() << ";\n";
  if(br.has_pot_source()){
    o__ "bool _pot" << br.code_name() << ";\n";
    for(auto n : br.names()){
      o__ "bool _pot_br_" << n << ";\n";
    }
  }else{
  }
  o__ "double _value" << br.code_name() << ";\n";
  o__ "double _st" << br.code_name();
  size_t k = br.num_states();
  o__ "[" << k << "];\n";

  for(auto n : br.names()){
    o__ "double _value_br_" << n << ";\n";
    o__ "double _st_br_" << n;
    o__ "[" << k << "];\n";
  }

  o__ "struct _st" << br.code_name() << "_ {\n";
  o____ "enum { ";
  std::string comma = "";
  o____ "VALUE, SELF";
  std::vector<char> seen(br.num_branches());
  for(auto d : br.deps()){
    Branch const* bb = d->branch();
    assert(bb);
    if(bb->is_short()){ untested();
    }else if(seen[bb->number()]){
    }else if(bb == &br){
    }else if(bb->has_flow_probe()){
    }else{
      seen[bb->number()] = 1;
      assert(d);
      o << ", dep" << d->code_name();
    }
  }
  o << "/* | */\n";
  seen.clear();
  seen.resize(br.num_branches());
  for(auto d : br.deps()){
    Branch const* bb = d->branch();
    assert(bb);
    if(bb->is_short()){ untested();
    }else if(seen[bb->number()]){
    }else if(bb == &br){
    }else if(!bb->has_flow_probe()){
    }else{
      seen[bb->number()] = 1;
      assert(d);
      o << ", dep" << d->code_name();
    }
  }
  o____ "};\n";
  o__ "} _dep" << br.code_name() << ";\n";

}
/*--------------------------------------------------------------------------*/
static void make_branch_states(std::ostream& o, const Module& m)
{
  for(auto x : m.branches()){
    assert(x);
    if(x->has_element()){
      make_module_one_branch_state(o, *x);
    }else if(x->is_filter()){
      make_module_one_branch_state(o, *x);
    }else{
      o__ "// branch no elt: " << x->code_name() << "\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_module(std::ostream& o, const Module& m)
{
  std::string class_name = "MOD_" + m.identifier().to_string();
  std::string base_name = baseclass(m);
  std::string common_name = "COMMON_" + m.identifier().to_string();
  o << "class " << class_name << " : public " << base_name << " {\n";
  o << "private:\n";
  o__ "static int _count;\n";
  o << "public:\n";
  declare_ddouble(o, m);
  o << "private: // data\n";
  size_t total_nodes = m.nodes().size() + 10;
  o << ind << "node_t _nodes[" << total_nodes << "];\n";
  o << "public: // netlist\n";
  for (Element_2_List::const_iterator
       p = m.element_list().begin();
       p != m.element_list().end();
       ++p) {
    o__ "// COMPONENT* " << (**p).code_name() << "{NULL};\n";
  }
  if(m.element_list().size()){
    o__ "COMPONENT const* _parent{NULL};\n";
  }else{
  }
  for (auto br : m.branches()){
    if(br->is_short()){ itested();
      o__ "// short : " << br->code_name() << "\n";
    }else{
      o__ "ELEMENT* " << br->code_name() << "{NULL}; // branch\n";
    }
  }
  o << "private: // func decl\n";
  make_func_dev(o, m.funcs());
  if(m.num_evt_slots()){
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
  if(m.element_list().size()){
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
  o__ "//void    map_nodes();         //BASE_SUBCKT\n";
  o__ "//void    tr_begin();          //BASE_SUBCKT\n";
  o__ "//void    tr_restore();        //BASE_SUBCKT\n";
  o__ "void    tr_load()override{ trace1(\"tr_load\", long_label());BASE_SUBCKT::tr_load();}\n";
  if(m.num_evt_slots()){
    o__ "TIME_PAIR  tr_review()override;         //BASE_SUBCKT\n";
    o__ "void    tr_accept()override;         //BASE_SUBCKT\n";
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
    o__ "void      tr_advance()override {set_not_converged(); BASE_SUBCKT::tr_advance();}\n";
    o__ "void      tr_regress()override {set_not_converged(); BASE_SUBCKT::tr_regress();}\n";
    o__ "bool      tr_needs_eval()const override;\n";
    o__ "void      tr_queue_eval()override {if(tr_needs_eval()){q_eval();}else{} }\n";
    o__ "bool      do_tr() override;\n";
  }
  o__ "double tr_probe_num(std::string const&)const override;\n";
  o__ "  //void    ac_begin();          //BASE_SUBCKT\n";
  o__ "  //void    do_ac();             //BASE_SUBCKT\n";
  o__ "  //void    ac_load();           //BASE_SUBCKT\n";
  o__ "  //XPROBE  ac_probe_ext(CS&)const;//CKT_BASE/nothing\n";
//  o << ind << "std::string dev_type()const override {return \"demo\";}\n";
  o__ "int max_nodes()const override {return "<< m.ports().size() <<";}\n";
  o__ "int min_nodes()const override {return "<< m.ports().size() <<";}\n";
  o__ "int int_nodes()const override    {return "
      << m.nodes().size() - m.ports().size() << ";}\n";
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
  std::string comma = "";
  o << ind << "enum {";
  for (auto nn : m.nodes()){
    // TODO: node aliases, shorts etc.
    if(nn->number() == int(m.ports().size())){
      o << " /* | */";
    }else{
    }
    o << comma << "n_" << nn->name();
    comma = ", ";
  }
  o << ind << "};\n";
  o << "private: // probe values\n";
  for(auto x : m.branches()){
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

  o << "}; // m_" << m.identifier() << ";\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_decl(std::ostream& out, const Module& d)
{
	make_common(out, d);
	make_module(out, d);
}
/*--------------------------------------------------------------------------*/
#if 0
static void make_device(std::ostream& out, const Device& d)
{
  std::string class_name = "DEV_" + d.name().to_string();
  out <<
    "class " << class_name << " : public BASE_SUBCKT {\n"
    "private:\n"
    "  explicit " << class_name << "(const " << class_name << "& p);\n"
    "public:\n"
    "  explicit " << class_name << "();\n"
    "           ~" << class_name << "() {--_count;}\n"
    "private: // override virtual\n"
    "  char      id_letter()const     {untested();return '" << d.id_letter() << "';}\n"
    "  bool      print_type_in_spice()const {return true;}\n"
    "  std::string value_name()const  {return \"area\";}\n"
    "  //std::string dev_type()const;   //BASE_SUBCKT\n"
    "  int       max_nodes()const     {return " << d.max_nodes() << ";}\n"
    "  int       min_nodes()const     {return " << d.min_nodes() << ";}\n";
  if (d.max_nodes() != d.min_nodes()) {
    out <<
      "  //int     matrix_nodes()const; //BASE_SUBCKT\n"
      "  //int     net_nodes()const;    //BASE_SUBCKT\n";
  }else{
    out <<
      "  //int     matrix_nodes()const; //BASE_SUBCKT\n"
      "  int       net_nodes()const     {return " << d.max_nodes() << ";}\n";
  }
  out << 
    "  CARD*     clone()const override        {return new "
      << class_name << "(*this);}\n"
    "  void      precalc_first()override {COMPONENT::precalc_first(); if(subckt()) subckt()->precalc_first();}\n"
    "  void      expand()override;\n"
    "  void      precalc_last() override {COMPONENT::precalc_last(); assert(subckt()); subckt()->precalc_last();}\n"
    "  //void    map_nodes();         //BASE_SUBCKT\n"
    "  //void    tr_begin();          //BASE_SUBCKT\n"
    "  //void    tr_restore();        //BASE_SUBCKT\n";
  out <<
    "public:\n"
    "  static int  count() {return _count;}\n"
    "public: // may be used by models\n";
  for (Function_List::const_iterator
       p = d.function_list().begin();
       p != d.function_list().end();
       ++p) {
    out << "  void " << (**p).name() << ";\n";
  }
  out << 
    "private: // not available even to models\n"
    "  static int _count;\n";
  out <<  "public: // input parameters\n";
  for (Parameter_1_List::const_iterator
       p = d.device().raw().begin();
       p != d.device().raw().end();
       ++p) {untested();
    untested();
    out << "  PARAMETER<" << (**p).type() << "> " << (**p).code_name()
	<< ";\t// " << (**p).comment() << '\n';
  }
  out << "public: // calculated parameters\n";
  for (Parameter_1_List::const_iterator
       p = d.device().calculated().begin();
       p != d.device().calculated().end();
       ++p) {
    out << "  " << (**p).type() << " " << (**p).code_name()
	<< ";\t// " << (**p).comment() << '\n';
  }
  out << "private: // node list\n"
    "  enum {";
  for (Port_1_List::const_iterator
       p = d.circuit().req_nodes().begin();
       p != d.circuit().req_nodes().end();
       ++p) {
    if (p != d.circuit().req_nodes().begin()) {
      out << ", ";
    }else{
    }
    out << "n_" << (**p).name();
  }
  for (Port_1_List::const_iterator
       p = d.circuit().opt_nodes().begin();
       p != d.circuit().opt_nodes().end();
       ++p) {
    out << ", ";
    out << "n_" << (**p).name();
  }
  for (Port_1_List::const_iterator
       p = d.circuit().local_nodes().begin();
       p != d.circuit().local_nodes().end();
       ++p) {
    out << ", n_" << (**p).name();
  }
  size_t total_nodes = d.circuit().req_nodes().size() + d.circuit().opt_nodes().size()
    + d.circuit().local_nodes().size();
  out << "};\n"
    "  node_t _nodes[" << total_nodes << "];\n"
    "  std::string port_name(int i)const {\n"
    "    assert(i >= 0);\n"
    "    assert(i < " << d.circuit().req_nodes().size() + d.circuit().opt_nodes().size() << ");\n"
    "    static std::string names[] = {";
  for (Port_1_List::const_iterator
	 p = d.circuit().req_nodes().begin();
       p != d.circuit().req_nodes().end();
       ++p) {
    out << '"' << (**p).name() << "\", ";
  }
  for (Port_1_List::const_iterator
       p = d.circuit().opt_nodes().begin();
       p != d.circuit().opt_nodes().end();
       ++p) {
    out << '"' << (**p).name() << "\", ";
  }
  out << "\"\"};\n"
    "    return names[i];\n"
    "  }\n"
    "};\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_eval(std::ostream& out, const Eval& e,
		      const String_Arg& dev_name)
{
  incomplete();
  assert(0);
  std::string class_name = "EVAL_" + dev_name.to_string() + '_' 
    + e.name().to_string();
  out <<
    "class " << class_name << " : public COMMON_COMPONENT {\n"
    "private:\n"
    "  explicit "<< class_name << "(const "<< class_name << "& p)\n"
    "    :COMMON_COMPONENT(p) {}\n"
    "public:\n"
    "  explicit "<< class_name << "(int c=0) :COMMON_COMPONENT(c) {}\n"
    "  bool operator==(const COMMON_COMPONENT& x)const "
		"{return COMMON_COMPONENT::operator==(x);}\n"
    "  COMMON_COMPONENT* clone()const {return new "<<class_name<<"(*this);}\n"
    "  std::string name()const {untested(); return \""<< class_name << "\";}\n"
    "  void tr_eval(ELEMENT*d)const;\n"
    "  bool has_tr_eval()const {return true;}\n"
    "  bool has_ac_eval()const {return false;}\n"
    "};\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
