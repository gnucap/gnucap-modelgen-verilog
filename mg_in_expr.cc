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

#include "mg_.h"
#include "m_tokens.h"
#include <stack>
#include <e_cardlist.h>
/*--------------------------------------------------------------------------*/
static Token_FILTER* resolve_filter_function(Expression& E, std::string const& n, Deps const& cdeps, Block* o)
{
  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else{
    assert(n=="ddt" || n=="idt"); // incomplete.
		      //
    assert(!E.is_empty());

    Filter const* f = o->new_filter(n, cdeps);
    assert(f);

    // arglist
//    delete E.back();
//    E.pop_back();

    return new Token_FILTER(n, f);
  }
}
/*--------------------------------------------------------------------------*/
static bool is_filter_function(std::string const& n)
{
  if (n == "ddt"){
    return true;
  }else if (n == "idt"){
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static bool is_system_function_call(std::string const& n)
{
  if(n=="$temperature"
    || n=="$vt"){
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
// BUG: returns arity
static int is_va_function_call(std::string const& n)
{
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
static bool is_xs_function(std::string const& f, Block const* owner)
{
  assert(owner);
  while(!dynamic_cast<Module const*>(owner)){
    owner = owner->owner();
    assert(owner);
  }
  auto m =dynamic_cast<Module const*>(owner);
  assert(m);
  File const* file = m->file();
  if(!file){
    // fallback. modelgen_0.cc // incomplete();
    return f=="V" || f=="I" || f=="flow" || f=="potential";
  }else{
  }

  for(auto n: file->nature_list()){
    if(n->access().to_string() == f){
      return true;
    }else{
    }
  }
  // stub, need discipline.h
  return false;
}
/*--------------------------------------------------------------------------*/
Token_PROBE* resolve_xs_function(Expression& E, std::string const& n, Deps const&, Block* o)
{
  trace1("xsf", n);
  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else{
    delete E.back();
    E.pop_back();
    assert(!E.is_empty());
    if(dynamic_cast<Token_STOP*>(E.back())) { untested();
      throw Exception("syntax error");
    }else{
    }
    std::string arg0 = E.back()->name();
    std::string arg1;
    delete E.back();
    E.pop_back();
    assert(!E.is_empty());

    while(!dynamic_cast<Token_STOP*>(E.back())) {
      arg1 = arg0;
      arg0 = E.back()->name();
      delete E.back();
      E.pop_back();
      assert(!E.is_empty());
    }

    delete E.back();
    E.pop_back();
    trace3("new_probe", n, arg0, arg1);
    Probe const* p = o->new_probe(n, arg0, arg1);
    std::string name = n+"("+arg0+", "+arg1+")";

    trace3("got a probe", name, arg1, arg0);
    return new Token_PROBE(name, p);
    // E.push_back(new Token_PROBE(name, p));
  }
}
/*--------------------------------------------------------------------------*/
void resolve_symbols(Expression const& e, Expression& E, Block* scope, Deps* deps)
{
  trace0("resolve symbols ===========");
  std::stack<Deps*> depstack;
  if(!deps){
    depstack.push(new Deps);
  }else{
    depstack.push(deps);
  }

  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    trace1("resolve symbols", (*ii)->name());
  }
  // resolve symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* t = *ii;
    trace1("resolve top:", t->name());

    auto s = dynamic_cast<Token_SYMBOL*>(t);
    std::string const& n = t->name();
    Base const* r = scope->resolve(n);
    trace2("resolve top found:", n, r);

    if(dynamic_cast<Token_STOP*>(t)) {
      E.push_back(t->clone());
      trace0("resolve STOP");
      depstack.push(new Deps);
    }else if(auto c = dynamic_cast<Token_CONSTANT*>(t)) { untested();
      Token* cl = c->clone();
      assert(t->name() == cl->name());
      E.push_back(cl);
    }else if(dynamic_cast<Token_PARLIST*>(t)
           ||dynamic_cast<Token_UNARY*>(t)
           ||dynamic_cast<Token_BINOP*>(t)) {
      E.push_back(t->clone());
    }else if(!s) {
      unreachable();
      trace1("huh", t->name());
      E.push_back(t->clone());
    }else if(is_xs_function(n, scope)) {
      trace0("resolve XS");
      Deps* td = depstack.top();
      Token_PROBE* t = resolve_xs_function(E, n, *td, scope);
      E.push_back(t);
      td->insert(t->prb());
      depstack.pop();
      assert(!depstack.empty());
      depstack.top()->update(*td);
      delete(td);
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
      E.push_back(new Token_PAR_REF(n, p));
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      E.push_back(new Token_VAR_REF(n, v));
      depstack.top()->update(v->deps());
    }else if(auto pr = dynamic_cast<Probe const*>(r)) { untested();
//      trace1("resolve: probe dep", pr->name());
      E.push_back(new Token_PROBE(n, pr));
      Deps* td = depstack.top();
      delete(td);
      depstack.pop();
      depstack.top()->insert(pr);
    }else if(r) { untested();
      assert(0);
      incomplete(); // unresolved symbol?
    }else if (strchr("0123456789.", n[0])) {
      // a number
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
    }else if(is_system_function_call(t->name())) {
      if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())){
	trace1("is_system_function_call w args", t->name());
	Deps* td = depstack.top();
//	TODO:: td must be empty??
	depstack.pop();
	depstack.top()->update(*td);
	delete(td);
      }else{
	E.push_back(new Token_STOP(".."));
	E.push_back(new Token_PARLIST("...", NULL));
	// E.push_back(t->clone()); // try later?
      }
	// E.push_back(t->clone()); // try later?
      E.push_back(new Token_SFCALL(t->name()));
    }else if(is_va_function_call(t->name())) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      Deps* td = depstack.top();
      depstack.pop();
      depstack.top()->update(*td);
      delete(td);
      E.push_back(t->clone()); // try later?
    }else if(is_filter_function(n)) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      Deps* td = depstack.top();
      Token_FILTER* t = resolve_filter_function(E, n, *td, scope);
      E.push_back(t);
      depstack.pop();
      assert(!depstack.empty());
      // depstack.top()->update(*td);
      delete(td);
      assert((*t)->prb());
      depstack.top()->insert((*t)->prb());
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
  trace1("depstack", depstack.size());

  assert(depstack.size()==1);
  if(deps){
  }else{
    if(depstack.top()->empty()) {
      delete depstack.top();
    }else{ untested();
      delete depstack.top();
      throw Exception("need constant expression\n");
    }
  }
}
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
  Expression tmp;
  assert(_owner);
  resolve_symbols(E, tmp, _owner);
  _e = new Expression(tmp, &CARD_LIST::card_list);
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
