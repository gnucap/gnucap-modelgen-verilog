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
 * absdelay function/filter
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_analog.h"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
#include "f__.cc"
/*--------------------------------------------------------------------------*/
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
      o__ "// dep " << probe(v)->code_name() << "\n";
      // if(f->branch() == v->branch()){ untested(); }
      if(branch(v)->is_short()){ untested();
      }else{
	o__ "assert(" << "t0[d" << probe(v)->code_name() << "] == t0[d" << probe(v)->code_name() << "]" << ");\n";
	o__ "// assert(!d->" << state << "[" << k << "]);\n";
	o__ "d->" << state << "[" //  << k << "]"
	  << "MOD::" << state << "_::dep" << probe(v)->code_name() << "] "
	  " = " << sign << " " << "t0[d" << probe(v)->code_name() << "]; // (4)\n";
	++k;
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
#if 0
class Token_ABSDELAY : public Token_CALL {
public:
  explicit Token_ABSDELAY(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) { untested();}
private:
  explicit Token_ABSDELAY(const Token_ABSDELAY& P, Base const* data, Expression_ const* e = nullptr)
    : Token_CALL(P, data, e) { untested();} // , _item(P._item) {}
  Token* clone()const override {untested(); return new Token_ABSDELAY(*this);}

  void stack_op(Expression* e)const override;
  Branch* branch() const;
};
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class ABSDELAY : public MGVAMS_FILTER /* FUNCTION_ */ {
  Probe const* _prb{nullptr};
public: // HACK
 // Branch* _br{nullptr};
  mutable Branch const* _output{nullptr};
//  Node_Ref _p;
//  Node_Ref _n;

  explicit ABSDELAY() : MGVAMS_FILTER() {
    set_label("absdelay");
  }
  explicit ABSDELAY(ABSDELAY const& p) : MGVAMS_FILTER(p) {
  }
  ~ABSDELAY(){
//    delete _prb; belongs to _m
  }
  int max_args()const override { return 3;}
  bool port_hack()const override {return false;}
  bool is_analog_filter()const override {return true;}
  std::string eval_name()const override {
    if(_m){
      std::string id = _m->identifier().to_string();
      return "COMMON_" + id + "::_common" + code_name();
    }else{
      // unreachable();
      return "missing_m??";
    }
  }
  virtual ABSDELAY* clone()const override {
    return new ABSDELAY(*this);
  }
protected:
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
  // Token* new_token(Expression_ const* e) ...
  // Module* m = e->owner()...
  Token* new_token(Module&, size_t)const override {
    return nullptr;
#if 0
    assert(na != size_t(-1));

    std::string filter_code_name = label() + "_" + std::to_string(n_filters++);

    ABSDELAY* cl = clone();
    { untested();
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
    { untested();
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

    return new Token_ABSDELAY(label(), cl);
#endif
  }

  void make_cc_common(std::ostream& o)const override{
    o << "public:\n";
    o__ "class common" << code_name() <<": public COMMON_FILT {\n";
    o____ "COMMON_COMPONENT* clone()const override{unreachable(); return nullptr;}\n";
    o__ "public:\n";
    o____ "common" << code_name() <<"(int i=CC_STATIC) : COMMON_FILT(i) {}\n";
    o__ "public:\n";
    o__ "private:\n";
    o____ "std::string name()const override {return \"absdelay_name\";}\n";
    o__ "};\n";
    o__ "static common" << code_name()
      << " _common" << code_name() << ";\n";
  }

  void make_cc_dev(std::ostream& o)const override{
    indent x(2);
    o__ "ddouble " << code_name() << "(ddouble t0";
      assert(num_args());
      assert(num_args() < 4);
      for(int n=1; n < int(num_args()); ++n){
	o << ", double t" << n;
      }

    o << "); // (d)\n";
    o__ "bool _short"+code_name()+"()const {return " << bool(_output) << ";}\n";
    o__ "ddouble " << code_name() << "__precalc(ddouble const&";
      assert(num_args());
      assert(num_args() < 4);
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
    indent xx(2);
    for(int n=1; n < int(num_args()); ++n){
      o__ "(void) " << "t" << n << ";\n";
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
    assert(num_args() < 4);
    if(num_args()>1){
      o << ", double delay=1";
    }else{ untested();
      unreachable();
    }
    if(num_args()>2){ untested();
      o << ", double maxdelay=1";
    }else{
    }
    o << ")\n{\n";
    {
      o__ "ddouble ret(0.);\n";
      o__ "COMPONENT* l = " << cn << ";\n";
      o__ "assert(l);\n";
      o__ "std::string reset;\n";
      if(num_args()>1){
	o__ "l->set_param_by_name(\"delay\", \"\");\n";
	o__ "l->set_param_by_name(\"delay\", to_string(delay));\n";
      }else{ untested();
	unreachable();
      }
      if(num_args()>2){ untested();
	o__ "l->set_param_by_name(\"maxdelay\", \"\");\n";
	o__ "l->set_param_by_name(\"maxdelay\", to_string(maxdelay));\n";
      }else{
      }
      o__ "ret[d_potential" << cn << "] = -1.;\n";
      o__ "return ret;\n";
    }
    o__ "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  }
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable();
    return "absdelay";
  }
  Probe const* prb()const { untested();return _prb;}
  void set_n_to_gnd__()const override {
    assert(_m);
    return MGVAMS_FILTER::set_n_to_gnd(_m);
  }
  void set_p_to_gnd__()const override {
    assert(_m);
    return MGVAMS_FILTER::set_p_to_gnd(_m);
  }
private:
  Branch const* output()const override;
  Node_Ref p()const override;
  Node_Ref n()const override;
private: // setup
  Branch* branch()const override { return _br; }
} absdel;
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "absdelay", &absdel);
/*--------------------------------------------------------------------------*/
/// Branch* Token_ABSDELAY::branch() const
/// { untested();
///   auto func = prechecked_cast<ABSDELAY const*>(f());
///   assert( func);
///   assert( func->_br);
///   return func->_br;
/// }
/*--------------------------------------------------------------------------*/
#if 0
static Expression_* clone_args(Base const* e)
{ untested();
  if(auto e_ = dynamic_cast<Expression_ const*>(e)) { untested();
    return e_->clone();
  }else{ untested();
    unreachable();
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
void Token_ABSDELAY::stack_op(Expression* e)const
{ untested();
  assert(e);
  Token_CALL::stack_op(e);
	
  assert(!e->is_empty());
  auto cc = prechecked_cast<Token_CALL const*>(e->back());
  assert(cc);
  assert(cc->args());
  trace1("Token_ABSDELAY::stack_op", cc->args()->size());
  e->pop_back();

  auto func = prechecked_cast<ABSDELAY const*>(f());
  assert(func);

  if(cc->args()->size() < 1){ untested();
    throw Exception("syntax error, need delay");
  }else{ untested();
  }

  if(auto dd = prechecked_cast<TData const*>(cc->data())) { untested();
    // cc->args()[0]->data?
    assert(dd);

    branch()->deps().clear();
    branch()->deps() = *dd; // HACK
    if(1){ untested();
      func->set_n_to_gnd();
    }else if(0 /*sth linear*/){ untested();
      // somehow set loss=0 and output ports to target.
    }else{ untested();
    }

    auto d = new TData;
    trace1("xdt output dep", func->prb()->code_name());
    d->insert(Dep(func->prb(), Dep::_LINEAR)); // BUG?
    auto N = new Token_ABSDELAY(*this, d, clone_args(cc->args()));
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
    auto N = new Token_ABSDELAY(*this, d);
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
  }else{ untested();
    unreachable();
  }
};
#endif
/*--------------------------------------------------------------------------*/
Branch const* ABSDELAY::output() const
{
  if(_output){ untested();
    return _output;
  }else{
    return _br;
  }
}
/*--------------------------------------------------------------------------*/
#if 1
Node_Ref ABSDELAY::p() const
{
  return _br->p();
}
/*--------------------------------------------------------------------------*/
Node_Ref ABSDELAY::n() const
{ untested();
  return _br->n();
}
#endif
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
