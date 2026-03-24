/*                        -*- C++ -*-
 * Copyright (C) 2026 Felix Salfelder
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
 * Verilog-AMS file io tasks
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_.h"
#include "mg_out.cc"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
void Data_Type::parse(CS&){unreachable();}
void Data_Type::dump(std::ostream&)const{unreachable();}
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class FWRITE : public MGVAMS_TASK {
public:
  explicit FWRITE() : MGVAMS_TASK(){
    set_label("$fwrite");
  }
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "$$fwrite";
  }
  FWRITE* clone()const override {
    return new FWRITE(*this);
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
  bool has_final()const override {return false;}
  bool static_code()const override {return false;}
  // Token* new_token(Module&, size_t)const override{ return nullptr; }
  std::ostream& args(std::ostream& o, bool names=false)const {
    o << "int";
    if(names){
      o << " fd";
    }else{
    }
    o << ", string";
    if(names){
      o << " a1";
    }else{
    }
    for(int i=2; i<int(num_args()); ++i) {
      o____  ", T" << i;
      if(names){
       o << " a" << i;
      }else{
      }
    }
    return o;
  }
  void voidargs(std::ostream& o)const {
    for(size_t i=2; i<num_args(); ++i) {
      o << " (void) a" << i << ";\n";
    }
  }
private:
  void template_header(std::ostream& o, bool mod=false)const {
    if(mod || num_args()>2){
      o____ "template<";
      std::string sep;
      if(mod){
	o << "class MOD";
	sep = ", ";
      }else{
      }
      for(size_t i=2; i<num_args(); ++i) {
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
    o____ "void tr_eval(CARD* d, "; args(o, false) << ")const { (void)d; }\n";

    template_header(o, true);
    o____ "void tr_initial(MOD* d, "; args(o, true); o << ")const {\n";
    o______ "tr_accept(d, fd\n";
    for(size_t i=1; i<num_args(); ++i) {
      o << ", a" << i;
    }
    o << ");\n";
    o____"}\n";

    template_header(o, true);
    o____ "void tr_begin(MOD* d,"; args(o, false) << ")const {\n";
    o______ " assert(d); d->q_accept();\n";
    o____"}\n";

    template_header(o, true);
    o____ "void tr_review(MOD* d,"; args(o); o << ") {\n";
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
    o____ "void tr_regress(CARD*, "; args(o) << ") {\n";
    o____"}\n";

    template_header(o);
    o____ "void tr_accept(CARD*, "; args(o, true) << ")const {\n";
    if(end().size()){
      o______ "a1 += \"" << end() << "\";\n";
    }else{
    }
    o______ "if(fd & (1<<31)){\n";
    o________ "dprintf(fd ^ (1<<31), a1.c_str()";
    for(int i=2; i<int(num_args()); ++i) {
      o << ", plain_value(a" << i << ")";
    }
    o << ");\n";
    o______ "}else{\n";
    o________ "for(int i=1; i<31; ++i) {\n";
    o__________ "if(fd & (1<<i)){\n";
    o__________ "  dprintf(i, a1.c_str()";
    for(int i=2; i<int(num_args()); ++i) {
      o << ", plain_value(a" << i << ")";
    }
    o << ");\n";
    o__________ "}else{\n";
    o__________ "}\n";
    o________ "}\n";
    o______ "}\n";
    o____ "}\n";

    template_header(o);
    o____ "void finish(CARD* d, "; args(o, true) << ")const {\n";
    o______ "tr_accept(d, fd\n";
    for(int i=1; i<int(num_args()); ++i) {
      o << ", a" << i;
    }
    o << ");\n";
    o____ "}\n";

    template_header(o);
    o____ "void precalc(CARD const*, "; args(o); o << ")const { /*nop*/ }\n";

    template_header(o);
    o____ "void af(CARD const*, "; args(o); o << ")const { incomplete(); /* BUG */ }\n";
    o__ "}_" << label() << ";\n";
  }
  std::string code_name()const override{
    return "/*w*/_" + label() + ".";
  }
  // Data_Type const* return_type()const override { return nullptr; }
  virtual std::string end()const{return "";}
} fwrite;
DISPATCHER<FUNCTION>::INSTALL d_fwrite(&function_dispatcher, "$fwrite", &fwrite);
/*--------------------------------------------------------------------------*/
class FSTROBE : public FWRITE {
public:
  explicit FSTROBE() : FWRITE() {
    set_label("$fstrobe");
  }
  std::string end()const override { untested(); return "\\n";}
  FWRITE* clone()const override { untested();
    return new FSTROBE(*this);
  }
} fstrobe;
DISPATCHER<FUNCTION>::INSTALL d_fstrobe(&function_dispatcher, "$fstrobe", &fstrobe);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
