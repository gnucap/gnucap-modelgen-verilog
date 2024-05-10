/*                        -*- C++ -*-
 * Copyright (C) 2023,2024 Felix Salfelder
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
 * laplace filters in modelgen-verilog
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
class Token_LAP : public Token_CALL {
//  LAP* _xdt{NULL};
public:
  explicit Token_LAP(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_LAP(const Token_LAP& P, Base const* data, Expression_ const* e = NULL)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override {untested(); return new Token_LAP(*this);}

  void stack_op(Expression* e)const override;
  Branch* branch() const;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class LAP : public MGVAMS_FILTER /* FUNCTION_ */ {
  Module* _m{NULL};
  Probe const* _prb{NULL};
  std::string _code_name;
public: // HACK
  Branch* _br{NULL};
  Node_Ref _p;
  Node_Ref _n;
  mutable int _nums{0};
  mutable int _dens{0};
protected:
  explicit LAP() : MGVAMS_FILTER() {
    set_label("LAP");
  }
  explicit LAP(LAP const& p) : MGVAMS_FILTER(p) {
  }
  ~LAP(){
//    delete _prb; belongs to _m
  }
  virtual LAP* clone()const = 0;
  void set_code_name(std::string x){
    _code_name = x;
  }
  std::string code_name()const override{
    return "/*LAP*/ d->" + _code_name;
  }
public:
  void set_n_to_gnd()const {
    assert(_m);
    return MGVAMS_FILTER::set_n_to_gnd(_m);
  }
  void set_p_to_gnd()const {
    assert(_m);
    return MGVAMS_FILTER::set_p_to_gnd(_m);
  }
  // Token* new_token(Expression_ const* e) ...
  // Module* m = e->owner()...
  Token* new_token(Module& m, size_t na)const override {
    assert(na != size_t(-1));
    if(na>3){ untested();
      throw Exception("syntax error, need 3 args");
    }else{
    }

    std::string filter_code_name = label() + "_" + std::to_string(n_filters++);

    LAP* cl = clone();
    {
      cl->set_label(filter_code_name); // label()); // "_b_" + filter_code_name);
      cl->set_code_name("_b_" + filter_code_name);
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

    return new Token_LAP(label(), cl);
  }

  void make_cc_common(std::ostream& o)const override{
    o << "public:\n";
    o__ "class common" << _code_name <<": public COMMON_FILT {\n";
    o____ "COMMON_COMPONENT* clone()const override{unreachable(); return NULL;}\n";
    o__ "public:\n";
    o____ "common" << _code_name <<"(int i=CC_STATIC) : COMMON_FILT(i) {}\n";
    o__ "public:\n";
    o____ "int args(int sel)const override{\n";
    o______ "switch(sel){\n";
    o________ "case 1: return " << numsize(_nums) << ";\n";
    o________ "case 2: return " << densize(_dens) << ";\n";
    o________ "default: return 0;\n";
    o______ "}\n";
    o____ "}\n";
    o__ "private:\n";
    o____ "std::string name()const override {return \"lap_name\";}\n";
    o__ "};\n";
    o__ "static common" << _code_name
      << " _common" << _code_name << ";\n";
  }

  void make_cc_precalc(std::ostream& o)const override{
    o__ "ddouble " << _code_name << "(ddouble const&";
      assert(num_args());
      assert(num_args() < 4);
      o << ", array_ const& num";
      o << ", array_ const& den";
    o << ");\n";
  }

  void make_cc_dev(std::ostream& o)const override{
    o__ "ddouble " << _code_name << "(ddouble t0";
      assert(num_args());
      assert(num_args() < 4);
      for(size_t n=1; n<num_args(); ++n){
	o << ", array_ const& t" << n;
      }
    o << ");\n";
    o__ "bool _short"+_code_name+"()const {return " << bool(_output) << ";}\n";
  }
  virtual std::string num_name_i()const = 0;
  virtual std::string den_name_i()const = 0;
  virtual int numsize(int x)const { return x; }
  virtual int densize(int x)const { return x; }

  void make_cc_impl(std::ostream&o)const override {
    std::string cn = _br->code_name();
    std::string id = _m->identifier().to_string();
    o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << _code_name << "(ddouble t0";
    std::string comma;
    for(size_t n=1; n<num_args(); ++n){
      // o << ", array_ const& t" << n;
      o << ", array_ const&";
    }
    o << ")\n{\n";
    o__ "MOD_" << id << "* d = this;\n";
    o__ "typedef MOD_" << id << " MOD;\n";
    std::string state = "_st" + cn;
    make_cc_tmp(o, state, _br->deps());
    trace2("xdt use", _br->code_name(), _br->is_used());

    if(_output){
      o__ "// subdevice\n";
      o__ "t0 = 0.;\n";
    }else{
      o__ "t0 = d->" << cn << "->tr_amps();\n";
      o__ "d->_potential" << cn << " = - t0;\n"; // -?
    }

    make_assign(o);

    if(_output){
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
    o << "ddouble PRECALC_" << id << "::" << _code_name << "(ddouble const&";
      assert(num_args());
      assert(num_args() < 4);
      o << ", array_ const& num";
      o << ", array_ const& den";
    o << ")\n{\n";
    {
      o__ "ddouble ret = 0.;\n";
//      std::string cn = _br->code_name();
      o__ "trace2(\"precalc" << cn << "\", num.size(), den.size());\n";
      o__ "COMPONENT* l = DEV(_d)->" << cn << ";\n";
      o__ "assert(l);\n";
      o__ "std::string reset;\n";
      o__ "l->set_param_by_index(-1,reset,0);\n";
      o__ "for(int i=0; i<int(num.size()); ++i){\n";
      o____ "trace2(\"precalc" << cn << "\", i, num[i]);\n";
      o____ "l->set_param_by_name(" + num_name_i() + ", to_string(num[i]));\n";
      o__ "}\n";
      o__ "for(int i=0; i<int(den.size()); ++i){\n";
      o____ "trace2(\"precalc" << cn << "\", i, den[i]);\n";
      o____ "l->set_param_by_name(" + den_name_i() + ", to_string(den[i]));\n";
      o__ "}\n";
      o__ "ret[d_potential" << cn << "] = -1.;\n";
      o__ "return ret;\n";
    }
    o__ "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  }
  std::string eval(CS&, const CARD_LIST*)const override{ untested();
    unreachable();
    return "laplace";
  }
  Probe const* prb()const {return _prb;}
private:
  Branch const* output()const override;
  Node_Ref p()const override;
  Node_Ref n()const override;
private:
  void make_assign(std::ostream& o)const {
    std::string cn = _br->code_name();
    o__ "t0[d_potential" << cn << "] = -1.;\n";
    o__ "assert(t0 == t0);\n";
  }
private: // setup
  Branch* branch() const override { return _br; }
}; // LAP
/*--------------------------------------------------------------------------*/
class LZP : public LAP{
public:
  explicit LZP() : LAP() {
    set_label("laplace_zp");
  }
  LZP* clone()const override{
    return new LZP(*this);
  }
  std::string num_name_i()const override {
    return "std::string(\"z\") + ((i%2)?'i':'r')  + to_string(i/2) /*A*/";
  }
  std::string den_name_i()const override {
    return "std::string(\"p\") + ((i%2)?'i':'r')  + to_string(i/2) /*B*/";
  }
  int numsize(int x)const override{ assert(!(x%2)); return x/2+1; }
  int densize(int x)const override{ assert(!(x%2)); return x/2+1; }
} lzp;
DISPATCHER<FUNCTION>::INSTALL d_zp(&function_dispatcher, "laplace_zp", &lzp);
/*--------------------------------------------------------------------------*/
class LND : public LAP{
public:
  explicit LND() : LAP() {
    set_label("laplace_nd");
  }
  LND* clone()const override{
    return new LND(*this);
  }
  std::string num_name_i()const override {
    return "\"n\"+to_string(i)";
  }
  std::string den_name_i()const override {
    return "\"d\"+to_string(i)";
  }
} lnd;
DISPATCHER<FUNCTION>::INSTALL d_nd(&function_dispatcher, "laplace_nd", &lnd);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch* Token_LAP::branch() const
{
  auto func = prechecked_cast<LAP const*>(f());
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
void Token_LAP::stack_op(Expression* e)const
{
  assert(e);
  Token_CALL::stack_op(e);
	
  assert(!e->is_empty());
  auto cc = prechecked_cast<Token_CALL const*>(e->back());
  assert(cc);
  assert(cc->args());
  trace1("Token_LAP::stack_op", cc->args()->size());
  e->pop_back();

  auto func = prechecked_cast<LAP const*>(f());
  assert(func);

  if(cc->args()->size() != 3){ untested();
    throw Exception("syntax error, need 3 args");
  }else{
  }
  auto argi = cc->args()->begin();
  ++argi;
  if(auto num = dynamic_cast<Token_ARRAY_ const*>(*argi)){
    trace1("array0", num->size());
    func->_nums = int(num->size());
  }else{ untested();
    throw Exception("syntax error, need array");
  }
  ++argi;
  if(auto den = dynamic_cast<Token_ARRAY_ const*>(*argi)){
    trace1("array1", den->size());
    func->_dens = int(den->size());
  }else{
    throw Exception("syntax error, need array");
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
    }else if(0 /*sth linear*/){
      // somehow set loss=0 and output ports to target.
    }else{ untested();
    }

    auto d = new TData;
    trace1("xdt output dep", func->prb()->code_name());
    d->insert(Dep(func->prb(), Dep::_LINEAR)); // BUG?
    auto N = new Token_LAP(*this, d, clone_args(cc->args()));
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
    assert(f()==N->f());
    delete(cc);
  }else if(!e->size()) { untested();
    unreachable();
  }else if ( dynamic_cast<Token_PARLIST_ const*>(e->back())) {
    auto d = new TData;
    d->insert(Dep(func->prb())); // BUG?
    auto N = new Token_LAP(*this, d);
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
Branch const* LAP::output() const
{ itested();
  if(_output){
    return _output;
  }else{
    return _br;
  }
}
/*--------------------------------------------------------------------------*/
#if 1
Node_Ref LAP::p() const
{
  return _p;
}
/*--------------------------------------------------------------------------*/
Node_Ref LAP::n() const
{
  return _n;
}
#endif
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
