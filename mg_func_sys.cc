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
/*--------------------------------------------------------------------------*/
class ABSTIME : public MGVAMS_FUNCTION {
public:
  explicit ABSTIME() {
    set_label("abstime");
  }
  ~ABSTIME() { }
private:
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
  explicit TEMPERATURE() {
    set_label("temperature");
  }
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
  explicit VT() {
    set_label("vt");
  }
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
class Token_PG : public Token_CALL {
public:
  explicit Token_PG(const std::string Name, FUNCTION_ const* f = NULL)
    : Token_CALL(Name, f) {}
private:
  explicit Token_PG(const Token_PG& P, Base const* data=NULL)
    : Token_CALL(P, data) {}
  Token* clone()const override {
    return new Token_PG(*this);
  }
  void stack_op(Expression* e)const override{
    Token const* arg=NULL;

    if(!e->size()){ untested();
      assert(e);
      Token_CALL::stack_op(e);
    }else if(auto p = dynamic_cast<Token_PARLIST_ const*>(e->back())){
      if(auto ee = dynamic_cast<Expression const*>(p->data())){
	arg = ee->back();
      }else{ untested();
	unreachable();
      }
    }else{
      unreachable();
    }

    if(auto p = dynamic_cast<Token_PAR_REF const*>(arg)){
      assert(p->item());
      if(auto p2 = dynamic_cast<Parameter_2 const*>(p->item())) {
	if(p2->is_given()){
	  delete e->back();
	  e->pop_back();
	  Float* f = new Float(1.);
	  e->push_back(new Token_CONSTANT("1.", f, ""));
	}else if(p2->is_local()){
	  delete e->back();
	  e->pop_back();
	  Float* f = new Float(0.);
	  e->push_back(new Token_CONSTANT("0.", f, ""));
	}else{
	  Token_CALL::stack_op(e);
	}
      }else{
	Token_CALL::stack_op(e);
      }
    }else if(dynamic_cast<Token_CONSTANT const*>(arg)){ untested();
      assert(0);
      delete e->back();
      e->pop_back();
      Float* f = new Float(0.);
      e->push_back(new Token_CONSTANT("0.", f, ""));
    }else if(arg) {
      incomplete(); // error?
    }else{
      incomplete(); // error?
    }
  }
};
/*--------------------------------------------------------------------------*/
class PARAM_GIVEN : public MGVAMS_FUNCTION {
public:
  explicit PARAM_GIVEN() {
    set_label("param_given");
  }
  ~PARAM_GIVEN(){ }
private:
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_PG("$param_given", this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$param_given";
  }
  std::string code_name()const override{
    return "param_given";
  }
//   void stack_op(Expression const& args, Expression* out) const override {
//     incomplete();
//   }
  void make_cc_common(std::ostream& o)const override {
    o__ "bool " << code_name() << "(PARA_BASE const& p)const {\n";
    o____ "return p.has_hard_value();\n";
    o__ "}\n";
  }
} pg;
DISPATCHER<FUNCTION>::INSTALL d_pg(&function_dispatcher, "$param_given", &pg);
/*--------------------------------------------------------------------------*/
class SIMPARAM : public MGVAMS_FUNCTION {
public:
  explicit SIMPARAM() {
    set_label("simparam");
  }
  ~SIMPARAM(){ }
private:
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_SFCALL("$simparam", this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$$simparam";
  }
  std::string code_name()const override{
    return "_f_simparam";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "(std::string const& what, double def=0)const {\n";
    o____ "if(what==\"gmin\") {\n";
    o______ "return OPT::gmin;\n";
    o____ "}else if(what==\"iteration\") {\n";
    o______ "return CKT_BASE::_sim->_iter[sCOUNT];\n";
    o____ "}else{\n";
    o______ "return def;\n";
    o____ "}\n";
    o__ "}\n";
  }
} simparam;
DISPATCHER<FUNCTION>::INSTALL d_simparam(&function_dispatcher, "$simparam", &simparam);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
