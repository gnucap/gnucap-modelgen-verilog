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
static size_t cnt;
class DEBUG_TASK : public MGVAMS_TASK {
public:
  explicit DEBUG_TASK() : MGVAMS_TASK(){
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable();
    return "$$debug";
  }
  MGVAMS_TASK* clone()const override {
    return new DEBUG_TASK(*this);
  }
  bool has_tr_accept()const override {return true;}
  Token* new_token(Module& m, size_t na)const override{
    MGVAMS_TASK* cl = clone();
    cl->set_num_args(na);
    cl->set_label("t_debug_" + std::to_string(cnt++));
    m.push_back(cl);
    return new Token_CALL("$debug", cl);
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "void " << label() << "(std::string const& a0";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", double a" << i;
    }
    o << ") {\n";
    o______ "fprintf(stdout, a0.c_str()";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", a" << i;
    }
    o << ");\n";
    o__ "}\n";
    o__ "void " << label() << "__precalc(std::string const&";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", double";
    }
    o << "){\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "d->" + label();
  }
  bool returns_void()const override { return true; }
} debug;
DISPATCHER<FUNCTION>::INSTALL d_debug(&function_dispatcher, "$debug", &debug);
/*--------------------------------------------------------------------------*/
class WRITE : public MGVAMS_TASK {
  Module* _m{NULL};
public:
  explicit WRITE() : MGVAMS_TASK(){
    set_label("$write");
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$$strobe";
  }
  WRITE* clone()const override {
    return new WRITE(*this);
  }
  bool is_common()const override {return true;} // extra CARD*
  bool has_modes()const override {return true;}
 // bool has_tr_review()const override {return true;} // need to q_accept?
  bool has_tr_advance()const override {return true;}
  bool has_tr_regress()const override {return false;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool static_code()const override {return false;}
  Token* new_token(Module& m, size_t na)const override{
    WRITE* cl = clone();
    cl->set_num_args(na);
    cl->set_label("t_write_" + std::to_string(cnt++));
    m.push_back(cl);
    cl->_m = &m;
    // WIP: remove: use has_*
    m.set_tr_begin(); // WIP, remove.
    m.set_tr_review(); // WIP, remove.
    m.set_tr_accept(); // WIP, remove.
    m.set_tr_advance(); // WIP, remove.
//    m.set_tr_regress(); // WIP, remove.

    return new Token_CALL(label(), cl);
  }
  void args(std::ostream& o, bool names=false)const {
    o << "MOD_" <<_m->identifier()<<"* d, std::string";
    for(size_t i=1; i<num_args(); ++i) {
      o____  ", double";
      if(names){
       o << " a " << i;
      }else{
      }
    }
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "struct cls" << label() << "{\n";

    //template?
    o____ "void tr_eval(CARD* d, std::string";
    for(size_t i=1; i<num_args(); ++i) {
      o____  ", double";
    }
    o << ")const {}\n";

    assert(_m);
    o____ "void tr_begin("; args(o); o << ") {\n";
    o______ "trace1(\"write::tr_begin\", _sim->_time0);\n";
    o______ "assert(d); d->q_accept();\n";
    o____"}\n";

    o____ "void tr_review("; args(o); o << ") {untested();\n";
    o______ " assert(d); d->q_accept();\n";
    o____"}\n";

    o____ "void tr_advance("; args(o); o << ") {\n";
    o______ "trace1(\"write::tr_advance\", _sim->_time0);\n";
    o______ "assert(d); d->q_accept();\n";
    o____"}\n";

    o____ "void tr_regress("; args(o); o << ") { /*nop*/ }\n";

    o____ "void tr_accept(CARD* d, std::string a0";
    for(size_t i=1; i<num_args(); ++i) {
      o____  ", double a" << i << "";
    }
    o << ")const {\n";
    o______ "trace1(\"write::tr_accept\", _sim->_time0);\n";
    if(end().size()){
      o______ "a0 += \"" << end() << "\";\n";
    }else{
    }
    o______ "fprintf(stdout, a0.c_str()";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", a" << i;
    }
    o << ");\n";
    o____ "}\n";

    o____ "void precalc("; args(o); o << ") { /*nop*/ }\n";
    o__ "}_" << label() << ";\n";
  }
  std::string code_name()const override{
    return "d->_" + label() + ".";
  }
  bool returns_void()const override { return true; }
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
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
