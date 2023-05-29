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
   || n == "log"
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
  std::stack<int> args;
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
      if((*var)->is_module_variable()){
	prefix = "d->_v_";
      }else{
	prefix = "_v_";
      }
      o__ "t" << idx << " = " << prefix << (*i)->name() << ".value();\n";
      for(auto v : (*var)->deps()) {
	o__ "t" << idx << "[d" << v->code_name() << "] = " << prefix << (*i)->name() << "[d" << v->code_name() << "];\n";
      }
    }else if (auto f = dynamic_cast<const Token_FILTER*>(*i)) {
      o__ "t" << idx << " = " << (*f)->code_name() << "(t" << idx << ", d);\n";
      for(auto v : (*f)->deps()) {
	o__ "// dep :" << v->code_name() << "\n";
//	o__ "t" << idx << "[d" << v->code_name() << "] = _v_" << (*i)->name() << "[d" << v->code_name() << "];\n";
      }
    }else if (dynamic_cast<const Token_PAR_REF*>(*i)) {
      o << ind << "t" << idx << " = pc->_p_" << (*i)->name() << ";\n";
    }else if (dynamic_cast<const Token_CONSTANT*>(*i)) {
      o << ind << "t" << idx << " = " << (*i)->name() << ";\n";
    }else if(auto pp = dynamic_cast<const Token_PROBE*>(*i)) {
      char sign = (*pp)->is_reversed()?'-':'+';

      o__ "t" << idx << " = "<<sign<<" p->" << (*pp)->code_name() << ";// "<< pp->name() <<"\n";
      o__ "t" << idx << "[d" << (*pp)->code_name() << "] = " << sign << "1.;\n";
    }else if(dynamic_cast<const Token_SYMBOL*>(*i)) {
      assert(args.size());
      int arity=0;
      MGVAMS_FUNCTION const* f = lookup_function((*i)->name(), arity);
      if(args.top()+1 == idx){
	assert(arity==1);
	o << ind << "t" << idx << " = va::" << (*i)->name();
	o << "(t" << idx << ");\n";
      }else if(arity == 2){
	assert(args.top()+2 == idx);
	int idy = idx;
	idxs.pop();
	idx = idxs.top();
	o << ind << "t" << idx << " = va::" << (*i)->name();

	o << "(t" << idx << ", t" << idy << ");\n";
      }else if(f && args.top() == idx){
	idxs.push(++idx);
	if(idx<idx_alloc) {
	  // re-use temporary variable
	}else{
	  assert(idx==idx_alloc);
	  ++idx_alloc;
	  o << ind << "ddouble t" << idx << ";\n";
	}
	o << ind << "t" << idx << " = " << f->code_name() << ";\n";
      }else{ untested();
	unreachable();
	std::cerr << "run time error in make_cc_expression: " << (*i)->name() << ": " << idx-args.top() << "\n";
      }
      args.pop();
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) {
    }else if (dynamic_cast<const Token_STOP*>(*i)) {
      args.push(idx);
    }else if (dynamic_cast<const Token_BINOP*>(*i)) {
      int idy = idxs.top();
      assert( idy == idx );
      idxs.pop();
      idx = idxs.top();
      assert((*i)->name().size());

      auto op = (*i)->name()[0];
      if ( op == '-'
	|| op == '+'
	|| op == '*'
	|| op == '/') {
	o__ "t" << idx << " "<< op << "= t" << idy << ";\n";
      }else if( op == '>'
	     || op == '<'
	     || op == '=' ) {
	o__ "t" << idx << " = t" << idx << " " << op << " t" << idy << ";\n";
      }else{
	std::cerr << "run time error in make_cc_expression: " << (*i)->name() << "\n";
	incomplete();
	unreachable();
      }
    }else if (dynamic_cast<const Token_UNARY*>(*i)) {
      auto op = (*i)->name()[0];
      if(op == '-') {
	o__ "t" << idx << " *= -1.;\n";
      }else{ untested();
	incomplete();
	unreachable();
      }
    }else if (auto t = dynamic_cast<const Token_TERNARY*>(*i)) {
      o__ "{\n";
      {
	indent y;
	o__ "ddouble& tt0 = t0;\n";
	o__ "if(t" << idx << "){\n";
	{
	  indent x;
	  make_cc_expression(o, *t->true_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}else{\n";
	{
	  indent x;
	  make_cc_expression(o, *t->false_part());
	  o__ "tt0 = t0;\n";
	}
	o__ "}\n";
      }
      o__ "}\n";
    }else{
      incomplete();
      unreachable();
      assert(false);
    }
  }
  assert(!idx);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
