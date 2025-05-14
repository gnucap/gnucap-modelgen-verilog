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
 * Verilog-AMS builtin math functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.cc"
#include "mg_token.h"
#include "mg_module.h"
#include <globals.h>
#include <u_parameter.h>
#include "f__.cc"
/*--------------------------------------------------------------------------*/
bool Statement::propagate_rdep(Base const* )
{
  unreachable();
  incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
void Expression_::dump(std::ostream& ) const
{
  unreachable();
  incomplete();
}
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
Base const* get_constant(Expression* E)
{
  assert(E);
  Base const* p = nullptr;
  assert(E->size());
  Token* pl = E->back();
  if(dynamic_cast<Token_PARLIST*>(pl)){
    E->pop_back();
    assert(E->size());
  }else{
    pl = nullptr;
  }

  if(auto cc=dynamic_cast<Token_CONSTANT const*>(E->back())) {
    p = cc->data();
  }else{
  }

  if(pl){
    E->push_back(pl);
  }else{
  }

  return p;
}
/*--------------------------------------------------------------------------*/
std::pair<Base const*, Base const*> get_constant2(Expression* E)
{
  assert(E);
  std::pair<Base const*, Base const*> ret;
  auto it = E->end();
  --it;

  Token* pl = E->back();
  if(dynamic_cast<Token_PARLIST*>(pl)) {
    assert(it!=E->begin());
    --it;
  }else{ untested();
    pl = nullptr;
  }

  if(auto cc=dynamic_cast<Token_CONSTANT const*>(*it)) {
    ret.second = cc->data();
    assert(it!=E->begin());
    --it;
    if(auto c2=dynamic_cast<Token_CONSTANT const*>(*it)) {
      ret.first = c2->data();
    }else{ untested();
    }
  }else{ untested();
  }

  if(!pl){ untested();
  }else if(it == E->begin()) { untested();
    unreachable();
  }else{
    --it;
    if(dynamic_cast<Token_STOP const*>(*it)) {
    }else{ untested();
      unreachable(); // hopefully.
      ret.first = ret.second = nullptr;
    }
  }

  return ret;
}
/*--------------------------------------------------------------------------*/
double get_double(Expression* E)
{
  Base const* b = get_constant(E);
  bool ok = false;
  double p;
  if(auto f = dynamic_cast<Float const*>(b)){
    p = f->value();
    ok = true;
  }else if(auto i = dynamic_cast<Integer const*>(b)){
    p = i->value();
    ok = true;
  }else{
  }

  if(ok){
    return p;
  }else{
    throw Exception("not double\n");
  }
}
/*--------------------------------------------------------------------------*/
#if 0
int get_int(Expression* E)
{
  Base const* b = get_constant(E);
  bool ok = false;
  int p;
  if(auto f = dynamic_cast<Integer const*>(b)){
    p = f->value();
    ok = true;
  }else{
  }

  if(ok){
    return p;
  }else{
    throw Exception("not double\n");
  }
}
#endif
/*--------------------------------------------------------------------------*/
void subs_double(Expression* E, double x)
{
  assert(E->size());
  if(dynamic_cast<Token_PARLIST*>(E->back())) {
    delete E->back();
    E->pop_back();
    delete E->back();
    E->pop_back();
    assert(dynamic_cast<Token_STOP const*>(E->back()));
    delete E->back();
    E->pop_back();
  }else{
    assert(dynamic_cast<Token_CONSTANT const*>(E->back()));
    delete E->back();
    E->pop_back();
  }
  auto v = new vReal(x);
  E->push_back(new Token_CONSTANT(v));
}
/*--------------------------------------------------------------------------*/
void subs_double2(Expression* E, double x)
{
  assert(E->size());
  if(dynamic_cast<Token_PARLIST*>(E->back())) {
    delete E->back();
    E->pop_back();
    delete E->back();
    E->pop_back();
    delete E->back();
    E->pop_back();
    assert(dynamic_cast<Token_STOP const*>(E->back()));
    delete E->back();
    E->pop_back();
  }else{
    assert(dynamic_cast<Token_CONSTANT const*>(E->back()));
    delete E->back();
    E->pop_back();
    assert(dynamic_cast<Token_CONSTANT const*>(E->back()));
    delete E->back();
    E->pop_back();
  }
  auto v = new vReal(x);
  E->push_back(new Token_CONSTANT(v));
}
/*--------------------------------------------------------------------------*/
void subs_int(Expression* E, int x)
{
  assert(E->size());
  if(dynamic_cast<Token_PARLIST*>(E->back())) { untested();
    delete E->back();
    E->pop_back();
    delete E->back();
    E->pop_back();
    assert(dynamic_cast<Token_STOP const*>(E->back()));
    delete E->back();
    E->pop_back();
  }else{
    assert(dynamic_cast<Token_CONSTANT const*>(E->back()));
    delete E->back();
    E->pop_back();
  }
  auto v = new vInteger(x);
  E->push_back(new Token_CONSTANT(v));
}
/*--------------------------------------------------------------------------*/
class STUB : public MGVAMS_FUNCTION {
public:
  explicit STUB(std::string const l) : MGVAMS_FUNCTION() {
    set_label(l);
  }
  ~STUB() { }
private:
  std::string code_name()const override{ untested();
    if(label()!=""){ untested();
      return "";
    }else{ untested();
      return "va::" + label();
    }
  }
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
};
/*--------------------------------------------------------------------------*/
// TODO: these are still in m_va.h
STUB acosh("acosh");
DISPATCHER<FUNCTION>::INSTALL d_acosh(&function_dispatcher, "acosh|$acosh", &acosh);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#if 1
/*--------------------------------------------------------------------------*/
class abs : public MGVAMS_FUNCTION {
public:
  explicit abs() : MGVAMS_FUNCTION(){
    set_label("abs");
  }
  ~abs() { }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { untested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::abs(x));
  }
  void stack_op(Expression* e)const override { untested();
    Base const* x = get_constant(e);
    if(auto f = dynamic_cast<Float const*>(x)) { untested();
      subs_double(e, std::abs(f->value()));
    }else if(auto i = dynamic_cast<Integer const*>(x)) { untested();
      subs_double(e, std::abs(i->value()));
    }else{
      throw Exception("invalid arg");
    }
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {\n";
    o____ "if(d>=0.){\n";
    o____ "}else{\n";
    o______ "d *= -1.;\n";
    o____ "}\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_abs";
  }
} p_abs;
DISPATCHER<FUNCTION>::INSTALL d_abs(&function_dispatcher, "abs|$abs", &p_abs);
/*--------------------------------------------------------------------------*/
class asinh : public MGVAMS_FUNCTION {
public:
  asinh() : MGVAMS_FUNCTION(){
    set_label("asinh");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::asinh(x));
  }
  void make_cc_common(std::ostream& o)const override{ untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o__ "double b = std::sqrt(1.+double(d)*double(d));\n";
    o____ "chain(d, 1./b);\n";
    o____ "::set_value(d, std::asinh(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{ untested();
    return "_f_asinh";
  }
} p_asinh;
DISPATCHER<FUNCTION>::INSTALL d_asinh(&function_dispatcher, "asinh|$asinh", &p_asinh);
/*--------------------------------------------------------------------------*/
class atan : public MGVAMS_FUNCTION {
public:
  atan() : MGVAMS_FUNCTION(){
    set_label("atan");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::atan(x));
  }
  void make_cc_common(std::ostream& o)const override{ untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "chain(d, 1./(1.+double(d)*double(d)));\n";
    o____ "::set_value(d, std::atan(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{ untested();
    return "_f_atan";
  }
} p_atan;
DISPATCHER<FUNCTION>::INSTALL d_atan(&function_dispatcher, "atan|$atan", &p_atan);
/*--------------------------------------------------------------------------*/
class atanh : public MGVAMS_FUNCTION {
public:
  atanh() : MGVAMS_FUNCTION(){
    set_label("atanh");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::atanh(x));
  }
  void make_cc_common(std::ostream& o)const override{ untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{untested();\n";
    o______ "chain(d, 1./((1.-double(d))*(1+double(d))));\n";
    o______ "return ::set_value(d, std::atanh(d));\n";
    o__ "}\n";
  }
  std::string code_name()const override{ untested();
    return "_f_atanh";
  }
} p_atanh;
DISPATCHER<FUNCTION>::INSTALL d_atanh(&function_dispatcher, "atanh|$atanh", &p_atanh);
/*--------------------------------------------------------------------------*/
class ceil : public MGVAMS_FUNCTION {
public:
  explicit ceil() : MGVAMS_FUNCTION(){
    set_label("$ceil");
  }
  void stack_op(Expression* e)const override { untested();
    Base const* x = get_constant(e);
    if(auto f = dynamic_cast<Float const*>(x)) { untested();
      subs_double(e, std::ceil(f->value()));
    }else if(auto i = dynamic_cast<Integer const*>(x)) { untested();
      subs_int(e, i->value());
    }else{ untested();
      throw Exception("invalid arg");
    }
  }
  std::string code_name()const override{ untested();
    return "_f_ceil";
  }
  void make_cc_common(std::ostream& o)const override { untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "return int(std::ceil(d));\n";
    o__ "}\n";
  }
} p_ceil;
DISPATCHER<FUNCTION>::INSTALL d_ceil(&function_dispatcher, "ceil|$ceil", &p_ceil);
/*--------------------------------------------------------------------------*/
class cos : public MGVAMS_FUNCTION {
public:
  explicit cos() : MGVAMS_FUNCTION(){
    set_label("cos");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::cos(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const {\n";
    o______ "chain(d, -std::sin(d));\n";
    o______ "return ::set_value(d, std::cos(d));\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_cos";
  }
} p_cos;
DISPATCHER<FUNCTION>::INSTALL d_cos(&function_dispatcher, "cos|$cos", &p_cos);
/*--------------------------------------------------------------------------*/
class cosh : public MGVAMS_FUNCTION {
public:
  explicit cosh() : MGVAMS_FUNCTION(){
    set_label("cosh");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::cosh(x));
  }
  void make_cc_common(std::ostream& o)const override{ untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "chain(d, std::sinh(d));\n";
    o____ "return ::set_value(d, std::cosh(d));\n";
    o__ "};\n";
  }
  std::string code_name()const override{ untested();
    return "_f_cosh";
  }
} p_cosh;
DISPATCHER<FUNCTION>::INSTALL d_cosh(&function_dispatcher, "cosh|$cosh", &p_cosh);
/*--------------------------------------------------------------------------*/
class exp : public MGVAMS_FUNCTION {
public:
  explicit exp() : MGVAMS_FUNCTION(){
    set_label("exp");
  }
  void stack_op(Expression* e)const override {
    double x = get_double(e);
    subs_double(e, std::exp(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "::set_value(d, std::exp(d));\n";
    o____ "return chain(d, d);\n";
    o__ "}\n";
//    o__ "double " << code_name() << "(double const& d) const{\n";
//    o____ "return std::exp(d);\n";
//    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_exp";
  }
} p_exp;
DISPATCHER<FUNCTION>::INSTALL d_exp(&function_dispatcher, "exp|$exp", &p_exp);
/*--------------------------------------------------------------------------*/
class expm1 : public MGVAMS_FUNCTION {
public:
  explicit expm1() : MGVAMS_FUNCTION(){
    set_label("expm1");
  }
  void stack_op(Expression* e)const override {
    double x = get_double(e);
    subs_double(e, std::expm1(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "::set_value(d, std::expm1(d));\n";
    o____ "return chain(d, d+1.);\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_expm1";
  }
} p_expm1;
DISPATCHER<FUNCTION>::INSTALL d_expm1(&function_dispatcher, "expm1|$expm1", &p_expm1);
/*--------------------------------------------------------------------------*/
class limexp : public MGVAMS_FUNCTION {
public:
  explicit limexp() : MGVAMS_FUNCTION(){
    set_label("limexp");
  }
  ~limexp() { }
  void stack_op(Expression* e)const override {
    double x = get_double(e);
    subs_double(e, std::exp(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "const double lt = 80.;\n";
    o____ "double nv;\n";
    o____ "double nd;\n";
    o____ "if(d>lt){\n";
    o______ "nv = std::exp(lt) * (1.0 - lt + d);\n";
    o______ "nd = std::exp(lt);\n";
    o____ "}else if(d<-lt){\n";
    o______ "nv = std::exp(-lt) * (1.0 + lt + d);\n";
    o______ "nd = std::exp(-lt);\n";
    o____ "}else{\n";
    o______ "nv = nd = std::exp(d);\n";
    o____ "}\n";
    // o____ "std::cerr << \"limexp: \" << d << \" \" << nv << \" \" << nd << \"\\n\";";
    o____ "::set_value(d, nv);\n";
    o____ "return chain(d, nd);\n";
    o__ "}\n";
//    o__ "double " << code_name() << "(double const& d) const{\n";
//    o____ "return std::exp(d);\n";
//    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_limexp";
  }
} p_limexp;
DISPATCHER<FUNCTION>::INSTALL d_limexp(&function_dispatcher, "limexp|$limexp", &p_limexp);
/*--------------------------------------------------------------------------*/
class max : public MGVAMS_FUNCTION {
public:
  explicit max() : MGVAMS_FUNCTION(){
    set_label("max");
  }
  ~max() { }
  std::string eval(CS&, const CARD_LIST*)const override { untested();
    unreachable();
    return "..";
  }
  void stack_op(Expression* e)const override { untested();
    auto args = get_constant2(e);
    if(auto a = dynamic_cast<Float const*>(args.first)) { untested();
      if(auto b = dynamic_cast<Float const*>(args.second)) { untested();
	subs_double2(e, std::max(a->value(), b->value()));
      }else if(auto bi = dynamic_cast<Integer const*>(args.second)) { untested();
	subs_double2(e, std::max(a->value(), double(bi->value())));
      }else{ untested();
	throw Exception("invalid");
      }
    }else if(auto ai = dynamic_cast<Integer const*>(args.first)) { untested();
      if(auto b = dynamic_cast<Float const*>(args.second)) { untested();
	subs_double2(e, std::max(double(ai->value()), b->value()));
      }else if(auto bi = dynamic_cast<Integer const*>(args.second)) { untested();
	subs_double2(e, std::max(bi->value(), ai->value()));
      }else{ untested();
	throw Exception("invalid");
      }
    }else{
      throw Exception("invalid");
    }
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T, class S>\n";
    o__ "typename va::ddouble_if<T, S>::type\n";
    o__ "    " << code_name() << "(T d, S e)const {\n";
    o____ "typedef typename va::ddouble_if<T, S>::type ret_t;\n";
    o____ "if(double(d) <= double(e)){itested();\n";
    o____ "  return ret_t(e);\n";
    o____ "}else{ itested();\n";
    o____ "  return ret_t(d);\n";
    o____ "}\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_max";
  }
} p_max;
DISPATCHER<FUNCTION>::INSTALL d_max(&function_dispatcher, "max|$max", &p_max);
/*--------------------------------------------------------------------------*/
class min : public MGVAMS_FUNCTION {
public:
  explicit min() : MGVAMS_FUNCTION(){
    set_label("min");
  }
  ~min() { }
  std::string eval(CS&, const CARD_LIST*)const override { untested();
    unreachable();
    return "..";
  }
  void stack_op(Expression* e)const override { untested();
    auto args = get_constant2(e);
    if(auto a = dynamic_cast<Float const*>(args.first)) { untested();
      if(auto b = dynamic_cast<Float const*>(args.second)) { untested();
	subs_double2(e, std::min(a->value(), b->value()));
      }else if(auto bi = dynamic_cast<Integer const*>(args.second)) { untested();
	subs_double2(e, std::min(a->value(), double(bi->value())));
      }else{ untested();
	throw Exception("invalid");
      }
    }else if(auto ai = dynamic_cast<Integer const*>(args.first)) { untested();
      if(auto b = dynamic_cast<Float const*>(args.second)) { untested();
	subs_double2(e, std::min(double(ai->value()), b->value()));
      }else if(auto bi = dynamic_cast<Integer const*>(args.second)) { untested();
	subs_double2(e, std::min(bi->value(), ai->value()));
      }else{ untested();
	throw Exception("invalid");
      }
    }else{
      throw Exception("invalid");
    }
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T, class S>\n";
    o__ "typename va::ddouble_if<T, S>::type\n";
    o__ "    " << code_name() << "(T d, S e)const {\n";
    o____ "typedef typename va::ddouble_if<T, S>::type ret_t;\n";
    o____ "if(double(d) <= double(e)){itested();\n";
    o____ "  return ret_t(d);\n";
    o____ "}else{ itested();\n";
    o____ "  return ret_t(e);\n";
    o____ "}\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_min";
  }
} p_min;
DISPATCHER<FUNCTION>::INSTALL d_min(&function_dispatcher, "min|$min", &p_min);
/*--------------------------------------------------------------------------*/
class floor : public MGVAMS_FUNCTION {
public:
  explicit floor() : MGVAMS_FUNCTION(){
    set_label("$floor");
  }
  void stack_op(Expression* e)const override { untested();
    Base const* x = get_constant(e);
    if(auto f = dynamic_cast<Float const*>(x)) { untested();
      subs_double(e, std::floor(f->value()));
    }else if(auto i = dynamic_cast<Integer const*>(x)) { untested();
      subs_int(e, i->value());
    }else{ untested();
      throw Exception("invalid arg");
    }
  }
  std::string code_name()const override{ untested();
    return "_f_floor";
  }
  void make_cc_common(std::ostream& o)const override { untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "return int(std::floor(d));\n";
    o__ "}\n";
  }
} p_floor;
DISPATCHER<FUNCTION>::INSTALL d_floor(&function_dispatcher, "floor|$floor", &p_floor);
/*--------------------------------------------------------------------------*/
class log : public MGVAMS_FUNCTION {
public:
  explicit log() : MGVAMS_FUNCTION(){
    set_label("$log10");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::log10(x));
  }
  std::string code_name()const override{ untested();
    return "_f_log10";
  }
  void make_cc_common(std::ostream& o)const override{ untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "double l=-1e99;\n";
    o____ "if(d>1e-60){itested();\n";
    o______ "l = std::log10(double(d));\n";
    o______ "chain(d, 1./double(d));incomplete();\n"; // scale.
    o____ "}else if(d>0){ untested();\n";
    o______ "l=-1e60;\n";
    o______ "chain(d, 1e60);\n";
    o____ "}else{\n";
    o______ "unreachable();\n";
    o______ "l=-1e40;\n";
    o______ "chain(d, 1e40);\n";
    o____ "}\n";
    o____ "::set_value(d, l);\n";
    o____ "return d;\n";
    o____ "}\n";
  }
} p_log;
DISPATCHER<FUNCTION>::INSTALL d_log(&function_dispatcher, "log|$log10", &p_log);
/*--------------------------------------------------------------------------*/
class ln : public MGVAMS_FUNCTION {
public:
  explicit ln() : MGVAMS_FUNCTION(){
    set_label("ln");
  }
  void stack_op(Expression* e)const override {
    double x = get_double(e);
    subs_double(e, std::log(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {\n";
    o____ "double l=-1e99;\n";
    o____ "if(d>1e-60) {\n";
    o______ "l = std::log(double(d));\n";
    o______ "chain(d, 1./double(d));\n";
    o____ "}else if(d>0){ untested();\n";
    o______ "l=-1e60;\n";
    o______ "chain(d, 1e60);\n";
    o____ "}else{\n";
    o______ "unreachable();\n";
    o______ "l=-1e40;\n";
    o______ "chain(d, 1e40);\n";
    o____ "}\n";
    o____ "::set_value(d, l);\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_ln";
  }
} p_ln;
DISPATCHER<FUNCTION>::INSTALL d_ln(&function_dispatcher, "ln|$ln|$log", &p_ln);
/*--------------------------------------------------------------------------*/
class ln1p : public MGVAMS_FUNCTION {
public:
  explicit ln1p() : MGVAMS_FUNCTION(){
    set_label("ln1p");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::log1p(x));
  }
  void make_cc_common(std::ostream& o)const override { untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {\n";
    o____ "double l=-1e99;\n";
    o____ "if(d>-1.) {\n";
    o______ "l = std::log1p(double(d));\n";
    o______ "chain(d, 1./double(1.+d));\n";
    o____ "}else{\n";
    o______ "unreachable();\n";
    o______ "l=-1e40;\n";
    o______ "chain(d, 1e40);\n";
    o____ "}\n";
    o____ "::set_value(d, l);\n";
    o____ "return d;\n";
    o____ "}\n";
  }
  std::string code_name()const override{
    return "_f_ln1p";
  }
} p_ln1p;
DISPATCHER<FUNCTION>::INSTALL d_ln1p(&function_dispatcher, "ln1p|$ln1p", &p_ln1p);
/*--------------------------------------------------------------------------*/
class pow: public MGVAMS_FUNCTION {
public:
  explicit pow() : MGVAMS_FUNCTION(){
    set_label("pow");
  }
  ~pow() { }
  std::string code_name()const override{
    return "_f_pow";
  }
  void stack_op(Expression* e)const override {
    auto args = get_constant2(e);
    if(auto base = dynamic_cast<Float const*>(args.first)) {
      if(auto expof = dynamic_cast<Float const*>(args.second)) {
	subs_double2(e, std::pow(base->value(), expof->value()));
      }else if(auto expo = dynamic_cast<Integer const*>(args.second)) {
	subs_double2(e, std::pow(base->value(), expo->value()));
      }else{ untested();
	throw Exception("invalid");
      }
    }else if(auto basei = dynamic_cast<Integer const*>(args.first)) {
      if(auto expof = dynamic_cast<Float const*>(args.second)) {
	subs_double2(e, std::pow(basei->value(), expof->value()));
      }else if(auto expo = dynamic_cast<Integer const*>(args.second)) {
	// TODO: deal with exponent sign etc.
	subs_double2(e, std::pow(basei->value(), expo->value()));
      }else{ untested();
	throw Exception("invalid");
      }
    }else{
      throw Exception("invalid");
    }
  }
  void make_cc_common(std::ostream& o)const override {
    o__ "template<class T, class S>\n";
    o__ "typename va::ddouble_if<T, S>::type " << code_name() << "(T b, S e)const {\n";
    o____ "assert(b==b);\n";
    o____ "assert(e==e);\n";
    o____ "double p;\n";
    o____ "if(double(b) != 0.){\n";
    o____ "  p = std::pow(b, e);\n";
    o____ "  chain(b, double(e)/double(b)*p);\n";
    o____ "}else{\n";
    o____ "  p = 0;\n";
    o____ "  chain(b, 0.);\n";
    o____ "}\n";
    o____ "\n";
    o____ "if(double(b) > 0.){\n";
    o____ "  double l = std::log(b);\n";
    o____ "  assert(l==l);\n";
    o____ "  chain(e, l*p);\n";
    o____ "  assert(e==e);\n";
    o____ "}else{\n";
    o____ "  // unreachable(); // numerical nonsense, sometimes\n";
    o____ "  chain(e, 0.);\n";
    o____ "}\n";
    o____ "\n";
    o____ "::set_value(b, p);\n";
    o____ "::set_value(e, 0.);\n";
    o____ "typedef typename va::ddouble_if<T, S>::type ret_t;\n";
    o____ "ret_t ret(b);\n";
//    o____ "ret = b;\n";
    o____ "ret += ret_t(e);\n";
    o____ "assert(b==b);\n";
    o____ "return ret;\n";
    o__ "}\n";
  }
} p_pow;
DISPATCHER<FUNCTION>::INSTALL d_pow(&function_dispatcher, "pow|$pow", &p_pow);
/*--------------------------------------------------------------------------*/
class sin : public MGVAMS_FUNCTION {
public:
  explicit sin() : MGVAMS_FUNCTION(){
    set_label("sin");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::sin(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {\n";
    o____ "chain(d, std::cos(d));\n";
    o____ "return ::set_value(d, std::sin(d));\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_sin";
  }
} p_sin;
DISPATCHER<FUNCTION>::INSTALL d_sin(&function_dispatcher, "sin|$sin", &p_sin);
/*--------------------------------------------------------------------------*/
class sinh : public MGVAMS_FUNCTION {
public:
  explicit sinh() : MGVAMS_FUNCTION(){
    set_label("sinh");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::sinh(x));
  }
  void make_cc_common(std::ostream& o)const override{ untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const { untested();\n";
    o____ "chain(d, std::cosh(d));\n";
    o____ "::set_value(d, std::sinh(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{ untested();
    return "_f_sinh";
  }
} p_sinh;
DISPATCHER<FUNCTION>::INSTALL d_sinh(&function_dispatcher, "sinh|$sinh", &p_sinh);
/*--------------------------------------------------------------------------*/
class sqrt : public MGVAMS_FUNCTION {
public:
  explicit sqrt() : MGVAMS_FUNCTION(){
    set_label("sqrt");
  }
  void stack_op(Expression* e)const override {
    double x = get_double(e);
    subs_double(e, std::sqrt(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const {itested();\n";
    o______ "if(double(d)>0.){ itested();\n";
    o________ "double s = std::sqrt(d);\n";
    o________ "::set_value(d, s);\n";
    o________ "chain(d, .5/s);\n";
    o______ "}else if(d==0){\n";
    o________ "chain(d, 1e99);\n"; // problem? need inf..
    o________ "::set_value(d, 0.);\n";
    o______ "}else{\n";
    o________ "// numerical bug in model...\n";
    o________ "chain(d, inf);\n";
    o________ "::set_value(d, 0.);\n";
    o______ "}\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_sqrt";
  }
} p_sqrt;
DISPATCHER<FUNCTION>::INSTALL d_sqrt(&function_dispatcher, "sqrt|$sqrt", &p_sqrt);
/*--------------------------------------------------------------------------*/
class tan : public MGVAMS_FUNCTION {
public:
  tan() : MGVAMS_FUNCTION(){
    set_label("tan");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::tan(x));
  }
  void make_cc_common(std::ostream& o)const override{ untested();
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "double cd = cos(d);\n";
    o____ "chain(d, 1./(cd*cd));\n";
    o____ "::set_value(d, std::tan(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{ untested();
    return "_f_tan";
  }
} p_tan;
DISPATCHER<FUNCTION>::INSTALL d_tan(&function_dispatcher, "tan|$tan", &p_tan);
/*--------------------------------------------------------------------------*/
class tanh : public MGVAMS_FUNCTION {
public:
  explicit tanh() : MGVAMS_FUNCTION(){
    set_label("tanh");
  }
  void stack_op(Expression* e)const override { untested();
    double x = get_double(e);
    subs_double(e, std::tanh(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "double t = std::tanh(d);\n";
    o____ "::set_value(d, t);\n";
    o____ "chain(d, 1. - t*t);\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_tanh";
  }
} p_tanh;
DISPATCHER<FUNCTION>::INSTALL d_tanh(&function_dispatcher, "tanh|$tanh", &p_tanh);
/*--------------------------------------------------------------------------*/
#endif
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
