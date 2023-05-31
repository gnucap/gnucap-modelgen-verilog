/*                                      -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
 */
/*--------------------------------------------------------------------------*/
#include "mg_out.h"
#include "mg_func.h"
#include "m_tokens.h"
#include <globals.h>
#include <stack>
/*--------------------------------------------------------------------------*/
// stub...
static MGVAMS_FUNCTION const* lookup_function(std::string const& n, int& arity)
{
  FUNCTION const* f = function_dispatcher[n];
  if (n == "exp"
   || n == "cos"
   || n == "sin") {
    arity = 1;
    return 0;
  }else if (n == "pow"){
    arity = 2;
    return 0;
  }else if(auto g=dynamic_cast<MGVAMS_FUNCTION const*>(f)) {
    arity = g->arity();
    return g;
  }else{ untested();
    return 0;
  }

}
/*--------------------------------------------------------------------------*/
class RPN_VARS {
  typedef enum{
    t_flt,
    // t_int,
    t_str
  } type;
  std::stack<type> _types;
  std::stack<int> _args;
  int _idx_alloc{0};

  int _flt_idx{-1};
  int _flt_alloc{0};
  int _str_idx{-1};
  int _str_alloc{0};
public:
  ~RPN_VARS(){
    assert(_flt_idx == -1);
    assert(_str_idx == -1);
  }
  void pop() {
    assert(!_types.empty());
    if(_types.top()==t_flt){
      assert(_flt_idx>-1);
      --_flt_idx;
    }else{
      assert(_str_idx>-1);
      --_str_idx;
    }
    _types.pop();
  }
  void new_string(std::ostream& o){
    ++_str_idx;
    if(_str_idx < _str_alloc){
    }else{
      assert(_str_idx==_str_alloc);
      ++_str_alloc;
      o << ind << "std::string s" << _str_idx << ";\n";
    }
    _types.push(t_str);
  }
  void new_float(std::ostream& o){
    ++_flt_idx;
    if(_flt_idx < _flt_alloc){
    }else{
      assert(_flt_idx==_flt_alloc);
      ++_flt_alloc;
      o << ind << "ddouble t" << _flt_idx << ";\n";
    }
    _types.push(t_flt);
  }
  void stop(){
    _args.push(int(_types.size())-1);
  }
  bool have_args()const{
    return !_args.empty();
  }
  int num_args() const{
    assert(!_args.empty());
    return int(_types.size()) - 1 - _args.top();
  }
  void args_pop(){
    assert(!_args.empty());
    _args.pop();
  }
  std::string code_name() const{
    if(_types.top() == t_flt){
      return "t" + std::to_string(_flt_idx);
    }else{
      return "s" + std::to_string(_str_idx);
    }
  }
};
/*--------------------------------------------------------------------------*/
void make_cc_expression(std::ostream& o, Expression const& e)
{
  typedef Expression::const_iterator const_iterator;

  RPN_VARS s;
  // The _list is the expression in RPN.
  // print a program that computes the function and the derivatives.
  for (const_iterator i = e.begin(); i != e.end(); ++i) {

    if (auto var = dynamic_cast<const Token_VAR_REF*>(*i)) {
      s.new_float(o);

      std::string prefix;
      if((*var)->is_module_variable()){
	prefix = "d->_v_";
      }else{
	prefix = "_v_";
      }
      o__ s.code_name() << " = " << prefix << (*i)->name() << ".value();\n";
      for(auto v : (*var)->deps()) {
	o__ s.code_name() << "[d" << v->code_name() << "] = " << prefix << (*i)->name() << "[d" << v->code_name() << "];\n";
      }
    }else if (auto f = dynamic_cast<const Token_FILTER*>(*i)) {
      o__ s.code_name() << " = " << (*f)->code_name() << "(" << s.code_name() << ", d);\n";
      for(auto v : (*f)->deps()) {
	o__ "// dep :" << v->code_name() << "\n";
//	o__ s.code_name() << "[d" << v->code_name() << "] = _v_" << (*i)->name() << "[d" << v->code_name() << "];\n";
      }
    }else if (dynamic_cast<const Token_PAR_REF*>(*i)) {
      s.new_float(o);
      o << ind << s.code_name() << " = pc->_p_" << (*i)->name() << ";\n";
    }else if (dynamic_cast<const Token_CONSTANT*>(*i)) {
      if(dynamic_cast<Float const*>((*i)->data())){
	s.new_float(o);
      }else{ untested();
	s.new_string(o);
      }
      o << ind << s.code_name() << " = " << (*i)->name() << ";\n";
    }else if(auto pp = dynamic_cast<const Token_PROBE*>(*i)) {
      s.new_float(o);
      char sign = (*pp)->is_reversed()?'-':'+';

      o__ s.code_name() << " = "<<sign<<" p->" << (*pp)->code_name() << ";// "<< pp->name() <<"\n";
      o__ s.code_name() << "[d" << (*pp)->code_name() << "] = " << sign << "1.;\n";
    }else if(/*parlist && ??*/dynamic_cast<const Token_SYMBOL*>(*i)) {
      // incomplete(); // TODO
      assert(s.have_args());
      int arity=0;
      MGVAMS_FUNCTION const* f = lookup_function((*i)->name(), arity);
      if(s.num_args() == 1) {
// 	assert(arity==1); $vt
	if(f && f->code_name()!=""){
	  o << ind << s.code_name() << " = " << f->code_name();
	}else{
	  o << ind << s.code_name() << " = va::" << (*i)->name();
	}
	o << "(" << s.code_name() << ");\n";
      }else if(s.num_args() == 2){
	std::string idy = s.code_name();
	s.pop();
	o << ind << s.code_name() << " = va::" << (*i)->name();
	o << "(" << s.code_name() << ", " << idy << ");\n";
      }else if(f && !s.num_args()) {
	s.new_float(o);
	o << ind << s.code_name() << " = " << f->code_name() << "();\n";
      }else if(s.num_args() == 3) {
	std::string idz = s.code_name();
	s.pop();
	std::string idy = s.code_name();
	s.pop();
	o << ind << s.code_name() << " = va::" << (*i)->name();
	o << "(" << s.code_name() << ", " << idy << ", " << idz << ");\n";

      }else{ untested();
	unreachable();
	std::cerr << "run time error in make_cc_expression: " << (*i)->name() << ": " << s.num_args() << "\n";
      }
      s.args_pop();
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) {
    }else if (dynamic_cast<const Token_STOP*>(*i)) {
      s.stop();
    }else if (dynamic_cast<const Token_BINOP*>(*i)) {
      std::string idy = s.code_name();
      s.pop();
      assert((*i)->name().size());

      auto op = (*i)->name()[0];
      if ( op == '-'
	|| op == '+'
	|| op == '*'
	|| op == '/') {
	o__ s.code_name() << " "<< op << "= " << idy << ";\n";
      }else if( op == '>'
	     || op == '<'
	     || op == '=' ) {
	o__ s.code_name() << " = " << s.code_name() << " " << (*i)->name() << " " << idy << ";\n";
      }else{ untested();
	std::cerr << "run time error in make_cc_expression: " << (*i)->name() << "\n";
	incomplete();
	unreachable();
      }
    }else if (dynamic_cast<const Token_UNARY*>(*i)) {
      auto op = (*i)->name()[0];
      if(op == '-') {
	o__ s.code_name() << " *= -1.;\n";
      }else{ untested();
	incomplete();
	unreachable();
      }
    }else if (auto t = dynamic_cast<const Token_TERNARY*>(*i)) { untested();
      o__ "{\n";
      { untested();
	indent y;
	o__ "ddouble& tt0 = t0;\n"; // BUG: float??
	o__ "if(" << s.code_name() << "){\n";
	{ untested();
	  indent x;
	  make_cc_expression(o, *t->true_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}else{\n";
	{ untested();
	  indent x;
	  make_cc_expression(o, *t->false_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}\n";
      }
      o__ "}\n";
    }else{ untested();
      incomplete();
      unreachable();
      assert(false);
    }
  }
  s.pop();
}
/*--------------------------------------------------------------------------*/
void make_cc_event_cond(std::ostream& o, Expression const& e)
{
  typedef Expression::const_iterator const_iterator;
  // TODO: var stack.
  o__ "bool evt = false\n;";
  o__ "{\n";
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    if((*i)->name()=="initial_step"){
      o__ "evt = _sim->is_initial_step();\n";
    }else{
      incomplete();
      o << "--> " << (*i)->name() << " <--\n";
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
