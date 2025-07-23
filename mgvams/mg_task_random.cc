/*                        -*- C++ -*-
 * Copyright (C) 2025 Felix Salfelder
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
 * Verilog-AMS random functions
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_.h"
#include "mg_out.cc"
#include "mg_token.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class RDIST : public MGVAMS_FUNCTION {
private:
  std::string _what;
  std::string _args;
public:
  explicit RDIST(std::string const& what, std::string const& args)
    : MGVAMS_FUNCTION(), _what(what), _args(args) {
    set_label("$rdist_"+what);
  }
protected:
  RDIST(RDIST const& p) : MGVAMS_FUNCTION(p), _what(p._what), _args(p._args) {}
  MGVAMS_FUNCTION* clone()const override {return new RDIST(*this);}
private:
  bool is_common()const override {return false;} // extra CARD* arg
  bool needs_context()const override {return true;}
  bool has_precalc()const override {return false;}
  bool has_modes()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool has_tr_review()const override {return true;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_advance()const override {return true;}
  bool returns_void()const override { return false; }
  bool static_code()const override {return false;}
private:
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
  Token* new_token(Module&, size_t)const override {
   return nullptr; // leave it to m;
  }
  void argnames(std::ostream& o)const {
    int i = 0;
    for(char _ : _args){
      o << ", a" << i++;
    }
  }
  void voidargs(std::ostream& o)const {
    int i = 0;
    for(char _ : _args){
      o << " (void) a" << i++ << ";";
    }
  }
  void args(std::ostream& o, bool names=true)const {
    int i = 0;
    for(char a : _args){
      switch(a){
      case 'i': o << ", int"; break;
      case 'd': o << ", double"; break;
      default: unreachable();
      }
      o << " const&";
      if(names) {
	o << " a" << i++;
      }else{
      }
    }
  }
  void make_cc_common(std::ostream&o)const override {
    o__ "struct cls" << label() << "{\n";
    o__ "  double _value{0};\n";

    o____ "double precalc(int32_t&"; args(o); o << ")const {\n";
    voidargs(o);
    o______ "unreachable(); return 0.;\n";
    o____ "}\n";
    o____ "double precalc(int32_t const& slot"; args(o); o << ")const {\n";
    voidargs(o);
    o______ "return rdist::"+_what+"(random_seed(slot)"; argnames(o); o << ");\n";
    o____ "}\n";
    o__ "}_" << label() << ";\n";
  }
  void make_cc_impl(std::ostream&)const override {
    // o____ "double COMMON::cls" << label() << "::tr_advance(MOD* d, int32_t& seed"; args(o); o << ") {\n";
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "struct cls" << label() << "{\n";
    o____ "double _value{0};\n";

    o____ "double precalc(CARD*, int32_t const& seed"; args(o); o << "){\n";
    o______ "trace3(\"random::precalc1\", CKT_BASE::_sim->_time0, seed, _value);\n";
    o______ "(void)seed;\n"; voidargs(o);
    o______ "return _value;\n";
    o____ "}\n";

    o____ "double tr_initial(CARD *d, int32_t& s"; args(o); o << ") {\n";
    o______ "return tr_begin(d, s"; argnames(o); o << ");\n";
    o____ "}\n";
    o____ "double tr_initial(CARD *d, int32_t const& s"; args(o); o << ") {untested();\n";
    o______ "return tr_begin(d, s"; argnames(o); o << ");\n";
    o____ "}\n";

    o____ "double tr_begin(CARD*, int32_t& seed"; args(o); o << ") {\n";
    o______ "trace2(\"random::begin01\", seed, _value);\n";
    o______ "_value = rdist::"+_what+"(seed"; argnames(o); o << ");\n";
    o______ "trace2(\"random::begin02\", seed, _value);\n";
    o______ "return _value;\n";
    o____ "}\n";

    o____ "double tr_begin(CARD*, int32_t const& slot"; args(o); o << ") {\n";
    o______ "trace2(\"random::begin1\", slot, _value);\n";
    o______ "_value = rdist::"+_what+"(random_seed(slot)"; argnames(o); o << ");\n";
    o______ "return _value;\n";
    o____ "}\n";

    o____ "double tr_eval(CARD*, int32_t const& seed"; args(o); o << ") {\n";
    o______ "trace2(\"random::eval\", seed, _value);\n";
    o______ "(void)seed;\n"; voidargs(o);
    o______ "return _value;\n";
    o____ "}\n";


    o____ "double tr_review(CARD*, int32_t const& seed"; args(o); o << ") {untested();\n";
    o______ "(void)seed;\n"; voidargs(o);
    o______ "trace2(\"random::review\", seed, _value);\n";
    o______ "return _value;\n";
    o____ "}\n";

    o____ "template<class MOD>\n";
    o____ "double tr_advance(MOD* d, int32_t& seed"; args(o); o << ") {\n";
    o______ "trace3(\"random::advance1\", CKT_BASE::_sim->_time0, seed, _value);\n";
    o______ "d->q_accept();\n";
    o______ "int32_t s = seed;\n";
    o______ "_value = rdist::"+_what+"(s"; argnames(o); o << ");\n";
    o______ "seed = s;\n";
    o______ "return _value;\n";
    o____ "}\n";

    o____ "template<class MOD>\n";
    o____ "double tr_advance(MOD* d, int32_t const& slot"; args(o); o << ") {\n";
    o______ "trace4(\"random::advance2\", CKT_BASE::_sim->_time0, slot, _value, OPT::foooo);\n";
    o______ "int32_t& s = random_seed(slot);\n";
    o______ "tr_advance(d, s"; argnames(o); o << ");\n";
    o______ "return _value;\n";
    o____ "}\n";

    o____ "double tr_regress(CARD*,int32_t const& seed"; args(o); o << ") {untested();\n";
    o______ "(void)seed;\n"; voidargs(o);
    o______ "incomplete();\n";
    o______ "return _value;\n";
    o____ "}\n";

    o____ "double tr_accept(CARD*, int32_t const& seed"; args(o); o << ") {\n";
    o______ "(void)seed;\n"; voidargs(o);
    o______ "return _value;\n";
    o____ "}\n";

    o____ "double tr_accept(CARD*, int32_t& seed"; args(o); o << ") {\n";
    o______ "trace3(\"random::accept\", CKT_BASE::_sim->_time0, seed, _value);\n";
    o______ "double new_value = rdist::"+_what+"(seed"; argnames(o); o << ");\n";
    o______ "// assert(_value == new_value);\n";
    o______ "return _value = new_value;\n";
    o____ "}\n";

    o__ "}_" << label() << ";\n";
  }
  std::string code_name()const override{
    return "_" + label() + ".";
    return "d->_" + label() + ".";
  }
};
/*--------------------------------------------------------------------------*/
RDIST d0("uniform", "dd");
DISPATCHER<FUNCTION>::INSTALL p0(&function_dispatcher, "$rdist_uniform", &d0);
RDIST d1("normal", "dd");
DISPATCHER<FUNCTION>::INSTALL p1(&function_dispatcher, "$rdist_normal", &d1);
RDIST d2("exponential", "d");
DISPATCHER<FUNCTION>::INSTALL p2(&function_dispatcher, "$rdist_exponential", &d2);
RDIST d3("poisson", "d");
DISPATCHER<FUNCTION>::INSTALL p3(&function_dispatcher, "$rdist_poisson", &d3);
RDIST d4("chi_square", "i");
DISPATCHER<FUNCTION>::INSTALL p4(&function_dispatcher, "$rdist_chi_square", &d4);
RDIST d5("t", "i");
DISPATCHER<FUNCTION>::INSTALL p5(&function_dispatcher, "$rdist_t", &d5);
RDIST d6("erlangian", "dd");
DISPATCHER<FUNCTION>::INSTALL p6(&function_dispatcher, "$rdist_erlangian", &d6);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
