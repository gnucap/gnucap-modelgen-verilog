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
static MGVAMS_FUNCTION const* lookup_function(std::string const& n)
{
  FUNCTION const* f = function_dispatcher[n];
  if (n == "exp"
   || n == "cos"
   || n == "sin") {
    return 0;
  }else if (n == "pow"){
    return 0;
  }else if(auto g=dynamic_cast<MGVAMS_FUNCTION const*>(f)) {
    return g;
  }else{
    return 0;
  }

}
/*--------------------------------------------------------------------------*/
class RPN_VARS {
  typedef enum{
    t_flt,
    // t_int,
    t_str,
    t_ref,
  } type;
  std::stack<type> _types;
  std::stack<int> _args;
  std::stack<std::string> _refs;
  int _idx_alloc{0};

  int _flt_idx{-1};
  int _flt_alloc{0};
  int _str_idx{-1};
  int _str_alloc{0};
public:
  ~RPN_VARS(){
    assert(_flt_idx == -1);
    assert(_str_idx == -1);
    assert(_refs.empty());
  }
  void pop() {
    assert(!_types.empty());
    switch(_types.top()){
    case t_flt:
      assert(_flt_idx>-1);
      --_flt_idx;
      break;
    case t_ref:
      _refs.pop();
      break;
    case t_str:
      assert(_str_idx>-1);
      --_str_idx;
      break;
    default:
      unreachable();
    }
    _types.pop();
  }
  void new_string(std::ostream& o){ itested();
    ++_str_idx;
    if(_str_idx < _str_alloc){ untested();
    }else{ itested();
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
  bool is_ref() const{
    assert(!_types.empty());
    return _types.top() == t_ref;
  }
  void new_ref(std::string name){
    _refs.push(name);
    _types.push(t_ref);
  }
  void stop(){
    _args.push(int(_types.size())-1);
  }
  bool have_args()const{
    return !_args.empty();
  }
  size_t num_args() const{
    assert(!_args.empty());
    return int(_types.size()) - 1 - _args.top();
  }
  void args_pop(){
    assert(!_args.empty());
    _args.pop();
  }
  size_t size() const{
    return _refs.size();
  }
  std::string code_name() const{
    switch(_types.top()) {
    case t_flt:
      return "t" + std::to_string(_flt_idx);
    case t_str:
      return "s" + std::to_string(_str_idx);
    case t_ref:
      return _refs.top();
    default:
      unreachable();
      return "";
    }
  }
};
/*--------------------------------------------------------------------------*/
void make_cc_string(std::ostream& o, String const& e)
{
  o << '"';
  for(char c : e.val_string()){itested();
    if(c=='\n'){ untested();
      o << '\\';
    }else{itested();
    }
    o << c;
  }
  o << '"';
}
/*--------------------------------------------------------------------------*/
void make_cc_expression(std::ostream& o, Expression const& e)
{
  typedef Expression::const_iterator const_iterator;

  RPN_VARS s;
//  s.new_ref("t0"); // HACK
//  o__ "ddouble t0; // top\n";
  // The _list is the expression in RPN.
  // print a program that computes the function and the derivatives.
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    trace2("mg_out_expr loop", (*i)->name(), s.size());

    if (auto var = dynamic_cast<const Token_VAR_REF*>(*i)) {
      std::string prefix;
      if((*var)->is_module_variable()){
	prefix = "d->_v_";
      }else{
	prefix = "_v_";
      }

// does gcc optimise them out?
//      s.new_float(o);
//       o__ s.code_name() << " = " << prefix << (*i)->name() << ".value();\n";
//       for(auto v : (*var)->deps()) { untested();
// 	o__ s.code_name() << "[d" << v->code_name() << "] = " << prefix << (*i)->name() << "[d" << v->code_name() << "];\n";
//       }
      s.new_ref((*var)->code_name());
    }else if (auto f = dynamic_cast<const Token_FILTER*>(*i)) {
      trace2("FILTER", s.code_name(), s.num_args());

      std::vector<std::string> argnames(s.num_args());
      if(s.num_args()==1){
	o__ s.code_name() << " = " << (*f)->code_name() << "(" << s.code_name() << ", d);\n";
	for(auto v : (*f)->deps()) {
	  o__ "// dep :" << v->code_name() << "\n";
  //	o__ s.code_name() << "[d" << v->code_name() << "] = _v_" << (*i)->name() << "[d" << v->code_name() << "];\n";
	}
      }else{
	for(auto n=argnames.begin(); n!=argnames.end(); ++n){
	  *n = s.code_name();
	  s.pop();
	}
	s.new_float(o);
	o__ s.code_name() << " = " << (*f)->code_name();
	// BUG: see SYMBOL
	o << "(";
       	std::string comma = "";
	for(size_t i=argnames.size(); i; --i){
	  o << comma << argnames[i-1];
	  comma = ", ";
	}
	o << ", d);\n";
      }

    }else if (dynamic_cast<const Token_PAR_REF*>(*i)) {
      s.new_ref("pc->_p_" + (*i)->name());
    }else if (dynamic_cast<const Token_CONSTANT*>(*i)) {
      if(auto ff=dynamic_cast<Float const*>((*i)->data())){
#if 1
	// s.new_ref((*i)->name());
	s.new_ref(ftos(ff->value(), 0, 20, ftos_EXP));
#else
	s.new_float(o);
	o << ind << s.code_name() << " = " << (*i)->name() << ";\n";
#endif
      }else if(auto S=dynamic_cast<String const*>((*i)->data())){
	s.new_string(o);
	o << ind << s.code_name() << " = ";
	make_cc_string(o, *S);
	o << ";\n";
      }else{untested();
	unreachable();
	s.new_string(o);
	o << ind << s.code_name() << " = " << (*i)->name() << "; (u)\n";
      }
    }else if(auto pp = dynamic_cast<const Token_PROBE*>(*i)) {
      s.new_float(o);
      assert((*pp)->branch());
      if((*pp)->branch()->is_short()){ untested();
	o__ s.code_name() << " = 0.; // short probe\n";
      }else{
	char sign = (*pp)->is_reversed()?'-':'+';
	o__ s.code_name() << " = " << sign << "p->" << (*pp)->code_name() << "; // "<< pp->name() <<"\n";
	o__ s.code_name() << "[d" << (*pp)->code_name() << "] = " << sign << "1.;\n";
      }
    }else if(/*parlist && ??*/dynamic_cast<const Token_SYMBOL*>(*i)) {
      trace1("Symbol", (*i)->name());
      assert(s.have_args());
      MGVAMS_FUNCTION const* f = lookup_function((*i)->name());
      std::vector<std::string> argnames(s.num_args());
      assert(s.num_args() == argnames.size());
      for(auto n=argnames.begin(); n!=argnames.end(); ++n){
	*n = s.code_name();
	s.pop();
      }

      if(f && !argnames.size()){
	s.new_float(o);
	o << ind << s.code_name() << " = " << f->code_name() << "();\n";
      }else if(argnames.size()) {
	s.new_float(o);
	// o << ind << s.code_name() << " = va::" << (*i)->name();
	if(f && f->code_name()!=""){
	  o << ind << s.code_name() << " = " << f->code_name();
	}else if(auto af = dynamic_cast<Token_AFCALL const*>(*i)){
	  o << ind << s.code_name() << " = " << af->code_name();
	}else{
	  o << ind << s.code_name() << " = va::" << (*i)->name();
	}

	o << "(";
       	std::string comma = "";
	for(size_t i=argnames.size(); i; --i){
	  o << comma << argnames[i-1];
	  comma = ", ";
	}
	o << ");\n";

      }else{ untested();
	unreachable();
	std::cerr << "run time error in make_cc_expression: " << (*i)->name() << ": " << s.num_args() << "\n";
      }
      s.args_pop();
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) {
    }else if (dynamic_cast<const Token_STOP*>(*i)) {
      s.stop();
    }else if (dynamic_cast<const Token_BINOP*>(*i)) {
      assert((*i)->name().size());
      std::string idy = s.code_name();
      s.pop();
      std::string arg1 = s.code_name();
      s.pop();
      s.new_float(o);

      auto op = (*i)->name()[0];
      if ( op == '-'
	|| op == '+'
	|| op == '*'
	|| op == '/'
	|| op == '<'
	|| op == '>'
	|| op == '='
	|| op == '&'
	|| op == '|'
	|| op == '!' ){
	o__ s.code_name() << " = " << arg1 << " " << (*i)->name() << " " << idy << ";\n";
      }else if(op == '%'){
	o__ s.code_name() << " = va::fmod(" << arg1 << ", " << idy << ");\n";
      }else{ untested();
	unreachable();
	assert(false);
	throw Exception("run time error in make_cc_expression: " + (*i)->name());
      }
    }else if (dynamic_cast<const Token_UNARY*>(*i)) {
      std::string arg1 = s.code_name();
      s.pop();
      s.new_float(o);

      auto op = (*i)->name()[0];
      if(op == '-' || op == '!') {
	o__ s.code_name() << " = " << op << arg1 << ";\n";
      }else{ untested();
	incomplete();
	unreachable();
	o__ s.code_name() << " INCOMPLETE = " << op << arg1 << ";\n";
      }
    }else if (auto t = dynamic_cast<const Token_TERNARY*>(*i)) {itested();
      o__ "{\n";
      {itested();
	indent y;
	o__ "ddouble& tt0 = t0;\n"; // BUG: float??
	o__ "if(" << s.code_name() << "){\n";
	{itested();
	  indent x;
	  make_cc_expression(o, *t->true_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}else{\n";
	{itested();
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

  if(s.is_ref()){
    s.new_float(o);
    s.pop();
    o__ "t0 = " << s.code_name() << ";\n";
  }else{
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
    }else if((*i)->name()=="initial_model"){ itested();
      std::cerr << "WARNING: ADMS style keyword encountered\n";
      o__ "evt = _sim->is_initial_step();\n";
    }else{ untested();
      incomplete();
      o << "--> " << (*i)->name() << " <--\n";
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
