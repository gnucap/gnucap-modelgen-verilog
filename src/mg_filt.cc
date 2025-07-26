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
 * Verilog-AMS filters
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
static int n_filters;
/*--------------------------------------------------------------------------*/
class Token_XDT : public Token_CALL {
public:
  explicit Token_XDT(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_XDT(const Token_XDT& P, Base const* data, Expression_ const* e = nullptr)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override { untested(); return new Token_XDT(*this);}

private:
  void stack_op(Expression* e)const override;
  Branch* branch() const;
  Expression_ const* args() const{ untested();
    if(auto a=prechecked_cast<Expression_ const*>(Token_CALL::args())){ untested();
      return a;
    }else{ untested();
      assert(!Token_CALL::args());
      return nullptr;
    }
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class XDT : public MGVAMS_FILTER {
  Module* _m{nullptr};
  Probe const* _prb{nullptr};
public:
  bool has_modes()const override {return true;}
  bool has_state()const override {return true;}
  bool is_common()const override {return false;} // TODO
  bool needs_context()const override {return false;} // TODO
  bool has_tr_begin()const override {return false;}
  bool has_tr_review()const override {return false;}
  bool has_tr_accept()const override {return false;}
  bool has_tr_advance()const override {return false;}

public: // HACK
  Branch* _br{nullptr};
  Node_Ref _p;
  Node_Ref _n;
protected:
  explicit XDT() : MGVAMS_FILTER() {
    set_label("XDT");
  }
  explicit XDT(XDT const& p) : MGVAMS_FILTER(p) {
  }
  ~XDT(){
//    delete _prb;
  }
  virtual XDT* clone()const override = 0;
  std::string id()const {
    assert(_m);
    return _m->identifier().to_string();
  }
  virtual void make_assign(std::ostream& o) const = 0;
  std::string code_name()const override {
    return "_b_" + short_label();
  }
  virtual int max_args()const = 0;
public:
  Token* new_token(Module& m, size_t na)const override {
    assert(na != size_t(-1));

    std::string filter_code_name = label() + "_" + std::to_string(n_filters++);

    XDT* cl = clone();
    {
      cl->set_label(filter_code_name); // label()); // "_b_" + filter_code_name);
      if(int(na) < max_args()) {
      }else{
	incomplete();
	error(bDANGER, "too many arguments\n");
      }
      cl->set_num_args(na);
      cl->_m = &m;
      m.set_tr_advance();
      m.push_back(cl); // cl?
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
      assert(m.circuit());
      m.new_filter();
    }

    return new Token_XDT(label(), cl);
  }
  void make_cc_precalc_(std::ostream& o)const{
    make_tag(o);
    o__ "ddouble " << code_name() << "precalc(";
      std::string comma;
      if(num_args() > 4) {
	incomplete();
      }else{
      }
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble";
	comma = ", ";
      }
    o << "){\n";
    o__ "ddouble ret = 0.;\n";
    std::string cn = _br->code_name();

    if(_br->is_short()){
      o____ "/* short, mfactor hack */ ret[d_potential" << cn << "] = 1.;\n";
    }else{
      o____ "ret[d_potential" << cn << "] = 1.; // not short.\n";
    }
    o____ "return ret;\n";
    o__ "}\n";
  }
  void args(std::ostream& o)const {
    std::string comma;
    for(size_t n=0; n<num_args(); ++n){
      o << comma << "ddouble t" << n;
      comma = ", ";
    }
  }
  void argnames(std::ostream& o)const {
    std::string comma;
    for(size_t n=0; n<num_args(); ++n){
      o << comma << "t" << n;
      comma = ", ";
    }
  }
  void make_cc_dev(std::ostream& o)const override { // XDT
    o__ "ddouble " << code_name() << "(";
      std::string comma;
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble t" << n;
	comma = ", ";
      }
    o << ");\n";
    o__ "bool _short"+code_name()+"()const {return " << bool(_output) << ";}\n";
    make_cc_precalc_(o);

    o__ "ddouble " << code_name() << "tr_begin("; args(o); o << "){\n";
    for(size_t n=0; n<num_args(); ++n){
      o << "(void)t" << n << ";\n";
    }
    o____ "return 0.;\n";
    o__ "}\n";
    o__ "ddouble " << code_name() << "tr_eval("; args(o); o << "){\n";
    o____ "return " << code_name() << "("; argnames(o); o << ");\n";
    o__ "}\n";
    o__ "ddouble " << code_name() << "tr_advance("; args(o); o << "){\n";
    o____ "return " << code_name() << "("; argnames(o); o << ");\n";
    o__ "}\n";
    o__ "ddouble " << code_name() << "tr_regress("; args(o); o << "){\n";
    o____ "return " << code_name() << "("; argnames(o); o << ");\n";
    o__ "}\n";
    o__ "ddouble " << code_name() << "tr_restore("; args(o); o << "){\n";
    for(size_t n=0; n<num_args(); ++n){
      o << "(void)t" << n << ";\n";
    }
    o____ "return 0.;\n"; // incomplete?
    o__ "}\n";

  }
  void make_cc_impl_comm(std::ostream&)const{ untested();
    unreachable();
  }
  void make_cc_impl(std::ostream&o)const override;
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable();
    return "ddt";
  }
  Probe const* prb()const {return _prb;}
#if 0
  void set_n_to_gnd()const { untested();
    assert(_m);
    _m->set_to_ground(_br->n());
  }
  void set_p_to_gnd()const { untested();
    assert(_m);
    _m->set_to_ground(_br->p());
  }
#else
  void set_n_to_gnd()const {
    assert(_m);
    return MGVAMS_FILTER::set_n_to_gnd(_m);
  }
  void set_p_to_gnd()const {
    assert(_m);
    return MGVAMS_FILTER::set_p_to_gnd(_m);
  }
#endif
private:
  Branch const* output()const override;

  // really?
  Node_Ref p()const override;
  Node_Ref n()const override;
private: // setup
  Branch* branch()const override { return _br; }
}; // XDT
/*--------------------------------------------------------------------------*/
class DDT : public XDT{
public:
  explicit DDT() : XDT() {
    set_label("ddt");
  }
  DDT* clone()const override{
    return new DDT(*this);
  }
  int max_args()const override {return 3;}
private:
  void make_assign(std::ostream&)const override {}
  void make_cc_dev(std::ostream& o)const override { // DDT
    XDT::make_cc_dev(o);
    std::string comma;
    comma = "";
    o__ "ddouble " << code_name() << "tr_accept(";
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble t" << n;
	comma = ", ";
      }
    o << ") {\n";
    if(num_args()>1){
      o____ "(void)t0;\n";
      o____ "(void)t1;\n";
    }else if(num_args()>0){
      o____ "(void)t0;\n";
    }else{
    }

#if 0 // IDT only. merge?
    if(num_args()>2){
      o____ "std::string tmp;\n";
      std::string cn = _br->code_name();
      o____ "((COMPONENT*)" << cn << ")->set_param_by_index(123456, tmp, int(t2));\n";
      // o____ "if(t2) return 0.;\n";
    }else{
    }
#endif

  //  o____ "return " << code_name() << "("; argnames(o); o << ");\n";
    if(has_refs()) {
      std::string cn = _br->code_name();
      o____ "typedef MOD_" << id() << " MOD;\n";
      std::string state = "_st" + cn;

      if(_output){
	o____ "// subdevice\n";
	o____ "t0 = 0.;\n";
      }else{
	o____ "MOD* d = this;\n";
	o____ "auto e = prechecked_cast<ELEMENT const*>(d->"<< cn << ");\n";
	o____ "assert(e);\n";
	// o____ "d->_potential" << cn << " = t0 = e->tr_amps(); // (313)\n";
	o____ "t0 = d->_potential" << cn << ";// = t0 = e->tr_amps(); // (313)\n";
      }

      if(_br->is_short()){
	// output sent to other branch
      }else{
	o____ "t0[d_potential" << cn << "] = 1.;\n";
      }
      o____ "assert(t0 == t0);\n";
      o____ "return t0;\n";
    }else{
      o____ "return 0.; // (no refs)\n";
    }
    o__ "} // ddt tr_accept\n";
/*--------------------------------------------------------------------------*/
    comma = "";
    o__ "ddouble " << code_name() << "tr_review(";
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble";
	comma = ", ";
      }
    o << "){\n";
    o____ "return 0.;";
    o__ "} // ddt tr_review\n";
  }
} ddt;
DISPATCHER<FUNCTION>::INSTALL d_ddt(&function_dispatcher, "ddt", &ddt);
/*--------------------------------------------------------------------------*/
class IDT : public XDT{
public:
  explicit IDT() : XDT() {
    set_label("idt");
  }
  IDT* clone()const override {
    return new IDT(*this);
  }
private:
  bool has_tr_review()const override {return num_args()>2;}
  bool has_tr_accept()const override {return num_args()>2;}

private:
  void make_assign(std::ostream& o)const override {
    make_tag(o);
    if(num_args()>1){
      o__ "t0 = t0 + t1.value();\n";
    }else{
    }
  }
  void make_cc_dev(std::ostream& o)const override { // IDT
    XDT::make_cc_dev(o);
    std::string comma;
    comma = "";
    o__ "ddouble " << code_name() << "tr_accept(";
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble t" << n;
	comma = ", ";
      }
    o << ") {\n";
    if(num_args()>1){
      o____ "(void)t0;\n";
      o____ "(void)t1;\n";
    }else if(num_args()>0){
      o____ "(void)t0;\n";
    }else{
    }
    if(num_args()>2){
      o____ "std::string tmp;\n";
      std::string cn = _br->code_name();
      o____ "((COMPONENT*)" << cn << ")->set_param_by_index(123456, tmp, int(t2));\n";
      // o____ "if(t2) return 0.;\n";
    }else{
    }

  //  o____ "return " << code_name() << "("; argnames(o); o << ");\n";
    if(has_refs()) {
      std::string cn = _br->code_name();
      std::string state = "_st" + cn;

      if(_output){
	o____ "// subdevice\n";
	o____ "t0 = 0.;\n";
      }else{
	o____ "MOD_" << id() << "* d = this;\n";
	o____ "typedef MOD_" << id() << " MOD;\n";
	o____ "auto e = prechecked_cast<ELEMENT const*>(d->"<< cn << ");\n";
	o____ "assert(e);\n";
	// o____ "d->_potential" << cn << " = t0 = e->tr_amps(); // (313)\n";
	o____ "t0 = d->_potential" << cn << ";// = t0 = e->tr_amps(); // (313)\n";
      }

     // make_assign(o);
      if(_br->is_short()){
	// output sent to other branch
      }else{
	o____ "t0[d_potential" << cn << "] = 1.;\n";
      }
      o____ "assert(t0 == t0);\n";
      o____ "return t0;\n";
    }else{ untested();
      o____ "return 0.; // (no refs)\n";
    }
    o__ "} // idt tr_accept\n";
/*--------------------------------------------------------------------------*/
    comma = "";
    o__ "ddouble " << code_name() << "tr_review(";
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble";
	comma = ", ";
      }
    o << "){\n";
    if(num_args()>2){
      o____ "q_accept();\n";
    }else{
    }

    o____ "return 0.;";
    o__ "} // idt tr_review\n";
/*--------------------------------------------------------------------------*/
  }
  int max_args()const override {return 4;}
} idt;
DISPATCHER<FUNCTION>::INSTALL d_idt(&function_dispatcher, "idt", &idt);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch* Token_XDT::branch() const
{
  auto func = prechecked_cast<XDT const*>(f());
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
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
void Token_XDT::stack_op(Expression* e)const
{
  assert(e);
  Token_CALL::stack_op(e);
  assert(!e->is_empty());
  auto cc = prechecked_cast<Token_CALL const*>(e->back());
  assert(cc);
  e->pop_back();
  // assert(!e->is_empty());

  auto func = prechecked_cast<XDT const*>(f());
  assert(func);
  Expression_* args = nullptr;

  assert(cc->args()->size());
  if(is_zero(*cc->args())){
    trace2("Token_XDT::stack_op1", name(), cc->args()->size());
    Float* f = new Float(0.);
    e->push_back(new Token_CONSTANT(f, ""));
    delete cc;
    cc = nullptr;
    func->set_p_to_gnd();
  }else if(auto dd = prechecked_cast<TData const*>(cc->data())) {
    trace2("Token_XDT::stack_op2", name(), cc->args()->size());

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
    args = clone_args(cc->args());
    auto N = new Token_XDT(*this, d, args);
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
    assert(f()==N->f());
    delete(cc);
  }else if(!e->size()) { untested();
    unreachable();
  }else if ( dynamic_cast<Token_PARLIST_ const*>(e->back())) { untested();
    trace2("Token_XDT::stack_op3", name(), cc->args()->size());
    auto d = new TData;
    d->insert(Dep(func->prb())); // BUG?
    auto N = new Token_XDT(*this, d);
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
  }else{ untested();
    unreachable();
  }

  if(args){
    RDeps rr;
    if(func->has_tr_accept()){
      rr.insert(&tr_accept_tag);
    }else{
    }
    rr.insert(func->prb()->branch());
    trace1("Token_XDT::stackop 4", args->size());
    args->update(&rr); // bug. more generic path.
  }else{
  }
  // ------------------------
  // branch: function->_br
}
/*--------------------------------------------------------------------------*/
Branch const* XDT::output() const
{
  if(_output){
    return _output;
  }else{
    return _br;
  }
}
/*--------------------------------------------------------------------------*/
#if 1
Node_Ref XDT::p() const
{
  return _p;
}
/*--------------------------------------------------------------------------*/
Node_Ref XDT::n() const
{
  return _n;
}
#endif
/*--------------------------------------------------------------------------*/
void XDT::make_cc_impl(std::ostream&o) const
{
  make_tag(o);
//    make_cc_impl_comm(o);
  std::string cn = _br->code_name();
  std::string id = _m->identifier().to_string();
  o << "//cc impl\n";
  o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << code_name() << "(";
  std::string comma;
  for(size_t n=0; n<num_args(); ++n){
    o << comma << "ddouble t" << n;
    comma = ", ";
  }
  o << ")\n{\n";
  if(num_args()>2) {
    o__ "(void)t1;\n";
    o__ "(void)t2;\n";
  }else if(num_args()>1) {
    o__ "(void)t1;\n";
  }else{
  }
  if(has_refs()) {
    o__ "MOD_" << id << "* d = this;\n";
    o__ "typedef MOD_" << id << " MOD;\n";
    std::string state = "_st" + cn;

    {// make_cc_tmp(o, state, _br->deps());
      TData const& deps = _br->deps();
      char sign = '+';
    //  indent a;
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
    trace2("make_cc_impl xdt use", _br->code_name(), _br->is_used());

    if(_output){
      o__ "// subdevice\n";
      o__ "t0 = 0.;\n";
    }else{
      o__ "auto e = prechecked_cast<ELEMENT const*>(d->"<< cn << ");\n";
      o__ "assert(e);\n";
      o__ "d->_potential" << cn << " = t0 = e->tr_amps(); // (236)\n";
    }

    make_assign(o);
    if(_br->is_short()){
      // output sent to other branch
    }else{
      o__ "t0[d_potential" << cn << "] = 1.;\n";
    }
    o__ "assert(t0 == t0);\n";
    o__ "return t0;\n";
  }else{ untested();
    o__ "return 0.; // (no refs)\n";
  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
} // XDT::make_cc_impl
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
