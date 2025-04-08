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
class TIMER : public MGVAMS_EVENT {
protected:
  std::string _code_name;
  Module* _m{nullptr};
public:
  explicit TIMER() : MGVAMS_EVENT() {
    set_label("timer");
  }
  ~TIMER(){ }
  virtual TIMER* clone()const override {return new TIMER(*this);}
private:
  bool static_code()const override {return false;}
  bool is_common()const override {return true;}
  bool has_modes()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool has_tr_restore()const override {return true;}
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
      if(na>4){ untested();
	error(bDANGER, "too many arguments\n");
      }else{
      }
      cl->set_num_args(na);
      cl->_m = &m;
      m.push_back(cl);
      m.set_times(2);
      // TODO
      m.set_tr_begin();
      m.set_tr_restore();
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
  bool have_tol()const { return num_args() >= 3; }
  bool have_en()const { untested(); return num_args() >= 4; }

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
    o____ "double _req_evt{0.};\n"; // -NEVER?
    o____ "double _previous_evt{-NEVER};\n";
    o____ "void set_event(MOD_" << _m->identifier() << "* d, double abstime, double abstol) {\n";
    o______ "trace2(\"timer::set_event\", _previous_evt, abstime);\n";
    o______ "assert(abstime> _sim->_time0);\n";
    o______ "double newtime = d->new_event(abstime, abstol);\n";
    o______ "trace3(\"set_event1\", _previous_evt, newtime,  _previous_evt - newtime);\n";
    o______ "assert(_previous_evt <= newtime || _previous_evt == NEVER);\n"; // == at startup?
    o______ "_req_evt = newtime;\n";
    o______ "trace3(\"timer::set_event1\", _previous_evt, newtime, _req_evt - _sim->_time0);\n";
    o______ "assert(is_q(_sim->_time0));\n";
    o______ "assert(is_q(_req_evt));\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o__ "public:\n";
    o____ "bool precalc(void*,\n";
    o____ "             double, double period=0., double tol=0., int en=1) {\n";
    o______ "(void)period;\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n";
    o______ "return false;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    // not needed?
    o____ "bool tr_eval" << args() << " {\n";
    o______ "(void)d;\n";
    o______ "(void)period;\n";
    o______ "(void)delay;\n";
    o______ "(void)period;\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n";
    o______ "trace2(\"tr_eval\", _req_evt, _sim->_time0);\n";
    o______ "if (_sim->_time0 == 0.){\n";
   // o________ "tr_begin(d, delay, period, tol, en);\n"; // lost init event in "TRANSIENT::first"?
    o______ "}else{\n";
    o______ "}\n";
    o______ "trace2(\"timer::tr_eval1\", _req_evt, _sim->_time0);\n";
    o______ "if (_req_evt < _sim->_time0){\n";
    o______ "}else if (_req_evt <= _sim->_time0 + _sim->_dtmin) {\n";
    o______ "}else{\n";
    o______ "}\n";
    o______ "return false;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_restore" << args() << " {\n";
    o______ "(void)d;\n";
    o______ "(void)period;\n";
    o______ "(void)delay;\n";
    o______ "(void)period;\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n"; // incomplete
    o______ "if(_req_evt == NEVER){\n";
    o______ "}else if(_req_evt == NEVER){\n";
    o______ "}else if(_req_evt == _sim->_time0){\n";
    o______ "}else{\n";
    o________ "assert(_sim->_dtmin);\n";
    o________ "_req_evt = std::round(_req_evt/_sim->_dtmin) * _sim->_dtmin;\n";
    o______ "}\n";
    o______ "assert(is_q(_req_evt));\n";
    o______ "return true;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_begin" << args() << " {\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n"; // incomplete
    o______ "_previous_evt = -NEVER;\n"; // -NEVER?
    o______ "_req_evt = 0.;\n"; // -NEVER?
    o______ "if(delay) {\n";
    o________ "_previous_evt = -NEVER;\n";
    o________ "trace2(\"timer::tr_begin2\", _sim->_time0, _req_evt - delay);\n";
    o______ "}else if(period){\n";
    o______ "}else{\n";
    o________ "incomplete();\n";
    o________ "_previous_evt = 0;\n";
    o________ "_req_evt = NEVER;\n;";
    o________ "trace2(\"timer::tr_begin2c\", _req_evt, _previous_evt);\n";
    o______ "}\n";

    // o________ "set_event(d, delay, 0);\n";
    o______ "d->q_accept();\n"; // BUG. don't know where to put it.
				// need to recover when omitted.
    o______ "return _previous_evt == 0.;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o << "#ifndef NDEBUG\n";
    o____ "bool is_q(double x) const {\n";
    o______ "return !_sim->_dtmin || x==NEVER || x==-NEVER || std::round(x/_sim->_dtmin) * _sim->_dtmin == x;\n";
    o____ "}\n";
    o << "#endif\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_advance" << args() << "const {\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n"; // incomplete
    o______ "trace3(\"timer::tr_advance\", _previous_evt, _req_evt, _sim->_time0);\n";
    o______ "trace4(\"timer::tr_advance\", delay, period, _sim->_time0, d->_time[1]);\n";
    o______ "assert(_sim->_phase != p_RESTORE);\n";
    o______ "assert(is_q(_sim->_time0));\n";
    o______ "assert(is_q(_req_evt));\n";

    o______ "if (_req_evt == NEVER) {\n";
    o______ "}else if (_sim->_time0 < _req_evt) {\n";
    o________ "trace2(\"timer::tr_advance not yet\", _sim->_time0, _req_evt);\n";
    o______ "}else if (_sim->_time0 <= _req_evt + " << accept_tol() << ") {\n";
    o________ "trace3(\"timer::tr_advance hit\", delay, period, _sim->_time0);\n";
    o________ "return true;\n";
    o______ "}else{\n";
    o______ "}\n;";

    o______ "return false;\n";
    o____ "} // tr_advance\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_regress" << args() << "{\n";
    o______ "(void)delay;\n";
    o______ "(void)period;\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n";
   // o______ "return tr_advance(d, delay, period, tol, en);\n";
    o______ "assert(_sim->_phase != p_RESTORE);\n";

    o______ "bool ret;\n";
    o______ "trace4(\"timer::tr_regress\", _previous_evt, _req_evt, _sim->_time0, _sim->_dtmin);\n";
    o______ "if (d->_time[1] == 0. && _sim->_time0 < _previous_evt + _sim->_dtmin) { untested();\n";
    o________ "trace3(\"timer::tr_regress2\", _previous_evt, _req_evt, _sim->_time0);\n";
    o______ "incomplete();\n"; // doesnt work
    o________ "_req_evt = NEVER; // _sim->_time0;\n";
   // o________ "_previous_evt = _sim->_time0;\n"; not needed. only updating _prev in accept.
    o________ "ret = true;\n";
    o______ "}else if (_sim->_time0 < _req_evt) {\n";
    o________ "trace3(\"timer::tr_regress3\", _previous_evt, _req_evt, _sim->_time0);\n";
    o________ "ret = false;\n";
    o______ "}else if (_sim->_time0 <= _req_evt + " << accept_tol() << ") { untested();\n";
    o________ "trace3(\"timer::tr_regress5\", _previous_evt, _req_evt, _sim->_time0);\n";
    o________ "ret = true;\n";
    o______ "}else{\n";
    o________ "ret = false;\n";
    o______ "}\n";

    o______ "trace5(\"timer::tr_regress done\", ret, _previous_evt, _req_evt, _sim->_time0, _sim->_time0 - _previous_evt);\n";
    o______ "assert(is_q(_sim->_time0));\n";
    o______ "assert(is_q(_req_evt));\n";
    o______ "return ret;\n";
    o____ "} // tr_regress\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_review" << args() << " {\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n";
    o______ "assert(_sim->_phase != p_RESTORE);\n";

    o______ "double next = next_event(delay, period, tol);\n";
    o______ "trace4(\"timer::tr_review\", _sim->_time0, delay, period, next);\n";
    o______ "trace3(\"timer::tr_review\", _req_evt, _sim->_time0, next);\n";
    o______ "trace1(\"timer::tr_review\", _req_evt - _sim->_time0);\n";
    o______ "assert(  _req_evt == _sim->_time0 || fabs( _req_evt - _sim->_time0) > 1e-18); // both quantised\n";
    o______ "assert(is_q(_sim->_time0));\n";
    o______ "assert(is_q(_req_evt) || _sim->_time0 == 0);\n";

    o______ "if (_sim->_time0 < _req_evt && _req_evt != NEVER) {\n";
    o________ "trace3(\"timer::tr_review early\", _req_evt, _sim->_time0, next);\n";
    o______ "}else if (_sim->_time0 <= _req_evt + " << accept_tol() << " && _req_evt != NEVER) {\n";
    o________ "trace3(\"timer::tr_review hit\", _req_evt, _sim->_time0, next);\n";
    o________ "d->q_accept();\n";
    o______ "}else if(next < _previous_evt + " << accept_tol() << "){\n";
    o________ "trace4(\"timer::tr_review norecover\", _previous_evt, _req_evt, _sim->_time0, next);\n";
    o______ "}else if(_sim->_time0 < next - " << accept_tol() << "){\n";
    o______ "}else if(_previous_evt == -NEVER && next < " << accept_tol() << ") { untested();\n";
    o________ "trace4(\"timer::tr_review min_event0\", _previous_evt, _req_evt, _sim->_time0, next);\n";
    o________ "d->_time_by.min_event(_sim->_dtmin);\n";
    o________ "return false;\n";
    o______ "}else{\n";
    o________ "trace4(\"timer::tr_review min_event2\", _previous_evt, _req_evt, _sim->_time0, next);\n";
    o________ "d->_time_by.min_event(_previous_evt + _sim->_dtmin);\n";
    o________ "return false;\n";
    o______ "}\n";

    o______ "if (next == NEVER) {\n";
    o______ "}else if ( _sim->_time0 < _req_evt && next == _req_evt) {\n"; // needed?
    o________ "trace3(\"timer::tr_review nop0\", _req_evt, _sim->_time0, _sim->_dtmin);\n";
    o______ "}else if (_sim->_time0 < next && _req_evt == NEVER) {\n";
    o________ "trace3(\"timer::tr_review qa\", _req_evt, _sim->_time0, _sim->_dtmin);\n";
    o________ "d->q_accept();\n"; // stray event. mg_evt.3{h,i}.gc
    o______ "}else if (next > _sim->_time0 && _req_evt > _sim->_time0) {\n";
    o________ "trace3(\"timer::tr_review nop1\", _req_evt, _sim->_time0, _sim->_dtmin);\n";
    o______ "}else if (next > _req_evt && next > _previous_evt) {\n";
    o________ "d->q_accept();\n"; // mg_evt.analysis.0c.gc
    o______ "}else if (next != _req_evt) {\n";
    o______ "}else{ untested();\n";
    o________ "incomplete();\n";
    o______ "}\n";

    o______ "if (_sim->_time0 < _req_evt) {\n";
    o______ "}else if (_sim->_time0 <= _req_evt + " << accept_tol() << ") {\n";
    o________ "trace2(\"timer::tr_review qa2\", _req_evt, _sim->_time0);\n";
    o______ "}else if(d->_time[1] == 0. && _req_evt == 0. && period && !delay) {\n";
    o________ "double back_to = period;\n";
    o________ "trace3(\"timer::tr_review2\", _req_evt, _sim->_time0, back_to);\n";
    o________ "if (period < _sim->_time0) {\n";
    o__________ "trace3(\"timer::tr_review min_event4\", _req_evt, _sim->_time0, back_to);\n";
    o__________ "d->_time_by.min_event(back_to);\n";
    o__________ "incomplete();\n";
    o__________ "_previous_evt = back_to;\n"; //????
    o________ "}else{\n";
    o________ "}\n";
    o______ "}else{\n";
    o______ "}\n";
    o______ "assert(is_q(_req_evt) || _sim->_time0 == 0);\n";
    o______ "return true;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    // compute time of next event from timer args and previous
    o____ "double next_event(double td, double period, double tol)const {\n";
    o______ "double ret;\n";
    o______ "// assert(_sim->_time0 <= _req_evt + " << accept_tol() << ");\n";
    o______ "if(_sim->analysis_is_static()) {\n";
    o________ "if(td) {\n";
    o__________ "return td;\n";
    o________ "}else if(period) {\n";
    o__________ "return period;\n";
    o________ "}else{\n";
    o__________ "return NEVER;\n";
    o________ "}\n";
    o______ "}else if(_sim->_time0 <= _previous_evt + 1.01*_sim->_dtmin + " << accept_tol() << ") {\n";
    o________ "if(period) {\n";
    o__________ "double raw_time = _sim->_time0 + 1.01*_sim->_dtmin + " << accept_tol() << ";\n";
    o__________ "int tick = int(( raw_time - td) / period);\n"; // +dtmin?!
    o__________ "return td + (tick+1)*period;\n";
    o________ "}else if(_sim->_time0 < td - " << accept_tol() << ") {\n";
    o__________ "return td;\n";
    o________ "}else{\n";
    o__________ "return NEVER;\n";
    o________ "}\n";
    o______ "}else{ // past previous\n";
    o________ "assert(_sim->_time0 > _previous_evt);\n";
    o________ "if(_sim->_time0 < td) {\n";
    o__________ "ret = td;\n";
    o________ "}else if(period) {\n"; // same as above?
    o__________ "double raw_time = _sim->_time0 + " << accept_tol() << ";\n";
    o__________ "int tick = int(( raw_time - td) / period);\n";
    o__________ "ret = td + (tick+1)*period;\n";
    o__________ "trace7(\"timer::next_event " << label() << " p2\", tol, _sim->_time0, raw_time, td, period, _previous_evt, ret);\n";
    o________ "}else if(td < _sim->_time0 + " << accept_tol() << ") {\n";
    o__________ "if(_previous_evt>0.) {\n";
    o__________ "  ret = td;\n";
    o__________ "}else{\n";
    o__________ "  ret = NEVER;\n";
    o__________ "}\n";
    o________ "}else{ untested();\n";
    o__________ "trace4(\"timer::next_event td2\", _sim->_time0, td, period, _previous_evt);\n";
    o__________ "return NEVER;\n";
    o________ "}\n";
    o______ "}\n";
    o______ "trace5(\"timer::next_event\", _sim->_time0, td, period, _previous_evt, ret);\n";
    o______ "// assert(ret >= td);\n"; // no. < means need to go back.
    o______ "trace6(\"timer::next_event done " << label() << "\", "
                  "_sim->_time0, _req_evt, _previous_evt, ret, td, period);\n";
    o______ "return ret;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_accept_static" << args() << " {\n";
    o______ "(void)tol;\n";
    o______ "if(delay){\n";
    o________ "_previous_evt = -NEVER;\n";
    o________ "_req_evt = delay;\n";
    o______ "}else if(period) {\n";
    o________ "_previous_evt = 0.;\n";
    o________ "_req_evt = period;\n";
    o______ "}else{\n";
    o________ "_previous_evt = -NEVER;\n";
    o________ "_req_evt = NEVER;\n";
    o______ "}\n";
    o______ "if(_req_evt == NEVER) {\n";
    o______ "}else if(_req_evt > _sim->_time0) {\n";
    o________ "set_event(d, _req_evt, " << tol() << ");\n";
    o______ "}\n";
    o______ "assert(is_q(_req_evt));\n";
    o______ "if(!delay) {\n";
    o________ "_previous_evt = 0.;\n";
    o______ "}else{\n";
    o______ "}\n";
    o______ "trace6(\"timer::tr_accept done s\", _sim->_time0, _req_evt, _sim->_dtmin, _previous_evt, delay, _sim->analysis_is_static());\n";
    o______ "assert(is_q(_sim->_time0));\n";
    o______ "assert(is_q(_req_evt));\n";
    o______ "return !delay && en;\n";
    o____ "}\n";
    /*----------------------------------------------------------------------*/
    o____ "bool tr_accept" << args() << " {\n";
    o______ "(void)tol;\n";
    o______ "(void)en;\n"; // incomplete.
    o______ "assert(_sim->_phase != p_RESTORE);\n";
    o______ "bool ret = false;\n";
    o______ "trace4(\"timer::tr_accept\", _sim->_time0, delay, period, _sim->analysis_is_static());\n";
    o______ "trace3(\"timer::tr_accept\", _sim->_time0, _previous_evt, _req_evt);\n";

    // check if this one was requested.
    o______ "if(_sim->analysis_is_static()) {\n";
    o________ "return tr_accept_static(d, delay, period);\n";
    o______ "}else if(_sim->_time0 < _req_evt){\n";
    o______ "}else if(_sim->_time0 <= _req_evt + " << accept_tol() << ") {\n";
    o________ "ret = true;\n";
    o______ "}else{\n";
    o______ "}\n";

    o______ "if (ret){\n;";
    o________ "// make the requested time permanent\n";
    o________ "_previous_evt = _req_evt;\n";
    o______ "}else{\n";
    o______ "}\n";

    o______ "double re = next_event(delay, period, tol);\n";
    o______ "assert(re >= delay);\n";

    o______ "if(re == NEVER) {\n";
    o________ "_req_evt = NEVER;\n";
    o______ "}else if(_sim->_time0 < re - " <<  accept_tol() << ") {\n";
    o________ "trace4(\"timer::tr_accept re nohit?\", _sim->_time0, _req_evt, _previous_evt,  re);\n";
    o________ "_req_evt = re;\n"; // new event, quantize below.
    o________ "if(ret){\n";
    o________ "}else{\n";
    o________ "}\n";
    o______ "}else{\n";
    o______ "}\n";

    o______ "if(_req_evt == NEVER) {\n";
    o________ "// assert(ret);\n";
    o______ "}else if(_sim->_time0 < _req_evt + " << accept_tol() << ") {\n";
    o________ "set_event(d, _req_evt, " << tol() << ");\n";
    o______ "}else{\n";
    o________ "trace6(\"timer::tr_accept bogus?\", _sim->_time0, _req_evt, _previous_evt, ret, re, _sim->analysis_is_static());\n";
    o________ "assert(  _req_evt == _sim->_time0 || fabs( _req_evt - _sim->_time0) > 1e-18); // both quantised\n";
    o________ "assert(is_q(_req_evt));\n";
    o______ "}\n";

    o______ "trace6(\"timer::tr_accept done " << label() << "\", "
                  "_sim->_time0, _req_evt, _previous_evt, ret, re, _sim->analysis_is_static());\n";
    o______ "assert(is_q(_sim->_time0));\n";
    o______ "assert(is_q(_req_evt));\n";

    o______ "return ret;\n";
    o____ "} // tr_accept\n";
    /*----------------------------------------------------------------------*/
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
  void make_cc_common(std::ostream&)const override {
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
