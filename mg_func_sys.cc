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
#include "mg_out.h"
#include "m_tokens.h"
#include <globals.h>
//#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class ANALYSIS : public MGVAMS_FUNCTION {
public:
  explicit ANALYSIS() : MGVAMS_FUNCTION(){
    set_label("analysis");
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "analysis";
  }
  std::string code_name()const override{
    return "_f_analysis";
  }
  Token* new_token(Module& m, size_t na)const override{
    m.set_analysis();
    return MGVAMS_FUNCTION::new_token(m, na);
    m.install(this);
    return new Token_CALL(label(), this);
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "(std::string const& what)const {\n";
    o____ "if(what==\"ic\"){\n";
    o______ "return _sim->analysis_is_tran_static();\n";
    o____ "}else if(what==\"static\"){\n";
    o______ "return _sim->analysis_is_static();\n";
    o____ "}else{ untested();\n";
    o______ "incomplete();\n";
    o______ "return false;\n";
    o____ "}\n";
    o__ "}\n";
  }
} analysis;
DISPATCHER<FUNCTION>::INSTALL d_analysis(&function_dispatcher, "analysis", &analysis);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class ABSTIME : public MGVAMS_FUNCTION {
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_SFCALL("$abstime", this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$$abstime";
  }
  std::string code_name()const override{
    return "_f_abstime";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "()const {\n";
    o____ "return _sim->_time0;\n";
    o__ "}\n";
  }
} abstime;
DISPATCHER<FUNCTION>::INSTALL d_abstime(&function_dispatcher, "$abstime", &abstime);
/*--------------------------------------------------------------------------*/
class TEMPERATURE : public MGVAMS_FUNCTION {
public:
  ~TEMPERATURE(){ }
private:
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_SFCALL("$temperature", this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ unreachable();
    return "$$temperature";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "()const {\n";
    o____ "return P_CELSIUS0 + _sim->_temp_c;\n";
    o__ "}\n";
  }
public:
  std::string code_name()const override{
    return "f_temp";
  }
} temperature;
DISPATCHER<FUNCTION>::INSTALL d1(&function_dispatcher, "$temperature", &temperature);
/*--------------------------------------------------------------------------*/
class VT : public MGVAMS_FUNCTION {
  mutable size_t _temp{0};
public:
  ~VT(){
    while(_temp){
      --_temp; temperature.dec_refs(); // hack
    }
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable(); // SFCALL won't eval
    return "$$vt";
  }
  Token* new_token(Module& m, size_t na)const override {
    m.install(&temperature);
    ++_temp; temperature.inc_refs(); // hack.
    m.install(this);
    if(na == 0) {
    }else if(na == size_t(-1)) {
      // na == -1 => no arglist?
    }else if(na == 1) {
    }else{ untested();
      incomplete();
      // syntax error
    }

    return new Token_SFCALL("$vt", this); // na?
  }
  std::string code_name()const override{
    return "_f_vt";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "()const {\n";
    o____ "return P_K * " << temperature.code_name() << "() / P_Q;\n";
    o__ "}\n";
    o__ "double " << code_name() << "(double T)const {\n";
    o____ "assert(T>=-P_CELSIUS0);\n";
    o____ "(void)T;\n";
    o____ "return P_K * " << temperature.code_name() << "() / P_Q;\n";
    o__ "}\n";
  }
} vt;
DISPATCHER<FUNCTION>::INSTALL d_vt(&function_dispatcher, "$vt", &vt);
/*--------------------------------------------------------------------------*/
class PARAM_GIVEN : public MGVAMS_FUNCTION {
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_SFCALL("$param_given", this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$param_given";
  }
  std::string code_name()const override{
    return "param_given";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "bool " << code_name() << "(PARA_BASE const& p)const {\n";
    o____ "return p.has_hard_value();\n";
    o__ "}\n";
  }
} pg;
DISPATCHER<FUNCTION>::INSTALL d_pg(&function_dispatcher, "$param_given", &pg);
/*--------------------------------------------------------------------------*/
class SIMPARAM : public MGVAMS_FUNCTION {
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_SFCALL("$simparam", this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$$simparam";
  }
  std::string code_name()const override{
    return "va::simparam";
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
} simparam;
DISPATCHER<FUNCTION>::INSTALL d_simparam(&function_dispatcher, "$simparam", &simparam);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
