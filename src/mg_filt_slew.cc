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
 * Verilog-AMS filters: "slew". This is a stub, proof-of-concept
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_filter.h" // BUG?
#include "mg_out.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
#include "mg_.h" // BUG
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
static int n_filters;
/*--------------------------------------------------------------------------*/
class SLEW : public MGVAMS_FILTER {
  Module* _m{NULL};
  Filter const* _f{NULL};
public:
  explicit SLEW() : MGVAMS_FILTER() {
    set_label("slew");
  }
  SLEW* clone()const /*override*/{
    return new SLEW(*this);
  }
  std::string code_name()const override {
    return "d->" + label();
  }
private:
  Token* new_token(Module& m, size_t na)const override{
    Filter* f = NULL;

    std::string filter_code_name = label() + "_" + std::to_string(n_filters++);

    SLEW* cl = clone();
    {
      f = new Filter(filter_code_name);
      f->set_owner(&m);
      f->set_dev_type("va_" + label());

      cl->set_label("_" + filter_code_name);
      cl->set_num_args(na);
      cl->_m = &m;
      m.push_back(cl);
    }

    f->set_state("_f_" + filter_code_name + "state");
    f->set_num_states(int(na)+2);
    f->set_owner(&m);
    f->set_dev_type("va_" + label());
    m.push_back(f);
    cl->_f = f;

    auto t = new Token_CALL(label(), cl);
    return t;
  }
  void make_cc_precalc(std::ostream& o)const override{
    o__ "ddouble " << label() << "(";
      std::string comma;
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble";
	comma = ", ";
      }
    o << "){\n";
    o____ "return 0.;\n";
    o__ "}\n";
  }
  void make_cc_dev(std::ostream& o)const override{
    assert(_f);
    std::string cn = _f->code_name();
    o__ "double " << cn << "state[" << _f->num_states() << "]\n;"; // filter/num_states?
    o__ "ELEMENT* _f" << label() << "{NULL};\n";
    make_cc_dev_(o);
  }
  void make_assign(std::ostream& o)const {
    assert(_f);
    char sign = '+';
    std::string cn = _f->code_name();
    o__ "double* st = d->" << cn << "state;\n";
    if(num_args() > 1){
      o__ "st[3] = t1;\n";
      o__ "trace1(\"slew\", st[3]);\n";
    }else{
    }
    if(num_args() > 2){
      o__ "st[4] = t2;\n";
      o__ "trace1(\"slew\", st[4]);\n";
    }else{
    }
    o__ "st[2] = " << sign << " " << "t0.value();\n";
    o__ "assert(d->" << cn << ");\n";
    o__ "d->" << cn << "->do_tr();\n";
    o__ "t0.set_value(st[0]);\n";
    o__ "t0.chain(st[1]);\n";
    o__ "trace3(\"slew\", st[0], st[1], st[2]);\n";

    o__ "assert(t0 == t0);\n";
    o__ "return t0;\n";
  }
  void make_cc_common(std::ostream&)const override{}
  void make_cc_dev_(std::ostream& o)const{
    o__ " /*dev*/ ddouble " << label() << "(";
      std::string comma;
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble t" << n;
	comma = ", ";
      }
    o << ");\n";
  }
  void make_cc_impl(std::ostream&o)const override{
    assert(_m); // owner?
    assert(_f);
    std::string id = _m->identifier().to_string();
    o << "MOD_" << id << "::ddouble";
    o << " MOD_" << id << "::" << label() << "(";
    std::string comma;
    for(size_t n=0; n<num_args(); ++n){
      o << comma << "ddouble t" << n;
      comma = ", ";
    }
    o << ")\n{\n";
    o__ "MOD_" << id << "* d = prechecked_cast<MOD_" << id << "*>(this);\n";
    o__ "assert(d);\n";
    make_assign(o);
    o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  }
} slew;
DISPATCHER<FUNCTION>::INSTALL d_slew(&function_dispatcher, "slew", &slew);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
