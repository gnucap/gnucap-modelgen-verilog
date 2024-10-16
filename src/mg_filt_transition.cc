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

    for(auto v : deps.ddeps()) {
      // char sign = f.reversed()?'-':'+';
      o__ "// dep " << v->code_name() << "\n";
      // if(f->branch() == v->branch()){ untested(); }
      if(v->branch()->is_short()){ untested();
      }else{
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "// assert(!d->" << state << "[" << k << "]);\n";
	o__ "d->" << state << "[" //  << k << "]"
	  << "MOD::" << state << "_::dep" << v->code_name() << "] "
	  " = " << sign << " " << "t0[d" << v->code_name() << "]; // (4)\n";
	++k;
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
static int n_filters;
/*--------------------------------------------------------------------------*/
class Token_TRANSITION : public Token_CALL {
public:
  explicit Token_TRANSITION(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_TRANSITION(const Token_TRANSITION& P, Base const* data, Expression_ const* e = NULL)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override {untested(); return new Token_TRANSITION(*this);}

  void stack_op(Expression* e)const override;
  Branch* branch() const;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
const int NUM_ARGS = 5;
/*--------------------------------------------------------------------------*/
class TRANSITION : public MGVAMS_FILTER /* FUNCTION_ */ {
  Module* _m{NULL};
  Probe const* _prb{NULL};
  std::string _code_name;
public: // HACK
  Branch* _br{NULL};
  mutable Branch const* _output{NULL};
  Node_Ref _p;
  Node_Ref _n;

  explicit TRANSITION() : MGVAMS_FILTER() {
    set_label("transition");
  }
  explicit TRANSITION(TRANSITION const& p) : MGVAMS_FILTER(p) {
  }
  ~TRANSITION(){
//    delete _prb; belongs to _m
  }
  virtual TRANSITION* clone()const {
    return new TRANSITION(*this);
  }
protected:
  void set_code_name(std::string x){
    _code_name = x;
  }
  std::string code_name()const override{
    return "/*TRANSITION*/ d->" + _code_name;
  }
public:
  // Token* new_token(Expression_ const* e) ...
  // Module* m = e->owner()...
  Token* new_token(Module& m, size_t na)const override {
    assert(na != size_t(-1));

    std::string filter_code_name = label() + "_" + std::to_string(n_filters++);

    TRANSITION* cl = clone();
    {
      cl->set_label(filter_code_name); // label()); // "_b_" + filter_code_name);
      cl->set_code_name("_b_" + filter_code_name);
      assert(na<6);
      cl->set_num_args(na);
      cl->_m = &m;
      m.push_back(cl);
    }

    Node* np = m.new_node(filter_code_name + "_p");
    Node* nn = m.new_node(filter_code_name + "_n"); // &mg_ground_node
    np->set_to(&Node_Map::mg_ground_node, "_short_b_"+filter_code_name+"()");

    cl->_p = np;
    cl->_n = nn;
    {
      Branch* br = m.new_branch(np, &Node_Map::mg_ground_node);
//      br->set_source();
      assert(br);
      assert(const_cast<Branch const*>(br)->owner());
      Branch_Ref prb(br);
      cl->_br = br;

      cl->_prb = m.new_probe("potential", prb);
      br->set_filter(cl);
      std::string id = m.identifier().to_string();
      br->set_eval("COMMON_" + id + "::_common_b_" + filter_code_name);
      m.new_filter();
    }

    return new Token_TRANSITION(label(), cl);
  }

  void make_cc_common(std::ostream& o)const override{
    o << "public:\n";
    o__ "class common" << _code_name <<": public COMMON_FILT {\n";
    o____ "COMMON_COMPONENT* clone()const override{unreachable(); return NULL;}\n";
    o__ "public:\n";
    o____ "common" << _code_name <<"(int i=CC_STATIC) : COMMON_FILT(i) {}\n";
    o__ "public:\n";
    o__ "private:\n";
    o____ "std::string name()const override {return \"transition_name\";}\n";
    o__ "};\n";
    o__ "static common" << _code_name
      << " _common" << _code_name << ";\n";
  }

  void make_cc_dev(std::ostream& o)const override{
    o__ "ddouble " << _code_name << "(ddouble t0";
      assert(num_args());
      assert(num_args() <= NUM_ARGS);
      for(int n=1; n < int(num_args()); ++n){
	o << ", double t" << n;
      }
    o << "); // (d)\n";
    o__ "bool _short"+_code_name+"()const {return " << bool(_output) << ";}\n";
    o__ "ddouble " << _code_name << "__precalc(ddouble const&";
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
    o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << _code_name << "(ddouble t0";
    for(int n=1; n < int(num_args()); ++n){
      o << ", double t" << n;
    }
    o << ")\n{\n";
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
    o << "COMMON_" + id + "::common" << _code_name
      << " COMMON_" + id + "::_common" << _code_name << ";\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
//    std::string id = m.identifier().to_string();
    o << "ddouble MOD_" << id << "::" << _code_name << "__precalc(ddouble const&";
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
    }else{ untested();
    }
    if(num_args()>4){ untested();
      o << ", double time_tol=0";
    }else{
    }
    o << ")\n{\n";
    {
      o__ "ddouble ret = 0.;\n";
      o__ "COMPONENT* l = " << cn << ";\n";
      o__ "assert(l);\n";
      o__ "if(rise_time < _sim->_dtmin) {\n";
      o____ "rise_time = _sim->_dtmin;\n";
      o__ "}else{\n";
      o__ "}\n";
      o__ "if(fall_time < _sim->_dtmin) {\n";
      o____ "fall_time = _sim->_dtmin;\n";
      o__ "}else{\n";
      o__ "}\n";
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
      }else{ untested();
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
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable();
    return "transition";
  }
  Probe const* prb()const {return _prb;}
  void set_n_to_gnd()const {
    assert(_m);
    _m->set_to_ground(_br->n());
  }
  void set_p_to_gnd()const { untested();
    assert(_m);
    _m->set_to_ground(_br->p());
  }
private:
  Branch const* output()const override;
  Node_Ref p()const override;
  Node_Ref n()const override;
private: // setup
  Branch* branch()const override { return _br; }
} trans;
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "transition", &trans);
/*--------------------------------------------------------------------------*/
Branch* Token_TRANSITION::branch() const
{
  auto func = prechecked_cast<TRANSITION const*>(f());
  assert( func);
  assert( func->_br);
  return func->_br;
}
/*--------------------------------------------------------------------------*/
static Expression_* clone_args(Base const* e)
{
  if(auto e_ = dynamic_cast<Expression_ const*>(e)) {
    return e_->clone();
  }else{ untested();
    unreachable();
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
void Token_TRANSITION::stack_op(Expression* e)const
{
  assert(e);
  Token_CALL::stack_op(e);
	
  assert(!e->is_empty());
  auto cc = prechecked_cast<Token_CALL const*>(e->back());
  assert(cc);
  assert(cc->args());
  trace1("Token_TRANSITION::stack_op", cc->args()->size());
  e->pop_back();

  auto func = prechecked_cast<TRANSITION const*>(f());
  assert(func);

  if(cc->args()->size() < 1){ untested();
    throw Exception("syntax error, need delay");
  }else{
  }

  if(auto dd = prechecked_cast<TData const*>(cc->data())) {
    // cc->args()[0]->data?
    assert(dd);
    for(auto i : dd->ddeps()) {
      trace1("xdt arg deps", i->code_name());
    }

    branch()->deps().clear();
    branch()->deps() = *dd; // HACK
    if(1){
      func->set_n_to_gnd();
    }else if(0 /*sth linear*/){ untested();
      // somehow set loss=0 and output ports to target.
    }else{ untested();
    }

    auto d = new TData;
    trace1("xdt output dep", func->prb()->code_name());
    d->insert(Dep(func->prb(), Dep::_LINEAR)); // BUG?
    auto N = new Token_TRANSITION(*this, d, clone_args(cc->args()));
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
    assert(f()==N->f());
    delete(cc);
  }else if(!e->size()) { untested();
    unreachable();
  }else if ( dynamic_cast<Token_PARLIST_ const*>(e->back())) { untested();
    auto d = new TData;
    d->insert(Dep(func->prb())); // BUG?
    auto N = new Token_TRANSITION(*this, d);
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
  }else{ untested();
    unreachable();
  }
};
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
  return _p;
}
/*--------------------------------------------------------------------------*/
Node_Ref TRANSITION::n() const
{
  return _n;
}
#endif
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
