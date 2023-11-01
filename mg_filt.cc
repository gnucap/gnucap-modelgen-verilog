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
#include "m_tokens.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
// todo: rearrange, avoid Filter
static void make_cc_tmp(Filter const* f, std::ostream& o)
{
  assert(f->has_branch());

  {
    char sign = '+';
    indent a;
    o__ "d->" << f->state() << "[0] = " << sign << " " << "t0.value();\n";
    size_t k = 2;

    for(auto v : f->deps()) {
      // char sign = f.reversed()?'-':'+';
      o__ "// dep " << v->code_name() << "\n";
      // if(f->branch() == v->branch()){ untested(); }
      if(v->branch()->is_short()){ untested();
      }else{
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "// assert(!d->" << f->state() << "[" << k << "]);\n";
	o__ "d->" << f->state() << "[" //  << k << "]"
	  << "MOD::" << f->state() << "_::dep" << v->code_name() << "] "
	  " = " << sign << " " << "t0[d" << v->code_name() << "]; // (4)\n";
	++k;
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
static int n_filters;
/*--------------------------------------------------------------------------*/
class Token_XDT : public Token_CALL {
//  XDT* _xdt{NULL};
public:
  explicit Token_XDT(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_XDT(const Token_XDT& P, Base const* data, Expression_ const* e = NULL)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override {untested(); return new Token_XDT(*this);}

  void stack_op(Expression* e)const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class XDT : public MGVAMS_FILTER {
  Module* _m{NULL};
  Token_CALL const* _token{NULL};
  // Branch_Ref _br;
  Probe const* _prb{NULL};
public: // HACK
  Filter* _f{NULL};
protected:
  Branch const* _br{NULL};
protected:
  explicit XDT() : MGVAMS_FILTER() {
  }
  explicit XDT(XDT const& p) : MGVAMS_FILTER(p) {
  }
  ~XDT(){
//    delete _prb;
  }
  virtual XDT* clone()const = 0;
  virtual void make_assign(std::ostream& o) const = 0;
  std::string code_name()const override{
    return "/*XDT*/ d->" + label();
  }
public:
  Token* new_token(Module& m, size_t na)const override {
    assert(na != size_t(-1));
    Filter* f = NULL;

    std::string filter_code_name = label() + "_" + std::to_string(n_filters++);

    XDT* cl = clone();
    {
      f = new Filter(filter_code_name);
      f->set_owner(&m);
      f->set_dev_type("va_" + label());

      cl->set_label("_b_" + filter_code_name);
      assert(na<3);
      cl->set_num_args(na);
      cl->_f = f;
      cl->_m = &m;
      m.push_back(cl);
    }

    Node* n = m.new_node(filter_code_name);
    {
      Branch* br = m.new_branch(n, &mg_ground_node);
      br->set_filter();
      assert(br);
      assert(const_cast<Branch const*>(br)->owner());
      Branch_Ref prb(br);
      cl->_br = br;
      f->set_output(prb);
      f->set_state( "_st" + prb.code_name() );

      cl->_prb = m.new_probe("potential", prb);
//	br->set_element(f);
      br->set_filter();
      m.push_back(f);
    }

    return new Token_XDT(label(), cl);
  }
  void make_cc_common(std::ostream& o)const override{
    assert(num_args()!=size_t(-1));
    o__ "class FILTER" << label() << "{\n";
    o__ "public:\n";
    o____ "ddouble operator()(";
      for(size_t n=0; n<num_args(); ++n){
	o << "ddouble t" << n << ", ";
      }
    o << "COMPONENT*) const;\n";
    o__ "} " << label() << ";\n";
  }
  void make_cc_dev(std::ostream& o)const override{
    o__ "ddouble " << label() << "(";
      std::string comma;
      assert(num_args() < 3);
      for(size_t n=0; n<num_args(); ++n){
	o << comma << "ddouble t" << n;
	comma = ", ";
      }
    o << ");\n";
  }
  void make_cc_impl_comm(std::ostream&o)const{
    assert(_m); // owner?
    assert(_f);
    std::string id = _m->identifier().to_string();
    o << "COMMON_" << id << "::";
    o << "ddouble COMMON_" << id << "::FILTER" << label() <<
      "::operator()(";
    assert(num_args() < 3);
    for(size_t n=0; n<num_args(); ++n){
      o << " ddouble t" << n << ", ";
    }
    o << "COMPONENT* dd) const\n{\n";
    o__ "MOD_" << id << "* d = prechecked_cast<MOD_" << id << "*>(dd);\n";
    o__ "assert(d);\n";
    make_cc_tmp(_f, o);

    make_assign(o);
    o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  }
  void make_cc_impl(std::ostream&o)const override{
//    make_cc_impl_comm(o);
    std::string id = _m->identifier().to_string();
    o << "MOD_"<< id <<"::ddouble MOD_" << id << "::" << label() << "(";
    std::string comma;
    for(size_t n=0; n<num_args(); ++n){
      o << comma << "ddouble t" << n;
      comma = ", ";
    }
    o << ")\n{\n";
    o__ "MOD_" << id << "* d = this;\n";
    o__ "typedef MOD_" << id << " MOD;\n";
    make_cc_tmp(_f, o);

    make_assign(o);
    o << "}\n";
  }
  std::string eval(CS&, const CARD_LIST*)const override{
    unreachable();
    return "ddt";
  }
  Probe const* prb() const{return _prb;}
};
/*--------------------------------------------------------------------------*/
class DDT : public XDT{
public:
  explicit DDT() : XDT() {
    set_label("ddt");
  }
  DDT* clone()const /*override*/{
    return new DDT(*this);
  }
private:
  void make_assign(std::ostream& o)const {
    std::string cn = _br->code_name();
    o__ "d->" << cn << "->do_tr();\n";
    o__ "t0 = d->" << cn << "->tr_amps();\n";
    o__ "d->_potential" << cn << " = - t0;\n";
    o__ "trace2(\"filt\", t0, d->"<< cn<<"->tr_outvolts());\n";
    o__ "t0[d_potential" << cn << "] = -1.;\n";
    o__ "assert(t0 == t0);\n";
    o__ "return t0;\n";
  }
} ddt;
DISPATCHER<FUNCTION>::INSTALL d_ddt(&function_dispatcher, "ddt", &ddt);
/*--------------------------------------------------------------------------*/
class IDT : public XDT{
public:
  explicit IDT() : XDT() {
    set_label("idt");
  }
  IDT* clone()const /*override*/{
    return new IDT(*this);
  }

private:
  void make_assign(std::ostream& o)const {
    std::string cn = _br->code_name();
    o__ "d->" << cn << "->do_tr();\n";
    o__ "t0 = d->" << cn << "->tr_amps();\n";
    o__ "d->_potential" << cn << " = - t0;\n";
    o__ "trace2(\"filt\", t0, d->"<< cn<<"->tr_outvolts());\n";
    if(num_args()>1){
      o__ "t0 = t0 + t1.value();\n";
    }else{
    }
    o__ "t0[d_potential" << cn << "] = -1.;\n";
    o__ "assert(t0 == t0);\n";
    o__ "return t0;\n";
  }
} idt;
DISPATCHER<FUNCTION>::INSTALL d_idt(&function_dispatcher, "idt", &idt);
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

  auto ff = prechecked_cast<XDT const*>(f());
  assert(ff);

  if(auto dd = prechecked_cast<Deps const*>(cc->data())){
    assert(dd);

    assert(ff->_f);
    ff->_f->set_deps(*dd); // HACK

    auto d = new Deps;
    d->insert(Dep(ff->prb())); // BUG?
    auto N = new Token_XDT(*this, d, cc->args()?cc->args()->clone():NULL);
    e->push_back(N);
    assert(f()==N->f());
    delete(cc);
  }else if(!e->size()) {
    unreachable();
  }else if ( dynamic_cast<Token_PARLIST_ const*>(e->back())) {
    auto d = new Deps;
    d->insert(Dep(ff->prb())); // BUG?
    auto N = new Token_XDT(*this, d);
    e->push_back(N);
  }else{
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
