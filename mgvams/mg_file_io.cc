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
class FILE_OPEN : public MGVAMS_TASK {
public:
  explicit FILE_OPEN() : MGVAMS_TASK(){
    set_label("$fopen");
  }
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "$$fopen";
  }
  FILE_OPEN* clone()const override {
    return new FILE_OPEN(*this);
  }
  Data_Type const* return_type()const override {
    static Data_Type_Int r; return &r;
  }
  bool is_common()const override {untested(); return true;}
  bool is_in_common()const override { return false;} // BUG?
  bool has_modes()const override { return true;}
  bool has_state()const override {untested(); return false;}
  bool needs_context()const override { return true;} // has accept?
  bool has_tr_review()const override {return true;}
  bool has_tr_advance()const override {return false;}
  bool has_tr_regress()const override { untested();return false;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool has_tr_restore()const override {return false;}
  bool static_code()const override {return false;}
  // Token* new_token(Module&, size_t)const override{ return nullptr; }
  std::ostream& args(std::ostream& o, bool names=false)const {
    o << "std::string";
    if(names){
      o << " fn";
    }else{
    }
    o << ", std::string";
    if(names){
      o << " mode";
    }else{
    }
    o << "=\"\"";
    return o;
  }
  void voidargs(std::ostream& o)const {
    o__ "(void) fn;\n";
    o__ "(void) mode;\n";
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

    o____ "integer tr_eval(CARD* d, "; args(o) << ")const { (void)d; return 0; }\n";

    o____ "integer tr_initial(CARD* d, std::string fn, std::string mode=\"\") {\n";
    o______ "return tr_accept(d, fn, mode);\n";
    o____ "}\n";

    o____ "integer tr_begin(MOD* d, "; args(o) << ") {\n";
    o______ "assert(d); d->q_accept(); return 0;\n";
    o____ "}\n";

    o____ "integer tr_review(MOD* d, "; args(o) << ") {\n";
    o______ "assert(d); d->q_accept(); return 0;\n";
    o____ "}\n";

    o____ "integer tr_advance(MOD* d, "; args(o) << ") {\n";
    o______ "assert(d); d->q_accept(); return 0;\n";
    o____ "}\n";

    o____ "integer tr_regress(CARD*, "; args(o) << ") { return 0; }\n";

    o____ "integer tr_accept(CARD*, std::string fn, std::string mode=\"\")const {\n";
    o______ "integer fd;\n";
    o______ "if(mode.size()) {\n";
    o________ "FILE* f = fopen(fn.c_str(), mode.c_str());\n";
    o________ "fd = fileno(f);\n";
    o________ "assert(!(fd & (1<<31)));\n";
    o________ "fd |= 1<<31;\n";
    o______ "}else{untested();\n";
    o________ "incomplete();\n";
    o______ "}\n";
    o______ "return fd;\n";
    o____ "}\n";

    o____ "integer precalc(CARD const*, "; args(o) << ")const { /*nop*/ return 0; }\n";

    o____ "integer af(CARD const*, "; args(o) << ")const { incomplete(); /* BUG */  return 0;}\n";
    o__ "}_" << label() << ";\n";
  }
  std::string code_name()const override{
    return "/*w*/_" + label() + ".";
  }
  // Data_Type const* return_type()const override { return nullptr; }
  virtual std::string end()const{return "";}
} fopen;
DISPATCHER<FUNCTION>::INSTALL d_fopen(&function_dispatcher, "$fopen", &fopen);
/*--------------------------------------------------------------------------*/
class FILE_CLOSE : public MGVAMS_TASK {
public:
  explicit FILE_CLOSE() : MGVAMS_TASK(){
    set_label("$fclose");
  }
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    return "$$fclose";
  }
  FILE_CLOSE* clone()const override {
    return new FILE_CLOSE(*this);
  }
  // Data_Type const* return_type()const override {
  //   static Data_Type_Int r; return &r;
  // }
  bool is_common()const override {untested(); return true;}
  bool is_in_common()const override { return false;} // BUG?
  bool has_modes()const override { return true;}
  bool has_state()const override {untested(); return false;}
  bool needs_context()const override { return true;} // has accept?
  bool has_tr_review()const override {return false;}
  bool has_tr_advance()const override {return false;}
  bool has_tr_regress()const override { untested();return false;}
  bool has_tr_accept()const override {return false;}
  bool has_tr_begin()const override {return false;}
  bool has_tr_restore()const override {return false;}
  bool has_final()const override { return true;}
  bool static_code()const override {return false;}
  // Token* new_token(Module&, size_t)const override{ return nullptr; }
  std::ostream& args(std::ostream& o, bool names=false)const {
    o << "integer";
    if(names){
      o << " fd";
    }else{
    }
    return o;
  }
  void voidargs(std::ostream& o)const {
    o__ "(void) fn;\n";
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

    o____ "void tr_eval(CARD*, "; args(o) << ")const { }\n";
    o____ "void tr_initial(CARD*, "; args(o) << ") { untested(); }\n";
    o____ "void tr_begin(CARD*, "; args(o) << ") { untested(); }\n";
    o____ "void tr_review(CARD*, "; args(o) << ") { untested(); }\n";
    o____ "void tr_advance(CARD*, "; args(o) << ") { untested(); }\n";
    o____ "void tr_regress(CARD*, "; args(o) << ") { untested(); }\n";
    o____ "void tr_accept(CARD*, "; args(o, true) << ") {\n";
    o______ "if(fd & (1<<31)){\n";
    o________ "close(fd ^ (1<<31));\n";
    o______ "}else{untested();\n";
    o______ "}\n";
    o____ "}\n";
    o____ "void precalc(CARD*, "; args(o) << ") { untested(); }\n";
    o____ "void finish(CARD* d, "; args(o, true) << ") {\n";
    o______ "tr_accept(d, fd);\n";
    o____ "}\n";

    o____ "void af(CARD const*, "; args(o) << ")const { incomplete(); /* BUG */ }\n";
    o__ "}_" << label() << ";\n";
  }
  std::string code_name()const override{
    return "/*w*/_" + label() + ".";
  }
  // Data_Type const* return_type()const override { return nullptr; }
  virtual std::string end()const{return "";}
} fclose;
DISPATCHER<FUNCTION>::INSTALL d_fclose(&function_dispatcher, "$fclose", &fclose);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
