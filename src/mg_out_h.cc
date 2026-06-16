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
#include "mg_token.h" // Deps
#include "mg_href.h" // Deps
#include "mg_assign.h"
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
    if(b->is_filter()){
      assert(!b->has_flow_probe());
      assert(b->has_pot_probe());
      if(b->is_short()){
      }else{
      }
      Base const* f = b->potential_dep(); // BUG
      Dep F(f, nullptr);
      o << "    d" << code_name(F) << ",\n";
    }else if(b->is_short()){
      // !has_element?
    }else{
      if(Base const* f = b->flow_dep()){
	Dep F(f, nullptr);
	o << "    d" << code_name(F) << ",\n";
      }else{
      }

      if(!b->potential_dep()){
      }else if(!b->has_name()){
	Base const* f = b->potential_dep();
	Dep F(f, nullptr);
	o << "    d" << code_name(F) << ",\n";
      }else{
	Dep F(b->potential_dep(), nullptr);
	auto nb = prechecked_cast<Named_Branch const*>(b);
	o << "    d" << code_name(F)
	  << " = d_potential" << nb->base()->code_name() << ",\n";
      }
    }
  }
  for(auto x : m.circuit()->filters()){
    (void)x;
    o << "//filter. incomplete\n";
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
      o<<"// nullptr\n";
    }else if( (*q)->has_refs() ){
      oindent oi(o);
      (*q)->make_cc_common(oi);
    }else if(dynamic_cast<MGVAMS_FUNCTION const*>(*q)){
      o<<"// FUNCTION no refs: " << (*q)->label() << "\n";
    }else if(dynamic_cast<MGVAMS_TASK const*>(*q)){ untested();
      o<<"// TASK no refs: " << (*q)->label() << "\n";
    }else if(dynamic_cast<MGVAMS_FILTER const*>(*q)){
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
static void make_hrefs_common(std::ostream& o, Hierarchical_Refs const& H)
{
  for (auto q = H.begin(); q != H.end(); ++q) {
    //o__ "Href_ "<< (*q)->code_name() << "(\"" << (*q)->name() << "\");\n";
    o__ "Href_ "<< (*q)->code_name() << "; // " << (*q)->name() << "\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_parameter_decl(std::ostream& o, const Parameter_List_Collection& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    if((*q)->is_local()){
      o__ (**q).type();
    }else{
      o__ "PARAMETER<p_" << (**q).type() << ">";
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
/*--------------------------------------------------------------------------*/
class VAR_DECL {
public:
  typedef enum {
    _instance = 1,
    _common = 2,
    _all = 3
  } select_t;
  typedef Variable_List_Collection VLC;
private:
  select_t _mode;
public:
  VAR_DECL(select_t m) : _mode(m) {}
private:
  bool mode_is_instance()const {return _mode == _instance || _mode == _all; }
  bool mode_is_common()const {return _mode == _common || _mode == _all; }
  void make_one_variable_decl(std::ostream& o, Token_VAR_REF const& V)const {
    if(V.type().is_real()) {
      o << "double _" << V.name() << "{0.}";
    }else if(V.type().is_int()) {
      o << "int _" << V.name() << "{0}";
    }else{ untested();
      incomplete();
      o << "unknown";
    }
    o << ";\n";
  }
  void make_one_variable_decl(std::ostream& o, Variable_Decl const& V)const {
    if(V.is_common() && !mode_is_common()){
      o__ "// common ";
    }else if(V.is_temporary()){
      o__ "// temporary ";
    }else if(!V.is_common() && !mode_is_instance()){
      o__ "// instance ";
    }else{
      o__ "";
    }
    return make_one_variable_decl(o, V.token());
  }
  void make_variable_collection(std::ostream& o, VLC const& P) {
    for (auto q = P.begin(); q != P.end(); ++q) {
      for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
	Variable_Decl const* V = *p;
	assert(V);
	make_one_variable_decl(o, *V);
      }
    }
  }
  void make_obsolete_comparison(std::ostream& o, Variable_List_Collection const& P) {
    o__ "bool operator==(state_" << " const& o)const {\n";
    o____ "(void)o;\n";
    o____ "return true";
    for (auto q = P.begin(); q != P.end(); ++q) {
      for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
	Variable_Decl const* V = *p;
	assert(V);
	if(V->is_common() && mode_is_common()) {
	  o << "&& _" << V->name() << "== o._" << V->name() << "\n";
	}else{
	}
      }
    }
    o__ ";\n";
    o__ "}\n";
  }
  void make_module_variable_decl(std::ostream& o, Module const& m) {
    Variable_List_Collection const& P = m.variables();
    o__ "struct state_{\n";
    {
      indent x;
      make_operators(o, "");
      o << "#if __cplusplus >= 202002L\n";
      o << "#else\n";
      make_obsolete_comparison(o, P);
      o << "#endif\n";
      make_variable_collection(o, P);

      for(auto s : analog(m).list()){
	if(auto a = dynamic_cast<AnalogConstruct const*>(s)){
	  assert(a->block());
	  make_variable_decl(o, *a->block());
	}else{
	}
      }
    }
    o__ "}_v_;\n";
    if (mode_is_instance()) {
      o__ "state_ _v_1;\n";
    }else{
    }
  }
  void make_subblock_variable_decl(std::ostream& o, SeqBlock const& s) {
    for(int i=0; i<s.num_blocks(); ++i){
      assert(s.blocks(i));
      if(auto sb = dynamic_cast<SeqBlock const*>(s.blocks(i))){
	make_block_variable_decl(o, *sb);
      }else{ untested();
      }
    }
  }
  void make_operators(std::ostream& o, std::string const& s) {
    o << "#if __cplusplus >= 202002L\n";
    o__ "constexpr std::partial_ordering operator<=>(state_" << s << " const&)const = default;\n";
    o << "#endif\n";
  }
  void make_block_variable_decl(std::ostream& o, SeqBlock const& s) {
    Variable_List_Collection const& P = s.variables_();
    if(s.has_identifier()){
      static int count;
      std::string name = to_string(count++);
      o__ "struct state_" << name << "{\n";
      {
	indent ns;
	make_operators(o, name);
	make_variable_collection(o, P);
	make_subblock_variable_decl(o, s);
      }
      o__ "}" << s.code_name() << ";\n";
    }else{
      make_subblock_variable_decl(o, s);
    }
  }
public:
  void make_variable_decl(std::ostream& o, Block const& b) {
    if(auto m = dynamic_cast<Module const*>(&b)){
      make_module_variable_decl(o, *m);
    }else if(auto s = dynamic_cast<SeqBlock const*>(&b)){
      make_block_variable_decl(o, *s);
    }else{ untested();
    incomplete();
    }
  }
}; // VAR_DECL
/*--------------------------------------------------------------------------*/
static void make_common(std::ostream& o, const Module& m)
{
  assert(m.circuit());
  std::string class_name = "COMMON_" + m.identifier().to_string();
  std::string base_class_name;
  base_class_name = "COMMON_COMPONENT";
  std::string common_name = "COMMON_" + m.identifier().to_string();
  // if(m.has_submodule()){ untested();
  //   base_class_name = "COMMON_PARAMLIST";
  // }else{ untested();
  //   base_class_name = "COMMON_COMPONENT";
  // }
  o << "class MOD_" << m.identifier() << ";\n";
  o << "class " << class_name << " :public " << base_class_name << "{\n";
  o__ "typedef " << common_name << " COMMON;\n";
  o__ "typedef MOD_" << m.identifier() << " MOD;\n";
  o__ "typedef enum { m_TR_ADVANCE, m_TR_ACCEPT, m_PRECALC, m_TR_REVIEW }eval_t;\n";
  if(m.has_submodule()) {
  o << "public:\n";
    o__ "PARAM_LIST _netlist_params;\n";
  }else{
  }
  declare_ddouble(o, m);
  o << "private:\n";
  o__ "explicit " << class_name << "(const " << class_name << "& p) = default;\n";
  o << "public:\n";
  o__ "explicit " << class_name << "(int c=0);\n";
  o__ "         ~" << class_name << "();\n";
  o << "#if __cplusplus >= 202002L\n";
  o__ "bool     operator==(const COMMON_COMPONENT& x)const override {\n";
  o____ "return " << class_name << "::operator<=>(x) == 0;\n";
  o__ "}\n";
  o__ "bool     operator<(const COMMON_COMPONENT& x)const override {\n";
  o____ "return " << class_name << "::operator<=>(x) < 0;\n";
  o__ "}\n";
  o__ "int      operator<=>(const COMMON_COMPONENT&)const;\n";
  o__ "bool     has_less()const override {return true;}\n";
  o << "#else\n";
  o__ "int      compare(const COMMON_COMPONENT&)const override;\n";
  o__ "bool     operator==(const COMMON_COMPONENT&)const override;\n";
  o << "#endif\n";
  o__ "COMMON_COMPONENT* clone()const override {return new "<<class_name<<"(*this);}\n";
  o__ "void     set_param_by_index(int, std::string&, int)override;\n";
  o__ "int     set_param_by_name(std::string, std::string)override;\n";
  o__ "int      is_valid(COMPONENT const*)const override;\n";
  o << "private:\n";
  o__ "int      is_valid_()const;\n";
  o__ "bool     param_is_printable(int)const override;\n";
  o__ "std::string param_name(int)const override;\n";
  o__ "std::string param_name(int,int)const override;\n";
  o__ "std::string param_value(int)const override;\n";
  o__ "int param_count()const override {return "
        << m.parameters().count_nonlocal()
        << " + " << base_class_name << "::param_count();}\n";
  o__ "void precalc_first(const PARAM_LIST*)override;\n";
  o__ "void expand(const COMPONENT*)override;\n";
  o__ "void precalc_last(const PARAM_LIST*)override;\n";
  o << "public:\n";
  if(m.has_expand_last()){
    o__ "void expand_last(const COMPONENT*); // override\n";
  }else{
  }
  // if has_analog?
  o__ "void tr_eval_analog(MOD_" << m.identifier() << "*)const;\n";
  if(m.has_tr_begin()){
    o__ "void tr_begin(COMPONENT*)const;\n";
  }else{
  }
  if(m.has_tr_review() && m.has_analog_block()){
    o__ "void tr_review_analog(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  if(m.has_tr_begin_analog()) {
    o__ "void tr_initial_analog(MOD_" << m.identifier() << "*);\n";
    o__ "void tr_begin_analog(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  if(m.has_tr_restore_analog()) {
    o__ "void tr_restore_analog(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  if(m.has_tr_accept() && m.has_analog_block()){
    o__ "void tr_accept_analog(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  if(m.has_tr_advance() && m.has_analog_block()){
    o__ "void tr_advance_analog(MOD_" << m.identifier() << "*)const;\n";
    o__ "void tr_regress_analog(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  if(m.has_final()){
    o__ "void final_(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  if(m.has_final_analog()){
    o__ "void final_analog(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  if(m.has_final_digital()){
    o__ "void final_digital(MOD_" << m.identifier() << "*)const;\n";
  }else{
  }
  o__ "void precalc_analog(MOD_" << m.identifier() << "*);\n";
  o__ "std::string name()const override {itested();return \"" << m.identifier() << "\";}\n";
//    "  const SDP_CARD* sdp()const {return _sdp;}\n"
//    "  bool     has_sdp()const {untested();return _sdp;}\n"
  o << "public: // input parameters\n";
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
  if(options().optimize_common()){
    incomplete();
    o << "public: // common values\n";
    VAR_DECL vv(VAR_DECL::_common);
    vv.make_variable_decl(o, m);
  }else{ untested();
    o << "// public: common values; disabled\n";
  }
  o << "private: // funcs\n";
  make_funcs_common(o, m.funcs());
  o << "private: // hrefs\n";
  make_hrefs_common(o, m.hrefs());
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
  size_t k = br.num_states();
  o__ "struct {\n"; // _state" << br.code_name();
  o____ "double _s[" << k << "]; // (s)\n";
  o____ "double* ptr() {return _s;}\n";
  o____ "double& operator[](int k) {return _s[k];}\n";
  o____ "void clear(){std::fill_n(_s+1, " << k-1 << ", 0.);}\n";
  o__ "}_st" << br.code_name() << ";\n";

//  for(auto n : br.names()){ untested();
//    o__ "double _value_br_" << n << ";\n";
//    o__ "double _st_br_" << n;
//    o__ "[" << k << "];\n";
//  }

  o__ "struct _st" << br.code_name() << "_ {\n";
  o____ "enum { ";
  std::string comma = "";
  o____ "VALUE, SELF";
  for(Dep const& d : br.ddeps()){
//      o << "/* found " << d->code_name() << "*/";
    Branch const* bbb = ::branch(d);
    assert(bbb);
    if(bbb->is_short()){
    }else if(bbb == &br){
    }else if(bbb->has_flow_probe()){
    }else{
      assert(d);
      o << ", dep" << probe(d)->code_name();
    }
  }
  o << "/* : */\n";
  for(Dep const& d : br.ddeps()){
    Branch const* bbb = probe(d)->branch();
    assert(bbb);
    if(bbb->is_short()){
    }else if(bbb == &br){
    }else if(!bbb->has_flow_probe()){
    }else{
      assert(d);
      o << ", dep" << probe(d)->code_name();
    }
  }
  o____ "};\n";
  o__ "} _dep" << br.code_name() << ";\n";

}
/*--------------------------------------------------------------------------*/
static int make_current_node_decl(std::ostream& o, const Module& m, int n)
{
  for (auto br : m.circuit()->branches()){
    std::string bcn = br->code_name();
    bool needed = false;
    if(br->is_filter()){
      if(!br->is_used() && options().optimize_unused()){
      }else{
	needed = true;
      }
    }else if(br->is_short()){
    }else if(!br->is_used() && options().optimize_unused()){
    }else if(br->has_element()){
	needed = true;
    }else{
    }
    if(needed) {
      o____ "I" << bcn << " = " << n++ <<",\n";
    }else{
    }
  }
  return n;
}
/*--------------------------------------------------------------------------*/
static void make_node_decl(std::ostream& o, const Module& m)
{
  std::string comma = "";
  o__ "enum {\n";
  std::vector<int> isport(m.circuit()->nodes().size()+1);
  int n = 0;
  for (auto p : m.circuit()->ports()) {
    o____ p->code_name() << ", // " << p->node_number() << "\n";
    ++n;
    if(p->node_number()!=-1){
      o << "// is node number " << p->node_number() << "\n";
      isport[p->node_number()] = n;
    }else{
    }
  }
  if (m.circuit()->nodes().size()){
    o____ "/* ---- */\n";
  }else{
  }
  for (int k=1; k <= int(m.circuit()->nodes().size()); ++k) {
    Node const* nn = m.circuit()->nodes()[k];
    if(isport[nn->number()]) {
      o____ "n_" << nn->name() << " = " << isport[nn->number()] - 1 << ", // .. connected to port\n";
    }else{
      o____ "";
      o << "n_" << nn->name() << " = " << n << " /* used: " << nn->is_used() << " */,\n";
      ++n;
    }
  }
  n = make_current_node_decl(o, m, n);
  o____ "_n_total = " << n << "\n";
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
  return;
  std::string class_name = "PRECALC_" + m.identifier().to_string();
  o << "class " << class_name << "{\n";
  o__ "COMPONENT* _d{nullptr};\n";
  o << "public:\n";
  declare_ddouble(o, m);
  o << "public:\n";
  o__ "explicit " << class_name << "(COMPONENT* d) : _d(d) {}\n";
  // ... functions->make_cc_precalc?
 // for(auto const& i : m.funcs()) { untested();
 //   // indent x;
 //  // i->make_cc_precalc(o);
 // }

  o << "};\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_elements(std::ostream& o, Element_2_List const& L)
{
  o__ "COMPONENT const* _parent{nullptr};\n";
  for (Element_2_List::const_iterator e = L.begin(); e != L.end();
      ++e) {
    {
      assert(*e);
      o__ "COMPONENT const* " << (*e)->code_name() << "{nullptr};\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_current_nodes(std::ostream& o, const Module& m)
{
  for (auto br : m.circuit()->branches()){
    std::string bcn = br->code_name();
    bool needed = false;
    if(br->is_filter()){
      if(!br->is_used() && options().optimize_unused()){
	o__ "// ELEMENT* " << br->code_name() << "{nullptr}; // unused filter\n";
      }else{
	needed = true;
      }
    }else if(br->is_short()){
      o__ "// short : " << br->code_name() << "\n";
    }else if(!br->is_used() && options().optimize_unused()){
      o__ "// ELEMENT* " << br->code_name() << "{nullptr}; // unused\n";
    }else if(br->has_element()){
      needed = true;
    }else{
      o__ "// ELEMENT* " << br->code_name() << "{nullptr}; // no element (not used)\n";
    }
    if(needed) {
      o__ "MOD_" << m.identifier();
      o << "::CURRENT_CTRL_" << bcn << " MOD_" << m.identifier() << "::_c" << bcn << ";\n";
    }else{
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
  o__ "typedef " << class_name << " MOD;\n";
  o__ "typedef " << common_name << " COMMON;\n";
  o << "private: // node list\n";
  make_node_decl(o, m);
  o << "private:\n";
 // o__ "bool _eval{false};\n";
  if(m.has_tr_accept()){
    o__ "bool _accept{false};\n";
  }else{
  }
  o << "public:\n";
  declare_ddouble(o, m);
  o << "private: // data\n";
  o__ "mutable node_t _nodes[_n_total];\n";
  if(m.times()){
    o__ "double _time[" << m.times() << "];\n";
    o__ "TIME_PAIR _time_by;\n";
  }else{
  }
  o << "public: // netlist\n";
  if(m.has_submodule()) {
    make_cc_elements(o, m.circuit()->element_list());
  }else{
  }
  for (auto br : m.circuit()->branches()){
    std::string bcn = br->code_name();
    bool needed = false;
    if(br->is_filter()){
      if(!br->is_used() && options().optimize_unused()){
	o__ "// ELEMENT* " << br->code_name() << "{nullptr}; // unused filter\n";
      }else{
	o__ "ELEMENT* " << bcn << "{nullptr}; // filter\n";
	needed = true;
      }
    }else if(br->is_short()){
      o__ "// short : " << br->code_name() << "\n";
    }else if(!br->is_used() && options().optimize_unused()){
      o__ "// ELEMENT* " << br->code_name() << "{nullptr}; // unused\n";
    }else if(br->has_element()){
	o__ "ELEMENT* " << bcn << "{nullptr}; // branch\n";
	needed = true;
    }else{
      o__ "// ELEMENT* " << br->code_name() << "{nullptr}; // no element (not used)\n";
    }
    if(needed) {
      o__ "static struct CURRENT_CTRL_" << bcn << ":public CURRENT_CTRL {\n";
      o____ "explicit CURRENT_CTRL_" << bcn << "() : CURRENT_CTRL(\"" << bcn << "\"){\n";
      o______ "n_(0) = this;\n";
      o____ "}\n";
      o____ "ELEMENT const* e(COMPONENT const* c)const {\n";
      o______ "auto m = prechecked_cast<MOD_" << m.identifier() << " const*>(c);\n";
      o______ "assert(m);\n";
      o______ "return m->" << bcn << ";\n";
      o____ "}\n";
      o____ "int user_number()const override {return I" << bcn << ";}\n";
      o__ "} _c" << bcn << ";\n";
    }else{
    }
  }
  o << "private: // func decl\n";
  make_func_dev(o, m.funcs());
  o << "private: // construct\n";
  o__ "explicit MOD_" << m.identifier() << "(MOD_" << m.identifier() << " const&);\n";
  o << "public:\n";
  o__ "explicit MOD_" << m.identifier() << "(); // : "<< base_name <<"() { _n = _nodes; }\n";
  o__ "~MOD_" << m.identifier() << "();\n";
  o__ "CARD* clone()const override;\n";
  o << "private: // overrides\n";
  if(m.has_submodule()) {
    o__ "bool is_device() const override{return _parent;}\n";
    o__ "CARD_LIST* scope() override;\n";
    o__ "const CARD_LIST* scope()const override " <<
	"{ return const_cast<MOD_" << m.identifier() << "*>(this)->scope();}\n";
  }else{
  }
/*--------------------------------------------------------------------------*/

  if(m.has_hsparam()){
    o__ "int     set_param_by_name(std::string, std::string)override;\n";
  }else{
  }
  o__ "void precalc_first()override;\n";
  o__ "void expand_first()override;\n";
  o__ "void expand()override;\n";
  if(m.has_expand_last()){
    o__ "void expand_last()override;\n";
  }else{
  }
  o__ "void precalc_last()override;\n";
  o__ "void zero_filter_readout();\n";
  o__ "//void    map_nodes();         //BASE_SUBCKT\n";
  o__ "//void    tr_restore();        //BASE_SUBCKT\n";
  o__ "void    tr_load()override{ trace1(\"tr_load\", long_label());BASE_SUBCKT::tr_load();}\n";

  if(m.has_tr_review()){
    o__ "TIME_PAIR  tr_review()override;\n";
  }else{
  }
  if(m.has_tr_begin()){
    o__ "void tr_begin()override;\n";
  }else{
  }
  if(m.has_tr_restore()){
    o__ "void tr_restore()override;\n";
  }else{
  }
  if(m.has_tr_accept()){
    o__ "void tr_accept()override;\n";
    o__ "void q_accept() { _accept = 1; }\n";
  }else{
  }
  if(m.has_events()) {
    o__ "double new_event(double newtime, double tol) {\n";
    o____ "trace3(\"new_event\", long_label(), _sim->_time0, newtime);\n";
    o____ "trace3(\"new_event\", long_label(), newtime - _sim->_time0, _sim->_dtmin);\n";
    o____ "if(tol) {\n";
             // not used.
    o____ "}else{\n";
    o____ "}\n";
    o____ "if(!_sim->analysis_is_dcop()) {\n";
    o______ "assert(_sim->_dtmin);\n";
    o______ "return _sim->new_event(newtime, this);\n";
    o____ "}else{\n";
    o______ "return NEVER;\n";
    o____ "}\n";
    o__ "}\n";
  }else{
  }
  if(m.has_tr_advance()){
    o__ "void tr_advance()override;\n";
    o__ "void tr_regress()override;\n";
  }else{
  }
  if(m.has_analysis()){
    // o__ "void    tr_advance()override;         //BASE_SUBCKT\n";
  }else{
    o__ "//TIME_PAIR  tr_review()override;         //BASE_SUBCKT\n";
    o__ "//void    tr_accept()override;         //BASE_SUBCKT\n";
  }
  o__ "//void    tr_unload();         //BASE_SUBCKT\n";
  o__ "void      dc_advance()override;\n";
  if (!m.has_analog_block()) {
    o__ "//void    tr_advance();        //BASE_SUBCKT\n";
    o__ "//void    tr_regress();        //BASE_SUBCKT\n";
    o__ "//bool    tr_needs_eval()const;//BASE_SUBCKT\n";
    o__ "//void    tr_queue_eval();     //BASE_SUBCKT\n";
    o__ "//bool    do_tr();             //BASE_SUBCKT\n";
  }else{
    o__ "bool      tr_needs_eval()const override;\n";
    o__ "void      tr_queue_eval()override {if(tr_needs_eval()){q_eval();}else{} }\n";
   //  o__ "void q_eval() { COMPONENT::q_eval(); }\n";
    o__ "bool      do_tr() override;\n";
//    o__ "void      ac_begin() override;\n";
//    o__ " void    do_ac();\n";
  }
  if(m.has_final()){
    o__ "void ac_final()override {BASE_SUBCKT::ac_final(); final_();}\n";
    o__ "void dc_final()override {BASE_SUBCKT::dc_final(); final_();}\n";
    o__ "void tr_final()override {BASE_SUBCKT::tr_final(); final_();}\n";
    o__ "void final_();\n";
  }else{
  }
  o__ "double tr_probe_num(std::string const&)const override;\n";
  o__ "  //void    ac_load();           //BASE_SUBCKT\n";
  o__ "  //XPROBE  ac_probe_ext(CS&)const;//CKT_BASE/nothing\n";
  o__ "std::string dev_type()const override {return \"" << m.identifier() << "\";}\n";
  o__ "int max_nodes()const override {return "<< m.circuit()->ports().size() <<";}\n";
 // o__ "int net_nodes()const override {return "<< m.circuit()->ports().size() <<";}\n";
  o__ "int min_nodes()const override {return 0;}\n";
  o__ "int int_nodes()const override    {return "
      << m.circuit()->nodes().size() - m.circuit()->ports().size() << ";}\n";
  o__ "std::string value_name()const override {itested(); return \"\";}\n";
  o__ "bool print_type_in_spice()const override {itested(); return false;}\n";
  o__ "std::string port_name(int i)const override;\n";
  o__ "node_t& n_(int i)const override {\n";
  o____ "assert(_nodes); assert(i>=0); assert(!i || i<_n_total); return _nodes[i];\n";
  o__ "}\n";
  o << "private: // impl\n";
  o << "/* ========== */\n";

  o << "public: // instance vars\n";
  if(options().optimize_common()){
    VAR_DECL vv(VAR_DECL::_instance);
    vv.make_variable_decl(o, m);
  }else{ untested();
    VAR_DECL vv(VAR_DECL::_all);
    vv.make_variable_decl(o, m);
  }
  o << "private: // branch state\n";
  make_branch_states(o, m);
  o << "private: // probe values\n";
  for(auto x : m.circuit()->branches()){
    assert(x);
    Branch const* b = x;
    if(b->has_flow_probe()){
      o__ "double _flow" << b->code_name() << "{0.};\n";
    }else{
    }
    if(b->has_pot_probe()){
      o__ "double _potential" << b->code_name() << "{0.};\n";
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

  o << "}; // m_" << m.identifier() << ";\n";
  make_current_nodes(o, m);
  o << "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
#if 0
static void make_eval(std::ofstream& out, const Eval& e,
		      const String_Arg& dev_name)
{
  std::string class_name = "EVAL_" + dev_name.to_string() + '_' 
    + e.name().to_string();
  out <<
    "class " << class_name << " : public COMMON_COMPONENT {\n"
    "private:\n"
    "  explicit "<< class_name << "(const "<< class_name << "& p)\n"
    "    :COMMON_COMPONENT(p) {}\n"
    "public:\n"
    "  explicit "<< class_name << "(int c=0) :COMMON_COMPONENT(c) {}\n"
    "  bool operator==(const COMMON_COMPONENT& x)const override"
		"{return COMMON_COMPONENT::operator==(x);}\n"
    "  COMMON_COMPONENT* clone()const override{return new "<<class_name<<"(*this);}\n"
    "  std::string name()const override {untested(); return \""<< class_name << "\";}\n"
    "  void tr_eval(ELEMENT*d)const override;\n"
    "  bool has_tr_eval()const override {return true;}\n"
    "  bool has_ac_eval()const override {return false;}\n"
    "};\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
#endif
/*--------------------------------------------------------------------------*/
// make_any_eval...
static void make_elt_eval(std::ostream& o, const Element_2& p)
{
  std::string id = p.short_label();
  std::string class_name = "COMMON_" + id;
  std::string base_class_name;
  base_class_name = "COMMON_LOGIC";
  o << "class " << class_name << " :public " << base_class_name << "{\n";
  o__ "explicit " << class_name << "(const " << class_name << "& p) : "
                  << base_class_name << "(p) { }\n";
  o__ "COMMON_COMPONENT* clone()const override {return new "<<class_name<<"(*this);}\n";
  o << "public:\n";
  o__ "explicit " << class_name << "(int c=0) : " << base_class_name << "(c) {}\n";
  o__ "         ~" << class_name << "() {}\n";
  o << "private:\n";
  o__ "bool    operator==(const COMMON_COMPONENT& x)const override {\n";
  o____ class_name << " const* p = dynamic_cast<const " << class_name << "*>(&x);\n";
  o____ "bool rv = p && " << base_class_name << "::operator==(x);\n";
  o____ "return rv;\n";
  o__ "}\n";
  o__ "virtual LOGICVAL logic_eval(node_l const*, int)const override;\n";
  o__ "std::string name()const override {itested();return \"" << id << "\";}\n";
  o__ "std::string port_name(int i)const override {\n";
  o____ "assert(i >= 0);\n";
  o____ "static std::string names[] = {";
  std::string comma = "";
  for (int nn = 0; nn < p.net_nodes(); ++nn){
    o << comma << '"' << p.port_name(nn) << '"';
    comma = ", ";
  }
  o____ "};\n";
  o____ "if(i < " << p.net_nodes() << "){\n";
  o______ "return names[i];\n";
  o____ "}else{ untested();\n";
  o______ "return \"\";\n";
  o____ "}\n";
  o__ "}\n";

  o << "}; //" << class_name << "\n";
  o << "static " << class_name << " Eval_" << p.eval() << "(CC_STATIC);\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
} // make_elt_eval
/*--------------------------------------------------------------------------*/
static void make_evals(std::ostream& o, const Module& m)
{
#if 0
  for (Eval_List::const_iterator
       e = d.eval_list().begin();
       e != d.eval_list().end();
       ++e) {
    make_eval(out, **e, d.name());
  }
#endif
  Element_2_List const& L = m.circuit()->element_list();
  for (auto e = L.begin(); e != L.end(); ++e) {
    if((*e)->eval()!=""){
      make_elt_eval(o, **e);
    }else{
      o << "// no eval in " << (*e)->short_label() << "\n";
    }
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_decl(std::ostream& o, const Module& m)
{
  make_common(o, m);
  make_precalc(o, m);
  make_module(o, m);
  if(m.has_submodule()){
    make_evals(o, m);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
