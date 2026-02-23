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
 * Verilog-AMS debug facilities
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_.h"
#include "mg_out.cc"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class DEBUG_TASK : public MGVAMS_TASK {
public:
  explicit DEBUG_TASK() : MGVAMS_TASK(){
    set_label("$debug");
  }
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable();
    return "$$debug";
  }
  MGVAMS_TASK* clone()const override {
    return new DEBUG_TASK(*this);
  }
  bool has_tr_accept()const override {return true;}
  // Token* new_token(Module&, size_t)const override{ return nullptr; }
  void make_cc_dev(std::ostream& o)const override {
    make_cc_dev_(o, "");
  }
protected:
  void make_cc_dev_(std::ostream& o, std::string const& s)const {
    if(num_args()>1){
      o____ "template<";
      std::string sep;
      for(size_t i=1; i<num_args(); ++i) {
	o << sep << "class T" << i;
	sep = ", ";
      }
      o << ">\n";
    }else{
    }
    o__ "void " << label() << s << "(std::string const& a0";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", T" << i << " a" << i;
    }
    o << ") {\n";
    if(s.size()){
      o______ "std::string aa = std::string(\"@" << s << ":\") + a0;\n";
      o______ "char const* cs = aa.c_str();\n";
    }else{
      o______ "char const* cs = a0.c_str();\n";
    }
    o______ "fprintf(stdout, cs";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", plain_value(a" << i << ")";
    }
    o << ");\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "/*d*/" + label();
  }
  Data_Type const* return_type()const override { return nullptr; }
} debug;
DISPATCHER<FUNCTION>::INSTALL d_debug(&function_dispatcher, "$debug", &debug);
/*--------------------------------------------------------------------------*/
class TRACE_TASK : public DEBUG_TASK {
public:
  explicit TRACE_TASK() : DEBUG_TASK(){
    set_label("$trace");
  }
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable();
    return "$$trace";
  }
  MGVAMS_TASK* clone()const override {
    return new TRACE_TASK(*this);
  }
  bool has_tr_advance()const override {return true;}
  bool has_tr_regress()const override { untested();return true;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool has_tr_restore()const override {return true;}
  bool static_code()const override {return false;} // ?
  bool has_modes()const override {return true;}
  void make_cc_dev(std::ostream& o)const override {
    make_cc_dev_(o, "ac_begin");
    make_cc_dev_(o, "tr_begin");
    make_cc_dev_(o, "tr_eval");
    make_cc_dev_(o, "tr_accept");
    make_cc_dev_(o, "tr_advance");
    make_cc_dev_(o, "tr_regress");
    make_cc_dev_(o, "tr_restore");
    make_cc_dev_(o, "tr_finish");
    make_cc_dev_(o, "precalc");
  }
#if 0 // share with DEBUG?
  void make_cc_dev_(std::ostream& o, std::string s)const { untested();
    o__ "void " << label() << s << "(std::string a0";
    for(size_t i=1; i<num_args(); ++i) { untested();
      o << ", double a" << i;
    }
    o << ") {\n";
    o______ "a0 = std::string(\"@" << s << ":\") + a0;\n";
    o______ "fprintf(stdout, a0.c_str()";
    for(size_t i=1; i<num_args(); ++i) { untested();
      o << ", a" << i;
    }
    o << ");\n";
    o__ "}\n";
  }
#endif
  std::string code_name()const override{
    return label();
  }
} trace;
DISPATCHER<FUNCTION>::INSTALL d_trace(&function_dispatcher, "$trace", &trace);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
