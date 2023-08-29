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
class DDX : public MGVAMS_FILTER {
  Probe const* _ddxprobe{NULL};
  Module* _m{NULL};
public:
  explicit DDX(){ set_label("ddx"); }
  DDX* clone()const /*override*/{
    return new DDX(*this);
  }
  Token* new_token(Module& m, size_t na, Deps& d)const override{
    std::string filter_code_name = "_b_ddx_" + std::to_string(n_filters++);
    DDX* cl = clone();
    trace0("===");
    for(auto p : d){
      trace4("probe", p->pname(), p->nname(), p->code_name(), d.size());
    }
    assert(d.size());
    cl->_ddxprobe = *d.begin();
    cl->set_label(filter_code_name);
    cl->set_num_args(na);
    m.push_back(cl);
    cl->_m = &m;

    Deps outdeps;
    d = outdeps;

    auto t = new Token_FILTER(label(), cl);
    return t;
  }
  void make_cc_dev(std::ostream&)const override{
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "class FILTER" << label() << "{\n";
    o__ "public:\n";
    o____ "ddouble operator()(";
    for(size_t n=0; n<num_args(); ++n){
      o << "ddouble t" << n << ", ";
    }
    o << "COMPONENT*) const\n";

    assert(_ddxprobe);
    o__ "{ // NEW  ddx " <<  _ddxprobe->code_name() << "\n";
    if(_ddxprobe->is_pot_probe()){
    }else{
      incomplete();
    }
    Node const* p = _ddxprobe->branch()->p();
    Node const* n = _ddxprobe->branch()->n();
    o__ "ddouble ret;\n";
    o__ "(void) t1;\n";
    assert(_m);
    for(auto x : _m->branches()){
      if(x->has_pot_probe()){
	o__ "// found probe " <<  x->code_name() << "\n";
	if(p == &mg_ground_node){ untested();
	}else if(n != &mg_ground_node){
	}else if(x->p() == p){
	  o__ "ret.value() += t0[d_potential" << x->code_name() << "];\n";
	}else if(x->n() == p){
	  o__ "ret.value() -= t0[d_potential" << x->code_name() << "];\n";
	}else{
	}

	if(n == &mg_ground_node){
	}else if(p == &mg_ground_node){ untested();
	}else if(x->p() == p && x->n() == n){
	  // oops. what does the standard say about reversed ddx?
	  bool rev = _ddxprobe->is_reversed();
	  o__ "ret.value() " << (rev?'-':'+') << "= t0[d_potential" << x->code_name() << "]; // fwd?\n";
	}else if(x->p() == n && x->n() == p){ untested();
	  unreachable();
	}else{ untested();
	}

      }else{
      }

    }
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
} ddx;
DISPATCHER<FUNCTION>::INSTALL d_ddx(&function_dispatcher, "ddx", &ddx);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
