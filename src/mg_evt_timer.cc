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
 * Verilog-AMS timer
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
class TIMER : public FUNCTION_ {
protected:
  std::string _code_name;
  Module* _m{NULL};
public:
  explicit TIMER() : FUNCTION_() {
    set_label("timer");
  }
  ~TIMER(){ }
  virtual TIMER* clone()const /*override*/ {return new TIMER(*this);}
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

    TIMER* cl = clone();
    {
      cl->set_label(event_code_name);
      cl->set_code_name(event_code_name);
      if(na>4){
	error(bDANGER, "too many arguments\n");
      }else{
      }
      cl->set_num_args(na);
      cl->_m = &m;
      m.push_back(cl);
      m.set_times(2);
      m.set_tr_review();
      m.set_tr_advance();
      m.set_set_event();
      m.set_tr_accept();
    }
    return new Token_EVT(label(), cl);
  }
  void set_code_name(std::string x){
    _code_name = x;
  }

private:
  void reltime(std::ostream& o, std::string hack="0.")const {
    o______ "double raw_time = d->_sim->_time0 + " << hack << ";\n";
    o______ "double reltime = fmod(raw_time - delay, period);\n";
  }
  bool have_tol()const { return num_args() >= 3; }
  bool have_en()const { return num_args() >= 4; }

  std::string args()const {
    return "(MOD_" + _m->identifier().to_string() + "* d, "
                   + "double delay, double period=0., double tol=0., int en=1)";
  }
  std::string tol()const {
    if(have_tol()) {
      return "tol /* user supplied */";
    }else{
      return "_sim->_dtmin";
    }
  }
  std::string accept_tol()const {
    if(have_tol()) {
      return "1.01*tol /* user supplied */";
    }else{
      return "1.01*_sim->_dtmin";
    }
  }

  void make_cc_dev(std::ostream& o)const override {
    assert(_m);
    make_tag(o);

    o__ "class cls" << _code_name << "{\n";
    o____ "double _req_evt{0.};\n";
    o____ "double _previous_evt{0.};\n";
    o____ "void set_event(MOD_" << _m->identifier() << "* d, double reltime, double abstol) {\n";
    o______ "double newtime = d->new_event(_previous_evt + reltime, abstol);\n";
    o______ "assert(_previous_evt <= newtime);\n"; // == at startup?
    o______ "_req_evt = newtime;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o__ "public:\n";
    o____ "bool precalc" << args() << " {\n";
   // o______ "unreachable();\n";
    o______ "return false;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_eval" << args() << " {\n";
    o______ "trace2(\"tr_eval\", _req_evt, _sim->_time0);\n";
    o______ "if (_sim->_time0 == 0.){\n";
    o________ "tr_begin(d, delay, period, tol, en);\n"; // lost init event in "TRANSIENT::first"?
    o______ "}else{\n";
    o______ "}\n";
    o______ "trace2(\"tr_eval1\", _req_evt, _sim->_time0);\n";
    o______ "if (_req_evt < _sim->_time0){\n";
    o______ "}else if (_req_evt <= _sim->_time0 + _sim->_dtmin) {\n";
    o______ "}else{\n";
    o______ "}\n";
    o______ "return false;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_begin" << args() << " {\n";
    o______ "_previous_evt = 0.;\n";
    o______ "if(delay) {\n";
    o________ "set_event(d, delay, tol);\n";
    o______ "}else{\n";
    o________ "_req_evt = 0.;\n";
    o______ "}\n";
    o______ "return _req_evt == 0.;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_advance" << args() << " {\n";
    o______ "trace3(\"timer::tr_advance\", _previous_evt, _req_evt, _sim->_time0);\n";
    o______ "_previous_evt = _req_evt;\n"; // consolidate previous "tr_accept"
    o______ "return true;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_regress" << args() << "{\n";
    o______ "trace2(\"timer::tr_regress\", _req_evt, _sim->_time0);\n";
    o______ "return true;\n"; // ?
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_review" << args() << " {\n";
    o______ "trace3(\"timer::tr_review\", _req_evt, _sim->_time0, _sim->_dtmin);\n";
    o______ "if (_sim->_time0 < _req_evt) {\n";
    o______ "}else if (_sim->_time0 <= _req_evt + " << accept_tol() << ") {\n";
    o________ "trace2(\"timer::tr_review q accept\", _req_evt, _sim->_time0);\n";
    o________ "d->q_accept();\n";
    o______ "}else if(d->_time[1] <= _req_evt) {\n";
    o________ "double back_to = _previous_evt;\n";
    o________ "trace3(\"timer::tr_review\", _req_evt, _sim->_time0, back_to);\n";
    o________ "d->_time_by.min_event(back_to + _sim->_dtmin);\n";
    o______ "}else{\n";
    o________ "// scheduler issue?\n";
    o________ "throw Exception(to_string(_sim->_time0) + \" \" + d->long_label() + \" timer: giving up on event at\""
              << " + to_string(_req_evt));\n";
    o______ "}\n";
    o______ "return false;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_accept" << args() << " {\n";
    o______ "trace3(\"timer::tr_accept\", _previous_evt, _req_evt, _sim->_time0);\n";
    o______ "trace2(\"timer::tr_accept\", delay, period);\n";
    o______ "if(_sim->_time0 < _req_evt) {\n";
    o________ "return false; // not ours\n";
    o______ "}else if(_sim->_time0 <= _req_evt + " << accept_tol() << ") {\n";
    o________ "if(period) {\n";
    o__________ "set_event(d, period, " << tol() << ");\n";
    o________ "}else if(delay) {\n";
    o__________ "_req_evt = NEVER;\n";
    o________ "}\n";
    o________ "return true;\n";
    o______ "}else if(_sim->analysis_is_static()) {untested();\n";
    o________ "if(!delay) {untested();\n";
    o__________ "set_event(d, period, " << tol() << ");\n";
    o________ "}else{ untested();\n";
    o________ "}\n";
    o________ "return true;\n";
    o______ "}else{ untested();\n";
    o________ "trace3(\"timer::tr_accept miss\", _previous_evt, _req_evt, _sim->_time0);\n";
    o________ "return false;\n";
    o______ "}\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o__ "}"<< _code_name <<";\n";
  }
  void make_cc_precalc(std::ostream& o)const override{
    make_tag(o);
    o__ "class cls" << _code_name << "{\n";
    o__ "public:\n";
    o____ "bool precalc(void* d,\n";
    o____ "               double delay, double period=0., double tol=0., int en=1) {\n";
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
    // o__ "bool " << _code_name << "()const {\n";
    // o____ "incomplete();\n";
    // o____ "assert(0);\n";
    // o__ "}\n";
  }
protected:
} timer;
DISPATCHER<FUNCTION>::INSTALL d_timer(&function_dispatcher, "timer", &timer);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
