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
static void make_module_class(std::ostream& o, Module const& m)
{

//  o << "template<>\n"
//       "Module::ddouble chain_rule(Module::ddouble& x, double d)\n{\n";
//  o << ind << "return x.chain_rule(d);\n"
//       "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o << "MOD_" << m.identifier() << "::MOD_" << m.identifier() << "(MOD_" << m.identifier() << " const&p) : COMPONENT(p)\n{\n"
    << ind << "_n = _nodes;\n"
    << ind << "for (int ii = 0; ii < max_nodes() + int_nodes(); ++ii) {\n"
    << ind << ind << "_n[ii] = p._n[ii];\n"
    << ind << "}\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o << "double MOD_" << m.identifier() << "::tr_probe_num(std::string const& n) const\n{\n";
  // no range check whatsoever. debugging/testing, remove later.
  for(int i=0; i<4; ++i){
    o << ind << "if(n == \"v" << i << "\") return _n[" << i << "].v0();\n";
    o << ind << "if(n == \"i" << i << "\") return (&_branches+" << i << "+ 1)->value();\n";
    for(int j=0; j<4; ++j){
      o << ind << "if(n == \"i" << i << "_d"<<j<<"\") return (&_branches+" << i << "+ 1)->d("<<j<<");\n";
    }
  }
  o << ind << "return NOT_VALID;\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o << "void MOD_" << m.identifier() << "::read_voltages()\n{\n";
  for(auto x : m.probes()){
    Probe const* p = x.second;
    assert(p);
    o << ind << "_" << p->name() << " = volts_limited(_n[n_"<< p->pname() <<"], _n[n_"<< p->nname() <<"]);\n";
  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  make_module_default_constructor(o, m);
  o << "// seq blocks\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  for(auto bb : m){ untested();
    assert(bb);
    if(auto ab = dynamic_cast<AnalogBlock const*>(bb)){
      o << "bool MOD_" << m.identifier() << "::do_tr() // Common::tr_eval? AnalogBlock\n{\n";
      o << ind << "clear_branch_contributions();\n";
      o << ind << "read_voltages();\n";
      o << ind << "COMMON_" << m.identifier() << " const* c = "
	<< ind << "prechecked_cast<COMMON_" << m.identifier() << " const*>(common());\n";
      o << ind << "assert(c);\n";
      o << ind << "c->tr_eval_analog(this);\n";
      o << ind << "return true; // converged();\n";
      o << "}\n";
      break;
    }else{
      incomplete();
    }
  }
} // make_module_class
/*--------------------------------------------------------------------------*/
static void make_module_expand(std::ostream& o, Module const& m)
{
  o << "void MOD_" << m.identifier() << "::expand()\n{\n";
// incomplete.
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
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
