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

#include "m_tokens.h" // $vt
#include "mg_.h"
#include "mg_func.h"
#include <stack>
#include <e_cardlist.h>
#include <globals.h>
/*--------------------------------------------------------------------------*/
Expression_* Expression_::clone() const
{
  Expression_* n = new Expression_;

  for (Expression::const_iterator i = begin(); i != end(); ++i) {
    // n->push_back((*i)->clone()); // BUG
    (*i)->stack_op(n);
  }

  return n;
}
/*--------------------------------------------------------------------------*/
Token* Expression_::resolve_function(FUNCTION_ const* f, size_t na)
{
  assert(owner());
  Token* t = owner()->new_token(f, na);
  return t;
}
/*--------------------------------------------------------------------------*/
static FUNCTION_ const* va_function(std::string const& n)
{
  FUNCTION const* f = function_dispatcher[n];
  return dynamic_cast<FUNCTION_ const*>(f);
}
/*--------------------------------------------------------------------------*/
static Module const* to_module(Block const* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<Module const*>(owner)){
      return m;
    }else{
    }
    owner = owner->owner();
    assert(owner);
  }
  unreachable();
  return NULL;
}
/*--------------------------------------------------------------------------*/
static FUNCTION_ const* is_analog_function_call(std::string const& f, Block const* owner)
{
  Module const* m = to_module(owner);

  for(auto n: m->analog_functions()){
    trace2("is_afcall", n->identifier(), f);
    if(n->identifier().to_string() == f){
      assert(n->function());
      return n->function();
    }else{
    }
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
// use dispatcher?
static bool is_xs_function(std::string const& f, Block const* owner)
{
  Module const* m = to_module(owner);
  assert(m);
  File const* file = m->file();
  if(!file){
    file = dynamic_cast<File const*>(m->owner());
  }else{ untested();
  }

  assert(file);
  if(f=="flow" || f=="potential"){ itested();
    return true;
  }else if(file){
    // use actual disciplines
    // auto const& nl = file->nature_list();
    // return find_ptr(nl.begin(), nl.end(), f);
  }else{ untested();
    // fallback. modelgen_0.cc // incomplete();
    return f=="V" || f=="I" || f=="Pwr";
  }

  /// TODO ///
  for(auto n: file->nature_list()){
    if(n->access().to_string() == f){
      return true;
    }else{
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
void Expression_::resolve_symbols_(Expression const& e, Deps*)
{
  Expression& E = *this;
  trace0("resolve symbols ===========");
  Block* scope = owner();

  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    trace1("resolve in", (*ii)->name());
  }

  // resolve symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* t = *ii;

    auto symbol = dynamic_cast<Token_SYMBOL*>(t);
    std::string const& n = t->name();
    Base const* r = scope->resolve(n);
    trace3("resolve top found:", n, r, symbol);

    if(dynamic_cast<Token_STOP*>(t)) {
      trace0("resolve STOP");
      t->stack_op(&E);
    }else if(dynamic_cast<Token_CONSTANT*>(t)) {
      t->stack_op(&E);
    }else if((E.is_empty() || !dynamic_cast<Token_PARLIST*>(E.back()))
          && symbol && t->name() == "inf") {
      Float* f = new Float(std::numeric_limits<double>::infinity());
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
    }else if(auto pl = dynamic_cast<Token_PARLIST*>(t)) {
      trace0("resolve PARLIST");
      Token_PARLIST_ tt(*pl);
      tt.stack_op(&E);
      assert(E.size());
    }else if(auto u = dynamic_cast<Token_UNARY*>(t)){
      Token_UNARY_ bb(*u);
      bb.stack_op(&E);
    }else if(auto b = dynamic_cast<Token_BINOP*>(t)){
      Token_BINOP_ bb(*b);
      bb.stack_op(&E);
    }else if(auto tt = dynamic_cast<Token_TERNARY const*>(t)){
      // BUG, move to stackop
      auto tp = new Expression_();
      auto fp = new Expression_();
      tp->set_owner(owner());
      fp->set_owner(owner());
      try{
	assert(tt->true_part());
	assert(tt->false_part());
	tp->resolve_symbols_(*tt->true_part());
	fp->resolve_symbols_(*tt->false_part());
      }catch(Exception const& e){ untested();
	delete tp;
	delete fp;
	throw e;
      }

      Token_TERNARY_ t3(t->name(), NULL, tp, fp, NULL);
      t3.stack_op(&E);
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
      E.push_back(new Token_PAR_REF(n, p));
      trace2("pushed par ref", n, size());
    }else if(auto aa = dynamic_cast<Analog_Function_Arg const*>(r)) {
      Token_VAR_REF a(n, aa);
      a.stack_op(&E);
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      Token_VAR_REF a(n, v);
      a.stack_op(&E);
    }else if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())
	  && is_xs_function(n, scope)) {
      Token_ACCESS t(n, NULL);
      t.stack_op(&E);
    }else if (strchr("0123456789.", n[0])) {
      // a number
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(n, f, ""));
    }else if(FUNCTION_ const* af = is_analog_function_call(n, scope)) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      Token* tt = resolve_function(af, size_t(-1));
      // Token_AFCALL a(n, af);
      assert(tt);
      tt->stack_op(&E);
      delete tt;
    }else if(FUNCTION_ const* vaf = va_function(n)) {
      size_t na = -1;

      // move to stack_op?
      if(E.is_empty()){
      }else if(auto p = dynamic_cast<Token_PARLIST_ const*>(E.back())){
	if(auto e = dynamic_cast<Expression const*>(p->data())){
	  na = e->size();
	}else{ untested();
	}
      }else{
	assert(!dynamic_cast<Token_PARLIST const*>(E.back()));
      }
      trace2("va_function?", n, na);
      Token* tt = resolve_function(vaf, na);
      assert(tt);
      tt->stack_op(&E);
      delete tt;
    }else if(scope->node(t->name())) {
      trace1("unresolved node", t->name());
      // incomplete();
      E.push_back(t->clone()); // try later?
    }else if(scope->branch(t->name())) {
      trace1("unresolved branch", t->name());
      // incomplete();
      E.push_back(t->clone()); // try later?
    }else{
      throw Exception("unresolved symbol: " + n);
    }
  }
} // resolve_symbols
/*--------------------------------------------------------------------------*/
/* A.8.3
+ mintypmax_expression ::=
+	  expression
-	| expression ":" expression ":" expression
*/
void ConstantMinTypMaxExpression::parse(CS& file)
{
  assert(!_e);
  Expression E(file);
  assert(_owner);
  _e = new Expression_();
  _e->set_owner(_owner);
  _e->resolve_symbols(E);
}
/*--------------------------------------------------------------------------*/
void ConstantMinTypMaxExpression::dump(std::ostream& o)const
{
  assert(_e);
  _e->dump(o);
}
/*--------------------------------------------------------------------------*/
ConstantMinTypMaxExpression::~ConstantMinTypMaxExpression()
{
  delete _e;
  _e = NULL;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
