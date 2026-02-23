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
 * Verilog-AMS, ac_stim. no phase.
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include "mg_analog.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
static bool is_zero(Token const* t)
{
  assert(t);
  if(Float const* f = dynamic_cast<const Float*>(t->data())){
    return f->value() == 0.;
  }else if(Integer const* ii = dynamic_cast<const Integer*>(t->data())){ untested();
    return ii->value() == 0;
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static void make_cc_tmp(std::ostream& o, std::string state, TData const& deps)
{

  {
    char sign = '+';
    indent a;
    o__ "d->" << state << "[0] = " << sign << " " << "t1.value();\n";
    size_t k = 2;

  // possibly unneeded. cf. ac_stim
    for(auto const& v : deps.ddeps()) { untested();
      // char sign = f.reversed()?'-':'+';
      o__ "// dep " << code_name(v) << "\n";
      // if(f->branch() == v->branch()){ untested(); }
      if(branch(v)->is_short()){ untested();
      }else{ untested();
	o__ "assert(" << "t1[d" << probe(v)->code_name() << "] == t1[d" << probe(v)->code_name() << "]" << ");\n";
	o__ "// assert(!d->" << state << "[" << k << "]);\n";
	o__ "d->" << state << "[" //  << k << "]"
	  << "MOD::" << state << "_::dep" << probe(v)->code_name() << "] "
	  " = " << sign << " " << "t1[d" << probe(v)->code_name() << "]; // (4)\n";
	++k;
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class ACSTIM : public MGVAMS_FILTER {
 // Module* _m{nullptr};
  Probe const* _prb{nullptr};
 // std::string _code_name;
public: // HACK
  //Branch* _br{nullptr};
  //Node_Ref _p;
  //Node_Ref _n;
  explicit ACSTIM() : MGVAMS_FILTER() {
    set_label("acs");
  }
  explicit ACSTIM(ACSTIM const& p) : MGVAMS_FILTER(p) {
  }
  ~ACSTIM(){
//    delete _prb;
  }
  bool static_code()const override {return false;}
  bool is_common()const override {return false;}
  bool has_state()const override {return true;}
  int max_args()const override { return 2;}
  bool port_hack()const override {return false;}
  bool is_analog_filter()const override {return true;}
protected:
  ACSTIM* clone()const override {
    return new ACSTIM(*this);
  }
  void make_assign(std::ostream&)const { untested();
  }
  //void set_code_name(std::string x){ untested();
  //  _code_name = x;
  //}
  //std::string code_name()const override{ untested();
  //  return _code_name;
  //}
  std::string code_name()const override {
    return "_b_" + short_label();
  }
public:
  void stack_op(Expression* e)const override {
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
    // }else if(auto cc = prechecked_cast<Token_CALL const*>(e->back())){ untested();
    //   incomplete();
    //   throw Exception("invalid");
  }else{ untested();
    throw Exception("invalid");
  }
  }
/*--------------------------------------------------------------------------*/
  Token* new_token(Module& , size_t)const override {
    return nullptr;
  }
  void make_cc_dev(std::ostream& o)const override{
    o__ "ddouble " << code_name() << "(";
      o << "std::string what";
      assert(num_args() < 3);
      for(size_t n=1; n<num_args(); ++n){
	o << ", ddouble t" << n;
      }
    o << ");\n";
    o__ "bool _short"+code_name()+"()const {return " << bool(_output) << ";}\n";
    o__ "ddouble " << code_name() << "__precalc(std::string";
      assert(num_args() < 3);
      for(size_t n=1; n<num_args(); ++n){
	o << ", ddouble";
      }
    o << ");\n";
  }
  void make_cc_impl(std::ostream&o)const override {
    std::string cn = _br->code_name();
    std::string id = _m->identifier().to_string();
    o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << code_name() << "(";
    o << "std::string what";
    for(size_t n=1; n<num_args(); ++n){
      o << ", ddouble t" << n;
    }
    o << ")\n{\n";
    o__ "if(what != \"ac\"){incomplete();}\n";
    o__ "MOD_" << id << "* d = this;\n";
    o__ "typedef MOD_" << id << " MOD;\n";
    std::string state = "_st" + cn;
    make_cc_tmp(o, state, _br->deps());
    trace2("acstim use", _br->code_name(), _br->is_used());

    if(_output){
      o__ "// subdevice\n";
      o__ "t1 = 0.;\n";
    }else{
      o__ "d->" << cn << "->do_tr();\n";
      o__ "t1 = d->" << cn << "->tr_amps();\n";
      o__ "d->_potential" << cn << " = - t1;\n";
    }
    o__ "trace2(\"filt\", t1, d->"<< cn <<"->tr_outvolts());\n";

    // std::string cn = _br->code_name();
    o__ "t1[d_potential" << cn << "] = -1.;\n";
    o__ "assert(t1 == t1);\n";

    if(_output){
      o__ "return t1; // (output)\n";
    }else{
      o__ "return t1; // (node)\n";
    }

    o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "ddouble MOD_" << id << "::" << code_name() << "__precalc(";
    o << "std::string what";
    for(size_t n=1; n<num_args(); ++n){
      o << ", ddouble t" << n;
    }
    o << ")\n{\n";
    {
      indent x;
//      std::string cn = _br->code_name();
      o__ "(void)what;\n";

      o__ "COMPONENT* l = " << cn << ";\n";
      o__ "assert(l);\n";
      o__ "l->set_param_by_name(\"mag\", \"\");\n";
      o__ "l->set_param_by_name(\"mag\", to_string(t1));\n";
      //o__ "l->set_param_by_name(\"phase\", to_string(t1));\n";

      o__ "ddouble ret(0.);\n";
      o__ "ret[d_potential" << cn << "] = -1.;\n";
      o__ "return ret;\n";
    }
    o << "}\n";
  }
  Probe const* prb()const {return _prb;}
  void set_n_to_gnd()const { untested();
    assert(_m);
    _m->set_to_ground(_br->p());
  }
private:
  Branch const* output()const override;

  // really?
  Node_Ref p()const override;
  Node_Ref n()const override;
private: // setup
  Branch* branch() const override { return _br; }
} ac_stim;
/*--------------------------------------------------------------------------*/
DISPATCHER<FUNCTION>::INSTALL dacs(&function_dispatcher, "ac_stim", &ac_stim);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch const* ACSTIM::output() const
{
  if(_output){
    return _output;
  }else{
    return _br;
  }
}
/*--------------------------------------------------------------------------*/
#if 1
Node_Ref ACSTIM::p() const
{
  return _br->p();
}
/*--------------------------------------------------------------------------*/
Node_Ref ACSTIM::n() const
{ untested();
  return _br->n();
}
#endif
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
