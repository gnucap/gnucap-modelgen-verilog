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
 * Verilog-AMS port flow xs function
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include "mg_analog.h"
#include "m_tokens.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
namespace {
class Token_PF : public Token_CALL {
public:
  explicit Token_PF(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_PF(const Token_PF& P, Base const* data, Expression_ const* e = NULL)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token_PF* clone()const override { return new Token_PF(*this);}

  void stack_op(Expression* e)const override;

  Expression_ const* args() const{ untested();
    if(auto a=prechecked_cast<Expression_ const*>(Token_CALL::args())){ untested();
      return a;
    }else{ untested();
      assert(!Token_CALL::args());
      return NULL;
    }
  }

};
/*--------------------------------------------------------------------------*/
class PORT_FLOW : public FUNCTION_ {
public:
  explicit PORT_FLOW() : FUNCTION_() {
    set_label("flow");
  }
  ~PORT_FLOW(){ }
private:
  bool static_code()const override {return false;}
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    return new Token_PF(label(), this);
  }
  std::string code_name()const override{ untested();
    incomplete();
    return "_PORT_FLOW";
  }
//   void stack_op(Expression const& args, Expression* out) const override { untested();
//     incomplete();
//   }
  void make_cc_common(std::ostream& o)const override { untested();
    o__ "double " << code_name() << "(int)const {\n";
    o____ "incomplete();\n";
    o____ "return 0.;\n";
    o__ "}\n";
  }
  std::string eval(CS&, const CARD_LIST*)const override { untested();
    unreachable();
    return "port_flow";
  }
} pf;
DISPATCHER<FUNCTION>::INSTALL d_pf(&function_dispatcher, ".port_flow", &pf);
/*--------------------------------------------------------------------------*/
void Token_PF::stack_op(Expression* E)const
{
	// incomplete();
	auto e = new Expression_;
	if(auto pb = dynamic_cast<Token_PORT_BRANCH*>(E->back())) {
		E->pop_back();
		e->push_back(pb);
	}else{ untested();
		assert(0);
	}

	if(dynamic_cast<Token_STOP*>(E->back())) {
		delete E->back();
		E->pop_back();
	}else{ untested();
		assert(0);
	}
	// E->push_back(clone());
	// use Token_CALL??
	auto n = clone();
	n->set_args(e);
	// n->set_label("I");
	E->push_back(n);
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
