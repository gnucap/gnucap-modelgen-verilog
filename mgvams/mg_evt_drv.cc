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
 * Verilog-AMS addition of a new pending value
 */
/*--------------------------------------------------------------------------*/
#include "f__.cc"
#include "mg_event.h"
#include <globals.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DRV_UPD : public MGVAMS_EVENT {
  std::string _code_name;
protected:
  Module* _m{nullptr};
  DRV_UPD(DRV_UPD const& e) : MGVAMS_EVENT(e) { }
public:
  explicit DRV_UPD() : MGVAMS_EVENT() {
    set_label("driver_update");
  }
  ~DRV_UPD(){ }
  virtual DRV_UPD* clone()const override { return new DRV_UPD(*this);}
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
    o__ "double " << label() + "update{NEVER};\n";
    o__ "bool " << label() + "transit{false};\n";
    o__ "bool " << label() + "now{false};\n";
    o__ "bool " << label() + "__tr_advance(va::LNR const& ll) {\n";
    o____ "auto l = prechecked_cast<LOGIC_NODE const*>(ll.ptr());\n";
    o____ "assert(l);\n";
    o____ "bool now = _sim->_time0 == l->last_change_time();\n";
    o____ "double larel = l->last_change_time() - _sim->_time0;\n";
    o____ "trace3(\"ADV\", _sim->_time0, larel, l->final_time());\n";
    o____ label() + "now = now;\n";
    o____ "if(now){ \n";
    o______ "q_eval();\n"; // hack?
    o____ "}else{\n";
    o____ "}\n";
    o____ "return now;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_eval(va::LNR const&) {\n";
    o____ "return false;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_accept(va::LNR const& i) {\n";
    o____ "auto l = prechecked_cast<LOGIC_NODE const*>(i.ptr());\n";
    o____ "trace4(\"acc transit?\", _sim->_time0, l->last_change_time(), l->final_time(), l->in_transit());\n";
    o____ "bool now = _sim->_time0 == l->last_change_time();\n";
    o____ "if(_sim->_time0 == l->final_time()){\n";
    o______ "trace2(\"acc update?\", _sim->_time0, l->final_time());\n";
    o____ "}else{\n";
    o____ "}\n";
    o____ "bool r = false;\n";
    o____ "if(" << label() + "update == _sim->_time0) {\n";
    o______ "r = true;\n";
    o______ label() + "update = NEVER;\n";
    o____ "}else{\n";
    o____ "}\n";
    o____ "return now && l->in_transit();\n";
    o__ "}\n";
    o__ "bool " << label() + "__is_evt(va::LNR const&) {\n";
    o____ "bool r = " << label() + "now;\n";
    o____ "return r;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_regress(va::LNR const&) { untested();\n";
    o____ "incomplete(); return true;\n";
    o__ "}\n";
    o__ "bool " << label() + "__tr_review(va::LNR const& i) {\n";
    o____ "auto l = prechecked_cast<LOGIC_NODE const*>(i.ptr());\n";
    //o____ "bool now = _sim->_time0 == l->last_change_time();\n";
    o____ label() << "transit = l->in_transit();\n";
    o____ "double larel = l->last_change_time() - _sim->_time0;\n";
    o____ "trace4(\"rev transit?\", _sim->_time0, l->final_time(), larel, l->in_transit());\n";
    o____ "if(l->final_time() == NEVER){\n";
    o____ "}else if(l->last_change_time() == _sim->_time0 && _sim->_time0 < l->final_time()){untested();\n";
    o____ "}else if(l->last_change_time() < _sim->_time0 && _sim->_time0 < l->final_time()){\n";
    o______ "// stray event\n";
    o______ "trace2(\"rev update?\", _sim->_time0, l->last_change_time());\n";
    o______ "q_accept();\n"; // hack? only needed if there is an event.
    o____ "}else if(!" << label() << "transit) {\n";
    o____ "}else if(_sim->_time0 == l->last_change_time()){\n";
    o______ "trace1(\"in transit\", _sim->_time0);\n";
    o______ "q_accept();\n"; // hack? only needed if there is an event.
    o____ "}else{\n";
    o____ "}\n";
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
}e0;
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "driver_update", &e0);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
