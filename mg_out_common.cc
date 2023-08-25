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
#include <numeric>
/*--------------------------------------------------------------------------*/
static void make_common_default_constructor(std::ostream& out, const Module& d)
{
  make_tag();
  out <<
    "COMMON_" << d.identifier() << "::COMMON_" << d.identifier() << "(int c)\n"
    "  :COMMON_COMPONENT(c)";
  make_construct_parameter_list(out, d.parameters());
//  out << ",\n   _sdp(0)";
  // make_construct_parameter_list(out, d.common().calculated());
//  for (Args_List::const_iterator
//       p = d.circuit().args_list().begin();
//       p != d.circuit().args_list().end();
//       ++p) {
//    out << ",\n   _" << (**p).name() << "(0)";
//  }
  out <<
    "\n"
    "{\n"
    "  ++_count;\n";
//  for (Parameter_1_List::const_iterator
//       p = d.common().override().begin();
//       p != d.common().override().end();
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
  out <<
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_copy_constructor(std::ostream& out, const Module& d)
{
  make_tag();
  out <<
    "COMMON_" << d.identifier() << "::COMMON_" << d.identifier() << "(const COMMON_" << d.identifier() << "& p)\n"
    "  :COMMON_COMPONENT(p)";
  make_copy_construct_parameter_list(out, d.parameters());
  //out << ",\n   _sdp(0)";
  //make_copy_construct_parameter_list(out, d.common().calculated());
//  for (Args_List::const_iterator
//       p = d.circuit().args_list().begin();
//       p != d.circuit().args_list().end();
//       ++p) {
//    out << ",\n   _" << (**p).name() << "(0)";
//  }
  out << 
    "\n"
    "{\n"
    "  ++_count;\n";
//  for (Parameter_1_List::const_iterator
//       p = d.common().override().begin();
//       p != d.common().override().end();
//       ++p) {untested();
//    out << ",\n   " << (**p).code_name() << "(p." << (**p).code_name() << ")";
//  }
  out <<
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_destructor(std::ostream& out, const Module& d)
{
  make_tag();
  out <<
    "COMMON_" << d.identifier() << "::~COMMON_" << d.identifier() << "()\n"
    "{\n";
//   for (Args_List::const_iterator
//        p = d.circuit().args_list().begin();
//        p != d.circuit().args_list().end();
//        ++p) {
//     out << "  detach_common(&_" << (**p).name() << ");\n";
//   }
  out <<
    "  --_count;\n"
//    "  delete _sdp;\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_operator_equal(std::ostream& out, const Module& d)
{
  make_tag();
  out <<
    "bool COMMON_" << d.identifier() << "::operator==(const COMMON_COMPONENT& x)const\n"
    "{\n"
    "  const COMMON_" << d.identifier() << "* p = dynamic_cast<const COMMON_" << d.identifier() << "*>(&x);\n"
    "  return (p\n";
  for (Parameter_List_Collection::const_iterator
       q = d.parameters().begin();
       q != d.parameters().end();
       ++q) {
    for (Parameter_2_List::const_iterator
	 p = (*q)->begin();
	 p != (*q)->end();
	 ++p) {
      out << "    && " << (**p).code_name() << " == p->" << (**p).code_name() << '\n';
    }
  }
  out << 
//    "    && _sdp == p->_sdp\n"
    "    && COMMON_COMPONENT::operator==(x));\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_set_param_by_name(std::ostream& o, const Module& m)
{
  o << "void COMMON_" << m.identifier() << "::set_param_by_name("
       "std::string Name, std::string Value)\n{\n";

  // BUG, mix into name/alias map below
  o__ "if(Name == \"$mfactor\"){ Name = \"m\"; }\n";

  o__ "static std::string names[] = {";
  int cnt = 0;
  std::vector<std::string const*> names;
  std::vector<int> alias;
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
  cnt = names.size();
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
    if(alias[n]){
      pn = "_p_" + *names[n - alias[n] + 1];
      o << "\n";
      o____ "if(!" << pn << ".has_hard_value()) {\n";
      o______ "_s" << pn << " = " << alias[n] << ";\n";
      o____ "}else if(_s" << pn << " != " << alias[n] << "){\n";
      o______ "throw Exception_No_Match(\"..\");\n";
      o____ "}else{\n";
      o____ "}\n";
      o____ "";
    }else{
      pn = cn;
    }
    o << pn << " = Value;";
    o____ "break; // " << alias[n] << "\n";
    --cnt;
  }
  o____ "case 0: COMMON_COMPONENT::set_param_by_name(Name, Value); break;\n";


  o__ "}\n";

  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_set_param_by_index(std::ostream& o, const Module& m)
{
  make_tag();
  o << "void COMMON_" << m.identifier() << "::set_param_by_index("
       "int I, std::string& Value, int Offset)\n{\n";
  o << "  switch (COMMON_" << m.identifier() << "::param_count() - 1 - I) {\n";
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

  o << "  default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset);\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_param_is_printable(std::ostream& out, const Module& m)
{
  make_tag();
  out <<
    "bool COMMON_" << m.identifier() << "::param_is_printable(int i)const\n"
    "{\n"
    "  switch (COMMON_" << m.identifier() << "::param_count() - 1 - i) {\n";
  size_t i = 0;
//  for (Parameter_1_List::const_iterator 
//       p = m.common().override().begin(); 
//       p != m.common().override().end();
//       ++p) {
//    if (!((**p).user_name().empty())) {
//      out << "  case " << i++ << ":  return (";
//      if (!((**p).print_test().empty())) {
//	out << (**p).print_test() << ");\n";
//      }else if ((**p).default_val() == "NA") {
//	out << (**p).code_name() << " != NA);\n";
//      }else{
//	out << "true);\n";
//      }
//    }else{unreachable();
//    }
//  }
//  assert(i == m.common().override().size());
  for (auto q = m.parameters().begin();
       q != m.parameters().end(); ++q) {
    if(!(*q)->is_local()) {
      for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
	out << "  case " << i++ << ":  return (";
	if (!((**p).print_test().empty())) {
	  out << (**p).print_test() << ");\n";
	    //      }else if ((**p).default_val() == "NA") {
	    //	out << (**p).code_name() << " != NA);\n";
	}else{
	  //out << "true);\n";
	  out << (*p)->code_name() << ".has_hard_value());\n";
	}
      }
    }
  }
//  assert(i == m.common().override().size() + m.common().raw().size());
  out <<
    "  default: return COMMON_COMPONENT::param_is_printable(i);\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_common_param_name(std::ostream& o, const Module& m)
{
  make_tag();
  o <<
    "std::string COMMON_" << m.identifier() << "::param_name(int i)const\n{\n"
    "  switch (COMMON_" << m.identifier() << "::param_count() - 1 - i) {\n";
  size_t i = 0;
//  for (Parameter_1_List::const_iterator 
//       p = m.common().override().begin(); 
//       p != m.common().override().end();
//       ++p) {
//    if (!((**p).user_name().empty())) {
//      o << "  case " << i++ << ":  return \"" << to_lower((**p).user_name()) << "\";\n";
//    }else{unreachable();
//    }
//  }
//  assert(i == m.common().override().size());
  for (auto q = m.parameters().begin();
       q != m.parameters().end();
       ++q)
  for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
    o << "  case " << i++ << ":  return \"" << (**p).name() << "\";\n";
  }
//  assert(i == m.common().override().size() + m.common().raw().size());
  o <<
    "  default: return COMMON_COMPONENT::param_name(i);\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
  o <<
    "std::string COMMON_" << m.identifier() << "::param_name(int i, int j)const\n"
    "{\n";
  o__ "if(j==0){\n";
  o____ "return param_name(i);\n";
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
    make_cc_expression(o, ri->lb().expression());
    o__ "lb = " << "t0" << ";\n";
    o__ "}\n";
    o__ "{\n";
    make_cc_expression(o, ri->ub().expression());
    o__ "ub = " << "t0" << ";\n";
    o__ "}\n";

  }else{
  }
}
/*--------------------------------------------------------------------------*/
static void make_param_check_range(std::ostream& o, ValueRange const& p,
    std::string const& n)
{
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
  }else if(auto c = dynamic_cast<ValueRangeConstant const*>(spec)){
    o << "(" << c->expr() << "==" << n << ")";
  }else{
    incomplete();
    assert(false);
  }
}
/*--------------------------------------------------------------------------*/
static void make_common_is_valid(std::ostream& o, const Module& m)
{
  make_tag();
  o << "bool COMMON_" << m.identifier() << "::is_valid() const\n{\n";
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
	o__ "if(";
	if(v->is_from()){
	  o << "!";
	}else if(v->is_exclude()){
	}else{
	  unreachable();
	}
	make_param_check_range(o, *v, (*p)->code_name());
	o << "){ return false; }else{ }\n";
	o__ "}\n";
      }
    }
  }

  o__ "return true; //COMMON_COMPONENT::is_valid();\n";
  o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
#if 0
void make_common_param_name_or_alias(std::ostream& out, const Device& m)
{
  make_tag();
  out <<
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
       ++p) {
    if (!((**p).user_name().empty())) {
      out << "    case " << i++ << ":  return \"" << to_lower((**p).alt_name()) << "\";\n";
    }else{unreachable();
    }
  }
  assert(i == d.common().override().size());
  for (Parameter_1_List::const_iterator 
       p = d.common().raw().begin(); 
       p != d.common().raw().end();
       ++p) {
    if (!((**p).user_name().empty())) {
      out << "    case " << i++ << ":  return \"" << to_lower((**p).alt_name()) << "\";\n";
    }else{unreachable();
    }
  }
  assert(i == d.common().override().size() + d.common().raw().size());
  out <<
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
void make_common_param_value(std::ostream& out, const Module& m)
{
  make_tag();
  out <<
    "std::string COMMON_" << m.identifier() << "::param_value(int i)const\n"
    "{\n"
    "  switch (COMMON_" << m.identifier() << "::param_count() - 1 - i) {\n";
  size_t i = 0;
//  for (Parameter_1_List::const_iterator 
//       p = m.common().override().begin(); 
//       p != m.common().override().end();
//       ++p) {
//    if (!((**p).user_name().empty())) {
//      out << "  case " << i++ << ":  return " << (**p).code_name() << ".string();\n";
//    }else{unreachable();
//    }
//  }
//  assert(i == m.common().override().size());
  for (auto q = m.parameters().begin();
       q != m.parameters().end();
       ++q)
    if(!(*q)->is_local())
  for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      out << "  case " << i++ << ":  return " << (**p).code_name() << ".string();\n";
  }
//  assert(i == m.common().override().size() + m.common().raw().size());
  out <<
    "  default: return COMMON_COMPONENT::param_value(i);\n"
    "  }\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_eval_subdevice_parameters(std::ostream& o , const Element_2& e)
{
  for(auto p : e.list_of_parameter_assignments()){
    o__ "{\n";
    {
      indent x;
      make_cc_expression(o, p->default_val().expression());
    }
    o____ "_netlist_params.set(\"["<< e.short_label() <<"]"<< p->name() <<"\", t0);\n";
    o__ "}\n";
  }

}
/*--------------------------------------------------------------------------*/
static void make_eval_netlist_parameters(std::ostream& o , const Module& m)
{
  for(auto i : m.element_list()){
    make_eval_subdevice_parameters(o, *i);
  }
}
/*--------------------------------------------------------------------------*/
static void make_common_expand(std::ostream& o , const Module& m)
{
  make_tag();
  o  <<
    "void COMMON_" << m.identifier() << "::expand(const COMPONENT* d)\n{\n";
    o__ "COMMON_COMPONENT::expand(d);\n"
    "}\n";
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
       ++p) {
    out << "  COMMON_" << (**p).type() << "* " << (**p).name() 
	<< " = new COMMON_" << (**p).type() << ";\n";
    for (Arg_List::const_iterator
	 a = (**p).begin();
	 a != (**p).end();
	 ++a) {
      out << "  " << (**p).name() << "->" << (**a).arg() << ";\n";
    }
    out << "  attach_common(" << (**p).name() << ", &_" << (**p).name() << ");\n\n";
  }
  o  <<
    "  assert(c == this);\n"
    "}\n"


#endif
  o << "/*--------------------------------------------------------------------------*/\n";
  o << "void COMMON_" << m.identifier() << "::precalc_first(const CARD_LIST* par_scope)\n{\n";
  o__ "assert(par_scope);\n";
  o__ "COMMON_COMPONENT::precalc_first(par_scope);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";
  make_final_adjust_eval_parameter_list(o , m.parameters());
  make_eval_netlist_parameters(o, m);
  o  << "}\n"
    "/*--------------------------------------------------------------------------*/\n";

  o << "void COMMON_" << m.identifier() << "::precalc_last(const CARD_LIST* par_scope)\n{\n";
  o__ "assert(par_scope);\n"
    "  COMMON_COMPONENT::precalc_last(par_scope);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";
  make_final_adjust_eval_parameter_list(o , m.parameters());
  make_eval_netlist_parameters(o, m);
    o << "}\n"
    "/*--------------------------------------------------------------------------*/\n";
#if 0
    "  COMMON_" << d.name() << "* c = this;\n"
    "  const MODEL_" << d.model_type() << "* m = prechecked_cast<const MODEL_" 
      << d.model_type() << "*>(model());\n";

  make_final_adjust(out, d.common());

  out <<
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
       ++p) {
    out << "  COMMON_" << (**p).type() << "* " << (**p).name() 
	<< " = new COMMON_" << (**p).type() << ";\n";
    for (Arg_List::const_iterator
	 a = (**p).begin();
	 a != (**p).end();
	 ++a) {
      out << "  " << (**p).name() << "->" << (**a).arg() << ";\n";
    }
    out << "  attach_common(" << (**p).name() << ", &_" << (**p).name() << ");\n\n";
  }

  out <<
    "  assert(c == this);\n"
    "}\n"
    "/*--------------------------------------------------------------------------*/\n";
#endif
}
/*--------------------------------------------------------------------------*/
void make_cc_common(std::ostream& o , const Module& m)
{
  make_tag();
  make_common_default_constructor(o, m);
  make_common_copy_constructor(o, m);
  make_common_destructor(o, m);
  make_common_operator_equal(o, m);
  make_common_set_param_by_index(o, m);
  make_common_set_param_by_name(o, m);
  make_common_param_is_printable(o, m);
  make_common_param_name(o, m);
//  make_common_param_name_or_alias(o, m);
  make_common_param_value(o, m);
  make_common_is_valid(o, m);
  make_common_expand(o, m);
  o  << "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
