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
class INITIAL_MODEL : public MGVAMS_EVENT {
public:
  explicit INITIAL_MODEL() : MGVAMS_EVENT() {
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
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
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
class INITIAL_STEP : public MGVAMS_EVENT {
public:
  explicit INITIAL_STEP() : MGVAMS_EVENT() {
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
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
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
class CROSS : public MGVAMS_EVENT {
protected:
  std::string _code_name;
  Module* _m{nullptr};
public:
  explicit CROSS() : MGVAMS_EVENT() {
    set_label("cross");
  }
  ~CROSS(){ }
  virtual CROSS* clone()const override {return new CROSS(*this);}
private:
  bool static_code()const override {return false;}
  bool is_common()const override {return true;}
  bool has_modes()const override {return true;}
  bool has_tr_begin()const override {return true;}
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
      // BUG: use has_* above
      m.set_tr_begin();
      m.set_tr_review();
      m.set_tr_advance();
      m.set_tr_accept();
      m.set_times(2);
    }
    return new Token_EVT(label(), cl);
  }
  void set_code_name(std::string x){
    _code_name = x;
  }
  virtual bool above_hack()const {return false;}
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
    make_tr_begin(o);
    make_tr_accept(o);

    o____ "bool tr_advance(" << args(o) << ") {\n";
    o______ "(void)d;\n";
    o______ "(void)dir;\n";
    o______ "(void)enable;\n";
    o______ "(void)time_tol;\n";
    o______ "(void)expr_tol;\n";
#if 1 // possibly move to accept?
    o______ "_in[1] = _in[0];\n";
    o______ "_state[1] = _state[0];\n";
    o______ "if(_in[0] == 0.) {\n";
    o______ "}else if(_in[0]>0.){\n";
    o________ "_state[0] = _ON;\n";
    o______ "}else{\n";
    o________ "_state[0] = _OFF;\n";
    o______ "}\n";
    o______ "_in[0] = input;\n";
    o______ "if(_state[1] != _state[0]) {\n";
   // o________ "d->q_eval();\n";
    o______ "}else{\n";
    o______ "}\n";
#endif
    o______ "return false;\n"; // no need to advance body, state change in accept.
    o____ "}\n";

    o____ "bool tr_review(" << args(o) << ") {\n";
    o______ "(void)time_tol;\n";
    o______ "(void)expr_tol;\n";
    o______ "(void)enable;\n";
    o______ "(void)dir;\n";
    o______ "_in[0] = input;\n";
    o______ "double old_dv = _in[0] - _in[1];\n";
    o______ "double old_dt = d->_time[0] - d->_time[1];\n";
    o______ "trace6(\"cross::tr_review\", _sim->_time0, input, old_dt, old_dv, _in[0], _in[1]);\n";
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
    o______ "if(!enable){\n";
    o________ "return false;\n";
    o______ "}else if(_in[0] * _in[1] < 0) {\n";
    // incomplete(); // avoid few accepts depending on dir flag.
    o________ "d->q_accept();\n";
    o________ "return true;\n";
    if(above_hack()){
      o______ "}else if(_sim->analysis_is_static()){\n";
      o________ "d->q_accept();\n";
      o________ "return _in[0]>0;\n";
    }else{
    }
    o______ "}else{\n";
    o________ "return false;\n";
    o______ "}\n";
    o____ "}\n";

    o____ "bool tr_regress(" << args(o) << ") {\n";
    o______ "(void)d;\n";
    o______ "(void)input;\n";
    o______ "(void)dir;\n";
    o______ "(void)time_tol;\n";
    o______ "(void)enable;\n";
    o______ "(void)expr_tol;\n";
    o______ "_in[0] = input;\n";
    o______ "return false;\n"; // ?
    o____ "}\n";
    o____ "bool precalc(" << args(o) << ") {\n";
    o______ "(void)d;\n";
    o______ "(void)input;\n";
    o______ "(void)dir;\n";
    o______ "(void)time_tol;\n";
    o______ "(void)enable;\n";
    o______ "(void)expr_tol;\n";
    o______ "return false;\n";
    o____ "}\n";
    o__ "}"<< _code_name <<";\n";
  }

  void make_cc_tr_review(std::ostream& o)const override {
    make_tag(o);
    o__ "// time_by.min_event(" << _code_name << ".review(this));\n";
  }

  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
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
  virtual std::string args(std::ostream& o)const {
    o << "MOD_" << _m->identifier() << "* d, double input,"
      << "int dir=0, double time_tol=0., double expr_tol=0., int enable=1";
    return "";
  }
  virtual std::string evalargs(std::ostream& o)const {
    o << "d, input, dir, time_tol, expr_tol, enable";
    return "";
  }
  virtual void make_tr_begin(std::ostream& o)const {
    o____ "bool tr_begin(" << args(o) << ") {\n";
    o______ "(void)d;\n";
    o______ "(void)dir;\n";
    o______ "(void)enable;\n";
    o______ "(void)input;\n";
    o______ "if(time_tol)incomplete();\n";
    o______ "if(expr_tol)incomplete();\n";
    o______ "(void)dir;\n";
    o______ "_state[0] = _state[1] = _UNKNOWN;\n";
    o______ "_in[0] = _in[1] = 0.;\n";
    if(above_hack()){
      o______ "return input>0.;\n";
    }else{
      o______ "return false;\n";
    }
    o____ "}\n";
  }
  virtual void make_tr_eval(std::ostream& o)const {
    o____ "bool tr_eval(" << args(o) << ") {\n";
    o________ "tr_eval_("<<evalargs(o)<<");\n";
    o______ "return false;\n";
    o____ "}\n";
    o____ "bool tr_eval_(" << args(o) << ") {\n";
    o______ "(void)d;\n";
    o______ "(void)dir;\n";
    o______ "(void)expr_tol;\n";
    o______ "(void)time_tol;\n";
    o______ "(void)enable;\n";
    o______ "trace4(\"sw::tr_eval\", _sim->_time0, input, _state[0], _state[1]);\n";
    o______ "trace3(\"sw::tr_eval\", _sim->_time0, _in[0], _in[1]);\n";
    o______ "if (_sim->analysis_is_static()) {\n";
    o________ "if(input == 0.){\n";
    o________ "  _state[0] = _state[1] = _UNKNOWN;\n";
    o________ "}else{\n";
    o________ "  _state[0] = _state[1] = (input>0.)?_ON:_OFF;\n";
    o________ "}\n";
    o________ "return false;\n";
    o______ "}else if(!_sim->analysis_is_tran_dynamic()) {\n";
    o________ "return false;\n";
    o______ "}else if(_state[0] == _state[1]) {\n";
    o________ "return false;\n";
    o______ "}else if(_state[0] == _UNKNOWN || _state[1] == _UNKNOWN) {\n";
    o________ "return false;\n";
    o______ "}else if(dir == 0) {\n";
    o________ "return _state[0] != _UNKNOWN && _state[0] != _state[1];\n";
    o______ "}else if(dir == 1) {\n";
    o________ "return _state[1] == _OFF && _state[0] == _ON;\n";
    o______ "}else if(dir == -1) {\n";
    o________ "return _state[1] == _ON && _state[0] == _OFF;\n";
    o______ "}else{\n";
    o________ "return false;\n";
    o______ "}\n";
    o____ "}\n";
  }
  virtual void make_tr_accept(std::ostream& o)const {
    o____ "bool tr_accept(" << args(o) << ") {\n";
    o______ "(void)d;\n";
    o______ "(void)time_tol;\n";
    o______ "bool ret;\n";
    o______ "(void)expr_tol;\n";
    o______ "if (_sim->analysis_is_static()) {\n";
    o________ "if(input == 0.){\n";
    o________ "  _state[0] = _state[1] = _UNKNOWN;\n";
    o________ "}else{\n";
    o________ "  _state[0] = _state[1] = (input>0.)?_ON:_OFF;\n";
    o________ "}\n";
    if(above_hack()){
      o________ "ret = tr_eval_("<<evalargs(o)<<");\n";
    }else{
      o________ "ret = false;\n";
    }
    o______ "}else if(dir==0) {\n";
    o________ "ret =  (_state[0] == _OFF && _in[0] > 0)\n";
    o________ "    || (_state[0] == _ON  && _in[0] < 0);\n";
    o______ "}else if(dir==1) {\n";
    o________ "ret = _state[0] == _OFF && _in[0] > 0;\n";
    o______ "}else if(dir==-1) {\n";
    o________ "ret = _state[0] == _ON && _in[0] < 0;\n";
    o______ "}else{untested();\n";
    o________ "ret = false;\n";
    o______ "}\n";
    o______ "if(ret) trace4(\"cross::tr_accept\", input, _state[0], _state[1], _in[0]);\n";
    o______ "return ret;\n";
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
  ABOVE* clone()const override {return new ABOVE(*this);}
  void make_tr_eval(std::ostream& o)const override {
    o____ "bool tr_eval(" << args(o) << ") {\n";
    o________ "tr_eval_("<<evalargs(o)<<");\n";
    o______ "return false;\n";
    o____ "}\n";
    o____ "bool tr_eval_(" << args(o) << ") {\n";
    o______ "(void)d;\n";
    o______ "(void)dir;\n";
    o______ "assert(dir==1);\n";
    o______ "if(time_tol)incomplete();\n";
    o______ "if(expr_tol)incomplete();\n";
    o______ "trace2(\"above::tr_eval\", input, _state[0]==_ON);\n";
    o______ "_in[0] = input;\n";
    o______ "if (!enable) {untested();\n";
    o________ "return false;\n";
    o______ "}else if (_sim->analysis_is_static()) {\n";
    o________ "return _in[0] > 0.;\n";
    o______ "}else{\n";
    o________ "return _in[0] > 0. && _state[1] != _ON;\n";
    o______ "}\n";
    o____ "}\n";
  }
  bool above_hack()const override {return true;}
private:
  std::string args(std::ostream& o)const override {
    o << "MOD_" << _m->identifier() << "* d, double input,"
      << "double time_tol=0., double expr_tol=0., int enable=1, int dir=1";
    return "";
  }
  std::string evalargs(std::ostream&)const override {
    return "d, input, time_tol, expr_tol, enable";
  }
} above;
DISPATCHER<FUNCTION>::INSTALL d_above(&function_dispatcher, "above", &above);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
