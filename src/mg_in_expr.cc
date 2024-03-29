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
#include "mg_in.h"
#include "mg_func.h"
#include "mg_code.h"
#include <stack>
#include <e_cardlist.h>
#include <globals.h>
#include "mg_.h" // TODO
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
static Token* resolve_function(FUNCTION_ const* f, Expression const* e, Block* owner)
{
  assert(f);
  assert(e);
  assert(owner);
  size_t na=-1;
  if(e->is_empty()){
  }else if(auto p = dynamic_cast<Token_PARLIST_ const*>(e->back())){
    if(auto d = dynamic_cast<Expression const*>(p->args())){
      na = d->size();
    }else if(auto ex = dynamic_cast<Expression const*>(p->data())){ untested();
      na = ex->size();
    }else{ untested();
    }
  }else if(dynamic_cast<Token_PARLIST const*>(e->back())){ untested();
  }else{
  }
  Token* t = owner->new_token(f, na);
  return t;
}
/*--------------------------------------------------------------------------*/
static FUNCTION_ const* va_function(std::string const& n)
{
  FUNCTION const* f = function_dispatcher[n];
  return dynamic_cast<FUNCTION_ const*>(f);
}
/*--------------------------------------------------------------------------*/
FUNCTION_ const* analog_function_call(std::string const& f, Block const* owner);
static FUNCTION_ const* is_analog_function_call(std::string const& f, Block const* owner)
{
  return analog_function_call(f, owner);
}
/*--------------------------------------------------------------------------*/
FUNCTION_ const* xs_function_call(std::string const& f, Block const* owner);
static bool is_xs_function(std::string const& f, Block const* owner)
{
  return xs_function_call(f, owner);
}

/*--------------------------------------------------------------------------*/
void Expression_::resolve_symbols(Expression const& e) // (, TData*)
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

    if(dynamic_cast<Token_STOP*>(t)) {
      trace0("resolve STOP");
      t->stack_op(&E);
    }else if(dynamic_cast<Token_CONSTANT*>(t)) {
      trace1("Token_CONSTANT", t->name());
      t->stack_op(&E);
    }else if((E.is_empty() || !dynamic_cast<Token_PARLIST*>(E.back()))
          && symbol && t->name() == "inf") {
      Float* f = new Float(std::numeric_limits<double>::infinity());
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
    }else if(auto pl = dynamic_cast<Token_PARLIST*>(t)) {
//      trace1("resolve PARLIST");
      Token_PARLIST_ tt(*pl);
      tt.stack_op(&E);
      assert(E.size());
    }else if(auto u = dynamic_cast<Token_UNARY*>(t)){
      Token_UNARY_ bb(*u);
      bb.stack_op(&E);
    }else if(auto b = dynamic_cast<Token_BINOP*>(t)){
      Token_BINOP_ bb(*b);
      bb.stack_op(&E);
    }else if(auto arr = dynamic_cast<Token_ARRAY*>(t)){
      Token_ARRAY_ bb(*arr);
      bb.stack_op(&E);
    }else if(auto tern = dynamic_cast<Token_TERNARY const*>(t)){
      // BUG, move to stackop
      auto tp = new Expression_();
      auto fp = new Expression_();
      tp->set_owner(owner());
      fp->set_owner(owner());
      try{
	assert(tern->true_part());
	assert(tern->false_part());
	tp->resolve_symbols(*tern->true_part());
	fp->resolve_symbols(*tern->false_part());
      }catch(Exception const& ee){ untested();
	delete tp;
	delete fp;
	throw ee;
      }

      Token_TERNARY_ t3(t->name(), NULL, tp, fp, NULL);
      t3.stack_op(&E);
    }else if (strchr("0123456789.", n[0])) {
      // a number. BUG: integer?
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(n, f, ""));
    }else{
      Base* r = scope->lookup(n);
      trace3("lookup top found:", n, r, symbol);
      if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
//	p->stack_op(&E); // ?
	Token_PAR_REF PP(p->name(), p);
	PP.stack_op(&E);
      }else if(auto v = dynamic_cast<Variable*>(r)) { untested();
	unreachable();
	if(n == v->name()){ untested();
	}else{ untested();
	}
	Token_VAR_REF a(v->name(), v);
	a.stack_op(&E);
      }else if(auto vt = dynamic_cast<Token_VAR_REF*>(r)) {
	trace2("resolve VAR_REF", n, vt->deps().size());
	vt->stack_op(&E);
      }else if(auto pp = dynamic_cast<Port_3 const*>(r)) {
	assert(symbol);
	Token_PORT_BRANCH a(*symbol, pp);
	a.stack_op(&E);
      }else if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())
	    && is_xs_function(n, scope)) {
	// this is upside down...
	Token_ACCESS tta(n, NULL);
	tta.stack_op(&E);
      }else if(FUNCTION_ const* af = is_analog_function_call(n, scope)) {
	// TODO: use "r"
	assert(dynamic_cast<Token_PARLIST*>(E.back()));
	Token* tt = resolve_function(af, &E, owner());
	// Token_AFCALL a(n, af);
	assert(tt);
	tt->stack_op(&E);
	delete tt;
      }else if(FUNCTION_ const* vaf = va_function(n)) {
	size_t na = -1;

	// move to stack_op?
	if(E.is_empty()){
	}else if(auto parlist = dynamic_cast<Token_PARLIST_ const*>(E.back())){
	  if(auto ed = dynamic_cast<Expression const*>(parlist->data())){ untested();
	    na = ed->size();
	  }else if(parlist->args()){
	    na = parlist->args()->size();
	  }else{ untested();
	  }
	}else{
	  assert(!dynamic_cast<Token_PARLIST const*>(E.back()));
	}
	trace2("va_function?", n, na);
	Token* tt = resolve_function(vaf, &E, owner());
	assert(tt);
	tt->stack_op(&E);
	delete tt;
      }else if(Node_Ref a = scope->node(t->name())) {
	Token_NODE tn(*symbol, a);
	tn.stack_op(&E);
      }else if(scope->lookup_branch(t->name())) {
	trace1("unresolved branch", t->name());
	// incomplete();
	E.push_back(t->clone()); // try later?
      }else if(symbol && n[0] == '<') { untested();
	incomplete();
  //      Token_PORT p(t);
  //      p.stack_op(E);
      }else{
	throw Exception("unresolved symbol: " + n);
      }
    }
  }
} // resolve_symbols
/*--------------------------------------------------------------------------*/
bool Expression_::update()
{
  size_t n = deps().ddeps().size();

  auto i = begin();
  for(size_t nn=size(); nn--;){
    if(auto variable = dynamic_cast<Token_VAR_REF*>(*i)) {
      trace2("Expression_::update", variable->name(), variable->deps().size());
    }else{
    }
    (*i)->stack_op(this);
    i = erase(i);
  }

  trace2("Expression_::update", size(), n);
  if(n<=deps().ddeps().size()){
  }else{ untested();
  }
  return n != deps().ddeps().size();
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
double ConstantMinTypMaxExpression::value() const
{
  return _e.eval();
}
/*--------------------------------------------------------------------------*/
void ConstExpression::parse(CS& file)
{
  trace1("ConstExpression::parse", file.tail().substr(0,19));
  assert(owner());
  Expression ce(file);
//  Expression_ tmp;
//  assert(owner());
  _expression.set_owner(owner());
  _expression.resolve_symbols(ce);
}
/*--------------------------------------------------------------------------*/
void ConstExpression::dump(std::ostream& o) const
{
  o << "(";
  o << _expression;
  o << ")";
}
/*--------------------------------------------------------------------------*/
TData const& Expression_::data() const
{
  static TData no_deps;
  if(is_empty()){ untested();
    return no_deps;
  }else if(auto d = dynamic_cast<TData const*>(back()->data())){
    return *d;
//  }else if(auto d = dynamic_cast<Attrib const*>(back()->data())){ untested();
//    return d->deps();
  }else{
    return no_deps;
  }
}
/*--------------------------------------------------------------------------*/
void Expression_::set_dep(Base*)
{
  incomplete();
}
/*--------------------------------------------------------------------------*/
bool Expression_::propagate_rdeps(TData const& dep)
{
  Sensitivities const& sens = data().sensitivities();
  bool ret = false;
  for(auto s : sens){
    trace1("rdep sens", s);
    if(auto st = dynamic_cast<Statement*>(s)){
      ret |= st->propagate_rdeps(dep);
    }else{
    }
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
Expression_::~Expression_()
{
//  for(auto i : _deps){ untested();
//    delete i;
//  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
