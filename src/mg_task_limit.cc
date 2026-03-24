/*                        -*- C++ -*-
 * Copyright (C) 2025 Felix Salfelder
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
 * Verilog-AMS $limit
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_.h"
#include "mg_out.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static size_t cnt;
/*--------------------------------------------------------------------------*/
class LIMIT : public MGVAMS_TASK {
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "$limit";
  }
  LIMIT* clone()const override{
    return new LIMIT(*this);
  }
  Token* new_token(Module& m, size_t na)const override{
    LIMIT* cl = clone();
    cl->set_label("t_limit_" + std::to_string(cnt++));
    cl->set_num_args(na);
    m.push_back(cl);
    // d untouched?
    return new Token_CALL("$limit", cl);
  }
  std::string code_name()const override{
    return label();
  }
  bool has_modes()const override {return true;}
  bool has_state()const override {return true;}
  Data_Type const* return_type()const override {
    static Data_Type_Real r;
    return &r;
  }
  void make_cc_common(std::ostream&)const override {
    // nothing.
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "class " << label() << "{\n";
    o____ "double _old;\n";
    o__ "public:\n";
    o____ "template<class F>\n";
    o____ "ddouble operator()(COMPONENT* d, ddouble in, F f";
    for(size_t i=0; i<num_args()-2; ++i) {
      o << ", double const& a" << i;
    }
    o << "){\n";
    o______ "(void)d;\n";
   // o______ "double old = in;\n";
   // o______ "auto dd=prechecked_cast<MOD const*>(d);\n";
    o______ "auto cc=prechecked_cast<COMMON const*>(d->common());\n";
    o______ "assert(cc);\n";
    o______ "if(_sim->is_initial_step()) {\n";
    o________ "in.set_value(0.);\n";
    o________ "_old = 0;\n";
    o______ "}else{\n";
    o________ "in.set_value((*cc.*f)(d, in, _old";
    for(size_t i=0; i<num_args()-2; ++i) {
      o << ", a" << i;
    }
    o << "));\n";
    o________ "_old = in;\n";
    o______ "}\n";
    o______ "return in;\n";
    o____ "}\n";
    o____ "void set(ddouble in){_old=in;}\n";

    o____ "ddouble operator()(COMPONENT* d, ddouble in, string const& what";
    for(size_t i=0; i<num_args()-2; ++i) {
      o << ", double const&";
      if(num_args()==4){
	o << " a" << i;
      }else{
      }
    }
    o << "){\n";
    o______ "(void)d;\n";
    o______ "(void)what;\n";
    // o______ "assert(what == \"pnjlim\"); // for now\n";
    o______ "if(_sim->is_initial_step()) {\n";
    o________ "in.set_value(0.);\n";
    o________ "_old = 0;\n";
    if(num_args()==4){
      o______ "}else if(what == \"pnjlim\") {\n";
      o________ "double old = in;\n";
      // o________ "in = ngspice_pnjlim(in, _old, a, b);\n";
      o________ "in.set_value(pnj_limit(in, _old, a0, a1));\n";
      o________ "if( !conchk(in, old, 1e-6)) {\n";
      o__________ "d->set_converged(false);\n";
      o________ "}else{\n";
      o________ "}\n";
      o________ "_old = old;\n";
#if 0 // debug.
    }else if(num_args()==3){ untested();
      o______ "}else if(what == \"fet_limit_vgs\") {\n"; // TODO.
      o________ "double old = in;\n";
      o________ "in.set_value(fet_limit_vgs(in, _old, a0));\n";
      o________ "if( !conchk(in, old, 1e-6)) {\n";
      o__________ "d->set_converged(false);\n";
      o________ "}else{\n";
      o________ "}\n";
      o________ "_old = in;\n";
    }else if(num_args()==2){ untested();
      o______ "}else if(what == \"fet_limit_vds\") {\n"; // TODO.
      o________ "double old = in;\n";
      o________ "in.set_value(fet_limit_vds(in, _old));\n";
      o________ "if( !conchk(in, old, 1e-6)) {\n";
      o__________ "d->set_converged(false);\n";
      o________ "}else{\n";
      o________ "}\n";
      o________ "_old = old;\n";
#endif
    }else{
      incomplete();
    }
    o______ "}\n";
     // o________ "if( d->converged() && !conchk(in, old, 1e-6)) {untested();\n";
     // o__________ "d->set_converged(false);\n";
     // o________ "}else{\n";
     // o________ "}\n";
    o______ "return in;\n";
    o____ "}\n";
    o____ "ddouble " << label() << "precalc(ddouble, string const&";
    for(size_t i=0; i<num_args()-2; ++i) {
      o << ", double const&";
    }
    o______ "){\n";
    o______ "return 0.;\n";
    o____ "}\n";
    o__ "} " << label() << "_;\n";

    make_cc_dev(o, "ac_begin");
    make_cc_dev(o, "tr_begin");
    make_cc_dev(o, "tr_review");
    // make_cc_dev(o, "tr_eval");
    make_cc_dev(o, "tr_accept");
    make_cc_dev(o, "tr_advance");
    make_cc_dev(o, "tr_regress");
    make_cc_dev(o, "tr_restore");
    make_cc_dev(o, "tr_finish");
    make_cc_dev(o, "precalc");
    o__ "template<class F>\n";
    o__ "ddouble " << label() << "tr_eval(ddouble in, F what";
    for(size_t i=0; i<num_args()-2; ++i) {
      o << ", double const& a" << i;
    }
    o << ") {\n";
    o____ "return " << label() << "_(this, in, what";
    for(size_t i=0; i<num_args()-2; ++i) {
      o << ", a" << i;
    }
    o << ");\n";
    o__ "}\n";
  }

  void make_cc_dev(std::ostream& o, std::string s)const {
    o__ "template<class F>\n";
    o__ "ddouble " << label() << s << "(ddouble in, F";
    for(size_t i=0; i<num_args()-2; ++i) {
      o << ", double const&";
    }
    o << ") {\n";
    o____ label() << "_.set(in);\n";
    o____ "return in;\n";
    o__ "}\n";
  }
} limit;
DISPATCHER<FUNCTION>::INSTALL d_limit(&function_dispatcher, "$limit", &limit);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
