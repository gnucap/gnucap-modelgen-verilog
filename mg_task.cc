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
 * Verilog-AMS builtin functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_.h"
#include "mg_out.h"
#include "m_tokens.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static size_t cnt;
class STROBE_TASK : public MGVAMS_TASK {
public:
  explicit STROBE_TASK() : MGVAMS_TASK(){
//    set_label("strobe");
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$$strobe";
  }
  MGVAMS_TASK* clone() const{
    return new STROBE_TASK(*this);
  }
  Token* new_token(Module& m, size_t num_args)const override{
    m.new_evt_slot();
    MGVAMS_TASK* cl = clone();
    cl->set_num_args(num_args);
    cl->set_label("t_strobe_" + std::to_string(cnt++));
    m.push_back(cl);
    return new Token_TASK("$strobe", cl);
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "struct _" << label() << " : public va::EVT {\n";
    o____ "std::string _a0;\n";
    for(size_t i=1; i<num_args(); ++i) {
      o____  "double _a" << i << ";\n";
    }

    o____ "void operator()()const override {\n";
    o______ "fprintf(stdout, _a0.c_str()";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", _a" << i;
    }
    o << ");\n";
    o____ "}\n";
    o__ "} __" << label() << ";\n";

    o__ "void " << label() << "(std::string const& a0";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", double a" << i;
    }
    o << "){\n";
    for(size_t i=0; i<num_args(); ++i) {
      o____ "__" << label()  << "._a" << i << " = a" << i << ";\n";
    }
    o____ "q_evt(&__" << label() << ");\n";
    o__ "}\n";
  }
} strobe;
DISPATCHER<FUNCTION>::INSTALL d_strobe(&function_dispatcher, "$strobe", &strobe);
/*--------------------------------------------------------------------------*/
class FINISH_TASK : public MGVAMS_TASK {
public:
  explicit FINISH_TASK() : MGVAMS_TASK(){
    set_label("t_finish");
  }
private:
  std::string eval(CS& cmd, const CARD_LIST*)const override{ untested();
    return "$finish" + cmd.fullstring();
  }
  MGVAMS_TASK* clone() const{ untested();
    return new FINISH_TASK(*this);
  }
  Token* new_token(Module& m, size_t /* na */)const override{
    m.install(this);
    return new Token_TASK("$finish", this);
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "void t_finish(int n=1){\n";
    o____ "throw Exception(\"finish\");\n";
    o__ "}\n";
    o__ "void t_finish(double x){return t_finish(int(x));}\n";
  }
} finish;
DISPATCHER<FUNCTION>::INSTALL d_finish(&function_dispatcher, "$finish", &finish);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
