/*                        -*- C++ -*-
 * Copyright (C) 2026 Felix Salfelder
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
 * Verilog-AMS $simparam$str function
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
{ untested();
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
namespace {
/*--------------------------------------------------------------------------*/
class SIMPARAM : public MGVAMS_FUNCTION {
public:
  explicit SIMPARAM() {
    set_label("$simparam");
  }
  ~SIMPARAM(){ }
private:
  bool static_code()const override {return true;}
  bool is_common()const override {return false;}
  bool is_constant()const override {return true;}
  // needed?
  void stack_op(Expression* e)const override {
    assert(e);
    assert(e->size());
    Token* parlist = e->back();
    e->pop_back();
    e->push_back(parlist);
    throw Exception("invalid");
  }
  std::string code_name()const override{
    return "_f_simparamstr";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "std::string " << code_name() << "(std::string const& what)const {\n";
    o____ "if(what==\"module\") {\n";
    o______ "return dev_type();\n";
    o____ "}else if(what==\"instance\") {\n";
    o______ "return long_label();\n";
    o____ "}else{\n";
    o______ "return \"path.incomplete\";\n";
    o____ "}\n";
    o__ "}\n";
  }
  Data_Type const* return_type()const override {
    static Data_Type_String r;
    return &r;
  }
} simparam;
DISPATCHER<FUNCTION>::INSTALL d_simparam(&function_dispatcher, "$simparam$str", &simparam);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
