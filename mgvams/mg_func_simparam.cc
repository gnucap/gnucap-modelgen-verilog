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
namespace {
/*--------------------------------------------------------------------------*/
class SIMPARAM : public MGVAMS_FUNCTION {
  mutable bool _const{true};
public:
  explicit SIMPARAM() {
    set_label("$simparam");
  }
  ~SIMPARAM(){ }
private:
  bool static_code()const override {return true;}
  bool is_constant()const override {return _const;}
  void stack_op(Expression* e)const override {
    _const = true;
    assert(e);
    assert(e->size());
    trace1("simparam", typeid(*e->back()).name());
    Token* parlist = e->back();
    e->pop_back();
    if(dynamic_cast<Token_PARLIST const*>(parlist)){
      assert(e->size());
      Token* b = e->back();
      e->pop_back();
      Token const* arg = e->back();

      if(dynamic_cast<Token_STOP const*>(arg)){
	arg = b;
      }else{
      }

      if(auto c = dynamic_cast<Token_CONSTANT const*>(arg)){
	auto s = dynamic_cast<String const*>(c->data());
	trace1("simparam const?", c->name());
	if(!s){ untested();
	}else if(std::string(*s) == "gmin"){
	  trace1("simparam gmin", c->name());
	  _const = false;
	}else{
	}
      }else{
	trace1("simparam noconst", b->name());
      }
      e->push_back(b);
    }else{
      trace1("simparam nopar", typeid(*e->back()).name());
    }
    e->push_back(parlist);
    throw Exception("invalid");
  }
  std::string code_name()const override{
    return "_f_simparam";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "double " << code_name() << "(std::string const& what, double def=0)const {\n";
    o____ "if(what==\"gmin\") {\n";
    o______ "return OPT::gmin;\n";
    o____ "}else if(what==\"scale\") {\n";
    o______ "return OPT::scale;\n";
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
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
