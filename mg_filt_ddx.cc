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
 * Verilog-AMS filters
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include "m_tokens.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static int n_filters;
/*--------------------------------------------------------------------------*/
class Token_DDX : public Token_CALL {
public:
  explicit Token_DDX(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_DDX(const Token_DDX& P, Base const* data, Expression_ const* e = NULL)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override {untested(); return new Token_DDX(*this);}

  void stack_op(Expression* e)const override;

  Probe const* ddxprobe()const {
    auto ee = prechecked_cast<Expression const*>(args());
    assert(ee);
    Token const* t = ee->back();
    assert(t);
    auto dd = prechecked_cast<Deps const*>(t->data());
    assert(dd);
    assert(dd->size()==1);
    return *dd->begin();
  }

};
/*--------------------------------------------------------------------------*/
class DDX : public MGVAMS_FILTER {
  Module* _m{NULL};
  mutable Probe const* _ddxprobe{NULL}; // hack.
public:
  explicit DDX(){ set_label("ddx"); }
  DDX* clone()const /*override*/{
    return new DDX(*this);
  }
  Token* new_token(Module& m, size_t na)const override{
    std::string filter_code_name = "_b_ddx_" + std::to_string(n_filters++);
    DDX* cl = clone();
    trace0("===");
    // cl->_ddxprobe = *d.begin();
    cl->set_label(filter_code_name);
    cl->set_num_args(na);
    m.push_back(cl);
    cl->_m = &m;

    auto t = new Token_DDX(label(), cl);
    return t;
  }
  std::string code_name()const override{
    return "/*DDX*/" + label();
  }
  void make_cc_dev(std::ostream&)const override{
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "class FILTER" << label() << "{\n";
    o__ "public:\n";
    o____ "ddouble operator()(";
    std::string comma;
    for(size_t n=0; n<num_args(); ++n){
      o << comma << "ddouble t" << n;
      comma = ", ";
    }
    o << ") const\n";
    assert(_ddxprobe);

    o__ "{ // NEW  ddx " <<  code_name() << "\n";
    if(_ddxprobe->is_pot_probe()){
    }else{
      incomplete();
    }
    Node const* p = _ddxprobe->branch()->p();
    Node const* n = _ddxprobe->branch()->n();
    o__ "double ret;\n";
    o__ "(void) t1;\n";
    assert(_m);
    for(auto x : _m->branches()){
      if(x->has_pot_probe()){
	o__ "// found probe " <<  x->code_name() << "\n";
	if(p == &mg_ground_node){ untested();
	}else if(n != &mg_ground_node){
	}else if(x->p() == p){
	  o__ "ret += t0[d_potential" << x->code_name() << "];\n";
	}else if(x->n() == p){
	  o__ "ret -= t0[d_potential" << x->code_name() << "];\n";
	}else{
	}

	if(n == &mg_ground_node){
	}else if(p == &mg_ground_node){ untested();
	}else if(x->p() == p && x->n() == n){
	  // oops. what does the standard say about reversed ddx?
	  bool rev = _ddxprobe->is_reversed();
	  o__ "ret " << (rev?'-':'+') << "= t0[d_potential" << x->code_name() << "]; // fwd?\n";
	}else if(x->p() == n && x->n() == p){ untested();
	  unreachable();
	}else{ untested();
	}

      }else{
      }

    }

    // incomplete(); no second derivatives..
    o__ "return ret;\n";
    o__ "}\n";

    o__  "} " << label() << ";\n";
  }
  void make_cc_impl(std::ostream&)const override{
    // nothing
  }
  std::string eval(CS&, const CARD_LIST*)const override{
    unreachable();
    return "ddx";
  }
  void set_ddxprobe(Probe const* d) const {
    _ddxprobe = d;
  }
} ddx;
DISPATCHER<FUNCTION>::INSTALL d_ddx(&function_dispatcher, "ddx", &ddx);
/*--------------------------------------------------------------------------*/
void Token_DDX::stack_op(Expression* e)const
{
  assert(e);
  Token_CALL::stack_op(e);
  assert(!e->is_empty());
  auto cc = prechecked_cast<Token_CALL const*>(e->back());
  assert(cc);

  auto ff = prechecked_cast<DDX const*>(f());
  assert(ff);

  auto ee = prechecked_cast<Expression const*>(cc->args());
  assert(ee);
  Token const* t = ee->back();
  assert(t);
  auto dd = prechecked_cast<Deps const*>(t->data());
  (void)dd;
  assert(dd);
  assert(dd->size());

  auto d = new Deps; // incomplete. second order derivatives?
  auto N = new Token_DDX(*this, d, cc->args()?cc->args()->clone():NULL);
  assert(N->args());

  ff->set_ddxprobe(N->ddxprobe());

  { // just stick to Token_CALL?
    e->pop_back();
    e->push_back(N);
    delete(cc);
  }
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
