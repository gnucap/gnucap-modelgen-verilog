/*                        -*- C++ -*-
 * Copyright (C) 2026 Felix Salfelder
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
 * Verilog-AMS edge events
 */
/*--------------------------------------------------------------------------*/
#include "f__.cc"
#include "mg_event.h"
#include <globals.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class EDG : public MGVAMS_EVENT {
  enum dir{
    dNeg = -1,
    dAny = 0,
    dPos = 1
  }_dir;
  std::string _code_name;
protected:
  Module* _m{nullptr};
  EDG(EDG const& e) : MGVAMS_EVENT(e), _dir(e._dir) { }
public:
  explicit EDG(int d) : MGVAMS_EVENT(), _dir(dir(d)) {
    std::string label[3]{"negedge", "edge", "posedge"};
    set_label(label[d+1]);
  }
  ~EDG(){ }
  virtual EDG* clone()const override { return new EDG(*this);}
private:
  bool static_code()const override {return false;}
  bool is_common()const override { untested();return true;}
  bool is_in_common()const override {return false;}
  bool has_state()const override { untested();return true;}
  bool has_modes()const override {return true;}
  bool has_tr_begin()const override {return false;}
  bool has_tr_restore()const override {return true;}
  bool has_tr_review()const override {return true;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_advance()const override {return true;}
  bool has_set_event()const override {return true;}
  bool needs_context()const override {return false;}
  Token* new_token(Module&, size_t)const override {
    return nullptr;
  }

private:

  void make_cc_tr_review(std::ostream& o)const override {
    o__ "// time_by.min_event(" << _code_name << ".review(this));\n";
  }

  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable();
    return "";
  }
  // "call_name"...
  std::string code_name()const override{
    return label() + "__"; // some_evt"; // _code_name+".";
  }
//   void stack_op(Expression const& args, Expression* out) const override { untested();
//     incomplete();
//   }
  void make_cc_dev(std::ostream& o)const override {
    indent x;
    o__ "// edge dev\n";
    std::string trans;
    std::string stable;
    if(_dir==0){
    }else if(_dir==1){
       trans = "1";
       stable = "3";
    }else{
       trans = "2";
       stable = "0";
    }
    o__ "bool " << label() + "now{false};\n";
    o__ "bool " << label() + "__tr_advance(va::LNR const& ll) {\n";
    o____ "auto l = prechecked_cast<LOGIC_NODE const*>(ll.ptr());\n";
    o____ "assert(l);\n";
    o____ "bool is_final = l->final_time() == _sim->_time0;\n";
    o____ "bool is_lct   = l->last_change_time() == _sim->_time0;\n";
    if(_dir){
      o____ label() + "now = is_final && (l->lv() == " << trans <<
                                       "||l->lv() == " << stable << ");\n";
      o____ label() + "now |= is_lct && (l->lv() == " << stable << ");\n";
    }else{
      o____ label() + "now = is_final;\n";
      o____ label() + "now |= is_lct && (l->lv() == " << 0 << ");\n";
      o____ label() + "now |= is_lct && (l->lv() == " << 3 << ");\n";
    }
    o____ "if(" << label() << "now){\n";
    o______ "q_eval();\n"; // hack?
    o____ "}else{\n";
    o____ "}\n";
//    o____ "return " << label() + "now;\n";
    o____ "return false;\n";
    o__ "}\n";
    o__ "bool " << label() + "__precalc(va::LNR const&) {\n";
    o____ "return false;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_begin(va::LNR const&) {\n";
    o____ "incomplete();\n";
    o____ "return false;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_restore(va::LNR const&) {\n";
    o____ "incomplete();\n";
    o____ "return false;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_eval(va::LNR const&) {\n";
    o____ "return false;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_accept(va::LNR const&) {\n";
    o____ "bool r = " << label() + "now;\n";
    o____ label() + "now = false;\n";
    o____ "return r;\n";
    o__ "}\n";
    o__ "bool " << label() + "__is_evt(va::LNR const&) {\n";
    o____ "return " << label() + "now;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_regress(va::LNR const& n) {\n";
    o____ "if( " << label() + "__tr_advance(n)) {untested();\n";
    o______ "return true;\n";
    o____ "}else{ untested();\n";
    o______ "return false;\n";
    o____ "}\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_review(va::LNR const& i) {\n";
    o____ "if(" << label() + "__is_evt(i)) {\n";
    o______ "q_accept();\n"; // hack? only needed if there is an event.
    o____ "}else{\n";
    o____ "}\n";
    o____ "return false;\n";
    o__ "}\n";
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "// edge common\n";
  }
};
EDG any(0);
EDG pos(1);
EDG neg(-1);
DISPATCHER<FUNCTION>::INSTALL d_any(&function_dispatcher, "edge", &any);
DISPATCHER<FUNCTION>::INSTALL d_pos(&function_dispatcher, "posedge", &pos);
DISPATCHER<FUNCTION>::INSTALL d_neg(&function_dispatcher, "negedge", &neg);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
