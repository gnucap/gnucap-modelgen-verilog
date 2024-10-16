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
#define Token_TERNARY KO_Token_TERNARY
#include <m_expression.h>
#undef Token_TERNARY
/*--------------------------------------------------------------------------*/
// temporary hack.
class Token_TERNARY : public Token {
  Expression const* _true{NULL};
  Expression const* _false{NULL};
protected:
  explicit Token_TERNARY(std::string /*Name*/, Base const* Data)
    : Token(Data) { untested();}
public:
  explicit Token_TERNARY(std::string /*Name*/, Expression const* t, Expression const* f, Base const* d)
    : Token(d), _true(t), _false(f) {}
  explicit Token_TERNARY(const Token_TERNARY& P) : Token(P) { untested();}
  ~Token_TERNARY();
  Token* clone()const override{ untested();return new Token_TERNARY(*this);}
  Token* op(const Token* t1, const Token* t2, const Token* t3)const;
  void stack_op(Expression*)const override;
public:
  Expression const* true_part() const{ return _true; }
  Expression const* false_part() const{ return _false; }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class TData;
class FUNCTION_;
class Probe;
class Branch_Ref;
class Block;
class RDeps;
class Expression_ : public Expression {
  Base* _owner{NULL};
  Block* _scope{NULL}; // remove. later.
public:
  explicit Expression_() : Expression() {}
  ~Expression_();
  void resolve_symbols(Expression const& e);
  void set_owner(Base* b);
//  void set_scope(Block* b){ untested(); _scope = b; }
  void dump(std::ostream& out)const override;
// private:
  Base* owner(){ return _owner; }
  Base const* owner() const { return _owner; }
  Block* scope();
  Block const* scope()const { return const_cast<Expression_*>(this)->scope(); }
public:
  void clear();
  Expression_* clone() const;
  TData const& data()const; // hmm
//  TData const& deps()const{ untested();return data();}
  // Attrib const& attrib()const;
  bool update(RDeps const* r=NULL);
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
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
