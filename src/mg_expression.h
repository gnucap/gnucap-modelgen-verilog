/*                                      -*- C++ -*-
 * Copyright (C) 2023, 2024 Felix Salfelder
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
#ifndef MG_EXPRESSION_H
#define MG_EXPRESSION_H
#include <m_expression.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class TData;
class FUNCTION_;
class Probe;
class Branch_Ref;
class Block;
class RDeps;
class Variable_Access;
class Expression_ : public Expression {
protected:
  typedef std::list<Token const*> tokenlist;
private:
  Base* _owner{nullptr};
  Block* _scope{nullptr}; // remove. later.
  tokenlist _used_variables;
  tokenlist _assignments;
public:
  explicit Expression_() : Expression() {}
  ~Expression_() {}
  void resolve_symbols(Expression const& e);
  void set_owner(Base* b);
//  void set_scope(Block* b){ untested(); _scope = b; }
  void parse(CS& f)override;
  void dump(std::ostream& out)const override;
// private:
  Base* owner(){ return _owner; }
  Base const* owner() const { untested(); return _owner; }
  Block* scope();
  Block const* scope()const { return const_cast<Expression_*>(this)->scope(); }
public: // assign. AF kludge. TODO, one list.
  void push_assign(Token const* t) { _assignments.push_back(t); }
public:
  void push_use(Token const* t) { _used_variables.push_back(t); }
  int num_used_variables()const { return int(_used_variables.size()); }
  void submit_variable_xs(Variable_Access&)const;
  void submit_variable_xs(Expression_&)const; // ternary hack
protected:
  std::list<Token const*> const& used_variables()const { return _used_variables; }
public:
  void clear();
  Expression_* clone()const;
  TData const& data()const; // hmm
//  TData const& deps()const{ untested();return data();}
  // Attrib const& attrib()const;
  bool update(RDeps const* r);
  void set_dep(Base*);
  void set_rdeps(RDeps const&);
  bool is_used_in(Base const*)const;
  bool is_constant()const;
private:
  bool propagate_rdeps(RDeps const& r);
//  TData& mutable_deps();
private: // all the same eventually?
  Token* resolve_xs_function(std::string const& n);
  Token* resolve_system_task(FUNCTION_ const* t);
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br);
}; // Expression_
/*--------------------------------------------------------------------------*/
inline Expression_* Expression_::clone() const
{
  Expression_* n = new Expression_;
  n->set_owner(_owner);

  for (Expression::const_iterator i = begin(); i != end(); ++i) {
    // n->push_back((*i)->clone()); // BUG
    (*i)->stack_op(n);
  }
  return n;
}
/*--------------------------------------------------------------------------*/
std::string make_cc_expression(std::ostream& o, Expression const& e, bool deriv=true,
    std::string ctx="");
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
