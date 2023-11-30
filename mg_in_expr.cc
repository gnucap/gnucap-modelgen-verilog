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
void Expression_::clear()
{
  while (!is_empty()){
    delete back();
    pop_back();
  }
}
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
static File const* to_file(Block const* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<File const*>(owner)){
      return m;
    }else{
    }
    owner = owner->owner();
    if(!owner){ untested();
      return NULL;
    }else{
    }
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
  File const* file = to_file(owner);
  if(!file){
    file = dynamic_cast<File const*>(m->owner());
  }else{
  }

  assert(file);
  if(f=="flow" || f=="potential") { itested();
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

  // TODO: return FUNCTION_*, VAMS_XS* from nature
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

  // lookup symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* t = *ii;

    auto symbol = dynamic_cast<Token_SYMBOL*>(t);
    std::string const& n = t->name();
    Base const* r = scope->lookup(n);
    trace3("lookup top found:", n, r, symbol);

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
#if 0
      // no. aliases are not supposed to be used here
    }else if(auto a = dynamic_cast<Aliasparam const*>(r)) {
      Token_PAR_REF PP(a->param_name(), a->param()); // BUG? use code name.
      PP.stack_op(&E);
#endif
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
//       owner()->new_token(p, 0);??
//      Token_PAR_REF PP(n, p);
      Token_PAR_REF PP(p->name(), p);
      PP.stack_op(&E);
      trace3("pushed par ref", n, size(), E.back()->name());
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      if(n == v->name()){
      }else{
      }
      Token_VAR_REF a(v->name(), v);
      a.stack_op(&E);
    }else if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())
	  && is_xs_function(n, scope)) {
      // this is upside down...
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
bool Expression_::update()
{
  size_t n = deps().size();

  auto i = begin();
  for(size_t n=size(); n--;){
    (*i)->stack_op(this);
    i = erase(i);
  }

  trace2("Expression_::update", size(), n);
  if(n<=deps().size()){
  }else{
  }
  return n != deps().size();
}
/*--------------------------------------------------------------------------*/
/* A.8.3
+ mintypmax_expression ::=
+	  expression
-	| expression ":" expression ":" expression
*/
void ConstantMinTypMaxExpression::parse(CS& file)
{
  assert(_e.is_empty());
//  Expression_ E;
  file >> _e;
  assert(_owner);
//  = new Expression_();
  _e.set_owner(_owner);
//  _e.resolve_symbols(E);
}
/*--------------------------------------------------------------------------*/
void ConstantMinTypMaxExpression::resolve()
{
  Expression_ tmp;
  tmp.set_owner(_owner);
  tmp.resolve_symbols(_e);
  _e.clear();

  for(auto j : tmp){
    _e.push_back(j);
  }
  while(tmp.size()){
    tmp.pop_back();
  }
}
/*--------------------------------------------------------------------------*/
void ConstantMinTypMaxExpression::dump(std::ostream& o)const
{
  _e.dump(o);
}
/*--------------------------------------------------------------------------*/
ConstantMinTypMaxExpression::~ConstantMinTypMaxExpression()
{
}
/*--------------------------------------------------------------------------*/
bool Parameter_2_List::is_local()const
{
  // really? ask *begin?
  return _is_local;
}
/*--------------------------------------------------------------------------*/
double /*?*/ Parameter_2::eval() const
{
  if(is_local()) {
    return _default_val.value();
  }else if (value_range_list().size() == 1) {
    return (*value_range_list().begin())->eval();
  }else{
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
double ConstantMinTypMaxExpression::value() const
{
  return _e.eval();
}
/*--------------------------------------------------------------------------*/
double ValueRange::eval() const
{
  if(spec()){
    return spec()->eval();
  }else{ untested();
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
