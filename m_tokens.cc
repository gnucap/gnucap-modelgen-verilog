/*                        -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 * Author: Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
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
 *------------------------------------------------------------------
 * arithmetic tokens
 */
#include "mg_expression.h"
#include "m_tokens.h"
#include "mg_func.h"
#include "mg_.h" // BUG, Probe
#include <stack>
/*--------------------------------------------------------------------------*/
static const bool optimize_binop = true;
static const bool swap_binop = true;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
static bool is_constant(Token const* t, bool val)
{ untested();
  auto tt = dynamic_cast<Token_CONSTANT const*>(t);
  if(!tt){ untested();
    return false;
  }else if(auto f = dynamic_cast<const Float*>(tt->data())){ untested();
    if(f->value() == NOT_VALID){ untested();
      return false;
    }else if(val){ untested();
      return f->value();
    }else{ untested();
      return f->value() == 0.;
    }
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static bool is_constant(Token const* t, double val=NOT_VALID)
{
  auto tt = dynamic_cast<Token_CONSTANT const*>(t);
  if(!tt){
    return false;
  }else if(val == NOT_VALID){
    return tt;
  }else if(auto f = dynamic_cast<const Float*>(tt->data())){
    return f->value() == val;
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class stash_op {
  Token* _op{NULL};
  Expression* _E{NULL};
public:
  stash_op(Expression* E) : _E(E) {
    assert(E);
    assert(!E->is_empty());
    _op = E->back();
    E->pop_back();
    if(E->is_empty()){
    }else if(auto u = dynamic_cast<Token_UNARY_*>(_op)) {
      assert(u->op1());
    }else if(auto b = dynamic_cast<Token_BINOP_*>(_op)) {
      assert(b->op1());
      assert(b->op2());
    }else{
      assert(!dynamic_cast<Token_BINOP*>(_op));
      assert(!dynamic_cast<Token_UNARY*>(_op));
      assert(!dynamic_cast<Token_PARLIST*>(_op));
      assert(!dynamic_cast<Token_PARLIST_*>(_op));
    }

  }
  ~stash_op(){
    assert(!_op);
  }
  bool is_constant(double val=NOT_VALID) const {
    assert(_op);
    return ::is_constant(_op, val);
  }
  bool is_true() const { untested();
    assert(_op);
    return ::is_constant(_op, true);
  }
  bool is_false() const { untested();
    assert(_op);
    return ::is_constant(_op, false);
  }
  void push() {
    _E->push_back(_op);
    _op = NULL;
  }
  void pop() {
    _op = NULL;
  }
  void erase() {
    delete(_op);
    _op = NULL;
  }
  operator Token const*()const {
    return _op;
  }
  Token const* operator->()const { untested();
    return _op;
  }
  Token* op() {
    return _op;
  }
}; // stash_op
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
static bool is_constant(stash_op const& o)
{
  return o.is_constant();
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
#if 0
void Token_TASK::stack_op(Expression* e)const
{ untested();
  assert(e);
  Token_CALL::stack_op(e);
  assert(e->back());
  if(auto cc=dynamic_cast<Token_TASK const*>(e->back())){ untested();
    Base const* dd = cc->data();
    Deps const* deps = prechecked_cast<Deps const*>(dd);
    assert(deps);
    auto t = new Token_TASK(*cc, deps->clone());
    delete(e->back());
    e->pop_back();
    e->push_back(t);
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
std::string Token_TASK::code_name()const
{ untested();
  unreachable();
  assert(f());
  return "/*task*/ " + f()->code_name();
}
#endif
/*--------------------------------------------------------------------------*/
Token_CALL::~Token_CALL()
{
  detach();
  delete _args;
}
/*--------------------------------------------------------------------------*/
void Token_CALL::attach()
{
  assert(_function);
  _function->inc_refs();
}
/*--------------------------------------------------------------------------*/
void Token_CALL::detach()
{
  assert(_function);
  _function->dec_refs();
}
/*--------------------------------------------------------------------------*/
bool Token_CALL::returns_void() const
{
//  assert(_function);
  if(_function){
    return _function->returns_void();
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
std::string Token_CALL::code_name()const
{
  assert(_function);
  if(_function->code_name()!=""){
    return "/*call1*/" + _function->code_name();
  }else if(_function->label()!=""){
    // incomplete(); // m_va.h, TODO
    return "/*INCOMPLETE*/ va::" + _function->label();
  }else{ untested();
    return "Token_CALL::code_name: incomplete";
  }
}
/*--------------------------------------------------------------------------*/
Deps const* Token_BINOP_::op_deps(Token const* t1, Token const* t2)const
{
  Deps const* d1 = dynamic_cast<Deps const*>(t1->data());
  Deps const* d2 = dynamic_cast<Deps const*>(t2->data());

  Deps const* ret = NULL;

  if(d1 && d2){
    Base* b = d2->multiply(d1);
    if(b){
      ret = prechecked_cast<Deps const*>(b);
      assert(ret);
    }else{ untested();
    }
  }else if(d1) {
    ret = d1->clone();
  }else if(d2) {
    ret = d2->clone();
  }else{
  }

  auto r = prechecked_cast<Deps const*>(ret);
  assert(r || !ret);
  return r;
}
/*--------------------------------------------------------------------------*/
void Token_UNARY_::stack_op(Expression* E)const
{
  if(op1()){

    op1()->stack_op(E); // clone??
    Token* t1 = E->back();
    E->pop_back();
    Deps const* d1 = dynamic_cast<Deps const*>(t1->data());

    E->push_back(new Token_UNARY_(name(), t1, copy_deps(d1)));

    return;
  }else{
    assert(!op1());
  }

  assert(E);
  // replace 1 token with 1 (result)
  Token const* t1 = E->back();
  E->pop_back();

  if (dynamic_cast<Token_CONSTANT const*>(t1)) {
    assert(!dynamic_cast<Deps const*>(t1->data()));
    Token* t = op(t1);
    assert(t);
    if (t->data()) {
      E->push_back(t);
    }else{untested();
      E->push_back(new Token_UNARY_(name(), t1));
      delete t;
    }
  }else{
    Deps const* d1 = dynamic_cast<Deps const*>(t1->data());
    Deps const* deps = NULL;
    if(d1) {
      deps = d1->clone();
    }else{itested();
    }
//    E->push_back(t1);
    E->push_back(new Token_UNARY_(name(), t1, deps));
  }

  // if(!deps){ untested();
  // }else if(auto x = dynamic_cast<Token_BINOP const*>(E->back())){ untested();
  //   auto n = new Token_BINOP_(*x, deps);
  //   delete x;
  //   E->pop_back();
  //   E->push_back(n);
  // }else{ untested();
  // }
}
/*--------------------------------------------------------------------------*/
void Token_BINOP_::stack_op(Expression* E)const
{
  if(op1()){
    assert(op2());

    op1()->stack_op(E); // clone??
    Token* t1 = E->back();
    E->pop_back();
    trace2("BINOP1a", name(), t1->name());
    op2()->stack_op(E); // clone??
    Token* t2 = E->back();
    E->pop_back();

    trace3("BINOP1b", name(), t1->name(), t2->name());

    Deps const* deps = op_deps(t1, t2);

    E->push_back(new Token_BINOP_(name(), t1, t2, deps));
    trace2("BINOP pushed", name(), E->size());

    return;
  }else{
    trace2("BINOP1c", name(), E->size());
    assert(!op2());
  }

  assert(E);
  stash_op t1(E);
  stash_op t2(E);

  Deps const* deps = op_deps(t1, t2);

  char n = name()[0];
  if(!optimize_binop) { itested();
    E->push_back(new Token_BINOP_(name(), t2, t1, deps));
    t1.pop();
    t2.pop();
  }else if (is_constant(t1)) {
    if (is_constant(t2)) {
      // have # # + .. becomes result (the usual)
      Token* t = op(t2, t1);
      assert(t);
      if (t->data()) {
	// success
	E->push_back(t);
	t1.erase();
	t2.erase();
      }else{ untested();
	// fail - one arg is unknown, push back args
	if (strchr("+*", name()[0]) && !dynamic_cast<const Float*>(t1->data())) { untested();
	  // change order to enable later optimization
	  std::swap(t1, t2);
	}else{ untested();
	}
	E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	t1.pop();
	t2.pop();
	delete t;
      }
    }else if(n=='*' && is_constant(t1, 0.)){
      // -ffinite-math?
      t2.erase();
      t1.push();
    }else if(n=='/' && is_constant(t1, 1.)) {
      t1.erase();
      t2.push();
    }else if(n=='*' && is_constant(t1, 1.)) {
      t1.erase();
      t2.push();
    }else if(n=='-' && is_constant(t1, 0.)) {itested();
      t1.erase();
      t2.push();
    }else if(n=='+' && is_constant(t1, 0.)) {
      t1.erase();
      t2.push();
    }else if(auto bb = dynamic_cast<Token_BINOP_*>(t2.op())) {
      char m = bb->name()[0];
      if(auto t3 = dynamic_cast<Token_CONSTANT const*>(bb->op2())) {
	if(( (m == '+' && n == '+')
//	  || (m == '+' && n == '-')
	  || (m == '*' && n == '*')
//	  || (m == '*' && n == '/') // (a * B) / C == a * (B/C)
	  )) {
	  Token* t = op(t3, t1);
	  assert(t);
	  if (t->data()) {
	    // success
	    E->push_back(new Token_BINOP_(bb->name(), bb->op1(), t, deps));
	    bb->pop1();
	    t1.erase();
	    t2.erase();
	  }else{ untested();
	    // fail - push all
	    E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	    t1.pop();
	    t2.pop();
	    delete t;
	  }
	}else if(bb->name()[0] != n){
	  E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	  t1.pop();
	  t2.pop();
	}else{
	  incomplete();
	  E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	  t1.pop();
	  t2.pop();
	}
      }else if(auto t3 = dynamic_cast<Token_CONSTANT const*>(bb->op1())) {
	if(( (m == '/' && n == '*') // (A / b) * C = (A*C) / b
	  || (m == '-' && n == '+')
	  || (m == '/' && n == '/')
	  )) {
	  Token* t = op(t3, t1);
	  assert(t);
	  if (t->data()) {
	    // success
	    E->push_back(new Token_BINOP_(bb->name(), t, bb->op2(), deps));
	    bb->pop2();
	    t1.erase();
	    t2.erase();
	  }else{ untested();
	    // fail - push all
	    E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	    delete t;
	    t1.pop();
	    t2.pop();
	  }

	}else{
	  // TODO: more optimisations?
	  E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	  t1.pop();
	  t2.pop();
	}

      }else{
	incomplete(); // why?
	E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	t1.pop();
	t2.pop();
      }
    }else{
      E->push_back(new Token_BINOP_(name(), t2, t1, deps));
      t1.pop();
      t2.pop();
    }
  }else{
    // t2 is constant?
    if(n=='+' && is_constant(t2, 0.)){
      t2.erase();
      t1.push();
    }else if(n=='*' && is_constant(t2, 1.)){
      t2.erase();
      t1.push();
    }else if(n=='*' && is_constant(t2, 0.)){
      // -ffinite-math?
      t1.erase();
      t2.push();
    }else if( swap_binop && is_constant(t2)){
      if(n=='*' || n=='+'){
	E->push_back(new Token_BINOP_(name(), t1, t2, deps));
      }else{
	E->push_back(new Token_BINOP_(name(), t2, t1, deps));
      }
      t1.pop();
      t2.pop();
    }else{
      E->push_back(new Token_BINOP_(name(), t2, t1, deps));
      t1.pop();
      t2.pop();
    }

  }
  return;
}
/*--------------------------------------------------------------------------*/
void Token_TERNARY_::stack_op(Expression* E)const
{
  Token const* cond;
  if(_cond){itested();
    _cond->stack_op(E);
  }else{
  }

  assert(E->size());
  cond = E->back();
  E->pop_back();

  assert(true_part());
  assert(false_part());
  if ( is_constant(cond) ) {
    bool select = !is_constant(cond, 0.);
    delete cond;
    cond = NULL;
    Expression const* sel;

    if(select){ untested();
      sel = true_part();
    }else{
      sel = false_part();
    }
    // E->reduce_copy(*sel);
    for (Expression::const_iterator i = sel->begin(); i != sel->end(); ++i) {
      (**i).stack_op(E);
    }

  }else{
    Deps* deps = new Deps;

    Expression_* t = new Expression_;
    for (Expression::const_iterator i = true_part()->begin(); i != true_part()->end(); ++i) {
      trace1("stackop stash arg", (**i).name());
      // already stackopped? just clone..
      (**i).stack_op(t);
    }
    deps->update(t->deps());

    Expression_* f = new Expression_;
    for (Expression::const_iterator i = false_part()->begin(); i != false_part()->end(); ++i) {
      trace1("stackop stash arg", (**i).name());
      // already stackopped? just clone..
      (**i).stack_op(f);
    }
    deps->update(f->deps());

    E->push_back(new Token_TERNARY_(name(), cond, t, f, deps));
  }
}
/*--------------------------------------------------------------------------*/
static Deps* new_deps(Base const* data)
{
  if(auto ee = dynamic_cast<Expression const*>(data)){
    auto d = new Deps;
    for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
      trace1("stackop get deps", (**i).name());
      if(auto dd=dynamic_cast<Deps const*>((*i)->data())){
	trace1("stackop get deps", dd->size());
	d->update(*dd);
      }else{
      }
    }
    return d;
  }else{ untested();
    assert(0);
    return new Deps;
  }
}
/*--------------------------------------------------------------------------*/
void Token_CALL::stack_op(Expression* E)const
{
  trace1("call stackop", name());
  assert(E);
  Expression const* arg_expr = args();

  Token const* T1 = NULL;
  if (arg_expr) {
    // repeat stack-op. why?
    // assert(false); // obsolete
    trace2("call stackop", name(), arg_expr->size());
  }else if (E->is_empty()) {
  }else if(dynamic_cast<Token_PARLIST const*>(E->back())) {
    T1 = E->back(); // Token_PARLIST?
    assert(T1);
    Base const* d = T1->data();

#if 1 // stuff Expression into args()
    E->pop_back();
    arg_expr = prechecked_cast<Expression const*>(d);
    assert(arg_expr || !d);
#endif
  }else{
    // finish task w/o args gets here.
  }

  if (arg_expr) {
    trace2("CALL got parlist", name(), arg_expr->size());
    FUNCTION_ const* f = _function;

    if(f){
      // this is wrong. need different token...
      try{
	f->stack_op(*arg_expr, E);
      }catch(Exception const&){
	f = NULL;
      }
    }else{ untested();
    }
    if(f){
      trace2("CALL stackopped", name(), E->back()->name());
    }else{
      auto EE = new Expression_;
      for (Expression::const_iterator i = arg_expr->begin(); i != arg_expr->end(); ++i) {
	trace1("stackop stash arg", (**i).name());
	(**i).stack_op(EE);
      }

      // here?
      auto deps = new_deps(arg_expr);
      E->push_back(new Token_CALL(*this, deps, EE));
    }
    delete T1;
  }else if (E->is_empty()){
    // SFCALL?
      E->push_back(clone());
  }else if(!dynamic_cast<const Token_PARLIST*>(E->back())) {
    // SFCALL
      E->push_back(clone());
  }else if(auto PL = dynamic_cast<const Token_PARLIST*>(E->back())) { untested();
    assert(0); // doesnt work
    trace2("collect deps?", name(), E->back()->name());
    auto deps = ::new_deps(PL);

    E->push_back(new Token_CALL(*this, deps));
  }else{ untested();
    trace2("no params?", name(), E->back()->name());
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
void Token_ACCESS::stack_op(Expression* e) const
{
  Expression& E = *e;
  auto SE = prechecked_cast<Expression_*>(e);
  assert(SE);

  if(_prb){
    // already resolved
    assert(e->is_empty() || !dynamic_cast<Token_PARLIST const*>(e->back()));
    e->push_back(clone());
  }else{
    assert(!e->is_empty() && dynamic_cast<Token_PARLIST const*>(e->back()));

    delete E.back(); // PARLIST
    E.pop_back();
    assert(!E.is_empty());
    if(dynamic_cast<Token_STOP*>(E.back())) { untested();
      throw Exception("syntax error");
    }else{
    }
    std::string arg0 = E.back()->name();
    size_t na=1;
    std::string arg1;
    delete E.back();
    E.pop_back();
    assert(!E.is_empty());

    while(!dynamic_cast<Token_STOP*>(E.back())) {
      arg1 = arg0;
      ++na;
      arg0 = E.back()->name();
      delete E.back();
      E.pop_back();
      assert(!E.is_empty());
    }

    delete E.back();
    E.pop_back();
    // BUG: push dep?
    //
    VAMS_ACCESS f(name(), arg0, arg1);
//    assert(ds.top());
    assert(SE->owner());
    Token* t = SE->owner()->new_token(&f, na);

    assert(t);
    e->push_back(t);
  }
}
/*--------------------------------------------------------------------------*/
Probe const* Token_ACCESS::prb() const
{
  auto p = prechecked_cast<Probe const*>(_prb);
  assert(p);
  return p;
}
/*--------------------------------------------------------------------------*/
bool Token_ACCESS::is_reversed() const
{
  auto p = prb();
  assert(p);
  return p->is_reversed();
}
/*--------------------------------------------------------------------------*/
std::string Token_ACCESS::code_name() const
{
  auto p = prb();
  assert(p);
  return p->code_name();
}
/*--------------------------------------------------------------------------*/
bool Token_ACCESS::is_short() const
{
  auto p = prb();
  assert(p);
  assert(p->branch());
  return p->branch()->is_short();
}
/*--------------------------------------------------------------------------*/
void Token_PAR_REF::stack_op(Expression* e)const
{
  assert(_item);
  e->push_back(new Token_PAR_REF(*this, NULL/*TODO*/));
}
/*--------------------------------------------------------------------------*/
void Token_VAR_REF::stack_op(Expression* e)const
{
  assert(_item);
  auto deps = _item->deps().clone();
  trace2("var::stack_op", name(), _item->deps().size());
  e->push_back(new Token_VAR_REF(*this, deps));
}
/*--------------------------------------------------------------------------*/
Deps* Token_PARLIST_::new_deps()const
{ untested();
  return ::new_deps(data());
}
/*--------------------------------------------------------------------------*/
void Token_PARLIST_::stack_op(Expression* E)const
{
  assert(E);
  if(auto ee = dynamic_cast<Expression const*>(data())){ untested();
    unreachable();
    auto arg_exp = new Expression_();
    for(auto const& i : *ee){ untested();
      // just clone?
      i->stack_op(arg_exp);
    }
    auto parlist = new Token_PARLIST("", arg_exp);
    E->push_back(parlist);
  }else{
    assert(!E->is_empty());
    std::stack<Token*> stack; // needed?
    auto arg_exp = new Expression();
    // replace multiple tokens of a PARLIST with a single token
    for (;;) {
      Token* t = E->back();
      E->pop_back();
      if (dynamic_cast<const Token_STOP*>(t)) {
	delete t;
	break;
      }else{
	stack.push(t);
      }
    }
    // turn over (there is no push_front, maybe on purpose)
    while(!stack.empty()){
      trace1("pushing", stack.top()->name());
      arg_exp->push_back(stack.top());
      stack.pop();
    }
    auto parlist = new Token_PARLIST_("", arg_exp);
    E->push_back(parlist);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
