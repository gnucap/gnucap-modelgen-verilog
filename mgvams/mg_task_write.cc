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
 * Verilog-AMS display tasks
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
class WRITE : public MGVAMS_TASK {
public:
  explicit WRITE() : MGVAMS_TASK(){
    set_label("$write");
  }
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "$$strobe";
  }
  WRITE* clone()const override {
    return new WRITE(*this);
  }
  bool is_common()const override {untested(); return true;}
  bool is_in_common()const override { return false;} // BUG?
  bool has_modes()const override { return true;}
  bool has_state()const override {untested(); return false;}
  bool needs_context()const override { return true;} // has accept?
  bool has_tr_review()const override {return true;}
  bool has_tr_advance()const override {return true;}
  bool has_tr_regress()const override { untested();return false;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool has_tr_restore()const override {return false;}
  bool static_code()const override {return false;}
  // Token* new_token(Module&, size_t)const override{ return nullptr; }
  void args(std::ostream& o, bool names=false)const {
    o << "std::string";
    if(names){
      o << " s";
    }else{
    }
    for(int i=1; i<int(num_args()); ++i) {
      o____  ", T" << i;
      if(names){
       o << " a" << i;
      }else{
      }
    }
  }
  void voidargs(std::ostream& o)const {
    for(size_t i=1; i<num_args(); ++i) {
      o << " (void) a" << i << ";\n";
    }
  }
private:
  void template_header(std::ostream& o, bool mod=false)const {
    if(mod || num_args()>1){
      o____ "template<";
      std::string sep;
      if(mod){
	o << "class MOD";
	sep = ", ";
      }else{
      }
      for(size_t i=1; i<num_args(); ++i) {
	o << sep << "class T" << i;
	sep = ", ";
      }
      o << ">\n";
    }else{
    }
  }
public: //overrides
  void make_cc_dev(std::ostream& o)const override {
    o__ "struct cls" << label() << "{\n";

    template_header(o, false);
    o____ "void tr_eval(CARD* d, std::string";
    for(size_t i=1; i<num_args(); ++i) {
      o____  ", T" << i <<" const&";
    }
    o << ")const { (void)d; }\n";

    template_header(o, true);
    o____ "void tr_initial(MOD* d, "; args(o, true); o << ")const {\n";
    o______ "tr_accept(d, s\n";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", a" << i;
    }
    o << ");\n";
    o____"}\n";

    template_header(o, true);
    o____ "void tr_begin(MOD* d, std::string";
    for(size_t i=1; i<num_args(); ++i) {
      o____  ", T" << i << " a" << i << "";
    }
    o << ")const {\n";
    voidargs(o);
    o______ "trace1(\"write::tr_begin\", _sim->_time0);\n";
    //o______ "assert(d); return d->q_accept();\n";
    o______ " assert(d); d->q_accept();\n";
    o____"}\n";

    template_header(o, true);
    o____ "void tr_review(MOD* d, "; args(o); o << ") {\n";
    o______ "(void)d;\n";
    o______ " assert(d); d->q_accept();\n";
    o____"}\n";

    template_header(o, true);
    o____ "void tr_advance(MOD* d, "; args(o); o << ") {\n";
    o______ "(void)d;\n";
    o______ "trace1(\"write::tr_advance\", _sim->_time0);\n";
    o______ "assert(d); d->q_accept();\n";
    o____"}\n";

    template_header(o);
    o____ "void tr_regress(CARD*, "; args(o); o << ") {\n";
    o____"}\n";

    template_header(o);
    o____ "void tr_accept(CARD*, std::string a0";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", T" << i << " const& a" << i;
    }
    o << ")const {\n";
    o______ "trace1(\"write::tr_accept\", _sim->_time0);\n";
    if(end().size()){
      o______ "a0 += \"" << end() << "\";\n";
    }else{
    }
    o______ "fprintf(stdout, a0.c_str()";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", plain_value(a" << i << ")";
    }
    o << ");\n";
    o____ "}\n";

    template_header(o);
    o____ "void precalc(CARD const*, "; args(o); o << ")const { /*nop*/ }\n";
    o__ "}_" << label() << ";\n";
  }
  std::string code_name()const override{
    return "/*w*/_" + label() + ".";
  }
  // Data_Type const* return_type()const override { return nullptr; }
  virtual std::string end()const{return "";}
} write;
DISPATCHER<FUNCTION>::INSTALL d_write(&function_dispatcher, "$write", &write);
/*--------------------------------------------------------------------------*/
class STROBE : public WRITE {
public:
  explicit STROBE() : WRITE() {
    set_label("$strobe");
  }
  std::string end()const override{return "\\n";}
  WRITE* clone()const override {
    return new STROBE(*this);
  }
} strobe;
DISPATCHER<FUNCTION>::INSTALL d_strobe(&function_dispatcher, "$strobe", &strobe);
/*--------------------------------------------------------------------------*/
class WARNING : public WRITE {
public:
  explicit WARNING() : WRITE() {
    set_label("$warning");
  }
  std::string end()const override{return "\\n";}
  WRITE* clone()const override {
    incomplete(); // need stderr or error(bWARNING..)
    return new WARNING(*this);
  }
} warning;
DISPATCHER<FUNCTION>::INSTALL d_warning(&function_dispatcher, "$warning", &warning);
/*--------------------------------------------------------------------------*/
class ERROR : public WRITE {
public:
  explicit ERROR() : WRITE() {
    set_label("$error");
  }
  std::string end()const override{return "\\n";}
  WRITE* clone()const override {
    incomplete(); // need stderr or error(bDANGER..)
    return new ERROR(*this);
  }
} error;
DISPATCHER<FUNCTION>::INSTALL d_error(&function_dispatcher, "$error", &error);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
