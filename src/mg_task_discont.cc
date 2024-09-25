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
class DISCONT : public MGVAMS_TASK {
public:
  explicit DISCONT() : MGVAMS_TASK(){
    set_label("discontinuity");
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    return "$$discontinuity";
  }
  DISCONT* clone()const override {
    return new DISCONT(*this);
  }
 // bool is_common()const override {return true;} // extra CARD*
  bool has_modes()const override { return true;}
  bool has_tr_review()const override {return true;}
  bool has_tr_accept()const override {return true;}
  bool static_code()const override {return false;}
  Token* new_token(Module& m, size_t na)const override{
    m.install(this);
    m.set_tr_review(); // WIP, remove.
    m.set_tr_accept(); // WIP, remove.
    return new Token_CALL(label(), this);
  }
  void make_cc_dev(std::ostream& o)const override {
    std::string n = label();
    //o__ "struct cls" << label() << "{\n";

    //template?
    o____ "void "<<n<<"tr_eval(int i=0)const {}\n";
    o____ "void "<<n<<"tr_review(int i=0) {\n";
    o______ "q_accept();\n";
    o____ "}\n";
    o____ "void "<<n<<"tr_accept(int i=0)const {\n";
    o______ "if(i){ untested();\n";
    o______ "}else{\n";
    o______ "}\n";
    o______ "_sim->new_event(_sim->_time0 + _sim->_dtmin);\n";
    o____ "}\n";
    // o__ "}_" << label() << ";\n";
    o__ "void "<<n<<"precalc(int i=0) const";
    o << "{\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "d->" + label() + "";
  }
  bool returns_void()const override { return true; }
  virtual std::string end()const{return "";}
} disc;
DISPATCHER<FUNCTION>::INSTALL d_disc(&function_dispatcher, "$discontinuity", &disc);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
