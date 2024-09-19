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
#include "mg_token.h"
#include <globals.h>
//#include <u_parameter.h>
#include "mg_.h" // BUG
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class ANALYSIS : public FUNCTION_ {
public:
  explicit ANALYSIS() : FUNCTION_(){
    set_label("analysis");
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "analysis";
  }
  std::string code_name()const override{
    return "_f_analysis";
  }
  bool static_code()const override {return false;}
  Token* new_token(Module& m, size_t)const override {
    m.set_analysis();
    m.install(this);
    return new Token_CALL(label(), this);
  }
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
/*--------------------------------------------------------------------------*/
class MFACTOR : public FUNCTION_ {
  mutable Module const* _m{NULL};
public:
  explicit MFACTOR() : FUNCTION_() {
    set_label("mfactor");
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable(); // SFCALL won't eval
    return "$$mfactor";
  }
  Token* new_token(Module& m, size_t)const override {
    _m = &m; // needed?
    m.install(this);
    return new Token_CALL("$mfactor", this);
  }
  std::string code_name()const override{
    return "d->_f_mfactor";
  }
  void make_cc_impl(std::ostream&)const override {
    assert(_m);
    // o << "double " << "PRECALC_" << _m->identifier() << "::_f_mfactor()const {\n";
    // o__ "return _d->mfactor();\n";
    // o << "}\n";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "double " << "_f_mfactor()const {\n";
    o____ "return mfactor();\n";
    o__ "}\n";
    o__ "double " << "_f_mfactor__precalc()const {\n";
    o____ "return mfactor();\n";
    o__ "}\n";
  }
} mfactor;
DISPATCHER<FUNCTION>::INSTALL d_mfactor(&function_dispatcher, "$mfactor", &mfactor);
/*--------------------------------------------------------------------------*/
class TEMPERATURE : public MGVAMS_FUNCTION {
public:
  explicit TEMPERATURE() {
    set_label("$temperature");
  }
  ~TEMPERATURE(){ }
private:
  bool static_code()const override {return true;}
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
class VT : public FUNCTION_ {
  mutable size_t _temp{0};
public:
  explicit VT() : FUNCTION_() {
    set_label("$vt");
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
  bool static_code()const override {return false;}
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

    return new Token_CALL("$vt", this); // na?
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
    : Token_CALL(P, data) { untested();}
  Token* clone()const override { untested();
    return new Token_PG(*this);
  }
  void stack_op(Expression* e)const override{
    Token const* arg=NULL;

    if(!e->size()){ untested();
      assert(e);
      Token_CALL::stack_op(e);
    }else if(auto p = dynamic_cast<Token_PARLIST_ const*>(e->back())){
      if(auto ee = dynamic_cast<Expression const*>(p->data())){ untested();
	arg = ee->back();
      }else if(p->args()){
	arg = p->args()->back();
      }else{ untested();
	unreachable();
      }
    }else{ untested();
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
      }else{ untested();
	Token_CALL::stack_op(e);
      }
    }else if(dynamic_cast<Token_CONSTANT const*>(arg)){ untested();
      assert(0);
      delete e->back();
      e->pop_back();
      Float* f = new Float(0.);
      e->push_back(new Token_CONSTANT("0.", f, ""));
    }else if(arg) { untested();
      incomplete(); // error?
    }else{ untested();
      incomplete(); // error?
    }
  }
};
/*--------------------------------------------------------------------------*/
class PARAM_GIVEN : public FUNCTION_ {
public:
  explicit PARAM_GIVEN() : FUNCTION_() {
    set_label("$param_given");
  }
  ~PARAM_GIVEN(){ }
private:
  bool static_code()const override {return false;}
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_PG(label(), this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$param_given";
  }
  std::string code_name()const override{
    return "param_given";
  }
//   void stack_op(Expression const& args, Expression* out) const override { untested();
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
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
