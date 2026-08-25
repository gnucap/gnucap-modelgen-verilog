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
 * transition function/filter
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include "mg_analog.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
#include "f__.cc"
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static void make_cc_tmp(std::ostream& o, std::string state, TData const& deps)
{

  {
    char sign = '+';
    indent a;
    o__ "d->" << state << "[0] = " << sign << " " << "t0.value();\n";
    size_t k = 2;

    for(auto const& v : deps.ddeps()) {
      // char sign = f.reversed()?'-':'+';
      o__ "// dep " << code_name(v) << "\n";
      // if(f->branch() == v->branch()){ untested(); }
      if(branch(v)->is_short()){ untested();
      }else{
	o__ "assert(" << "t0[d" << code_name(v) << "] == t0[d" << code_name(v) << "]" << ");\n";
	o__ "// assert(!d->" << state << "[" << k << "]);\n";
	o__ "d->" << state << "[" //  << k << "]"
	  << "MOD::" << state << "_::dep" << code_name(v) << "] "
	  " = " << sign << " " << "t0[d" << code_name(v) << "]; // (4)\n";
	++k;
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
#if 0
class Token_TRANSITION : public Token_CALL {
public:
  explicit Token_TRANSITION(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_TRANSITION(const Token_TRANSITION& P, Base const* data, Expression_ const* e = nullptr)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override {untested(); return new Token_TRANSITION(*this);}

  void stack_op(Expression* e)const override;
  Branch* branch() const{unreachable(); return nullptr;}
};
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
const int NUM_ARGS = 5;
/*--------------------------------------------------------------------------*/
class TRANSITION : public MGVAMS_FILTER /* FUNCTION_ */ {
  Probe const* _prb{nullptr};
public: // HACK
  mutable Branch const* _output{nullptr};

  explicit TRANSITION() : MGVAMS_FILTER() {
    set_label("transition");
  }
  explicit TRANSITION(TRANSITION const& p) : MGVAMS_FILTER(p) {
  }
  ~TRANSITION(){
//    delete _prb; belongs to _m
  }
  virtual TRANSITION* clone()const override {
    return new TRANSITION(*this);
  }
protected:
  int max_args()const override {untested(); return 4;}
  bool port_hack()const override {untested(); return false;}
  bool is_analog_filter()const override {untested(); return true;}
  std::string eval_name()const override {
    if(_m){
      std::string id = _m->identifier().to_string();
      return "COMMON_" + id + "::_common" + code_name();
    }else{
      // unreachable();
      return "missing_m??";
    }
  }
//  void set_code_name(std::string x){
//    _code_name = x;
//  }
protected:
  std::string code_name()const override {
    return "_b_" + short_label();
  }
public:
  Token* new_token(Module&, size_t)const override { untested();
    return nullptr;
  }

  void make_cc_common(std::ostream& o)const override{
    o << "public:\n";
    o__ "class common" << code_name() <<": public COMMON_FILT {\n";
    o____ "COMMON_COMPONENT* clone()const override{unreachable(); return nullptr;}\n";
    o__ "public:\n";
    o____ "common" << code_name() <<"(int i=CC_STATIC) : COMMON_FILT(i) {}\n";
    o__ "public:\n";
    o__ "private:\n";
    o____ "std::string name()const override {return \"transition_name\";}\n";
    o__ "};\n";
    o__ "static common" << code_name()
      << " _common" << code_name() << ";\n";
  }

  void make_cc_dev(std::ostream& o)const override{
    indent x;
    o__ "ddouble " << code_name() << "(ddouble t0";
      assert(num_args());
      assert(num_args() <= NUM_ARGS);
      for(int n=1; n < int(num_args()); ++n){
	o << ", double t" << n;
      }
    o << "); // (d)\n";
    o__ "bool _short"+code_name()+"()const {return " << bool(_output) << ";}\n";
    o__ "ddouble " << code_name() << "__precalc(ddouble const&";
      assert(num_args());
      assert(num_args() <= NUM_ARGS);
      for(int n=1; n < int(num_args()); ++n){
	o << ", double t" << n;
      }
    o << ");\n";
  }

  void make_cc_impl(std::ostream&o)const override {
    std::string cn = _br->code_name();
    std::string id = _m->identifier().to_string();
    o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << code_name() << "(ddouble t0";
    for(int n=1; n < int(num_args()); ++n){
      o << ", double t" << n;
    }
    o << ")\n{\n";
    indent x;
    for(int n=1; n < int(num_args()); ++n){
      o__ "(void)t" << n << ";\n";
    }
    o__ "MOD_" << id << "* d = this;\n";
    o__ "typedef MOD_" << id << " MOD;\n";
    std::string state = "_st" + cn;
    make_cc_tmp(o, state, _br->deps());

    if(_output){ untested();
      o__ "// subdevice\n";
      o__ "t0 = 0.;\n";
    }else{
      o__ "t0 = d->" << cn << "->tr_amps();\n";
 //     o__ "d->_potential" << cn << " = t0;\n";
    }

    o__ "t0[d_potential" << cn << "] = -1.;\n";
    o__ "assert(t0 == t0);\n";

    if(_output){ untested();
      o__ "return t0; // (output)\n";
    }else{
      o__ "return t0; // (node)\n";
    }

    o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "COMMON_" + id + "::common" << code_name()
      << " COMMON_" + id + "::_common" << code_name() << ";\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
//    std::string id = m.identifier().to_string();
    o << "ddouble MOD_" << id << "::" << code_name() << "__precalc(ddouble const&";
    assert(num_args());
    assert(num_args() <= NUM_ARGS);

    if(num_args()>1){
      o << ", double td=1";
    }else{ untested();
      unreachable();
    }
    if(num_args()>2){
      o << ", double rise_time=0.";
    }else{ untested();
    }
    if(num_args()>3){
      o << ", double fall_time=0.";
    }else{
    }
    if(num_args()>4){ untested();
      o << ", double time_tol=0";
    }else{
    }
    o << ")\n{\n";
    {
      o__ "ddouble ret(0.);\n";
      o__ "COMPONENT* l = " << cn << ";\n";
      o__ "assert(l);\n";
      o__ "if(rise_time < _sim->_dtmin) {\n";
      o____ "rise_time = _sim->_dtmin;\n";
      o__ "}else{\n";
      o__ "}\n";
      if(num_args()>3){
	o__ "if(fall_time < _sim->_dtmin) {\n";
	o____ "fall_time = _sim->_dtmin;\n";
	o__ "}else{\n";
	o__ "}\n";
      }else{
      }
      // todo: arguments are dynamic, move to accept (or so)
      if(num_args()>1){
	o__ "l->set_param_by_name(\"delay\", \"\");\n";
	o__ "l->set_param_by_name(\"delay\", to_string(td));\n";
      }else{ untested();
	unreachable();
      }
      if(num_args()>2){
	o__ "l->set_param_by_name(\"rise\", \"\");\n";
	o__ "l->set_param_by_name(\"rise\", to_string(rise_time));\n";
      }else{ untested();
      }
      if(num_args()>3){
	o__ "l->set_param_by_name(\"fall\", \"\");\n";
	o__ "l->set_param_by_name(\"fall\", to_string(fall_time));\n";
      }else{
      }
      if(num_args()>4){ untested();
	o__ "l->set_param_by_name(\"tol\", \"\");\n";
	o__ "l->set_param_by_name(\"tol\", to_string(time_tol));\n";
      }else{
      }
      o__ "l->precalc_last();\n";
      o__ "ret[d_potential" << cn << "] = -1.;\n";
      o__ "return ret;\n";
    }
    o__ "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  }
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable();
    return "transition";
  }
//  void stack_op(Expression*)const override {
//    throw Exception("invalid");
//  }
  Probe const* prb()const {return _prb;}
#if 0
  void set_n_to_gnd()const {
    assert(_m);
    _m->set_to_ground(_br->n());
  }
  void set_p_to_gnd()const { untested();
    assert(_m);
    _m->set_to_ground(_br->p());
  }
#endif
private:
  Branch const* output()const override;
  Node_Ref p()const override;
  Node_Ref n()const override;
private: // setup
  Branch* branch()const override { return _br; }
} trans;
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "transition", &trans);
/*--------------------------------------------------------------------------*/
Branch const* TRANSITION::output() const
{
  if(_output){ untested();
    return _output;
  }else{
    return _br;
  }
}
/*--------------------------------------------------------------------------*/
#if 1
Node_Ref TRANSITION::p() const
{
  return _br->p();
}
/*--------------------------------------------------------------------------*/
Node_Ref TRANSITION::n() const
{
  return _br->n();
}
#endif
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
