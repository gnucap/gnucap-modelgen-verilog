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
//testing=script 2006.11.01
#include "mg_out.h"
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
void make_common_set_param_by_index(std::ostream& out, const Module& m)
{
  make_tag();
  out <<
    "void COMMON_" << m.identifier() << "::set_param_by_index(int I, std::string& Value, int Offset)\n"
    "{\n"
    "  switch (COMMON_" << m.identifier() << "::param_count() - 1 - I) {\n";
  size_t i = 0;
//  for (Parameter_1_List::const_iterator 
//       p = d.common().override().begin(); 
//       p != d.common().override().end();
//       ++p) {
//    if (!((**p).user_name().empty())) {
//      out << "  case " << i++ << ":  " << (**p).code_name() << " = Value; break;\n";
//    }else{unreachable();
//    }
//  }
//  assert(i == d.common().override().size());
  for (Parameter_List_Collection::const_iterator
       q = m.parameters().begin();
       q != m.parameters().end();
       ++q) {
    if(!(*q)->is_local())
    for (Parameter_2_List::const_iterator
	 p = (*q)->begin();
	 p != (*q)->end();
	 ++p) {
      out << "  case " << i++ << ":  " << (**p).code_name() << " = Value; break;\n";
    }
  }

  out <<
    "  default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset);\n"
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
       q != m.parameters().end();
       ++q)
    if(!(*q)->is_local())
  for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
    {
      out << "  case " << i++ << ":  return (";
      if (!((**p).print_test().empty())) {
	out << (**p).print_test() << ");\n";
//      }else if ((**p).default_val() == "NA") {
//	out << (**p).code_name() << " != NA);\n";
      }else{
	out << "true);\n";
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
static void make_common_expand(std::ostream& o , const Module& m)
{
  make_tag();
  o  <<
    "void COMMON_" << m.identifier() << "::expand(const COMPONENT* d)\n{\n"

    "}\n";
#if 0
    "  COMMON_COMPONENT::expand(d);\n"
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
  o  <<
    "/*--------------------------------------------------------------------------*/\n"
    "void COMMON_" << m.identifier() << "::precalc_first(const CARD_LIST* par_scope)\n"
    "{\n"
    "  assert(par_scope);\n"
    "  COMMON_COMPONENT::precalc_first(par_scope);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  make_final_adjust_eval_parameter_list(o , m.parameters());
    o  << "}\n";

  o <<
    "/*--------------------------------------------------------------------------*/\n"
    "void COMMON_" << m.identifier() << "::precalc_last(const CARD_LIST* par_scope)\n"
    "{\n"
    "  assert(par_scope);\n"
    "  COMMON_COMPONENT::precalc_last(par_scope);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  make_final_adjust_eval_parameter_list(o , m.parameters());
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
static void make_common_tr_eval(std::ostream& o, const Module& m)
{
  o << "void COMMON_" << m.identifier() << 
    "::tr_eval_analog(MOD_" << m.identifier() << "* d) const\n{\n";

  // parameters are here.
  o__ "assert(d);\n";
  o__ "MOD_" << m.identifier() << " const* p = d;\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";

  for(auto bb : m.analog_list()){
    assert(bb);
    if(auto ab = dynamic_cast<AnalogBlock const*>(bb)){
      o << ind << "{\n";
      {
	indent a(2);
	make_cc_analog(o, *ab);
      }
      o << ind << "}\n";
    }else{
    }
  }
  o << "}\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_common(std::ostream& o , const Module& d)
{
  make_tag();
  make_common_default_constructor(o , d);
  make_common_copy_constructor(o , d);
  make_common_destructor(o , d);
  make_common_operator_equal(o , d);
  make_common_set_param_by_index(o , d);
  make_common_param_is_printable(o , d);
  make_common_param_name(o , d);
//  make_common_param_name_or_alias(o , d);
  make_common_param_value(o , d);
  make_common_expand(o , d);
  make_common_tr_eval(o , d);
  o  << "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
