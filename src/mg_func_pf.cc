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
#include "mg_token.h"
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
    : Token_CALL(P, data, e) { untested();} // , _item(P._item) {}
  Token_PF* clone()const override { return new Token_PF(*this);}

  void stack_op(Expression* e)const override;

  Expression_ const* args() const{
    if(auto a=prechecked_cast<Expression_ const*>(Token_CALL::args())){
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
  bool has_precalc()const override {return true;}
  std::string code_name()const override{
    return "d->PORT_FLOW";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "double PORT_FLOW(int i){\n";
	 o____ "return va::PORT_FLOW(i, this);\n";
	 o__ "}\n";
    o__ "double PORT_FLOW__precalc(int)const {\n";
	 o____ "return 0.;\n";
	 o__ "}\n";
  }
#if 0
  void make_cc_common(std::ostream& o)const override { untested();
//    o__ "double " << code_name() << "(int i, BASE_SUBCKT const* c)const {\n";
//	 o____ "return va::" << code_name() << "(i, c);\n";
//	 o__ "}\n";
  }
  void make_cc_dev(std::ostream& o)const override { untested();
    o__ "double " << code_name() << "(int i)const {\n";
	 o____ "node_t n = c->n_(i);\n";
	 o____ "double I(0.);\n";
    o____ "assert(c->subckt());\n";
    o____ "for(CARD const* c : *c->subckt()){\n";
	 o______ "auto e = dynamic_cast<ELEMENT const*>(c);\n";
	 o______ "if(!e){\n";
	 o______ "}else if(e->n_(1) == e->n_(0)){ untested();\n";
	 o______ "}else if(e->n_(1) == n){\n";
	 o________ "I+= e->tr_amps();\n";
	 o______ "}else if(e->n_(0) == n){\n";
	 o________ "I-= e->tr_amps();\n";
	 o______ "}else{\n";
	 o______ "}\n";
    o____ "}\n";
    o____ "return I;\n";
    o__ "}\n";
  }
#endif
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
	if(auto pb = dynamic_cast<Token_PORT_BRANCH*>(E->back())) {
		auto e = new Expression_;
		E->pop_back();
		e->push_back(pb);

		assert(prechecked_cast<Token_STOP*>(E->back()));
		delete E->back();
		E->pop_back();

		auto n = clone();
		n->set_args(e);
		// n->set_label("I");
		E->push_back(n);
	}else if(args()){
		// return Token_CALL::stack_op(E);
		auto c = new Token_PF(name(), f()); // clone?
		c->set_args(args()->clone());
		assert(args());
		E->push_back(c);
	}else{ untested();
		E->push_back(clone());
	}
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
