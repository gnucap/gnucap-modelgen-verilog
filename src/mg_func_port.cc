/*                        -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
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
class PORT_CONNECTED : public FUNCTION_ {
public:
  explicit PORT_CONNECTED() : FUNCTION_() {
    set_label("$port_connected");
  }
  ~PORT_CONNECTED(){ }
private:
  bool static_code()const override {return false;}
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_CALL("$port_connected", this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$port_connected";
  }
  std::string code_name()const override{
    return "d->node_is_connected";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "bool node_is_connected(int i)const override {\n";
    o____ "return _n[i].is_connected();\n";
    o__ "}\n";
  }
  void make_cc_precalc(std::ostream& o)const override {
    o__ "bool node_is_connected(int i)const {\n";
    o____ "return (_d)->node_is_connected(i);\n";
    o__ "}\n";
  }
} pg;
DISPATCHER<FUNCTION>::INSTALL d_pg(&function_dispatcher, "$port_connected", &pg);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
