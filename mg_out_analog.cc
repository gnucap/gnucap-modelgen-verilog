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
      o__ "t" << idx << " = _v_" << (*i)->name() << ".value();\n";
      for(auto v : var->deps()) {
	o__ "t" << idx << "[d" << v->code_name() << "] = _v_" << (*i)->name() << "[d" << v->code_name() << "];\n";
      }
    }else if (auto f = dynamic_cast<const Token_FILTER*>(*i)) {
      o__ "t" << idx << " = p->" << (*i)->name() << "(t" << idx <<" )\n";
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
static void make_cc_variable(std::ostream& o, Variable const& v)
{
  o << ind << "ddouble _v_" << v.name() << "; // Variable";
  for(auto i : v.deps()) { untested();
    o << ind << " Dep: " << i->code_name();
  }
  o << ind << "\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_block_real_identifier_list(std::ostream& o, ListOfBlockRealIdentifiers const& rl)
{
  for(Variable const* v : rl){
    assert(v);
    make_cc_variable(o, *v);
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_assignment(std::ostream& o, Assignment const& a)
{
  assert(a.rhs());
  Expression const& e = *a.rhs();
//   o << "/* RPN ";
//   auto const& e = *a.rhs();
//   for (Expression::const_iterator i = e.begin(); i != e.end(); ++i) {
//     o << "" << (*i)->full_name() << " ";
//   }
//   o << "*/\n";


  o << ind << "{ // Assignment '" << a.lhsname() << "'.";
  for(auto i : a.deps()) {
    o << " Dep: " << i->code_name();
  }
  o << "\n";

  {
    indent x("  ");
    make_cc_expression(o, e);
    o << ind << "_v_" << a.lhsname() << ".value() = t0.value();\n";
    for(auto v : a.deps()) {
      assert(!v->is_reversed());
      o__ "_v_" << a.lhsname() << "[d" << v->code_name() << "] = " << "t0[d" << v->code_name() << "];\n";
      o__ "assert(_v_" << a.lhsname() << " == " << "_v_" << a.lhsname() << ");\n";
    }
  }
  o << ind << "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_contrib(std::ostream& o, Contribution const& f)
{
  o << ind << "{ // FlowContribution " << f.lhsname() << "\n";
  {
    indent x("  ");
    if(f.rhs()) {
      make_cc_expression(o, *f.rhs());
    }else{ untested();
    }

    char sign = f.reversed()?'-':'+';

    assert(f.branch());
    o__ "d->_value" << f.branch()->code_name() << " "<<sign<<"= t0.value();\n";
    for(auto v : f.deps()) {
      if(f.branch() == v->branch()){
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "d->" << f.branch()->state() << "[1]"
	  " "<<sign<<"= " << "t0[d" << v->code_name() << "];\n";
      }else{
      }
    }
    size_t k = 2;
    for(auto v : f.deps()) {
      o__ "// dep " << v->code_name() << "\n";
      if(f.branch() == v->branch()){
      }else{
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "d->" << f.branch()->state() << "[" << k << "]"
	  " "<<sign<<"= " << "t0[d" << v->code_name() << "];\n";
	++k;
      }
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
      make_cc_contrib(o, *fc);
    }else if(auto pc=dynamic_cast<PotContribution const*>(i)) {
      make_cc_contrib(o, *pc);
    }else if(auto a=dynamic_cast<Assignment const*>(i)) {
      make_cc_assignment(o, *a);
    }else if(auto rl=dynamic_cast<ListOfBlockRealIdentifiers const*>(i)) {
      make_cc_block_real_identifier_list(o, *rl);
    }else if(auto v=dynamic_cast<Variable const*>(i)) {
      unreachable();
      make_cc_variable(o, *v);
    }else{ untested();
      incomplete();
    }
  }
}
/*--------------------------------------------------------------------------*/
std::string const& Branch::omit() const
{
  static std::string const n = "";
  return n;
//  return !(_has_contibutions || _has_iprobe);
}
/*--------------------------------------------------------------------------*/
std::string Probe::code_name() const
{
    return "_" + _xs + _br->code_name();
  }
/*--------------------------------------------------------------------------*/
std::string Branch::code_name() const
{
  return "_b_" + p()->name() + "_" + n()->name();
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
