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
#include "mg_token.h"
#include <globals.h>
#include "mg_.h" // BUG
#include "f__.cc"
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class PORT_CONNECTED : public FUNCTION_ {
public:
  explicit PORT_CONNECTED() : FUNCTION_() {
    set_label("$port_connected");
  }
  ~PORT_CONNECTED(){ }
  PORT_CONNECTED* clone()const override { untested();
    return new PORT_CONNECTED(*this);
  }
private:
  bool static_code()const override {return true;}
  Token* new_token(Module&, size_t)const override {
    return nullptr;
  }
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "$port_connected";
  }
  void stack_op(Expression*)const override { untested();
    throw Exception("invalid");
  }
  std::string code_name()const override{
    return "d->node_is_connected";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "bool node_is_connected(int i)const override {\n";
    o____ "trace2(\"conn\", i, n_(i).e_());\n";
    o____ "return n_(i).e_() != INVALID_NODE;\n";
    o__ "}\n";
  }
  Data_Type const* return_type()const override {
    static Data_Type_Int r;
    return &r;
  }
} pg;
DISPATCHER<FUNCTION>::INSTALL d_pg(&function_dispatcher, "$port_connected", &pg);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
