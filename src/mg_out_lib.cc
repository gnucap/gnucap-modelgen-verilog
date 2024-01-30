/*                          -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023, 2024 Felix Salfelder
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
#include "mg_.h" // TODO
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_final_adjust_eval_parameter(std::ostream& o, const Parameter_2& p)
{
  // if (!(p.calculate().empty())) {untested();
  //   out << "    this->" << p.code_name() << " = " << p.calculate() << ";\n";
  // }else{ untested();
  // }
  o__ "{\n";
//  o__ p.type() << " val = ";
  if (!(p.default_val().empty())) {
    // o << p.default_val();
    indent i2;
    make_cc_expression(o, p.default_val().expression());
  }else{ untested();
    o << "NA;";
  }
//  o << ";\n";
  o____ p.type() << " def = " << p.type() << "(t0.value());\n";
  o____ "e_val(&(this->" << p.code_name() << "), ";
  o____ "def , par_scope);\n";
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_final_adjust_eval_local_parameter(std::ostream& o, const Parameter_2& p)
{
  // if (!(p.calculate().empty())) {untested();
  //   out << "    this->" << p.code_name() << " = " << p.calculate() << ";\n";
  // }else{ untested();
  // }
  o__ "{\n";
//  o__ p.type() << " val = ";
  if (!(p.default_val().empty())) {
    // o << p.default_val();
    indent i2;
    make_cc_expression(o, p.default_val().expression());
  }else{ untested();
    o << "NA;";
  }
//  o << ";\n";
  o____ p.type() << " def = " << p.type() << "(t0.value());\n";
  o____ "this->" << p.code_name() << " = def;\n";
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void make_final_adjust_eval_parameter_list(std::ostream& out,
                                           const Parameter_List_Collection& P)
{
  for(auto const& pl : P){
    if(!pl->is_local()){
      for (Parameter_2_List::const_iterator p=pl->begin(); p!=pl->end(); ++p) {
	make_final_adjust_eval_parameter(out, **p);
      }
    }else{
      for (Parameter_2_List::const_iterator p=pl->begin(); p!=pl->end(); ++p) {
	make_final_adjust_eval_local_parameter(out, **p);
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
void make_final_adjust_parameter_list(std::ostream&, const Parameter_1_List&)
{
  unreachable();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_construct_parameter(std::ostream& out, const Parameter_2& p)
{
  if (!(p.default_val().empty())) {
    out << ",\n   " << p.code_name() << "(/*default*/)";
  }else{ untested();
    out << ",\n   " << p.code_name() << "(NA)";
  }
}
/*--------------------------------------------------------------------------*/
void make_construct_parameter_list(std::ostream& out,const Parameter_List_Collection& P)
{
  for(auto const& pl : P){
    for (Parameter_2_List::const_iterator p = pl->begin(); p != pl->end(); ++p) {
      make_construct_parameter(out, **p);
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_get_one_param(std::ostream& out, const Parameter_1& p,
		     const std::string& name)
{ untested();
  if (!(name.empty())) { untested();
    out << "    || get(cmd, \"" << name << "\", &" << p.code_name();
    if (!(p.offset().empty())) {untested();
      incomplete();
      out << ", m_OFFSET, " << p.offset();
    }else if (p.positive()) { untested();
      //incomplete();
      //out << ", mPOSITIVE";
    }else if (p.octal()) {untested();
      incomplete();
      out << ", m_OCTAL";
    }else if (!(p.scale().empty())) {untested();
      incomplete();       
      out << ", m_SCALE, " << p.scale();
    }else{ untested();
    }
    out << ")\n";
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void make_get_param_list(std::ostream& out, const Parameter_1_List& pl)
{ untested();
  for (Parameter_1_List::const_iterator p = pl.begin(); p != pl.end(); ++p) { untested();
    make_get_one_param(out, **p, (**p).user_name());
    make_get_one_param(out, **p, (**p).alt_name());
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_print_one_param(std::ostream& out, const Parameter_1& p)
{ untested();
  if (!(p.user_name().empty())) { untested();
    out << "  print_pair(o, lang, \"" << to_lower(p.user_name()) << "\", "
    	<< p.code_name();

    if (!(p.offset().empty())) {untested();
      out << "-(" << p.offset() << ")";
    }else{ untested();
    }
    if (!(p.scale().empty())) {untested();
      out << "/(" << p.scale() << ")";
    }else{ untested();
    }

    if (!(p.print_test().empty())) { untested();
      out << ", " << p.print_test() << "";
//    }else if (p.default_val() == "NA") { untested();
//      out << ", " << p.code_name() << " != NA";
    }else{ untested();
    }

    out << ");\n";
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void make_print_param_list(std::ostream& out, const Parameter_1_List& pl)
{ untested();
  for (Parameter_1_List::const_iterator p = pl.begin(); p != pl.end(); ++p) { untested();
    make_print_one_param(out, **p);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_print_one_calc_param(std::ostream& out, const Parameter_1& p)
{ untested();
  if (!(p.user_name().empty()) && !(p.calc_print_test().empty())) { untested();
    out << "  if (" << p.calc_print_test() << ")\n  "
      "  o << \"* " << to_lower(p.user_name()) << "=\" "
	<< p.code_name();
    if (!(p.offset().empty())) {untested();
      out << "-(" << p.offset() << ")";
    }else{ untested();
    }
    if (!(p.scale().empty())) {untested();
      out << "/(" << p.scale() << ")";
    }else{ untested();
    }
    out << ";\n";
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void make_print_calc_param_list(std::ostream& out, const Parameter_1_List& pl)
{ untested();
  for (Parameter_1_List::const_iterator p = pl.begin(); p != pl.end(); ++p) { untested();
    make_print_one_calc_param(out, **p);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_copy_construct_parameter(std::ostream&out,const Parameter_2&p)
{
  out << ",\n   " << p.code_name() << "(p." << p.code_name() << ")";
  if(p.aliases().size()) {
    out << ",\n   _s" << p.code_name() << "(p._s" << p.code_name() << ")";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void make_copy_construct_parameter_list(std::ostream& out,
					const Parameter_List_Collection& P)
{
  for(auto const& pl : P){
    if(!pl->is_local()) {
      for (Parameter_2_List::const_iterator p = pl->begin(); p != pl->end(); ++p) {
	make_copy_construct_parameter(out, **p);
      }
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
