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
 * Verilog-AMS event functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include "mg_token.h"
#include <globals.h>
#include "mg_.h" // BUG
/*--------------------------------------------------------------------------*/
namespace {
static int n_events;
/*--------------------------------------------------------------------------*/
class INITIAL_MODEL : public FUNCTION_ {
public:
  explicit INITIAL_MODEL() : FUNCTION_() {
    set_label("initial_model");
  }
  ~INITIAL_MODEL(){ }
private:
  bool static_code()const override {return false;} // really?
  Token* new_token(Module& m, size_t)const override { untested();
    std::cerr << "WARNING: ADMS style keyword encountered\n";
    // throw? just fix the model source..
    m.install(this);
    return new Token_EVT(label(), this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable();
    return "";
  }
  void make_cc_common(std::ostream& o)const override { untested();
    incomplete();
    o__ "bool " << code_name() << "(PARA_BASE const& p)const {\n";
    o__ "evt = _sim->is_initial_step();\n"; // really?
    o__ "}\n";
  }
} initial_model;
DISPATCHER<FUNCTION>::INSTALL d_initial_model(&function_dispatcher, "initial_model", &initial_model);
/*--------------------------------------------------------------------------*/
class INITIAL_STEP : public FUNCTION_ {
public:
  explicit INITIAL_STEP() : FUNCTION_() {
    set_label("initial_step");
  }
  ~INITIAL_STEP(){ }
private:
  bool static_code()const override {return false;} // really?
  Token* new_token(Module& m, size_t)const override {
    m.set_tr_advance();
    m.install(this);
    return new Token_EVT(label(), this);
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable();
    return "";
  }
  std::string code_name()const override {
    return "initial_step";
  }
//   void stack_op(Expression const& args, Expression* out) const override { untested();
//     incomplete();
//   }
  void make_cc_common(std::ostream& o)const override {
    o__ "bool " << code_name() << "()const {\n";
    o__ "return _sim->_phase == p_INIT_DC; //common\n";
//    o__ "evt = _sim->vams_initial_step();\n"; // TODO
    o__ "}\n";
  }
} initial_step;
DISPATCHER<FUNCTION>::INSTALL d_initial_step(&function_dispatcher, "initial_step", &initial_step);
/*--------------------------------------------------------------------------*/
class CROSS : public FUNCTION_ {
protected:
  std::string _code_name;
  Module* _m{NULL};
public:
  explicit CROSS() : FUNCTION_() {
    set_label("cross");
  }
  ~CROSS(){ }
  virtual CROSS* clone()const /*override*/ {return new CROSS(*this);}
private:
  bool static_code()const override {return false;}
  bool is_common()const override {return true;}
  bool has_modes()const override {return true;}
  bool has_tr_review()const override {return true;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_advance()const override {return true;}
  // bool is_class()const override { untested();return true;}

  Token* new_token(Module& m, size_t na)const override {
    std::string event_code_name = "_evt_" + label() + "_" + std::to_string(n_events++);

    CROSS* cl = clone();
    {
      cl->set_label(event_code_name);
      cl->set_code_name(event_code_name);
      if(na<3){
      }else{ untested();
	incomplete();
	error(bDANGER, "too many arguments\n");
      }
      cl->set_num_args(na);
      cl->_m = &m;
      m.push_back(cl);
      m.set_tr_review();
      m.set_tr_advance();
     // m.set_tr_accept(); //?
      m.set_times(2);
    }
    return new Token_EVT(label(), cl);
  }
  void set_code_name(std::string x){
    _code_name = x;
  }
  void make_cc_dev(std::ostream& o)const override {
    assert(_m);
    make_tag(o);
   // o__ "enum state_t {_UNKNOWN, _ON, _OFF};\n";
   // TODO:: remove precalc.
    o__ "class cls" << _code_name << "{\n";
    o____ "enum state_t {_OFF = -1, _UNKNOWN = 0, _ON = 1};\n";
    o____ "double _in[2]{0.};\n";
    o____ "state_t _state[2]{_UNKNOWN};\n";
    o__ "public:\n";

    make_tr_eval(o);

    o____ "bool tr_accept(MOD_" << _m->identifier() << "* d,\n";
    o____ "               double input, int dir, int more=0){\n";
    o______ "trace2(\"cross::tr_accept\", _state[0], _state[1]);\n";
    o______ "return tr_eval(d, input, dir, more);\n";
    o____ "}\n";

    o____ "bool tr_advance(MOD_" << _m->identifier() << "* d,\n";
    o____ "                double input, int dir, int more=0){\n";
    o______ "_state[1] = _state[0];\n";
    o______ "_in[1] = _in[0];\n";
    o______ "trace3(\"cross::tr_advance\", _in[0], _in[1], input);\n";
    o______ "_state[0] = (_in[0] == 0.)?_UNKNOWN:(_in[0]>0.)?_ON:_OFF;\n";
    o______ "if(_state[1] != _state[0]) {\n";
    o________ "d->q_eval();\n";
    o________ "return true;\n"; // really?
    o______ "}else{\n";
    o______ "}\n";
    o______ "return tr_eval(d, input, dir, more);\n";
    o______ "return false;\n";
    o____ "}\n";

    o____ "bool tr_review(MOD_" << _m->identifier() << "* d,\n";
    o____ "               double input, int dir, int more=0){\n";
    o______ "_in[0] = input;\n";
    o______ "double old_dv = _in[0] - _in[1];\n";
    o______ "double old_dt = d->_time[0] - d->_time[1];\n";
    o______ "trace5(\"cross::tr_review\", old_dt, old_dv, _in[0], _in[1], input);\n";
    o______ "if(_state[0] != _ON && old_dv > 0) {\n";
    o________ "double new_dv = - _in[1];\n";
    o________ "double new_dt = old_dt * new_dv / old_dv;\n";
    o________ "trace2(\"cross::tr_review\", d->_time[1], new_dt);\n";
    o________ "d->_time_by.min_event(d->_time[1] + new_dt);\n";
    o______ "}else if(_state[0] != _OFF && old_dv < 0){\n";
    o________ "double new_dv = - _in[1];\n";
    o________ "double new_dt = old_dt * new_dv / old_dv;\n";
    o________ "trace2(\"cross review\", d->_time[1], new_dt);\n";
    o________ "d->_time_by.min_event(d->_time[1] + new_dt);\n";
    o______ "}else{\n";
    o______ "}\n";
    o______ "return tr_eval(d, input, dir, more);\n";
    //o______ "return _state[1] != _state[0];\n";
    o____ "}\n";

    o____ "bool tr_regress(MOD_" << _m->identifier() << "* d,\n";
    o____ "                double input, int dir, int more=0) {\n";
    o______ "_in[0] = input;\n";
    o______ "return false;\n"; // ?
    o____ "}\n";
    o____ "bool precalc(MOD_" << _m->identifier() << "* d,\n";
    o____ "             double input, int dir, int more=0) {\n";
    o______ "return false;\n";
    o____ "}\n";
    o__ "}"<< _code_name <<";\n";
  }

  void make_cc_tr_review(std::ostream& o)const override {
    make_tag(o);
    o__ "// time_by.min_event(" << _code_name << ".review(this));\n";
  }

  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable();
    return "";
  }
  // "call_name"...
  std::string code_name()const override{
    return "d->"+_code_name+".";
  }
//   void stack_op(Expression const& args, Expression* out) const override { untested();
//     incomplete();
//   }
  void make_cc_common(std::ostream& o)const override {
    incomplete();
    o__ "bool " << _code_name << "()const {\n";
    o____ "incomplete();\n";
    o____ "assert(0);\n";
    o__ "}\n";
  }
protected:
  virtual void make_tr_eval(std::ostream& o)const {
    o____ "bool tr_eval(MOD_" << _m->identifier() << "* d,\n";
    o____ "             double input, int dir, int more=0) {\n";
    o______ "trace3(\"sw::tr_eval\", _sim->_time0, _state[0], _state[1]);\n";
    o______ "if (_sim->analysis_is_static()) {\n";
    o________ "if(input == 0.){\n";
    o________ "  _state[0] = _state[1] = _UNKNOWN;\n";
    o________ "}else{\n";
    o________ "  _state[0] = _state[1] = (input>0.)?_ON:_OFF;\n";
    o________ "}\n";
    o________ "return false;\n";
    o______ "}else if(!_sim->analysis_is_tran_dynamic()) {untested();\n";
    o________ "return false;\n";
    o______ "}else if(_state[0] == _state[1]) {\n";
    o________ "return false;\n";
    o______ "}else if(_state[0] == _UNKNOWN || _state[1] == _UNKNOWN) {\n";
    o________ "return false;\n";
    o______ "}else if(dir==0) {\n";
    o________ "return _state[0] != _state[1];\n";
    o______ "}else if(dir==1) {\n";
    o________ "return _state[0] == _ON;\n";
    o______ "}else if(dir==-1) {\n";
    o________ "return !_state[0] == _OFF;\n";
    o______ "}else{\n";
    o________ "return false;\n";
    o______ "}\n";
    o____ "}\n";
  }
} cross;
DISPATCHER<FUNCTION>::INSTALL d_cross(&function_dispatcher, "cross", &cross);
/*--------------------------------------------------------------------------*/
class ABOVE : public CROSS {
public:
  explicit ABOVE() : CROSS() {
    set_label("above");
  }
  CROSS* clone()const override {return new ABOVE(*this);}
  void make_tr_eval(std::ostream& o)const override {
    o____ "bool tr_eval(MOD_" << _m->identifier() << "* d,\n";
    o____ "             double input, int dir, int more=0){\n";
    o______ "trace2(\"above::tr_eval\", input, _state[0]==_ON);\n";
    o______ "return _state[0] == _ON;\n";
    o____ "}\n";
  }
} above;
DISPATCHER<FUNCTION>::INSTALL d_above(&function_dispatcher, "above", &above);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
