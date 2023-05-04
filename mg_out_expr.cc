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
#include "m_tokens.h"
#include <stack>
/*--------------------------------------------------------------------------*/
static int is_function(std::string const& n)
{
  // incomplete();
  // stub, need sth.
  if (n == "exp"
   || n == "log"
   || n == "cos"
   || n == "sin") {
    return 1;
  }else if (n == "pow"){
    return 2;
  }else{
    return 0;
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_expression(std::ostream& o, Expression const& e)
{
  typedef Expression::const_iterator const_iterator;

  // The _list is the expression in RPN.
  // print a program that computes the function and the derivatives.
  std::stack<int> idxs;
  int idx = -1;
  int idx_alloc = 0;
  for (const_iterator i = e.begin(); i != e.end(); ++i) {
    if (dynamic_cast<const Token_VAR_REF*>(*i)
      ||dynamic_cast<const Token_PAR_REF*>(*i)
      ||dynamic_cast<const Token_CONSTANT*>(*i)
      ||dynamic_cast<const Token_PROBE*>(*i)
      ) {
      idxs.push(++idx);
      if(idx<idx_alloc) {
	// re-use temporary variable
      }else{
	assert(idx==idx_alloc);
	++idx_alloc;
	o << ind << "ddouble t" << idx << ";\n";
      }
    }else{
    }

    if (auto var = dynamic_cast<const Token_VAR_REF*>(*i)) {
		  // code_name?
		  //
      std::string prefix;
      if(var->is_module_variable()){
	prefix = "d->_v_";
      }else{
	prefix = "_v_";
      }
      o__ "t" << idx << " = " << prefix << (*i)->name() << ".value();\n";
      for(auto v : var->deps()) {
	o__ "t" << idx << "[d" << v->code_name() << "] = " << prefix << (*i)->name() << "[d" << v->code_name() << "];\n";
      }
    }else if (auto f = dynamic_cast<const Token_FILTER*>(*i)) {
      o__ "t" << idx << " = " << f->code_name() << "(t" << idx << ", d);\n";
      for(auto v : f->deps()) {
	o__ "// dep :" << v->code_name() << "\n";
//	o__ "t" << idx << "[d" << v->code_name() << "] = _v_" << (*i)->name() << "[d" << v->code_name() << "];\n";
      }
    }else if (dynamic_cast<const Token_PAR_REF*>(*i)) {
      o << ind << "t" << idx << " = pc->_p_" << (*i)->name() << ";\n";
    }else if (dynamic_cast<const Token_CONSTANT*>(*i)) {
      o << ind << "t" << idx << " = " << (*i)->name() << ";\n";
    }else if(auto pp = dynamic_cast<const Token_PROBE*>(*i)) {
      char sign = pp->reversed()?'-':'+';

      o__ "t" << idx << " = "<<sign<<" p->" << pp->code_name() << ";// "<< pp->name() <<"\n";
      o__ "t" << idx << "[d" << pp->code_name() << "] = " << sign << "1.;\n";
    }else if(dynamic_cast<const Token_SYMBOL*>(*i)) {
      int arity = is_function((*i)->name());
      assert(arity);
      if(arity == 1){
	o << ind << "t" << idx << " = va::" << (*i)->name();
	o << "(t" << idx << ");\n";
      }else if(arity == 2){
	int idy = idx;
	idxs.pop();
	idx = idxs.top();
	o << ind << "t" << idx << " = va::" << (*i)->name();

	o << "(t" << idx << ", t" << idy << ");\n";
      }else{ untested();
	unreachable();
      }
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) {
    }else if (dynamic_cast<const Token_STOP*>(*i)) {
    }else if (dynamic_cast<const Token_BINOP*>(*i)) {
      int idy = idxs.top();
      assert( idy == idx );
      idxs.pop();
      idx = idxs.top();
      assert((*i)->name().size());

      char op = (*i)->name()[0];
      switch(op) {
      case '-':
      case '+':
      case '*':
      case '/':
	o__ "t" << idx << " "<< op << "= t" << idy << ";\n";
	break;
      default:
	incomplete();
	unreachable();
	;
      }
    }else if (dynamic_cast<const Token_UNARY*>(*i)) {
      char op = (*i)->name()[0];
      if(op == '-') {
	o__ "t" << idx << " *= -1.;\n";
      }else{ untested();
	incomplete();
	unreachable();
      }
    }else{
      unreachable();
      assert(false);
    }
  }
  assert(!idx);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
