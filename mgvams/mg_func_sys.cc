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
#include "mg_out.cc"
#include "mg_token.h"
#include "mg_module.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
// inline Token* MGVAMS_FUNCTION::new_token(Module& m, size_t na) const
// {
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
//  Token* new_token(Module& m, size_t)const override {
//    m.install(this);
//    return new Token_CALL("$abstime", this);
//  }
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
class SIMPARAM : public MGVAMS_FUNCTION {
public:
  explicit SIMPARAM() {
    set_label("$simparam");
  }
  ~SIMPARAM(){ }
private:
  bool static_code()const override {return true;}
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
/*--------------------------------------------------------------------------*/
