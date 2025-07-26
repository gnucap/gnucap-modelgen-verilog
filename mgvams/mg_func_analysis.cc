/*                        -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 * Author: Felix Salfelder
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
 *------------------------------------------------------------------
 * Verilog-AMS system functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.cc"
#include "mg_token.h"
#include <globals.h>
#include "mg_.h" // BUG
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class ANALYSIS : public FUNCTION_ {
public:
  explicit ANALYSIS() : FUNCTION_(){
    set_label("analysis");
  }
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "analysis";
  }
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  std::string code_name()const override{
    return "_f_analysis";
  }
  bool static_code()const override {return true;}
  bool is_common()const override {return true;}
  bool has_state()const override { untested();return false;}
  bool has_analysis()const override {return true;}
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "(std::string const& what)const {\n";
    o____ "if(what==\"ic\"){\n";
    o______ "return _sim->analysis_is_tran_static();\n";
    o____ "}else if(what==\"static\"){\n";
    o______ "return _sim->analysis_is_static();\n";
    o____ "}else if(what==\"dc\"){\n";
    o______ "return _sim->analysis_is_dcop();\n";
    o____ "}else if(what==\"noise\"){ itested();\n";
    o______ "return false; // later\n";
    o____ "}else{ untested();\n";
    o______ "incomplete();\n";
    o______ "return false;\n";
    o____ "}\n";
    o__ "}\n";
  }
} analysis;
DISPATCHER<FUNCTION>::INSTALL d_analysis(&function_dispatcher, "analysis", &analysis);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
