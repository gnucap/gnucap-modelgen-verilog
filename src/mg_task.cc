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
  bool has_tr_review()const override {return true;}
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable(); // SFCALL won't eval
    return "$$bound_step";
  }
  Token* new_token(Module& m, size_t)const override {
    m.install(this);
    m.set_tr_review(); // set based on has_tr_review?!
    return new Token_CALL("$bound_step", this);
  }
  std::string code_name()const override{
    return "d->_f_bound_step_";
  }
 void make_cc_precalc(std::ostream& o)const override {
   o__ "void " << "_f_bound_step_precalc(double)const{}\n";
   // o__ "void " << "_f_bound_step_CALLMODE_hack(double d) {\n}\n";
 }
 void make_cc_dev(std::ostream& o)const override {
   // o__ "double _bound_step{NEVER};\n";
   o__ "void " << "_f_bound_step_tr_eval(double d) {\n";
   o__ "}\n";
   o__ "void " << "_f_bound_step_tr_review(double d) {\n";
   o____ "_time_by.min_error_estimate(_sim->_time0 + d);\n";
   o__ "}\n";
 }
 void make_cc_tr_review(std::ostream& o)const override {
   // TODO. set_tr_review
   // o__ "_time_by.min_error_estimate(_sim->_time0 + _bound_step);\n";
 }
 void make_cc_tr_advance(std::ostream& o)const override {
   // o__ "_bound_step = NEVER;\n";
 }
 bool returns_void()const override { return true; }
} bound_step;
DISPATCHER<FUNCTION>::INSTALL d_bound_step(&function_dispatcher, "$bound_step", &bound_step);
/*--------------------------------------------------------------------------*/
static size_t cnt;
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
  void make_cc_precalc(std::ostream& o)const override {
    o__ "void t_finish(int n=1){\n";
    o____ "(void)n;\n";
    o__ "}\n";
    o__ "void t_finish(double x){return t_finish(int(x));}\n";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "void t_finish(int n=1){\n";
    o____ "(void)n;\n";
    o____ "throw Exception(\"finish\");\n";
    o__ "}\n";
    o__ "void t_finish(double x){return t_finish(int(x));}\n";
  }
  std::string code_name()const override{
    return "d->" + label();
  }
  bool returns_void()const override { return true; }
} finish;
DISPATCHER<FUNCTION>::INSTALL d_finish(&function_dispatcher, "$finish", &finish);
/*--------------------------------------------------------------------------*/
class LIMIT : public MGVAMS_TASK {
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
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
    return "d->" + label();
  }
  void make_cc_common(std::ostream&)const override {
    // nothing.
  }
  void make_cc_precalc(std::ostream& o)const override {
    o__ "ddouble " << label() << "(ddouble in, std::string const& what, double const& a, double const& b){\n";
    o____ "return 0.;\n";
    o__ "}\n";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "class " << label() << "{\n";
    o____ "double _old;\n";
    o____ "public:\n";
    o____ "ddouble operator()(ddouble in, std::string const& what, double const& a, double const& b){\n";
    o______ "double old = in;\n";
    o______ "assert(what == \"pnjlim\"); // for now\n";
    o______ "if(_sim->is_initial_step()) {\n";
    o________ "in.set_value(0.);\n";
    o________ "_old = 0;\n";
    o______ "}else{\n";
    // o________ "in = ngspice_pnjlim(in, _old, a, b);\n";
    // BUG: what about the derivatives?
    o________ "in.set_value(pnj_limit(in, _old, a, b));\n";
    o________ "_old = old;\n";
    o______ "}\n";
    o______ "// convcheck old vs in?\n";
    o______ "return in;\n";
    o____ "}\n";
    o__ "} " << label() << ";\n";
  }
} limit;
DISPATCHER<FUNCTION>::INSTALL d_limit(&function_dispatcher, "$limit", &limit);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
