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
 * Verilog-AMS builtin math functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class STUB : public MGVAMS_FUNCTION {
public:
  explicit STUB(std::string const l) : MGVAMS_FUNCTION() {
    set_label(l);
  }
private:
  std::string code_name()const override{
    if(label()!=""){
      return "";
    }else{
      return "va::" + label();
    }
  }
  std::string eval(CS&, const CARD_LIST*)const override {
	  unreachable();
	  return "AAA";
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
};
/*--------------------------------------------------------------------------*/
// TODO: these are still in m_va.h
STUB acosh("acosh");
DISPATCHER<FUNCTION>::INSTALL d_acosh(&function_dispatcher, "acosh|$acosh", &acosh);
STUB asinh("asinh");
DISPATCHER<FUNCTION>::INSTALL d_asinh(&function_dispatcher, "asinh|$asinh", &asinh);
STUB atanh("atanh");
DISPATCHER<FUNCTION>::INSTALL d_atanh(&function_dispatcher, "atanh|$atanh", &atanh);
STUB atan("atan");
DISPATCHER<FUNCTION>::INSTALL d_atan(&function_dispatcher, "atan|$atan", &atan);
STUB cos("cos");
DISPATCHER<FUNCTION>::INSTALL d_cos(&function_dispatcher, "cos|$cos", &cos);
STUB cosh("cosh");
DISPATCHER<FUNCTION>::INSTALL d_cosh(&function_dispatcher, "cosh|$cosh", &cosh);
STUB sin("sin");
DISPATCHER<FUNCTION>::INSTALL d_sin(&function_dispatcher, "sin|$sin", &sin);
STUB sinh("sinh");
DISPATCHER<FUNCTION>::INSTALL d_sinh(&function_dispatcher, "sinh|$sinh", &sinh);
STUB tanh("tanh");
DISPATCHER<FUNCTION>::INSTALL d_tanh(&function_dispatcher, "tanh|$tanh", &tanh);
STUB min("min");
DISPATCHER<FUNCTION>::INSTALL d_min(&function_dispatcher, "min|$min", &min);
STUB max("max");
DISPATCHER<FUNCTION>::INSTALL d_max(&function_dispatcher, "max|$max", &max);
/*--------------------------------------------------------------------------*/
// TODO: implement small signal noise
STUB white_noise("white_noise");
DISPATCHER<FUNCTION>::INSTALL d_white_noise(&function_dispatcher, "white_noise", &white_noise);
STUB flicker_noise("flicker_noise");
DISPATCHER<FUNCTION>::INSTALL d_flicker_noise(&function_dispatcher, "flicker_noise", &flicker_noise);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class exp : public MGVAMS_FUNCTION {
public:
  explicit exp() : MGVAMS_FUNCTION(){
    set_label("exp");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::exp(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "::set_value(d, std::exp(d));\n";
    o____ "return chain(d, d);\n";
    o__ "}\n";
//    o__ "double " << code_name() << "(double const& d) const{\n";
//    o____ "return std::exp(d);\n";
//    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_exp";
  }
} p_exp;
DISPATCHER<FUNCTION>::INSTALL d_exp(&function_dispatcher, "exp|$exp", &p_exp);
/*--------------------------------------------------------------------------*/
class limexp : public MGVAMS_FUNCTION {
public:
  explicit limexp() : MGVAMS_FUNCTION(){
    set_label("limexp");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::exp(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "const double lt = 80.;\n";
    o____ "double nv;\n";
    o____ "double nd;\n";
    o____ "if(d>lt){\n";
    o______ "nv = std::exp(lt) * (1.0 - lt + d);\n";
    o______ "nd = std::exp(lt);\n";
    o____ "}else if(d<-lt){\n";
    o______ "nv = std::exp(-lt) * (1.0 + lt + d);\n";
    o______ "nd = std::exp(-lt);\n";
    o____ "}else{\n";
    o______ "nv = nd = std::exp(d);\n";
    o____ "}\n";
    // o____ "std::cerr << \"limexp: \" << d << \" \" << nv << \" \" << nd << \"\\n\";";
    o____ "::set_value(d, nv);\n";
    o____ "return chain(d, nd);\n";
    o__ "}\n";
//    o__ "double " << code_name() << "(double const& d) const{\n";
//    o____ "return std::exp(d);\n";
//    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_limexp";
  }
} p_limexp;
DISPATCHER<FUNCTION>::INSTALL d_limexp(&function_dispatcher, "limexp|$limexp", &p_limexp);
/*--------------------------------------------------------------------------*/
class abs : public MGVAMS_FUNCTION {
public:
  explicit abs() : MGVAMS_FUNCTION(){
    set_label("abs");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { untested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::abs(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const {itested();\n";
    o____ "if(d>=0.){itested();\n";
    o____ "}else{itested();\n";
    o______ "d *= -1.;\n";
    o____ "}\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_abs";
  }
} p_abs;
DISPATCHER<FUNCTION>::INSTALL d_abs(&function_dispatcher, "abs|$abs", &p_abs);
/*--------------------------------------------------------------------------*/
class floor : public MGVAMS_FUNCTION {
public:
  explicit floor() : MGVAMS_FUNCTION(){
    set_label("$floor");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::floor(x));
  }
  std::string code_name()const override{
    return "va::floor";
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
} p_floor;
DISPATCHER<FUNCTION>::INSTALL d_floor(&function_dispatcher, "floor|$floor", &p_floor);
/*--------------------------------------------------------------------------*/
class log : public MGVAMS_FUNCTION {
public:
  explicit log() : MGVAMS_FUNCTION(){
    set_label("$log10");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { untested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::log10(x));
  }
  std::string code_name()const override{
    return "va::log10";
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
} p_log;
DISPATCHER<FUNCTION>::INSTALL d_log(&function_dispatcher, "log|$log10", &p_log);
/*--------------------------------------------------------------------------*/
class ln : public MGVAMS_FUNCTION {
public:
  explicit ln() : MGVAMS_FUNCTION(){
    set_label("ln");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::log(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "double l=-1e99;\n";
    o____ "if(d>1e-60){itested();\n";
    o______ "l = std::log(double(d));\n";
    o______ "chain(d, 1./double(d));\n";
    o____ "}else if(d>0){ untested();\n";
    o______ "l=-1e60;\n";
    o______ "chain(d, 1e60);\n";
    o____ "}else{\n";
    o______ "unreachable();\n";
    o______ "l=-1e40;\n";
    o______ "chain(d, 1e40);\n";
    o____ "}\n";
    o____ "::set_value(d, l);\n";
    o____ "return d;\n";
    o____ "}\n";
  }
  std::string code_name()const override{
    return "_f_ln";
  }
} p_ln;
DISPATCHER<FUNCTION>::INSTALL d_ln(&function_dispatcher, "ln|$log", &p_ln);
/*--------------------------------------------------------------------------*/
class pow: public MGVAMS_FUNCTION {
public:
  explicit pow() : MGVAMS_FUNCTION(){
    set_label("pow");
  }
  std::string code_name()const override{ return "va::" + label(); }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {untested();
    trace1("pow", Cmd.tail());
    PARAMETER<double> x, y;
    Cmd >> x >> y;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::pow(x, y));
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
} p_pow;
DISPATCHER<FUNCTION>::INSTALL d_pow(&function_dispatcher, "pow|$pow", &p_pow);
/*--------------------------------------------------------------------------*/
class sqrt : public MGVAMS_FUNCTION {
public:
  explicit sqrt() : MGVAMS_FUNCTION(){
    set_label("sqrt");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::sqrt(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const {itested();\n";
    o______ "if(double(d)>1e-90){ itested();\n";
    o________ "double s = std::sqrt(d);\n";
    o________ "d.value() = s;\n";
    o________ "chain(d, .5/s);\n";
    o______ "}else if(d>0){ untested();\n";
    o________ "chain(d, 5e91);\n";
    o________ "d.value() = std::sqrt(d);\n";
    o______ "}else if(d==0){ itested();\n";
    o________ "chain(d, .5e200);\n";
    o________ "d.value() = 0.;\n";
    o______ "}else{\n";
    o________ "unreachable();\n";
    o________ "chain(d, .5e99);\n";
    o________ "d.value() = 0.;\n";
    o______ "}\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_sqrt";
  }
} p_sqrt;
DISPATCHER<FUNCTION>::INSTALL d_sqrt(&function_dispatcher, "sqrt|$sqrt", &p_sqrt);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
