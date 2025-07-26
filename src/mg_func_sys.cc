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
/*--------------------------------------------------------------------------*/
class Token_PG : public Token_CALL {
public:
  explicit Token_PG(const std::string Name, FUNCTION_ const* f = nullptr)
    : Token_CALL(Name, f) {}
private:
  explicit Token_PG(const Token_PG& P, Base const* data=nullptr)
    : Token_CALL(P, data) { untested();}
  Token* clone()const override { untested();
    return new Token_PG(*this);
  }
  void stack_op(Expression* e)const override{
    Token const* arg=nullptr;

    if(!e->size()){ untested();
      assert(e);
      Token_CALL::stack_op(e);
    }else if(auto p = dynamic_cast<Token_PARLIST_ const*>(e->back())){
      if(auto ee = dynamic_cast<Expression const*>(p->data())){ untested();
	arg = ee->back();
      }else if(p->args()){
	arg = p->args()->back();
      }else{ untested();
	unreachable();
      }
    }else{ untested();
      unreachable();
    }

    if(auto p = dynamic_cast<Token_PAR_REF const*>(arg)){
      assert(p->item());
      if(auto p2 = dynamic_cast<Parameter_2 const*>(p->item())) {
	if(p2->is_given()){
	  delete e->back();
	  e->pop_back();
	  Float* f = new Float(1.);
	  e->push_back(new Token_CONSTANT(f, ""));
	}else if(p2->is_local()){
	  delete e->back();
	  e->pop_back();
	  Float* f = new Float(0.);
	  e->push_back(new Token_CONSTANT(f, ""));
	}else{
	  Token_CALL::stack_op(e);
	//  is_constant();
	}
      }else{ untested();
	Token_CALL::stack_op(e);
	// is_constant();
      }
    }else if(dynamic_cast<Token_CONSTANT const*>(arg)){ untested();
      assert(0);
      delete e->back();
      e->pop_back();
      Float* f = new Float(0.);
      e->push_back(new Token_CONSTANT(f, ""));
    }else if(arg) { untested();
      incomplete(); // error?
    }else{ untested();
      incomplete(); // error?
    }
  }
};
/*--------------------------------------------------------------------------*/
class PARAM_GIVEN : public FUNCTION_ {
public:
  explicit PARAM_GIVEN() : FUNCTION_() {
    set_label("$param_given");
  }
  ~PARAM_GIVEN(){ }
private:
  bool static_code()const override {return true;}
  bool is_common()const override {return true;}
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_PG(label(), this);
  }
  void stack_op(Expression*)const override { untested();
    throw Exception("invalid");
  }
  std::string code_name()const override{
    return "param_given";
  }
//   void stack_op(Expression const& args, Expression* out) const override { untested();
//     incomplete();
//   }
  void make_cc_common(std::ostream& o)const override {
    o__ "bool " << code_name() << "(PARA_BASE const& p)const {\n";
    o____ "return p.has_hard_value();\n";
    o__ "}\n";
  }
} pg;
DISPATCHER<FUNCTION>::INSTALL d_pg(&function_dispatcher, "$param_given", &pg);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
