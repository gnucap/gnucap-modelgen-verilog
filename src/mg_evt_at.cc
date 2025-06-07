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
class EVT_AT : public FUNCTION_ {
public:
  explicit EVT_AT() : FUNCTION_() {
    set_label("@");
  }
  ~EVT_AT(){ }
private:
  bool static_code()const override {return true;}
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_CALL("@", this);
  }
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "@";
  }
  void stack_op(Expression*)const override { untested();
    throw Exception("invalid");
  }
  std::string code_name()const override{
    return "or_evt";
  }
  void make_cc_common(std::ostream& o)const override {
    o << "  bool or_evt()const {return false;}\n";
    o << "  template<typename ... Args>\n";
    o << "  bool or_evt(bool a, Args... args)const {\n";
    o << "    return a || or_evt(args...);\n";
    o << "  }\n";
 // TODO, keep track of argc. or something like
//       o << "  void or_evt(bool t...)const {\n";
//       o << "    va_list args;\n";
//       o << "    va_start(args, t);\n";
//        [..]
//       o << "    if(va_arg(args,bool)) {
//       o << "    }
//       o << "    va_end(args);\n";
//       o << "  }\n";
  }
} at;
DISPATCHER<FUNCTION>::INSTALL d_at(&function_dispatcher, "@", &at);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
