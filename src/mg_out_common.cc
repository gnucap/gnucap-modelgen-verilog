/*$Id: mg_out_common.cc,v 26.130 2009/11/15 21:51:08 al Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
#include "mg_circuit.h"
#include "mg_.h" // TODO
#include "mg_options.h"
#include "mg_href.h"
#include <numeric>
/*--------------------------------------------------------------------------*/
static void make_common_default_constructor(std::ostream& o, const Module& d)
{
  make_tag(o);
  o <<
    "COMMON_" << d.identifier() << "::COMMON_" << d.identifier() << "(int c)\n"
    "  :COMMON_COMPONENT(c)";
  make_construct_parameter_list(o, d.parameters());
//  o << ",\n   _sdp(0)";
  // make_construct_parameter_list(o, d.common().calculated());
//  for (Args_List::const_iterator
//       p = d.circuit().args_list().begin();
//       p != d.circuit().args_list().end();
//       ++p) { untested();
//    o << ",\n   _" << (**p).name() << "(0)";
//  }
  o <<
    "\n"
    "{\n";
//  for (Parameter_1_List::const_iterator
//       p = d.common().override().begin();
//       p != d.common().override().end();
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
  o <<
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_copy_constructor(std::ostream& o, const Module& d)
{
  o<< "#if 0\n";
  make_tag(o);
  o <<
    "COMMON_" << d.identifier() << "::COMMON_" << d.identifier() << "(const COMMON_" << d.identifier() << "& p)\n"
    "  :COMMON_COMPONENT(p)";
  if(d.circuit()->element_list().size()){
    o << ",\n    _netlist_params(p._netlist_params)";
  }else{
  }
  make_copy_construct_parameter_list(o, d.parameters());
  if (options().optimize_common()){
    o << ",\n   _v_(p._v_)";
  }else{ untested();
  }
  //o << ",\n   _sdp(0)";
  //make_copy_construct_parameter_list(o, d.common().calculated());
//  for (Args_List::const_iterator
//       p = d.circuit().args_list().begin();
//       p != d.circuit().args_list().end();
//       ++p) { untested();
//    o << ",\n   _" << (**p).name() << "(0)";
//  }
  o << 
    "\n"
    "{\n";
//  for (Parameter_1_List::const_iterator
//       p = d.common().override().begin();
//       p != d.common().override().end();
//       ++p) {untested();
//    o << ",\n   " << (**p).code_name() << "(p." << (**p).code_name() << ")";
//  }
  o <<
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
  o << "#endif\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_destructor(std::ostream& o, const Module& d)
{
  make_tag(o);
  o <<
    "COMMON_" << d.identifier() << "::~COMMON_" << d.identifier() << "()\n"
    "{\n";
//   for (Args_List::const_iterator
//        p = d.circuit().args_list().begin();
//        p != d.circuit().args_list().end();
//        ++p) { untested();
//     o << "  detach_common(&_" << (**p).name() << ");\n";
//   }
  o <<
//    "  delete _sdp;\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_operator_threeway(std::ostream& o, const Module& d)
{
  make_tag(o);
  o <<
    "int COMMON_" << d.identifier() << "::operator<=>(const COMMON_COMPONENT& x) const\n{\n";
  o__ "if(this == &x){\n";
  o____ "return 0;\n";
  o__ "}else{\n";
  o__ "}\n";
  o__ "std::type_index a(typeid(*this));\n";
  o__ "std::type_index b(typeid(x));\n";
  o__ "if(a < b){\n";
  o____ "return -1;\n";
  o__ "}else if(a > b){\n";
  o____ "return 1;\n";
  o__ "}else{\n";
  o__ "}\n";

  o__ "int c;\n";
  o__ "if((c = COMMON_COMPONENT::compare(x))){\n";
  o____ "return c;\n";
  o__ "}else{\n";
  o__ "}\n";

  o__ "auto p = prechecked_cast<const COMMON_" << d.identifier() << "*>(&x);\n";
  o__ "assert(p);\n";

  if(d.circuit()->element_list().size()){
    o__ "if((c = _netlist_params.compare(p->_netlist_params))) {\n";
    o____ "return c;\n";
    o__ "}else{\n";
    o__ "}\n";
  }else{
  }

  std::string sep;
  for (Parameter_List_Collection::const_iterator
       q = d.parameters().begin();
       q != d.parameters().end();
       ++q) {
    Parameter_2_List::const_iterator p = (*q)->begin();
    if((*q)->is_local()) {
      for ( ; p != (*q)->end(); ++p) {
	o__ sep << "if(" << (*p)->code_name() << " < p->" << (*p)->code_name() << ") {\n";
	o____ "return -1;\n";
	o__ "}else if(" << (*p)->code_name() << " > p->" << (*p)->code_name() << ") {\n";
	o____ "return 1;\n";
	sep = "}else ";
      }
    }else{
      for ( ; p != (*q)->end(); ++p) {
	o__ sep << "if((c = " << (*p)->code_name() << ".compare(p->" << (*p)->code_name() << "))){\n";
	o____ "return c;\n";
	sep = "}else ";
      }
    }
  }
  o__ sep << "{\n";
  o__ "}\n";

  for(FUNCTION_ const* f : d.funcs()){
    make_tag(o);
    if(f->has_refs()){
      f->make_cc_common_compare(o);
    }else{
    }
  }
  o__ "auto ord = _v_ <=> p->_v_;\n";
  o__ "if(ord == std::weak_ordering::equivalent){\n";
  o____ "return 0;\n";
  o__ "}else if(ord == std::weak_ordering::less){\n";
  o____ "return -1;\n";
  o__ "}else if(ord == std::weak_ordering::greater){\n";
  o____ "return 1;\n";
  o__ "}else{untested();\n";
  o____ "unreachable();\n";
  o____ "return 0;\n";
  o__ "}\n";

  o <<  "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
static void make_common_operator_compare(std::ostream& o, const Module& d)
{
  make_tag(o);
  o <<
    "int COMMON_" << d.identifier() << "::compare(const COMMON_COMPONENT& x)const\n{\n"
    "  const COMMON_" << d.identifier() << "* p = dynamic_cast<const COMMON_" << d.identifier() << "*>(&x);\n"
    "  assert (p);\n";

  o__ "if(this == &x){\n";
  o____ "return 0;\n";
  o__ "}else{\n";
  o__ "}\n";

  for(FUNCTION_ const* f : d.funcs()){
    make_tag(o);
    if(f->has_refs()){
      f->make_cc_common_compare(o);
    }else{
    }
  }

  o__ "if(_v_ == p->_v_){\n";
  o____ "return 0;\n";
  o__ "}else{\n";
  o____ "return 1;\n";
  o__ "}\n";

  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_operator_equal(std::ostream& o, const Module& d)
{
  make_tag(o);
  o <<
    "bool COMMON_" << d.identifier() << "::operator==(const COMMON_COMPONENT& x)const\n"
    "{\n"
    "  const COMMON_" << d.identifier() << "* p = dynamic_cast<const COMMON_" << d.identifier() << "*>(&x);\n"
    "  return (p\n";
  if(d.circuit()->element_list().size()){
    o << "      && _netlist_params == p->_netlist_params\n";
  }
  for (Parameter_List_Collection::const_iterator
       q = d.parameters().begin();
       q != d.parameters().end();
       ++q) {
    if(1 || !(*q)->is_local()) {
      for (Parameter_2_List::const_iterator
	   p = (*q)->begin();
	   p != (*q)->end();
	   ++p) {
	o << "    && " << (**p).code_name() << " == p->" << (**p).code_name() << '\n';
      }
    }else{ untested();
    }
  }
  Hierarchical_Refs const& H = d.hrefs();
  for (auto q = H.begin(); q != H.end(); ++q) {
    o____ "&& " << (*q)->code_name() << " == p-> " << (*q)->code_name() << "\n";
  }
  o____ "&& !compare(*p)\n"; // kludge.
  o____ "&& COMMON_COMPONENT::operator==(x));\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_set_param_by_name(std::ostream& o, const Module& m)
{
  o << "int COMMON_" << m.identifier() << "::set_param_by_name("
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

  for (Parameter_List_Collection::const_iterator
      q = m.parameters().begin();
      q != m.parameters().end(); ++q) {
    if(!(*q)->is_local()) {
      for (Parameter_2_List::const_iterator
	  p = (*q)->begin();
	  p != (*q)->end();
	  ++p) {
	names.push_back(&(*p)->name());
	if((*p)->aliases().size()) {
	  alias.push_back(1);
	  for(auto a : (*p)->aliases()){
	    alias.push_back(alias.back()+1);
	    names.push_back(&a->name());
	  }
	}else{
	  alias.push_back(0);
	}
      }
    }else{
    }
  }

  std::vector<int> idx(names.size());
  std::iota(idx.begin(), idx.end(), 0);
  std::sort(idx.begin(), idx.end(),
      [&](int A, int B) -> bool {
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
      o__ "return COMMON_COMPONENT::set_param_by_name(\"" << *hs[n] << "\", Value);\n";
    }else if(alias[n]){
      cn = *names[n - alias[n] + 1];
      pn = "_p_" + cn;
      o << "\n";
      o____ "if(!" << pn << ".has_hard_value()) {\n";
      o______ "_s" << pn << " = " << alias[n] << ";\n";
      o____ "}else if(! _s" << pn << ") { untested();\n";
      o______ "_s" << pn << " = " << alias[n] << ";\n";
      o____ "}else if(_s" << pn << " != " << alias[n] << "){\n";
      o______ "throw Exception_No_Match(\"" + *names[n] + ": unavailable alias for "+ cn +".\");\n";
      o____ "}else{\n";
      o____ "}\n";
      o____ "";
      o << pn << " = Value;";
    }else{
      pn = cn;
      o << pn << " = Value;";
    }
    o____ "break; // " << alias[n] << "\n";
    --cnt;
  }
  o____ "case 0: return COMMON_COMPONENT::set_param_by_name(Name, Value);\n";


  o__ "}\n";
  o__ "return lb;\n";
  o__ "}\n";

  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_set_param_by_index(std::ostream& o, const Module& m)
{
  make_tag(o);
  o << "void COMMON_" << m.identifier() << "::set_param_by_index("
       "int I, std::string& Value, int /*Offset*/)\n{\n";
  o__ "switch (I) {\n";
  size_t i = 0;

  for (Parameter_List_Collection::const_iterator
       q = m.parameters().begin();
       q != m.parameters().end(); ++q) {
    if(!(*q)->is_local()) {
      for (Parameter_2_List::const_iterator
	  p = (*q)->begin();
	  p != (*q)->end();
	  ++p) {
	o__ "case " << i++ << ":  " << (**p).code_name() << " = Value; break;\n";
      }
    }
  }

  o____ "default: incomplete(); // throw? COMMON_COMPONENT::set_param_by_index(I - "
	 << i << ", Value, Offset + " << i << ");\n";
  o__ "}\n";
  o__ "(void) Value;\n";
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_param_is_printable(std::ostream& o, const Module& m)
{
  make_tag(o);
  o <<
    "bool COMMON_" << m.identifier() << "::param_is_printable(int I)const\n"
    "{\n"
    "  switch (I) {\n";
  size_t i = 0;
//  for (Parameter_1_List::const_iterator 
//       p = m.common().override().begin(); 
//       p != m.common().override().end();
//       ++p) { untested();
//    if (!((**p).user_name().empty())) { untested();
//      o << "  case " << i++ << ":  return (";
//      if (!((**p).print_test().empty())) { untested();
//	o << (**p).print_test() << ");\n";
//      }else if ((**p).default_val() == "NA") { untested();
//	o << (**p).code_name() << " != NA);\n";
//      }else{ untested();
//	o << "true);\n";
//      }
//    }else{unreachable();
//    }
//  }
//  assert(i == m.common().override().size());
  for (auto q = m.parameters().begin();
       q != m.parameters().end(); ++q) {
    if(!(*q)->is_local()) {
      for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
	o << "  case " << i++ << ":  return (";
	if (!((**p).print_test().empty())) { untested();
	  o << (**p).print_test() << ");\n";
	    //      }else if ((**p).default_val() == "NA") { untested();
	    //	o << (**p).code_name() << " != NA);\n";
	}else{
	  //o << "true);\n";
	  o << (*p)->code_name() << ".has_hard_value());\n";
	}
      }
    }else{
      for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
	o << "// local " << (*p)->code_name() << "\n";
      }
    }
  }
//  assert(i == m.common().override().size() + m.common().raw().size());
  o <<
    "  default: return COMMON_COMPONENT::param_is_printable(I - " << i << ");\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_param_name(std::ostream& o, const Module& m)
{
  make_tag(o);
  o <<
    "std::string COMMON_" << m.identifier() << "::param_name(int I)const\n{\n"
    "  switch (I) {\n";
  size_t i = 0;
//  for (Parameter_1_List::const_iterator 
//       p = m.common().override().begin(); 
//       p != m.common().override().end();
//       ++p) { untested();
//    if (!((**p).user_name().empty())) { untested();
//      o << "  case " << i++ << ":  return \"" << to_lower((**p).user_name()) << "\";\n";
//    }else{unreachable();
//    }
//  }
//  assert(i == m.common().override().size());
  for (auto q = m.parameters().begin();
       q != m.parameters().end();
       ++q){
    if((*q)->is_local()){
    }else{
      for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
	o << "  case " << i++ << ":  return \"" << (**p).name() << "\";\n";
      }
    }
  }
//  assert(i == m.common().override().size() + m.common().raw().size());
  o <<
    "  default: return COMMON_COMPONENT::param_name(I - " << i << ");\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
  o <<
    "std::string COMMON_" << m.identifier() << "::param_name(int I, int j)const\n"
    "{\n";
  o__ "if(j==0){\n";
  o____ "return param_name(I);\n";
  o__ "}else{\n";
  o____ "return \"\";\n";
  o__ "}\n";
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_param_eval_range(std::ostream& o, ValueRange const& p)
{
  ValueRangeSpec const* spec = p.spec();
  if(auto ri = dynamic_cast<ValueRangeInterval const*>(spec)){
    o__ "double lb, ub;\n";
    o__ "{\n";
    make_cc_expression(o, ri->lb().expression(), false, "precalc");
    o__ "lb = " << "f0" << ";\n";
    o__ "}\n";
    o__ "{\n";
    make_cc_expression(o, ri->ub().expression(), false, "precalc");
    o__ "ub = " << "f0" << ";\n";
    o__ "}\n";

  }else{
  }
}
/*--------------------------------------------------------------------------*/
static void make_param_check_range(std::ostream& o, ValueRange const& p,
    std::string const& n, std::string const& nn)
{
  std::string q;
  o__ "if(";
  if(p.is_from()){
    o << "!";
    q = "from";
  }else if(p.is_exclude()){
    q = "exclude";
  }else{ untested();
    unreachable();
  }
  ValueRangeSpec const* spec = p.spec();
  if(auto ri = dynamic_cast<ValueRangeInterval const*>(spec)){
       	o << "(lb<";
	if(ri->lb_is_closed()){
	  o << "=";
	}else{
	}
	o << n << " && " << n << "<";
	if(ri->ub_is_closed()){
	  o << "=";
	}else{
	}
	o << "ub)";
	o << "){\n";
	o______ "throw Exception_OutOfRange_(\"" << nn << "\","
		<< "\"" << q << " \" + to_string(lb) + \" ..\" + to_string(ub));\n";
  }else if(auto s = dynamic_cast<ValueRangeStrings const*>(spec)){
    o << "(ONE_OF";
    for(auto ss : s->strings()){
      assert(ss);
      o << "|| (" << n << " == vString(std::string(\"" << *ss << "\")))";
    }
    o << ")){\n";
    o______ "throw Exception_OutOfRange_(\"" << nn << "\","
            << "\"" << q << " '{";
    std::string sep;
    for(auto ss : s->strings()){
      o << sep << "\\\"" << *ss << "\\\"";
      sep = ", ";
    }
    o << "}\");\n"; // BUG? evaluate c->expr?
  }else if(auto c = dynamic_cast<ValueRangeConstant const*>(spec)){
    o << "(" << c->expr() << "==" << n << ")";
	o << "){\n";
	o______ "throw Exception_OutOfRange_(\"" << nn << "\","
		<< "\"exclude " + c->expr() + "\");\n"; // BUG? evaluate c->expr?
  }else{ untested();
	o << "){\n";
    incomplete();
    assert(false);
  }
  o____ "}else{\n";
  o____ "}\n";
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_is_valid(std::ostream& o, const Module& m)
{
  o << "int COMMON_" << m.identifier() << "::is_valid(COMPONENT const* x)const\n{\n";
  o__ "assert(x);\n";
  o__ "try{\n";
  o____ "return is_valid_();\n";
  o__ "}catch(Exception_OutOfRange_ const& e){\n";
  o____ "error(bDEBUG, x->long_label() + \" invalid: \" + e.message() + \"\\n\");\n";
  o____ "return false;\n";
  o__ "}\n";
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_is_valid_(std::ostream& o, const Module& m)
{
  make_tag(o);
  o << "int COMMON_" << m.identifier() << "::is_valid_() const\n{\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";

  // move to precalc?
  for (Parameter_List_Collection::const_iterator
       q = m.parameters().begin();
       q != m.parameters().end();
       ++q) {
    if(!(*q)->is_local())
    for (Parameter_2_List::const_iterator
	 p = (*q)->begin();
	 p != (*q)->end();
	 ++p) {
      for(auto v : (*p)->value_range_list()){
	assert(v);

	o__ "{\n";
	make_param_eval_range(o, *v);
	make_param_check_range(o, *v, (*p)->code_name(), (*p)->name());
      }
    }
  }

  o__ "return true; //COMMON_COMPONENT::is_valid();\n";
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
#if 0
void make_common_param_name_or_alias(std::ostream& o, const Device& m)
{ untested();
  make_tag(o);
  o <<
    "std::string COMMON_" << d.name() << "::param_name(int i, int j)const\n"
    "{\n"
    "  if (j == 0) {\n"
    "    return param_name(i);\n"
    "  }else if (j == 1) {\n"
    "    switch (COMMON_" << d.name() << "::param_count() - 1 - i) {\n";
  size_t i = 0;
  for (Parameter_1_List::const_iterator 
       p = d.common().override().begin(); 
       p != d.common().override().end();
       ++p) { untested();
    if (!((**p).user_name().empty())) { untested();
      o << "    case " << i++ << ":  return \"" << to_lower((**p).alt_name()) << "\";\n";
    }else{unreachable();
    }
  }
  assert(i == d.common().override().size());
  for (Parameter_1_List::const_iterator 
       p = d.common().raw().begin(); 
       p != d.common().raw().end();
       ++p) { untested();
    if (!((**p).user_name().empty())) { untested();
      o << "    case " << i++ << ":  return \"" << to_lower((**p).alt_name()) << "\";\n";
    }else{unreachable();
    }
  }
  assert(i == d.common().override().size() + d.common().raw().size());
  o <<
    "    default: return \"\";\n"
    "    }\n"
    "  }else{untested();//281\n"
    "    return COMMON_COMPONENT::param_name(i, j);\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
#endif
/*--------------------------------------------------------------------------*/
void make_common_param_value(std::ostream& o, const Module& m)
{
  make_tag(o);
  o <<
    "std::string COMMON_" << m.identifier() << "::param_value(int I)const\n"
    "{\n"
    "  switch (I) {\n";
  size_t i = 0;
//  for (Parameter_1_List::const_iterator 
//       p = m.common().override().begin(); 
//       p != m.common().override().end();
//       ++p) { untested();
//    if (!((**p).user_name().empty())) { untested();
//      o << "  case " << i++ << ":  return " << (**p).code_name() << ".string();\n";
//    }else{unreachable();
//    }
//  }
//  assert(i == m.common().override().size());
  for (auto q = m.parameters().begin();
       q != m.parameters().end();
       ++q){
    if(!(*q)->is_local()){
      for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
	  o << "  case " << i++ << ":  return " << (**p).code_name() << ".string();\n";
      }
    }else{
    }
  }

//  assert(i == m.common().override().size() + m.common().raw().size());
  o <<
    "  default: return COMMON_COMPONENT::param_value(I - " << i << ");\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_eval_subdevice_parameters(std::ostream& o , const Element_2& e)
{
  for(auto p : e.list_of_parameter_assignments()){
    o__ "{ // eval_subdevice_parameters \n";
    std::string t0;
    {
      indent x;
      t0 = make_cc_expression(o, p->default_val().expression(), false, "adjust");
    }
    o____ "PARAM_INSTANCE p;\n";
    o____ "p = PARAMETER<vReal>();\n";
    o____ "vReal r(" << t0 << ");\n";
    o____ "p.set_fixed(&r);\n";
    o____ "_netlist_params.set(\"["<< e.short_label() <<"]"<< p->name() <<"\", p);\n";
    o____ "trace2(\"make eval ["<< e.short_label() <<"]"<< p->name() <<"\", p, _netlist_params.size());\n";
    o__ "}\n";
  }

}
/*--------------------------------------------------------------------------*/
static void make_eval_netlist_parameters(std::ostream& o , const Module& m)
{
  for(auto i : m.circuit()->element_list()){
    make_eval_subdevice_parameters(o, *i);
  }
}
/*--------------------------------------------------------------------------*/
static void make_common_expand_last(std::ostream& o , const Module& m)
{
  make_tag(o);
  o  <<
    "void COMMON_" << m.identifier() << "::expand_last(const COMPONENT* d)\n{\n";

  Hierarchical_Refs const& H = m.hrefs();
  for (auto q = H.begin(); q != H.end(); ++q) {
    o__ "try{\n";
    o____ (*q)->code_name() << ".link_card(d, \"" << (*q)->name() << "\");\n";
    o__ "}catch(Exception_Cant_Find const& e){\n";
    o____ "throw(Exception_Precalc(d->long_label() + \": cant find " << (*q)->name() << "\"));\n";
    o__ "}\n";
  }
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_expand(std::ostream& o , const Module& m)
{
  make_tag(o);
  o  <<
    "void COMMON_" << m.identifier() << "::expand(const COMPONENT* d)\n{\n";
    o__ "COMMON_COMPONENT::expand(d);\n";
  o << "}\n";
#if 0
    "  attach_model(d);\n"
    "  COMMON_" << d.identifier() << "* c = this;\n"
    "  const MODEL_" << d.model_type() << "* m = dynamic_cast<const MODEL_" 
      << d.model_type() << "*>(model());\n"
    "  if (!m) {\n"
    "    throw Exception_Model_Type_Mismatch(d->long_label(), modelname(), \"" 
      << d.parse_name() << "\");\n"
    "  }else{\n"
    "  }\n"
    "  // size dependent\n"
    "  //delete _sdp;\n"
    "  _sdp = m->new_sdp(this);\n"
    "  assert(_sdp);\n"
    "  const SDP_" << d.model_type() << "* s = prechecked_cast<const SDP_" 
      << d.model_type() << "*>(_sdp);\n"
    "  assert(s);\n"
    "\n"
    "  // subcircuit commons, recursive\n";
  for (Args_List::const_iterator
       p = d.circuit().args_list().begin();
       p != d.circuit().args_list().end();
       ++p) { untested();
    o << "  COMMON_" << (**p).type() << "* " << (**p).name() 
	<< " = new COMMON_" << (**p).type() << ";\n";
    for (Arg_List::const_iterator
	 a = (**p).begin();
	 a != (**p).end();
	 ++a) { untested();
      o << "  " << (**p).name() << "->" << (**a).arg() << ";\n";
    }
    o << "  attach_common(" << (**p).name() << ", &_" << (**p).name() << ");\n\n";
  }
  o  <<
    "  assert(c == this);\n"
    "}\n"


#endif
  o << "/*--------------------------------------------------------------------------*/\n";
  o << "void COMMON_" << m.identifier() << "::precalc_first(const PARAM_LIST* par_scope)\n{\n";
  o__ "assert(par_scope);\n";
  o__ "COMMON_COMPONENT::precalc_first(par_scope);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";
  // BUG: only structurally relevant
  make_final_adjust_eval_parameter_list(o , m.parameters());
  make_eval_netlist_parameters(o, m);
  o  << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
// It is important to note that for parametric sweep type analyses like dc
// sweep, the tool shall re-evaluate the Elaboration and Pre-simulation steps
// as outlined in Figure 8-1 for each sweep point to ensure that all parameter
// value changes are captured.
static void make_common_init_constants(std::ostream& o, const Variable_List_Collection& V)
{
  make_tag(o);
  o__ "{ // init common constants\n";
  for(Variable_Stmt const* i : V) {
    indent x;
    assert(i);
    for(Variable_Decl const* j : *i) {
      assert(j);
      if(j->rhs().is_empty()){
      }else if(j->is_common()){
	o__ "{\n";

	{ indent y;
	  std::string name = make_cc_expression(o, j->rhs(), false, "adjust");
	  // o__ "_v_." << j->code_name() << " = " << name << ";\n";
	  o__ "_v_._" << j->name() << " = " << name << ";\n";
	}
	o__ "}\n";
      }else{
	incomplete();
      }
    }
  }

  o__ "};\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_tr_begin(std::ostream& o, Module const& m)
{
  make_tag(o);
  Variable_List_Collection const& V = m.variables();
  o << "void COMMON_" << m.identifier() << "::tr_begin(COMPONENT* c) const\n{\n";
  o__ "auto d = prechecked_cast<MOD_" << m.identifier() << "*>(c);\n";
  o__ "(void)d;\n";
  for(Variable_Stmt const* i : V) {
    indent x;
    assert(i);
    for(Variable_Decl const* j : *i) {
      assert(j);
      if(j->rhs().is_empty()){
      }else if(j->is_state_var()){
	o__ "{\n";

	{ indent y;
	  std::string name = make_cc_expression(o, j->rhs(), false, "adjust");
	  // o__ "_v_." << j->code_name() << " = " << name << ";\n";
	  o__ "d->_v_._" << j->name() << " = " << name << ";\n";
	}
	o__ "}\n";
      }else{
	incomplete();
      }
    }
  }
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_final(std::ostream& o, Module const& m)
{
  make_tag(o);
  // Variable_List_Collection const& V = m.variables();
  o << "void COMMON_" << m.identifier() << "::final_(MOD_"
    << m.identifier() << "* c) const\n{\n";
  if(m.has_final_analog()){
    o__ "final_analog(c);\n";
  }else{
    assert(!m.has_final());
  }
  if(m.has_final_digital()){
    incomplete();
//    o__ "final_digital(c);\n";
  }else{
    assert(!m.has_final());
  }
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_precalc_last(std::ostream& o , const Module& m)
{
  o << "void COMMON_" << m.identifier() << "::precalc_last(const PARAM_LIST* par_scope)\n{\n";
  o__ "assert(par_scope);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";
  // BUG: omit structurally relevant?
  make_final_adjust_eval_parameter_list(o , m.parameters());
  make_common_init_constants(o , m.variables());
  make_eval_netlist_parameters(o, m);

  for(FUNCTION_ const* f : m.funcs()){
    make_tag(o);
    if(f->has_refs()){
      f->make_cc_common_precalc(o);
    }else{
    }
  }

  o__ "COMMON_COMPONENT::precalc_last(par_scope);\n";
    o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
#if 0
    "  COMMON_" << d.name() << "* c = this;\n"
    "  const MODEL_" << d.model_type() << "* m = prechecked_cast<const MODEL_" 
      << d.model_type() << "*>(model());\n";

  make_final_adjust(o, d.common());

  o <<
    "\n"
    "  // size dependent\n"
    "  //delete _sdp;\n"
    "  _sdp = m->new_sdp(this);\n"
    "  assert(_sdp);\n"
    "  const SDP_" << d.model_type() << "* s = prechecked_cast<const SDP_" 
      << d.model_type() << "*>(_sdp);\n"
    "  assert(s);\n"
    "\n"
    "  // subcircuit commons, recursive\n";
  for (Args_List::const_iterator
       p = d.circuit().args_list().begin();
       p != d.circuit().args_list().end();
       ++p) { untested();
    o << "  COMMON_" << (**p).type() << "* " << (**p).name() 
	<< " = new COMMON_" << (**p).type() << ";\n";
    for (Arg_List::const_iterator
	 a = (**p).begin();
	 a != (**p).end();
	 ++a) { untested();
      o << "  " << (**p).name() << "->" << (**a).arg() << ";\n";
    }
    o << "  attach_common(" << (**p).name() << ", &_" << (**p).name() << ");\n\n";
  }

  o <<
    "  assert(c == this);\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
#endif
/*--------------------------------------------------------------------------*/
} // common::expand
/*--------------------------------------------------------------------------*/
void make_cc_common(std::ostream& o , const Module& m)
{
  make_tag(o);
  make_common_default_constructor(o, m);
  make_common_copy_constructor(o, m);
  make_common_destructor(o, m);
  o << "#if __cplusplus >= 202002L\n";
  make_common_operator_threeway(o, m);
  o << "#else\n";
  make_common_operator_compare(o, m);
  make_common_operator_equal(o, m);
  o << "#endif\n";
  make_common_set_param_by_index(o, m);
  make_common_set_param_by_name(o, m);
  make_common_param_is_printable(o, m);
  make_common_param_name(o, m);
//  make_common_param_name_or_alias(o, m);
  make_common_param_value(o, m);
  make_common_is_valid(o, m);
  make_common_is_valid_(o, m);
  make_common_expand(o, m);
  if(m.has_expand_last()){
    make_common_expand_last(o, m);
  }else{
  }
  make_common_precalc_last(o, m);
  if(m.has_tr_begin()){
    make_common_tr_begin(o, m);
  }else{
  }
  if(m.has_final()){
    make_common_final(o, m);
  }else{ untested();
  }
  o  << "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
