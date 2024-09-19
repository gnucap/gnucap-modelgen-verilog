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
 * Verilog-AMS builtin functions
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
static int cnt;
/*--------------------------------------------------------------------------*/
class TRACE_TASK : public MGVAMS_TASK {
public:
  explicit TRACE_TASK() : MGVAMS_TASK(){
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable();
    return "$$debug";
  }
  MGVAMS_TASK* clone()const override {
    return new TRACE_TASK(*this);
  }
  bool has_tr_advance()const override {return true;}
  bool has_tr_regress()const override {return true;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool static_code()const override {return false;}
  bool has_modes()const override {return true;}
  Token* new_token(Module& m, size_t na)const override{
    MGVAMS_TASK* cl = clone();
    cl->set_num_args(na);
    cl->set_label("t_trace_" + std::to_string(cnt++));
    m.push_back(cl);
    return new Token_CALL("$trace", cl);
  }
  void make_cc_dev(std::ostream& o)const override {
    make_cc_dev(o, "ac_begin");
    make_cc_dev(o, "tr_begin");
    make_cc_dev(o, "tr_eval");
    make_cc_dev(o, "tr_accept");
    make_cc_dev(o, "tr_advance");
    make_cc_dev(o, "tr_regress");
    make_cc_dev(o, "tr_finish");
    make_cc_dev(o, "precalc");
  }
  void make_cc_dev(std::ostream& o, std::string s)const {
    o__ "void " << label() << s << "(std::string a0";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", double a" << i;
    }
    o << ") {\n";
    o______ "a0 = std::string(\"@" << s << ":\") + a0;\n";
    o______ "fprintf(stdout, a0.c_str()";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", a" << i;
    }
    o << ");\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "d->" + label();
  }
  bool returns_void()const override { return true; }
} trace;
DISPATCHER<FUNCTION>::INSTALL d_trace(&function_dispatcher, "$trace", &trace);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
