/*                        -*- C++ -*-
 * Copyright (C) 2023-26 Felix Salfelder
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
 * Verilog-AMS $bound_step
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_.h"
#include "mg_out.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
#include "f__.cc"
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class BOUND_STEP : public MGVAMS_TASK {
  BOUND_STEP(BOUND_STEP const&p) : MGVAMS_TASK(p) {}
public:
  explicit BOUND_STEP() : MGVAMS_TASK() {
    set_label("bound_step");
  }
  BOUND_STEP* clone()const override {return new BOUND_STEP(*this);}
  bool has_modes()const override {return true;}
  bool is_common()const override {return false;}
  bool has_state()const override { untested();return true;}
  bool has_tr_review()const override {return true;}
  bool static_code()const override {return true;}
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable(); // SFCALL won't eval
    return "$$bound_step";
  }
  Token* new_token(Module&, size_t)const override {
    return nullptr;
  }
  std::string code_name()const override{
    return "_f_bound_step_";
  }
  void make_cc_dev(std::ostream& o)const override {
    // o__ "double _bound_step{NEVER};\n";
    o__ "void " << "_f_bound_step_tr_eval(double) {\n";
    o__ "}\n";
    o__ "void " << "_f_bound_step_tr_review(double d) {\n";
    o____ "_time_by.min_dt_estimate(d);\n";
    o__ "}\n";
    o__ "void " << "_f_bound_step_precalc(double)const{}\n";
  }
} bound_step;
DISPATCHER<FUNCTION>::INSTALL d_bound_step(&function_dispatcher, "$bound_step", &bound_step);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
