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
 * Verilog-AMS, ac_stim
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.h"
#include "mg_analog.h"
#include "m_tokens.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
static void make_cc_tmp(std::ostream& o, std::string state, Deps const& deps)
{

  {
    char sign = '+';
    indent a;
    o__ "d->" << state << "[0] = " << sign << " " << "t1.value();\n";
    size_t k = 2;

    for(auto v : deps) {
      // char sign = f.reversed()?'-':'+';
      o__ "// dep " << v->code_name() << "\n";
      // if(f->branch() == v->branch()){ untested(); }
      if(v->branch()->is_short()){ untested();
      }else{
	o__ "assert(" << "t1[d" << v->code_name() << "] == t1[d" << v->code_name() << "]" << ");\n";
	o__ "// assert(!d->" << state << "[" << k << "]);\n";
	o__ "d->" << state << "[" //  << k << "]"
	  << "MOD::" << state << "_::dep" << v->code_name() << "] "
	  " = " << sign << " " << "t1[d" << v->code_name() << "]; // (4)\n";
	++k;
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
static int n_filters;
/*--------------------------------------------------------------------------*/
class Token_ACSTIM : public Token_CALL {
public:
  explicit Token_ACSTIM(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_ACSTIM(const Token_ACSTIM& P, Base const* data, Expression_ const* e = NULL)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override {untested(); return new Token_ACSTIM(*this);}

  void stack_op(Expression* e)const override;
  Branch* branch() const;
  Expression_ const* args() const{
    if(auto a=prechecked_cast<Expression_ const*>(Token_CALL::args())){
      return a;
    }else{
      assert(!Token_CALL::args());
      return NULL;
    }
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class ACSTIM : public MGVAMS_FILTER {
  Module* _m{NULL};
  Token_CALL const* _token{NULL};
  Probe const* _prb{NULL};
  std::string _code_name;
public: // HACK
  Branch* _br{NULL};
  mutable Branch const* _output{NULL};
  Node_Ref _p;
  Node_Ref _n;
  explicit ACSTIM() : MGVAMS_FILTER() {
    set_label("acs");
  }
  explicit ACSTIM(ACSTIM const& p) : MGVAMS_FILTER(p) {
  }
  ~ACSTIM(){
//    delete _prb;
  }
  bool static_code()const override {return false;}
protected:
  ACSTIM* clone()const {
    return new ACSTIM(*this);
  }
  void make_assign(std::ostream&)const {
  }
  void set_code_name(std::string x){
    _code_name = x;
  }
  std::string code_name()const override{
    return "d->" + _code_name;
  }
public:
  Token* new_token(Module& m, size_t na)const override {
    assert(na != size_t(-1));

    std::string filter_code_name = label() + "_" + std::to_string(n_filters++);

    ACSTIM* cl = clone();
    {
      cl->set_label(filter_code_name); // label()); // "_b_" + filter_code_name);
      cl->set_code_name("_b_" + filter_code_name);
      assert(na<3);
      cl->set_num_args(na);
      cl->_m = &m;
      m.push_back(cl);
    }

    Node* np = m.new_node(filter_code_name + "_p");
    cl->_p = np;
    Node* nn = m.new_node(filter_code_name + "_n"); // &mg_ground_node
   // np->set_to(nn, "_short_"+code_name()+"()");
    nn->set_to(np, "_short_b_"+filter_code_name+"()");
    cl->_n = nn;
    {
      // Branch* br = m.new_branch(np, &Node_Map::mg_ground_node);
      Branch* br = m.new_branch(&Node_Map::mg_ground_node, nn);
//      br->set_source();
      assert(br);
      assert(const_cast<Branch const*>(br)->owner());
      Branch_Ref prb(br);
      cl->_br = br;

      cl->_prb = m.new_probe("potential", prb);
      br->set_filter(cl);
      m.new_filter();
    }

    return new Token_ACSTIM(label(), cl);
  }
  void make_cc_precalc(std::ostream& o)const override{
    o__ "ddouble " << _code_name << "(std::string";
      assert(num_args() < 3);
      for(size_t n=1; n<num_args(); ++n){
	o << ", ddouble";
      }
    o << "){\n";
    o__ "ddouble ret = 0.;\n";
    std::string cn = _br->code_name();
    o__ "ret[d_potential" << cn << "] = -1.;\n";
    o__ "return ret;\n";
    o << "}\n";
  }
  void make_cc_dev(std::ostream& o)const override{
    o__ "ddouble " << _code_name << "(";
      o << "std::string what";
      assert(num_args() < 3);
      for(size_t n=1; n<num_args(); ++n){
	o << ", ddouble t" << n;
      }
    o << ");\n";
    o__ "bool _short"+_code_name+"()const {return " << bool(_output) << ";}\n";
  }
  void make_cc_impl_comm(std::ostream&)const{ untested();
    unreachable();
  }
  void make_cc_impl(std::ostream&o)const override {
//    make_cc_impl_comm(o);
    std::string cn = _br->code_name();
    std::string id = _m->identifier().to_string();
    o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << _code_name << "(";
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
  }
  std::string eval(CS&, const CARD_LIST*)const override{
    unreachable();
    return "ac_stim";
  }
  Probe const* prb()const {return _prb;}
  void set_n_to_gnd()const {
    assert(_m);
    _m->set_to_ground(_br->p());
  }
private:
  Branch const* output()const override;

  // really?
  Node_Ref p()const override;
  Node_Ref n()const override;
} ac_stim;
/*--------------------------------------------------------------------------*/
DISPATCHER<FUNCTION>::INSTALL d_ddt(&function_dispatcher, "ac_stim", &ac_stim);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch* Token_ACSTIM::branch() const
{
  auto func = prechecked_cast<ACSTIM const*>(f());
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
void Token_ACSTIM::stack_op(Expression* e)const
{
  assert(e);
  Token_CALL::stack_op(e);
  assert(!e->is_empty());
  auto cc = prechecked_cast<Token_CALL const*>(e->back());
  assert(cc);
  e->pop_back();
  // assert(!e->is_empty());

  auto func = prechecked_cast<ACSTIM const*>(f());
  assert(func);

  if(auto dd = prechecked_cast<Deps const*>(cc->data())) {
    assert(dd);
    for(auto i : *dd){
      trace1("acstim arg deps", i->code_name());
    }

    branch()->deps().clear();
    branch()->deps() = *dd; // HACK
    if(1){
      func->set_n_to_gnd();
    }else if(0 /*sth linear*/){
      // somehow set loss=0 and output ports to target.
    }else{
    }

    auto d = new Deps;
    trace1("acstim output dep", func->prb()->code_name());
    d->insert(Dep(func->prb(), Dep::_LINEAR)); // BUG?
    auto N = new Token_ACSTIM(*this, d, clone_args(cc->args()));
    assert(N->data());
    assert(dynamic_cast<Deps const*>(N->data()));
    e->push_back(N);
    assert(f()==N->f());
    delete(cc);
  }else if(!e->size()) { untested();
    unreachable();
  }else if ( dynamic_cast<Token_PARLIST_ const*>(e->back())) { untested();
    auto d = new Deps;
    d->insert(Dep(func->prb())); // BUG?
    auto N = new Token_ACSTIM(*this, d);
    assert(N->data());
    assert(dynamic_cast<Deps const*>(N->data()));
    e->push_back(N);
  }else{ untested();
    unreachable();
  }

  trace0("acstim use?");
  int c_cnt = 0;
  bool assigned = false;
  bool always = false;
  Contribution const* cont=NULL;
  for(auto b : branch()->used_in()) {
    if(auto c = dynamic_cast<Contribution const*>(b)){
      if(c->is_flow_contrib()) {
	trace1("acstim used_in", c->name());
	++c_cnt;
	cont = c;
      }else{
	// incomplete();
      }
      if(c->is_always()){
	always = true;
      }else{
      }
    }else if(dynamic_cast<Assignment const*>(b)){
      assigned = true;
    }else{untested();
    }
  }
  func->_output = NULL;
  if(c_cnt!=1){
  }else if(assigned){
  }else if(cont->has_sensitivities()) { itested();
  }else if(always){
    for(auto d : cont->deps()){
      if(d->branch() != branch()) {
      }else if(d.is_linear()){
	// incomplete();
	func->_output = cont->branch(); // polarity?
      }
      if(cont->reversed()){
      }else{
      }
    }
  }else{
  }
}
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
  return _p;
}
/*--------------------------------------------------------------------------*/
Node_Ref ACSTIM::n() const
{
  return _n;
}
#endif
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
