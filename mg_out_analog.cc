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
#include <stack>
#include "m_tokens.h"
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
static void make_cc_expression(std::ostream& out, Expression const& e)
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
	out << ind << "ddouble t" << idx << ";\n";
      }
    }else{
    }

    if (auto var = dynamic_cast<const Token_VAR_REF*>(*i)) {
      out << ind << "t" << idx << " = _v_" << (*i)->name() << ".value();\n";
      for(auto v : var->deps()) {
        out << ind << "t" << idx << "[d_" << v->name() << "] = _v_" << (*i)->name() << "[d_" << v->name() << "];\n";
      }
    }else if (auto var = dynamic_cast<const Token_PAR_REF*>(*i)) {
      out << ind << "t" << idx << " = pc->_p_" << (*i)->name() << ";\n";
    }else if (dynamic_cast<const Token_CONSTANT*>(*i)) {
      out << ind << "t" << idx << " = " << (*i)->name() << ";\n";
    }else if(auto pp = dynamic_cast<const Token_PROBE*>(*i)) {
      out << ind << "t" << idx << " = p->" << pp->code_name() << ";// "<< pp->name() <<"\n";
      out << ind << "t" << idx << "[d" << pp->code_name() << "] = " << "1.;\n";
    }else if(dynamic_cast<const Token_SYMBOL*>(*i)) {
      int arity = is_function((*i)->name());
      assert(arity);
      out << ind << "t" << idx << " = va::" << (*i)->name();
      if(arity == 1){
	out << "(t" << idx << ");\n";
      }else if(arity == 2){
	int idy = idx;
	idxs.pop();
	idx = idxs.top();

	out << "(t" << idx << ", t" << idy << ");\n";
      }else{ untested();
	unreachable();
      }
    }else if (dynamic_cast<const Token_PARLIST*>(*i)) {
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
	out << ind << "t" << idx << " "<< op << "= t" << idy << ";\n";
      default:
	;
      }
      // ++idx;
    }
  }
  assert(!idx);
}
/*--------------------------------------------------------------------------*/
static void make_cc_variable(std::ostream& o, Variable const& v)
{
  o << ind << "ddouble _v_" << v.name() << "; // Variable";
  for(auto i : v.deps()) { untested();
    o << ind << " Dep: " << i->name();
  }
  o << ind << "\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_assignment(std::ostream& o, Assignment const& a)
{
  o << "/* RPN ";
  assert(a.rhs());
  auto e = *a.rhs();
  for (Expression::const_iterator i = e.begin(); i != e.end(); ++i) {
    o << "" << (*i)->full_name() << " ";
  }
  o << "*/\n";


  o << ind << "{ // Assignment '" << a.lhsname() << "'.";
  for(auto i : a.deps()) {
    o << " Dep: " << i->name();
  }
  o << "\n";

  if(1) {
    indent x("  ");
    //      _rhs->dump(o);
    make_cc_expression(o, e);
    o << ind << "_v_" << a.lhsname() << ".value() = t0.value();\n";
    for(auto v : a.deps()) {
      o << ind << "_v_" << a.lhsname() << "[d_" << v->name() << "] = " << "t0[d_" << v->name() << "];\n";
    }
  }else{ untested();
  }
  o << ind << "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_pc(std::ostream& o, PotContribution const& f)
{
  o << ind << "{ // PotContribution " << f.lhsname() << "\n";
  o << ind << "  incomplete(); \n";
  o << ind << "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_fc(std::ostream& o, FlowContribution const& f)
{
  o << ind << "{ // FlowContribution " << f.lhsname() << "\n";
  {
    indent x("  ");
    if(f.rhs()) {
      make_cc_expression(o, *f.rhs());
    }else{ untested();
    }

    assert(f.branch());
    o << ind << ind << "d->_branch" << f.branch()->name() << ".value() += t0.value();\n";
    for(auto v : f.deps()) {
      o << ind << ind << "d->_branch" << f.branch()->name()
	<< "[d_" << v->name() << "] += " << "t0[d_" << v->name() << "];\n";
    }
  }
  o << ind <<  "}\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_analog(std::ostream& o, AnalogBlock const& ab)
{
  for(auto i : ab) {
    // need commmon baseclass...
    if(auto fc=dynamic_cast<FlowContribution const*>(i)) {
      make_cc_fc(o, *fc);
    }else if(auto pc=dynamic_cast<PotContribution const*>(i)) { untested();
      make_cc_pc(o, *pc);
    }else if(auto a=dynamic_cast<Assignment const*>(i)) {
      make_cc_assignment(o, *a);
    }else if(auto v=dynamic_cast<Variable const*>(i)) {
      make_cc_variable(o, *v);
    }else{ untested();
      incomplete();
    }
  }
//  o << ind << "return true;\n";
//  o << "} // AnalogBlock\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
