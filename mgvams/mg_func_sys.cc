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
 * Verilog-AMS builtin system functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_token.h"
#include "mg_module.h"
#include <globals.h>
#include <u_parameter.h>
#include "f__.cc"
/*--------------------------------------------------------------------------*/
bool Statement::propagate_rdep(Base const* )
{
  unreachable();
  incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
// namespace?
/*--------------------------------------------------------------------------*/
// inline Token* MGVAMS_FUNCTION::new_token(Module& m, size_t na) const
// { untested();
//      incomplete();
//      return NULL;
// }
/*--------------------------------------------------------------------------*/
class ABSTIME : public MGVAMS_FUNCTION {
public:
  explicit ABSTIME() {
    set_label("$abstime");
  }
  ~ABSTIME() { }
private:
  bool static_code()const override {return true;}
//  Token* new_token(Module& m, size_t)const override { untested();
//    m.install(this);
//    return new Token_CALL("$abstime", this);
//  }
  void stack_op(Expression*)const override {
    throw Exception("invalid");
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
class MFACTOR : public MGVAMS_FUNCTION {
  mutable Module const* _m{nullptr};
public:
  explicit MFACTOR() : MGVAMS_FUNCTION() {
    set_label("$mfactor");
  }
private:
  bool static_code()const override {return true;}
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable(); // SFCALL won't eval
    return "$$mfactor";
  }
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  std::string code_name()const override {
    return "d->_f_mfactor";
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
class SIMPARAM : public MGVAMS_FUNCTION {
public:
  explicit SIMPARAM() {
    set_label("$simparam");
  }
  ~SIMPARAM(){ }
private:
  bool static_code()const override {return true;}
  bool is_constant()const override {return false;}
  void stack_op(Expression*)const override {
    throw Exception("invalid");
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
class TEMPERATURE : public MGVAMS_FUNCTION {
public:
  explicit TEMPERATURE() {
    set_label("$temperature");
  }
  ~TEMPERATURE(){ }
private:
  bool static_code()const override {return true;}
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "()const {\n";
    o____ "return temp_k();\n";
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
public:
  explicit VT() : MGVAMS_FUNCTION() {
    set_label("$vt");
  }
  ~VT(){ }
private:
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  bool static_code()const override {return true;}
  std::string code_name()const override{
    return "_f_vt";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "()const {\n";
    o____ "return P_K * temp_k() / P_Q;\n";
    o__ "}\n";
    o__ "double " << code_name() << "(double T)const {\n";
    o____ "assert(T>=-P_CELSIUS0);\n";
    o____ "return P_K * T / P_Q;\n";
    o__ "}\n";
  }
} vt;
DISPATCHER<FUNCTION>::INSTALL d_vt(&function_dispatcher, "$vt", &vt);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
