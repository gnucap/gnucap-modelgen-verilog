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
class RDIST : public MGVAMS_TASK {
private:
  std::string _what;
public:
  explicit RDIST(std::string const& what) : MGVAMS_TASK(), _what(what) {
    set_label("$rdist_"+what);
  }
protected:
  RDIST(RDIST const& p) : MGVAMS_TASK(p), _what(p._what) {}
  MGVAMS_TASK* clone()const override {return new RDIST(*this);}
private:
  bool is_common()const override {return true;} // extra CARD*
  bool has_modes()const override {return true;}
  bool has_tr_begin()const override {return true;}
  bool has_tr_review()const override {return true;}
  bool has_tr_accept()const override {return true;}
  bool has_tr_advance()const override {return true;}
  bool returns_void()const override { return false; }
private:
  std::string eval(CS&, const PARAM_LIST*)const override{ untested();
    unreachable(); // SFCALL won't eval
    return "$$rdist_"+_what;
  }
  Token* new_token(Module&, size_t)const override {
   return nullptr; // leave it to m;
  }
  void args(std::ostream& o, bool names=true)const {
    if(names){
      o << "int32_t seed, double const& l, double const& u";
    }else{
      o << "int32_t, double const&, double const&";
    }
  }
  void make_cc_dev(std::ostream& o)const override {
    o__ "struct cls" << label() << "{\n";
    o____ "double _value{0};\n";
    o____ "int32_t _seed{0};\n";

    o__ "double precalc(CARD*, int32_t& seed, double l, double u) {\n";
    o____ "trace3(\"random::precalc\", CKT_BASE::_sim->_time0, seed, _value);\n";
    o____ "(void)seed; (void)l; (void)u;\n";
    o____ "return _value;\n";
    o__ "}\n";

    o__ "double precalc(CARD*, int32_t const& seed, double l, double u) {\n";
    o____ "trace3(\"random::precalc1\", CKT_BASE::_sim->_time0, seed, _value);\n";
    o____ "(void)seed; (void)l; (void)u;\n";
    o____ "return _value;\n";
    o__ "}\n";

    o__ "double tr_initial(CARD *d, int32_t& s, double l, double u) {\n";
    o____ "return tr_begin(d, s, l, u);\n";
    o__ "}\n";
    o__ "double tr_initial(CARD *d, int32_t const& s, double l, double u) {untested();\n";
    o____ "return tr_begin(d, s, l, u);\n";
    o__ "}\n";

    o__ "double tr_begin(CARD*, int32_t& seed, double l, double u) {\n";
    o____ "trace2(\"random::begin01\", seed, _value);\n";
    o____ "_value = rdist::"+_what+"(seed, l, u);\n";
    o____ "trace2(\"random::begin02\", seed, _value);\n";
    o____ "return _value;\n";
    o__ "}\n";

    o__ "double tr_begin(CARD*, int32_t const& slot, double l, double u) {\n";
    o____ "trace2(\"random::begin1\", slot, _value);\n";
    o____ "_value = rdist::"+_what+"(random_seed(slot), l, u);\n";
    o____ "return _value;\n";
    o__ "}\n";

    o << "    double tr_eval(CARD*, int32_t const& seed, double l, double u) {\n";
    o << "      trace3(\"random::eval\", _seed, seed, _value);\n";
    o << "      (void)seed; (void)l; (void)u;\n";
    o << "      return _value;\n";
    o << "    }\n";


    o << "    double tr_review(CARD*, "; args(o); o << ") {untested();\n";
    o << "      (void)seed; (void)l; (void)u;\n";
    o << "      trace2(\"random::review\", seed, _value);\n";
    o << "      return _value;\n";
    o << "    }\n";

    o << "    double tr_advance(MOD* d, int32_t& seed, double l, double u) {\n";
    o << "      trace3(\"random::advance1\", CKT_BASE::_sim->_time0, seed, _value);\n";
    o << "      d->q_accept();\n";
    o << "      int32_t s = seed;\n";
    o << "      _value = rdist::"+_what+"(s, l, u);\n";
    o << "      seed = s;\n";
    o << "      return _value;\n";
    o << "    }\n";

    o << "    double tr_advance(MOD* d, int32_t const& slot, double l, double u) {\n";
    o << "      trace4(\"random::advance2\", CKT_BASE::_sim->_time0, slot, _value, OPT::foooo);\n";
    o << "      int32_t& s = random_seed(slot);\n";
    o << "      tr_advance(d, s, l, u);\n";
    o << "      return _value;\n";
    o << "    }\n";

    o << "    double tr_regress(CARD*, "; args(o); o << ") {untested();\n";
    o << "      (void)seed; (void)l; (void)u;\n";
    o << "      incomplete();\n";
    o << "      return _value;\n";
    o << "    }\n";

    o << "    double tr_accept(CARD*, int32_t const& seed, double l, double u) {\n";
    o << "      (void)seed; (void)l; (void)u;\n";
    o << "      return _value;\n";
    o << "    }\n";

    o << "    double tr_accept(CARD*, int32_t& seed, double l, double u) {\n";
    o << "      trace3(\"random::accept\", CKT_BASE::_sim->_time0, seed, _value);\n";
    o << "      double new_value = rdist::"+_what+"(seed, l, u);\n";
    o << "      // assert(_value == new_value);\n";
    o << "      return _value = new_value;\n";
    o << "    }\n";

    o__ "}_" << label() << ";\n";
  }
  std::string code_name()const override{
    return "d->_" + label() + ".";
  }
};
/*--------------------------------------------------------------------------*/
RDIST d0("uniform");
DISPATCHER<FUNCTION>::INSTALL p0(&function_dispatcher, "$rdist_uniform", &d0);
RDIST d1("normal");
DISPATCHER<FUNCTION>::INSTALL p1(&function_dispatcher, "$rdist_normal", &d1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
