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
#include "mg_out.h"
// #include "mg_func.cc"
// #include "mg_out.cc"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class STUB : public MGVAMS_FUNCTION {
public:
  explicit STUB(std::string const l) : MGVAMS_FUNCTION() {
    set_label(l);
  }
  ~STUB() { }
private:
  std::string code_name()const override{
    if(label()!=""){
      return "";
    }else{
      return "va::" + label();
    }
  }
  std::string eval(CS&, const CARD_LIST*)const override {
	  unreachable();
	  return "AAA";
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
};
/*--------------------------------------------------------------------------*/
// TODO: these are still in m_va.h
STUB acosh("acosh");
DISPATCHER<FUNCTION>::INSTALL d_acosh(&function_dispatcher, "acosh|$acosh", &acosh);
STUB min("min");
DISPATCHER<FUNCTION>::INSTALL d_min(&function_dispatcher, "min|$min", &min);
STUB max("max");
DISPATCHER<FUNCTION>::INSTALL d_max(&function_dispatcher, "max|$max", &max);
/*--------------------------------------------------------------------------*/
// TODO: implement small signal noise
STUB white_noise("white_noise");
DISPATCHER<FUNCTION>::INSTALL d_white_noise(&function_dispatcher, "white_noise", &white_noise);
STUB flicker_noise("flicker_noise");
DISPATCHER<FUNCTION>::INSTALL d_flicker_noise(&function_dispatcher, "flicker_noise", &flicker_noise);
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
  double evalf(double const* x)const override { untested();
    return std::abs(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const {itested();\n";
    o____ "if(d>=0.){itested();\n";
    o____ "}else{itested();\n";
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    unreachable();
    PARAMETER<double> x;
    Cmd >> x;
    trace1("asinh", x);
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::asinh(x));
  }
  double evalf(double const* x)const override { untested();
    return std::asinh(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o__ "double b = std::sqrt(1.+double(d)*double(d));\n";
    o____ "chain(d, 1./b);\n";
    o____ "::set_value(d, std::asinh(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    unreachable();
    PARAMETER<double> x;
    Cmd >> x;
    trace1("atan", x);
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::atan(x));
  }
  double evalf(double const* x)const override { untested();
    return std::atan(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    o____ "chain(d, 1./(1.+double(d)*double(d)));\n";
    o____ "::set_value(d, std::atan(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    unreachable();
    PARAMETER<double> x;
    Cmd >> x;
    trace1("atanh", x);
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::atanh(x));
  }
  double evalf(double const* x)const override { untested();
    return std::atanh(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d) const{\n";
    incomplete();
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_atanh";
  }
} p_atanh;
DISPATCHER<FUNCTION>::INSTALL d_atanh(&function_dispatcher, "atanh|$atanh", &p_atanh);
/*--------------------------------------------------------------------------*/
class cos : public MGVAMS_FUNCTION {
public:
  explicit cos() : MGVAMS_FUNCTION(){
    set_label("cos");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::cos(x));
  }
  double evalf(double const* x)const override { untested();
    return std::cos(*x);
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::cosh(x));
  }
  double evalf(double const* x)const override { untested();
    return std::cosh(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "chain(d, std::sinh(d));\n";
    o____ "return ::set_value(d, std::cosh(d));\n";
    o__ "};\n";
  }
  std::string code_name()const override{
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    unreachable();
    PARAMETER<double> x;
    Cmd >> x;
    trace1("exp", x);
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::exp(x));
  }
  double evalf(double const* x)const override {
    return std::exp(*x);
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
class limexp : public MGVAMS_FUNCTION {
public:
  explicit limexp() : MGVAMS_FUNCTION(){
    set_label("limexp");
  }
  ~limexp() { }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::exp(x));
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
class floor : public MGVAMS_FUNCTION {
public:
  explicit floor() : MGVAMS_FUNCTION(){
    set_label("$floor");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::floor(x));
  }
  std::string code_name()const override{
    return "_f_floor";
  }
  void make_cc_common(std::ostream& o)const override {
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { untested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::log10(x));
  }
  std::string code_name()const override{
    return "va::log10";
  }
  void make_cc_common(std::ostream& o)const override {
    o << "// dummy " << label() << "\n";
  }
} p_log;
DISPATCHER<FUNCTION>::INSTALL d_log(&function_dispatcher, "log|$log10", &p_log);
/*--------------------------------------------------------------------------*/
class ln : public MGVAMS_FUNCTION {
public:
  explicit ln() : MGVAMS_FUNCTION(){
    set_label("ln");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { untested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::log(x));
  }
  double evalf(double const* x)const override {
    return std::log(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "double l=-1e99;\n";
    o____ "if(d>1e-60){itested();\n";
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
    o____ "}\n";
  }
  std::string code_name()const override{
    return "_f_ln";
  }
} p_ln;
DISPATCHER<FUNCTION>::INSTALL d_ln(&function_dispatcher, "ln|$log", &p_ln);
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {untested();
    trace1("pow", Cmd.tail());
    PARAMETER<double> x, y;
    Cmd >> x >> y;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::pow(x, y));
  }
  double evalf(double const* x)const override {
    return std::pow(x[0], x[1]);
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::sin(x));
  }
  double evalf(double const* x)const override { untested();
    return std::sin(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const {itested();\n";
    o______ "chain(d, std::cos(d));\n";
    o______ "return ::set_value(d, std::sin(d));\n";
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::sinh(x));
  }
  double evalf(double const* x)const override { untested();
    return std::sinh(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const { untested();\n";
    o____ "chain(d, std::cosh(d));\n";
    o____ "::set_value(d, std::sinh(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
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
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::sqrt(x));
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o____ "T " << code_name() << "(T d)const {itested();\n";
    o______ "if(double(d)>1e-90){ itested();\n";
    o________ "double s = std::sqrt(d);\n";
    o________ "::set_value(d, s);\n";
    o________ "chain(d, .5/s);\n";
    o______ "}else if(d>0){ untested();\n";
    o________ "chain(d, 5e91);\n";
    o________ "::set_value(d, std::sqrt(d));\n";
    o______ "}else if(d==0){ itested();\n";
    o________ "chain(d, .5e200);\n";
    o________ "::set_value(d, 0.);\n";
    o______ "}else{\n";
    o________ "unreachable();\n";
    o________ "chain(d, .5e99);\n";
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
class tanh : public MGVAMS_FUNCTION {
public:
  explicit tanh() : MGVAMS_FUNCTION(){
    set_label("tanh");
  }
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override { itested();
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::tanh(x));
  }
  double evalf(double const* x)const override { untested();
    return std::tanh(*x);
  }
  void make_cc_common(std::ostream& o)const override{
    o__ "template<class T>\n";
    o__ "T " << code_name() << "(T d)const {itested();\n";
    o____ "double t = std::tanh(d);\n";
    o____ "::set_value(d, t);\n";
    o____ "chain(d, 1. - t*t);\n";
    o____ "assert(d.is_same(d));\n";
    o____ "return d;\n";
    o__ "}\n";
  }
  std::string code_name()const override{
    return "_f_tanh";
  }
} p_tanh;
DISPATCHER<FUNCTION>::INSTALL d_tanh(&function_dispatcher, "tanh|$tanh", &p_tanh);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
