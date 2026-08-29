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
#include "mg_code.h"
#include <globals.h>
#include <u_parameter.h>
#include "f__.cc" // TODO
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class DRIVER_DELAY : public MGVAMS_FUNCTION {
  bool _real{false};
public:
  explicit DRIVER_DELAY(double) : MGVAMS_FUNCTION(), _real(true) { set_label("$driver_delay"); }
  explicit DRIVER_DELAY() : MGVAMS_FUNCTION() { set_label("$driver_delay"); }
  explicit DRIVER_DELAY(DRIVER_DELAY const&p) : MGVAMS_FUNCTION(p), _real(p._real) {
    set_label(p.short_label());
  }
  ~DRIVER_DELAY(){ }
  FUNCTION_* clone()const override { return new DRIVER_DELAY(*this);}
private:
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  bool static_code()const override {return false;}
  // bool has_precalc()const override { untested();return true;}
  std::string code_name()const override{
    return "drv_dly";
  }
  void make_cc_dev(std::ostream& o)const override {
    o << "  " <<  "double drv_dly(va::LNR const& l, int i){\n";
    o << "  " <<  "  double ft = l.final_time();\n";
    o << "  " <<  "  if(ft == NEVER) {\n";
    o << "  " <<  "    return -1.;\n";
    o << "  " <<  "  }else{\n";
    o << "  " <<  "    return (ft - _sim->_time0)"
                       << (_real?"":"/OPT::dtmin") << ";\n";
    o << "  " <<  "  }\n";
    o << "  " <<  "}\n";
    o << "  " <<  "double drv_dly__precalc(int)const {untested();\n";
    o << "  " <<  "  return 0.;\n";
    o << "  " <<  "}\n";
  }
} f0;
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "$driver_delay", &f0);
DRIVER_DELAY f1(0.);
DISPATCHER<FUNCTION>::INSTALL d1(&function_dispatcher, "$driver_real_delay", &f1);
/*--------------------------------------------------------------------------*/
class DRIVER_NEXT_STATE : public MGVAMS_FUNCTION {
public:
  explicit DRIVER_NEXT_STATE() : MGVAMS_FUNCTION() {
    set_label("$driver_next_state");
  }
  ~DRIVER_NEXT_STATE(){ }
  FUNCTION_* clone()const override { untested(); return new DRIVER_NEXT_STATE(*this);}
private:
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  bool static_code()const override {return true;}
  // bool has_precalc()const override { untested();return true;}
  std::string code_name()const override{
    return "d->drv_next_state";
  }
  void make_cc_dev(std::ostream& o)const override {
    o << "  " <<  "double drv_next_state(va::LNR const& l, int i){\n";
    o << "  " <<  "  assert(!i);\n";
    o << "  " <<  "  trace1(\"DRVNEXT\", l.lv_future());\n";
    o << "  " <<  "  return l.lv_future();\n";
    o << "  " <<  "}\n";
    o << "  " <<  "double drv_next_state__precalc(int)const {untested();\n";
    o << "  " <<  "  return 0.;\n";
    o << "  " <<  "}\n";
  }
  // void make_cc_common(std::ostream& o)const override { untested(); }
  // void make_cc_dev(std::ostream& o)const override { untested(); }
  Data_Type const* return_type()const override {
    static Data_Type_Real r;
    return &r;
  }
} f2;
DISPATCHER<FUNCTION>::INSTALL d2(&function_dispatcher, "$driver_next_state", &f2);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
