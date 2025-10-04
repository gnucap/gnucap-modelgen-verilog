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
 * Verilog-AMS, *_noise
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.cc"
#include "mg_in.cc"
#include "mg_analog.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
Node_Ref MGVAMS_FILTER::p() const {unreachable(); return nullptr;}
Node_Ref MGVAMS_FILTER::n() const {unreachable(); return nullptr;}
Node_Ref Branch::p() const {return _p;}
Node_Ref Branch::n() const {return _n;}
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class NOISE : public MGVAMS_FILTER {
  int _na{0}; // derived class?
public: // HACK
  explicit NOISE(std::string variant, int na=2) : MGVAMS_FILTER(), _na(na) {
    set_label(variant + "_noise");
  }
  explicit NOISE(NOISE const& p) : MGVAMS_FILTER(p), _na(p._na) {
  }
  ~NOISE(){
//    delete _prb;
  }
  bool static_code()const override {return false;}
  bool is_common()const override {return false;}
  bool has_state()const override { untested();return true;}
  bool is_analog_filter()const override {return true;}
  bool port_hack()const override {return false;}
  int max_args()const override {return _na;}
  Data_Type const* return_type()const override {
    static Data_Type_Real r;
    return &r;
  }
protected:
  NOISE* clone()const override {
    return new NOISE(*this);
  }
  void make_assign(std::ostream&)const { untested();
  }
  std::string code_name()const override{
    return "_b_" + short_label();
  }
public:
  void stack_op(Expression* e)const override;

  Token* new_token(Module&, size_t)const override { return nullptr; }
  void make_cc_dev(std::ostream& o)const override{
    int na = int(num_args());
    o__ "ddouble " << code_name() << "(";
      assert(na  <= _na);
      o << "ddouble t0";
      for(int n=2; n<_na; ++n){
	o << ", ddouble t" << n-1;
      }
      if(na ==_na){
      o << ", std::string what";
      }else{
      }
    o << ");\n";
    o__ "bool _short"+code_name()+"()const {return " << bool(_output) << ";}\n";
    o__ "ddouble " << code_name() << "__precalc(";
    assert(na  <= _na);
    if(na >0){
    o << "ddouble";
    }else{ untested();
    }
    for(int n=2; n<_na; ++n){
      o << ", ddouble";
    }
    if(na == _na) {
      o << ", std::string what=\"\"";
    }else{
    }
    o << ");\n";
  }
  void make_cc_impl(std::ostream&o)const override {
    indent x;
    int na = int(num_args());
    std::string cn = _br->code_name();
    std::string id = _m->identifier().to_string();
    // o << " // " << __FILE__ << ":" << __LINE__ << "\n"; // macro?
    o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << code_name() << "(";
    o << "ddouble t0";
    for(int n=2; n<_na; ++n){
      o << ", ddouble t" << n-1;
    }
    if(na ==_na){
      o << ", std::string what";
    }else{
    }
    o << ")\n{\n";
    for(int n=2; n<_na; ++n){
      o << "(void) t" << n-1 << ";\n";
    }
    if(na ==_na){
      o__ "(void)what;\n";
    }else{
    }
    o__ "MOD_" << id << "* d = this;\n";
    o__ "typedef MOD_" << id << " MOD;\n";
    std::string state = "_st" + cn;
    { // make_cc_tmp(o, state, _br->deps());
      TData const& deps = _br->deps();
      char sign = '+';
      indent a;
      o__ "d->" << state << "[0] = " << sign << " " << "t0.value();\n";
      size_t k = 2;

      // possibly unneeded. cf. ac_stim
      for(auto v : deps.ddeps()) {
	// char sign = f.reversed()?'-':'+';
	o__ "// dep " << ::code_name(v) << "\n";
	// if(f->branch() == v->branch()){ untested(); }
//	if(v->branch()->is_short()){ untested();
//	} else
	{
	  o__ "assert(" << "t0[d" << ::code_name(v) << "] == t0[d" << ::code_name(v) << "]" << ");\n";
	  o__ "// assert(!d->" << state << "[" << k << "]);\n";
	  o__ "d->" << state << "[" //  << k << "]"
	    << "MOD::" << state << "_::dep" << ::code_name(v) << "] "
	    " = " << sign << " " << "t0[d" << ::code_name(v) << "]; // (4)\n";
	  ++k;
	}
      }
    }
    trace2("noise use", _br->code_name(), _br->is_used());

    if(_output){
      o__ "// subdevice\n";
      o__ "t0 = 0.;\n";
    }else{
      o__ "d->" << cn << "->do_tr();\n";
      o__ "t0 = d->" << cn << "->tr_amps();\n";
      o__ "d->_potential" << cn << " = - t0;\n";
    }
    o__ "trace2(\"filt\", t0, d->"<< cn <<"->tr_outvolts());\n";

    // std::string cn = _br->code_name();
    o__ "t0[d_potential" << cn << "] = -1.;\n";
    o__ "assert(t0 == t0);\n";

    if(_output){
      o__ "return t0; // (output)\n";
    }else{
      o__ "return t0; // (node)\n";
    }

    o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "ddouble MOD_" << id << "::" << code_name() << "__precalc(";
    if(na >0){
      o << "ddouble";
    }else{ untested();
    }
    for(int n=2; n<_na; ++n){
      o << ", ddouble t" << n-1;
    }
    if(na ==_na){
      o << ", std::string what";
    }else{
    }
    o << ") {\n";
    if(na ==_na){
      o__ "(void)what;\n";
    }else{
    }
    o__ "ddouble ret = 0.;\n";
    if(na ==_na){
      o__ "COMPONENT* l = " << cn << ";\n";
      o__ "assert(l);\n";
      o__ "l->set_param_by_name(\"name\", \"\");\n";
      o__ "l->set_param_by_name(\"name\", what);\n";
    }else{
    }
    if(na >1 && _na==3){
      o__ "{\n";
      o____ "COMPONENT* l = " << cn << ";\n";
      o____ "assert(l);\n";
      o____ "l->set_param_by_name(\"e\", \"\");\n";
      o____ "l->set_param_by_name(\"e\", to_string(t1));\n";
      o__ "}\n";
    }else{
    }
    o__ "ret[d_potential" << cn << "] = -1.;\n";
    o__ "return ret;\n";
    o << "}\n";
  }
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable();
    return "ac_stim";
  }
  Probe const* prb()const {return _prb;}
  void set_n_to_gnd()const {
    assert(_m);
    return MGVAMS_FILTER::set_n_to_gnd(_m);
  }
  void set_p_to_gnd()const { untested();
    assert(_m);
    return MGVAMS_FILTER::set_p_to_gnd(_m);
  }
private:
  Branch const* output()const override;

  // really?
  Node_Ref p()const override;
  Node_Ref n()const override;
}; // NOISE
/*--------------------------------------------------------------------------*/
// dup in acstim
static bool is_zero(Token const* t)
{
  assert(t);
  if(Float const* f = dynamic_cast<const Float*>(t->data())){
    return f->value() == 0.;
  }else if(Integer const* ii = dynamic_cast<const Integer*>(t->data())){
    return ii->value() == 0;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
void NOISE::stack_op(Expression* e) const
{
  assert(e);
  if(!e->size()) { untested();
    throw Exception("invalid");
 // }else if(auto pp = dynamic_cast<Token_PARLIST_ const*>(e->back())){ untested();
 //   unreachable();
 //   throw Exception("invalid");
  }else if(dynamic_cast<Token_PARLIST const*>(e->back())){
    Token* pl = e->back();
    e->pop_back();
    if(is_zero(e->back())){
      while(!dynamic_cast<Token_STOP const*>(e->back())) {
	delete e->back();
	e->pop_back();
      }
      delete e->back();
      e->pop_back();
      Float* f = new Float(0.);
      e->push_back(new Token_CONSTANT(f, ""));
      delete(pl);
    }else{
      e->push_back(pl);
      throw Exception("invalid");
    }
  // func->set_p_to_gnd(); ??
 // }else if(auto cc = prechecked_cast<Token_CALL const*>(e->back())){
 //   incomplete();
 //   throw Exception("invalid");
  }else{ untested();
    throw Exception("invalid");
  }
}
/*--------------------------------------------------------------------------*/
NOISE wn("white", 2);
DISPATCHER<FUNCTION>::INSTALL dwn(&function_dispatcher, "white_noise", &wn);
/*--------------------------------------------------------------------------*/
NOISE fn("flicker", 3);
DISPATCHER<FUNCTION>::INSTALL dfn(&function_dispatcher, "flicker_noise", &fn);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch const* NOISE::output() const
{
  if(_output){
    return _output;
  }else{
    return _br;
  }
}
/*--------------------------------------------------------------------------*/
Node_Ref NOISE::p() const
{
  assert(_br);
  return _br->p();
}
/*--------------------------------------------------------------------------*/
Node_Ref NOISE::n() const
{
  assert(_br);
  return _br->n();
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
