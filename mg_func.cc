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
 * Verilog-AMS builtin functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class DUMMY1 : public MGVAMS_FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "AAA";
  }
//  int arity()const override {
//	  return 1;
//  }
} dummy1;
DISPATCHER<FUNCTION>::INSTALL d_abs(&function_dispatcher, "abs|$abs", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_acosh(&function_dispatcher, "acosh|$acosh", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_asinh(&function_dispatcher, "asinh|$asinh", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_atanh(&function_dispatcher, "atanh|$atanh", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_atan(&function_dispatcher, "atan|$atan", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_cos(&function_dispatcher, "cos|$cos", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_cosh(&function_dispatcher, "cosh|$cosh", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_limexp(&function_dispatcher, "limexp|$limexp", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_sin(&function_dispatcher, "sin|$sin", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_sinh(&function_dispatcher, "sinh|$sinh", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_sqrt(&function_dispatcher, "sqrt|$sqrt", &dummy1);
DISPATCHER<FUNCTION>::INSTALL d_tanh(&function_dispatcher, "tanh|$tanh", &dummy1);
/*--------------------------------------------------------------------------*/
class DUMMY2 : public MGVAMS_FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "AAA";
  }
//  int arity()const override {
//	  return 2;
//  }
} dummy2;
DISPATCHER<FUNCTION>::INSTALL d_min(&function_dispatcher, "min|$min", &dummy2);
DISPATCHER<FUNCTION>::INSTALL d_max(&function_dispatcher, "max|$max", &dummy2);
/*--------------------------------------------------------------------------*/
class PARAM_GIVEN : public MGVAMS_FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  return "$param_given";
  }
  std::string code_name()const override{
	  return "param_given";
  }
} pg;
DISPATCHER<FUNCTION>::INSTALL d_pg(&function_dispatcher, "$param_given", &pg);
/*--------------------------------------------------------------------------*/
class SIMPARAM : public MGVAMS_FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  return "$$simparam";
  }
  std::string code_name()const override{
	  return "va::simparam";
  }
} simparam;
DISPATCHER<FUNCTION>::INSTALL d_simparam(&function_dispatcher, "$simparam", &simparam);
/*--------------------------------------------------------------------------*/
class STROBE : public MGVAMS_FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  return "$$strobe";
  }
  std::string code_name()const override{
	  return "temp_hack";
  }
} strobe;
DISPATCHER<FUNCTION>::INSTALL d_strobe(&function_dispatcher, "$strobe", &strobe);
/*--------------------------------------------------------------------------*/
class TEMPERATURE : public MGVAMS_FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  return "$$temperature";
  }
//  int arity()const override {
//	  return 0;
//  }
  std::string code_name()const override{
	  return "temp_hack";
  }
} temperature;
DISPATCHER<FUNCTION>::INSTALL d1(&function_dispatcher, "$temperature", &temperature);
/*--------------------------------------------------------------------------*/
class VT : public MGVAMS_FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  return "$$vt";
  }
//  int arity()const override {
//	  return 0;
//  }
  std::string code_name()const override{
	  return "vt_hack";
  }
} vt;
DISPATCHER<FUNCTION>::INSTALL d_vt(&function_dispatcher, "$vt", &vt);
/*--------------------------------------------------------------------------*/
class exp : public FUNCTION {
public:
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::exp(x));
  }
} p_exp;
DISPATCHER<FUNCTION>::INSTALL d_exp(&function_dispatcher, "exp|$exp", &p_exp);
/*--------------------------------------------------------------------------*/
class floor : public MGVAMS_FUNCTION {
public:
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::floor(x));
  }
  std::string const& name() const{ untested();
	  static std::string n = "$floor";
	  return n;
  }
  std::string code_name()const override{
	  return "va::floor";
  }
} p_floor;
DISPATCHER<FUNCTION>::INSTALL d_floor(&function_dispatcher, "floor|$floor", &p_floor);
/*--------------------------------------------------------------------------*/
class log : public MGVAMS_FUNCTION {
public:
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { untested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::log10(x));
  }
  std::string const& name() const{ untested();
	  static std::string n = "$log10";
	  return n;
  }
  std::string code_name()const override{
	  return "va::log10";
  }
//  int arity()const override {
//	  return 1;
//  }
} p_log;
DISPATCHER<FUNCTION>::INSTALL d_log(&function_dispatcher, "log|$log10", &p_log);
/*--------------------------------------------------------------------------*/
class ln : public MGVAMS_FUNCTION {
public:
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { untested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::log(x));
  }
  std::string const& name() const{ untested();
	  static std::string n = "$log";
	  return n;
  }
//  int arity()const override {
//	  return 1;
//  }
} p_ln;
DISPATCHER<FUNCTION>::INSTALL d_ln(&function_dispatcher, "ln|$log", &p_ln);
/*--------------------------------------------------------------------------*/
class pow: public MGVAMS_FUNCTION {
public:
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {untested();
	  trace1("pow", Cmd.tail());
    PARAMETER<double> x, y;
    Cmd >> x >> y;
    x.e_val(NOT_INPUT, Scope);
	  trace2("pow", x, y);
    return to_string(std::pow(x, y));
  }
  std::string const& name() const{ untested();
	  static std::string n = "$log";
	  return n;
  }
//  int arity()const override {
//	  return 2;
//  }
} p_pow;
DISPATCHER<FUNCTION>::INSTALL d_pow(&function_dispatcher, "pow", &p_pow);
/*--------------------------------------------------------------------------*/

// not really functions, but syntactically so.
class DUMMYFILTER : public FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "filter";
  }
} dummyfilter;
DISPATCHER<FUNCTION>::INSTALL d_ddt(&function_dispatcher, "ddt", &dummyfilter);
DISPATCHER<FUNCTION>::INSTALL d_idt(&function_dispatcher, "idt", &dummyfilter);
DISPATCHER<FUNCTION>::INSTALL d_ddx(&function_dispatcher, "ddx", &dummyfilter);
DISPATCHER<FUNCTION>::INSTALL d_white_noise(&function_dispatcher, "white_noise", &dummyfilter);
DISPATCHER<FUNCTION>::INSTALL d_flicker_noise(&function_dispatcher, "flicker_noise", &dummyfilter);

}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
