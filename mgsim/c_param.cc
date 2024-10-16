/*$Id: c_param.cc,v 26.130 2009/11/15 21:51:59 al Exp $ -*- C++ -*-
 * Copyright (C) 2005 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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
 */
//testing=script,complete 2006.07.17
#include <c_comand.h>
#include <u_parameter.h>
#include <globals.h>
#include <d_dot.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
static const std::string IS_VALID = "_..is_valid";
/*--------------------------------------------------------------------------*/
class PARAM_ANY : public PARA_BASE {
  Base* _value{nullptr};
public:
  explicit PARAM_ANY() : PARA_BASE () {}
  explicit PARAM_ANY(PARAM_ANY const&p) : PARA_BASE (p) {
    if(p._value){
      _value = p._value->assign(p._value);
    }else{
    }
  }
  ~PARAM_ANY() {delete _value;}
  PARA_BASE* clone()const override{ untested();return new PARAM_ANY(*this);}
  PARA_BASE* pclone(void*p)const override{return new(p) PARAM_ANY(*this);}
  bool operator==(const PARA_BASE& v)const { untested();
    // PARAMETER const* p = dynamic_cast<PARAMETER const*>(&b);
    // return (p && _v == p->_v  &&  _s == p->_s);
    Base* eq = nullptr;
   // if(_s != v._s){ untested();
   //   return false;
   // }else
    if(auto f = dynamic_cast<Float const*>(v.value())){ untested();
      eq = f->equal(_value);
    }else if(auto i = dynamic_cast<Integer const*>(v.value())){ untested();
      eq = i->equal(_value);
    }else{ untested();
      // incomplete();
    }

    bool ret = false;
    if(auto ii=dynamic_cast<Integer const*>(eq)){ untested();
      ret = ii->value();
    }else{ untested();
    }
    delete eq;

    if(ret){ untested();
    }else{ untested();
    }
    return ret;
  }

  void parse(CS& cmd) override {
    std::string name;
    //cmd >> name;
    name = cmd.ctos(",=();", "'{\"", "'}\"");
    if (cmd) {
      if (cmd.match1('(')) { untested();
	_s = name + '(' + cmd.ctos("", "(", ")") + ')';
      }else{
	_s = name;
      }
    }else{ untested();
    }
  }
  PARA_BASE& operator=(const std::string& s) override{
    _s = s;
    return *this;
  }
  PARA_BASE& operator=(Base const* v)override {
    delete _value;
    if(dynamic_cast<Float const*>(v)){
      // assert(dynamic_cast<vReal const*>(v));
      vReal n;
      _value = n.assign(v);
      trace3("now real", _s, v->val_string(), _value->val_string());
    }else if(dynamic_cast<Integer const*>(v)){ untested();
      vInteger n;
      _value = n.assign(v);
      trace3("now integer", _s, v->val_string(), _value->val_string());
    }else{ untested();
      _value = v->assign(v);
      assert(_value);
      trace3("don't know", _s, v->val_string(), _value->val_string());
    }
    _s = "#";
    return *this;
  }
  std::string string()const override {
    if (_s == "#") {
      if (_value) {
	return _value->val_string();
      }else{ untested();
	return "";
      }
   // }else if (_s == "") { untested();
   //   return "NA(" + _value->val_string() + ")";
    }else{
      return _s;
    }
  }

  Base const* value()const override {
    if(_value){
    }else{
    }
    return _value;
  }
  bool has_good_value()const override { untested();unreachable(); return false;}
  Base const* e_val_(const Base*, const CARD_LIST*, int)const { untested();unreachable(); return nullptr;}
}param_any;
/*--------------------------------------------------------------------------*/
class CMD_PARAM : public CMD {
public:
  void do_it(CS& cmd, CARD_LIST* Scope)override {
    PARAM_LIST* pl = Scope->params();
    assert(pl);
    if (cmd.is_end()) { untested();
      pl->print(IO::mstdout, OPT::language);
      IO::mstdout << '\n';
    }else{
      trace1("parameterparse", cmd.tail());
      parse(cmd, Scope);
      DEV_DOT* dd = new DEV_DOT();
      assert(dd);
      dd->set(cmd.fullstring());
      Scope->push_back(dd);
    }
  }

  void parse(CS& cmd, CARD_LIST* Scope);

  void parse_range(CS& cmd, CARD_LIST* Scope, std::string Name) {
    PARAM_LIST* pl = Scope->params();
    {
      std::string range_expr = "1";
      std::string range_type;
      size_t here = cmd.cursor();

      for(;;){
	if(cmd >> "from "){
	  range_expr = range_expr + "*(";
	}else if(cmd>> "exclude ") { untested();
	  range_expr = range_expr + "*(1-";
	}else{
	  break;
	}

	range_type = cmd.trimmed_last_match();
	std::string what;
	std::string lb, ub, uo, lo;
	bool range = true;
	if(cmd.skip1('[')){
	  lo = "<=";
	}else if(cmd.skip1('(')){
	  lo = "<";
	}else{ untested();
	  range = false;
	}

	if(range){
	  {
	    Expression L;
	    trace1("lb?", cmd.tail());
	    cmd >> L;
	    // lb = cmd.ctos(":");
	    Expression LL(L, Scope); // reduce_copy.
	    Base const* lb_ = LL.value();
	    assert(lb_);
	    if(!lb_){ untested();
	      incomplete();
	    }else if(auto ii = dynamic_cast<Integer const*>(lb_)){
	      incomplete();
	      lb = to_string(ii->value());
	    }else if(auto ff = dynamic_cast<Float const*>(lb_)){
	      incomplete();
	      lb = to_string(ff->value());
	    }else{
	      incomplete();
	    }
	  }

	  cmd.skip1(":");
//	  ub = cmd.ctos("])");
	  {
	    Expression L;
	    cmd >> L;
	    // lb = cmd.ctos(":");
	    Expression LL(L, Scope); // reduce_copy.
	    Base const* ub_ = LL.value();
	    if(!ub_){ untested();
	      incomplete();
	    }else if(auto ii = dynamic_cast<Integer const*>(ub_)){
	      ub = to_string(ii->value());
	    }else if(auto ff = dynamic_cast<Float const*>(ub_)){
	      ub = to_string(ff->value());
	    }else{
	      incomplete();
	    }
	  }
	  trace2("bounds", lb, ub);

	  if(cmd.skip1(']')){
	    uo = "<=";
	  }else if(cmd.skip1(')')){
	    uo = "<";
	  }else{ untested();
	  }
	  what = "(" + lb + lo + Name + ")*(" + Name + uo + ub + ")";
	}else{ untested();
	  cmd >> what;
	  what = "(" + Name + "==" + what + ")";
	}
	range_expr = range_expr + what + ")";
	if(cmd.skip1(',')){ untested();
	  break;
	}else{
	}
      }
//      range_expr = range_expr + "}";

      if (cmd.stuck(&here)) {
	incomplete();
	trace2("c_param stuck", cmd.tail(), range_expr);
	return;
      }else{
	trace2("c_param more", cmd.tail(), range_expr);
      }

      _range = range_expr;

     //  if(Value.string() == "NA( NA)"){ untested();
     //    pl->set(Name, "");
     //  }else{ untested();
     //    pl->set(Name, Value.string());
     //  }
      // trace3("set check", Name, (*pl)[Name], (*pl)[Name].string());

      PARAM_INSTANCE v = pl->deep_lookup(IS_VALID);
      if(range_expr=="1"){ untested();
      }else if(v.has_hard_value()){ untested();
	range_expr = v.string() + "*" + range_expr;
	trace2("c_param hv", IS_VALID, range_expr);
	pl->set(IS_VALID, range_expr);
      }else{
	v = PARAMETER<vInteger>();
	trace2("c_param", IS_VALID, range_expr);
	pl->set(IS_VALID, range_expr);
      }
    }
    cmd.check(bDANGER, "syntax error");
  }
  std::string const& range() const { untested();return _range;}
private:
  std::string _range;
} p;
DISPATCHER<CMD>::INSTALL d(&command_dispatcher, "param|parameters|parameter", &p);
/*--------------------------------------------------------------------------*/
void CMD_PARAM::parse(CS& cmd, CARD_LIST* Scope)
{
  assert(Scope);
  PARAM_LIST* pl = Scope->params();
  assert(pl);
  int type = 0;
  size_t here = cmd.cursor();
  if(cmd >> "real"){
    type = 1;
  }else if(cmd >> "integer"){
    type = 2;
// TODO: realtime | time | string
  }else{
    cmd.reset(here);
  }
  here = cmd.cursor();
  for (;;) {
    if (!(cmd.more() && (cmd.is_alpha() || cmd.match1('_')))) {
      break;
    }else{
    }
    std::string Name;
    PARAM_INSTANCE par;
    switch(type){
    case 1:
      par = PARAMETER<vReal>();
      break;
    case 2:
      par = PARAMETER<vInteger>();
      break;
    default:
      par = param_any;
      // par = PARAMETER<vReal>();
      break;
    }

//     cmd >> Name >> '=' >> par;
    cmd >> Name;
    if(cmd >> "="){
      par.parse(cmd);
    }else{
      // Value = Name;
      trace2("no =", cmd.fullstring(), par.string());
    }


    trace1("parsed", par.string());
    if (cmd.stuck(&here)) {untested();
      break;
    }else{
    }
    if (OPT::case_insensitive) {
      notstd::to_lower(&Name);
    }else{
    }
    pl->set(Name, par);

    parse_range(cmd, Scope, Name);
  }
  cmd.check(bDANGER, "syntax error");
}
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
