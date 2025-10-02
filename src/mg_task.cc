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
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class BOUND_STEP : public MGVAMS_TASK {
public:
  explicit BOUND_STEP() : MGVAMS_TASK() {
    set_label("bound_step");
  }
  bool has_modes()const override {return true;}
  bool is_common()const override {return false;}
  bool has_state()const override {return true;}
  bool has_tr_review()const override {return true;}
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable(); // SFCALL won't eval
    return "$$bound_step";
  }
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    m.set_tr_review(); // set based on has_tr_review?!
    return new Token_CALL("$bound_step", this);
  }
  std::string code_name()const override{
    return "_f_bound_step_";
  }
  void make_cc_dev(std::ostream& o)const override {
    // o__ "double _bound_step{NEVER};\n";
    o__ "void " << "_f_bound_step_tr_eval(double) {\n";
    o__ "}\n";
    o__ "void " << "_f_bound_step_tr_review(double d) {\n";
    o____ "_time_by.min_error_estimate(_sim->_time0 + d);\n";
    o__ "}\n";
    o__ "void " << "_f_bound_step_precalc(double)const{}\n";
  }
} bound_step;
DISPATCHER<FUNCTION>::INSTALL d_bound_step(&function_dispatcher, "$bound_step", &bound_step);
/*--------------------------------------------------------------------------*/
class FINISH_TASK : public MGVAMS_TASK {
public:
  explicit FINISH_TASK() : MGVAMS_TASK(){
    set_label("t_finish");
  }
private:
  std::string eval(CS& cmd, const PARAM_LIST*)const override{ untested();
    return "$finish" + cmd.fullstring();
  }
  MGVAMS_TASK* clone()const override{ untested();
    return new FINISH_TASK(*this);
  }
  Token* new_token(Module& m, size_t na)const override{
    m.install(this);
    m.set_tr_advance();
    Token_CALL* t = new Token_CALL("$finish", this);
    t->set_num_args(na); // still needed?
    return t;
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "void t_finish(int n=1){\n";
    o____ "(void)n;\n";
    o____ "throw Exception(\"finish\");\n";
    o__ "}\n";
    o__ "void t_finish(double x){return t_finish(int(x));}\n";
    o__ "void t_finish__precalc(int n=1){\n";
    o____ "(void)n;\n";
    o__ "}\n";
    o__ "void t_finish__precalc(double x){return t_finish(int(x));}\n";
  }
  std::string code_name()const override{
    return label();
  }
} finish;
DISPATCHER<FUNCTION>::INSTALL d_finish(&function_dispatcher, "$finish", &finish);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
