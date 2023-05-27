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
#include <u_function.h>
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class DUMMY : public FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "AAA";
  }
} dummy;
DISPATCHER<FUNCTION>::INSTALL d_pow(&function_dispatcher, "pow", &dummy);
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "abs", &dummy);
DISPATCHER<FUNCTION>::INSTALL d_vt(&function_dispatcher, "$vt", &dummy);
/*--------------------------------------------------------------------------*/
class TEMPERATURE : public FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  return "$$temperature";
  }
} temperature;
DISPATCHER<FUNCTION>::INSTALL d1(&function_dispatcher, "$temperature", &temperature);
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
DISPATCHER<FUNCTION>::INSTALL d_exp(&function_dispatcher, "exp", &p_exp);
/*--------------------------------------------------------------------------*/
class ln : public FUNCTION {
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
} p_ln;
DISPATCHER<FUNCTION>::INSTALL d_ln(&function_dispatcher, "ln|$log", &p_ln);
/*--------------------------------------------------------------------------*/

// not really functions, but syntactically so.
DISPATCHER<FUNCTION>::INSTALL d2(&function_dispatcher, "ddt", &dummy);
DISPATCHER<FUNCTION>::INSTALL d3(&function_dispatcher, "idt", &dummy);
DISPATCHER<FUNCTION>::INSTALL d_limexp(&function_dispatcher, "limexp|$limexp", &dummy);

}
