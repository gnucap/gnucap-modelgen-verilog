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
#include "mg_token.h"
#include "mg_func.h"
#include "mg_options.h"
#include "mg_analog.h" // BUG
#include <stack>
#include <globals.h> // TODO: Expression->resolve?
/*--------------------------------------------------------------------------*/
rdep_tag tr_begin_tag;
rdep_tag tr_restore_tag;
rdep_tag tr_eval_tag;
rdep_tag tr_review_tag;
rdep_tag tr_advance_tag;
rdep_tag tr_accept_tag;
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
static bool is_constant(Token const* t)
{
  if(dynamic_cast<const Float*>(t->data())){ untested();
    assert(dynamic_cast<Token_CONSTANT const*>(t)
	  ||dynamic_cast<Token_PAR_REF const*>(t));
    return true;
  }else if(dynamic_cast<const Integer*>(t->data())){ untested();
    assert(dynamic_cast<Token_CONSTANT const*>(t)
	  ||dynamic_cast<Token_PAR_REF const*>(t));
    return true;
  }else if(auto data = dynamic_cast<const TData*>(t->data())){
    return data->is_constant();
  }else if(dynamic_cast<Token_PAR_REF const*>(t)){
    // why does it not carry data?
    return true;
  }else{ untested();
    // unreachable(); incomplete?
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static bool is_literal(Token const* t, bool val)
{ untested();
  if(auto f = dynamic_cast<const Float*>(t->data())){ untested();
    assert(dynamic_cast<Token_CONSTANT const*>(t)
	  ||dynamic_cast<Token_PAR_REF const*>(t));
    if(f->value() == NOT_VALID){ untested();
      return false;
    }else if(val){ untested();
      return bool(f->value());
    }else{ untested();
      return f->value() == 0.;
    }
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static bool is_literal(Token const* t, double val=NOT_VALID)
{
  if(auto f = dynamic_cast<const Float*>(t->data())){
    assert(dynamic_cast<Token_CONSTANT const*>(t)
	  ||dynamic_cast<Token_PAR_REF const*>(t));
    if(val == NOT_VALID){
      trace1("constant", t->name());
      return true;
    }else{
      return f->value() == val;
    }
  }else if(auto i = dynamic_cast<const Integer*>(t->data())){
    assert(dynamic_cast<Token_CONSTANT const*>(t)
	  ||dynamic_cast<Token_PAR_REF const*>(t));
    if(val == NOT_VALID){
      trace1("constant", t->name());
      return true;
    }else{
      return i->value() == val;
    }
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class stash_op {
  Token* _op{nullptr};
  Expression* _E{nullptr};
public:
  stash_op(Expression* E) : _E(E) {
    assert(E);
    assert(!E->is_empty());
    _op = E->back();
    E->pop_back();
    if(E->is_empty()){
    }else if(auto u = dynamic_cast<Token_UNARY_*>(_op)) {
      (void)u;
      assert(u->op1());
    }else if(auto b = dynamic_cast<Token_BINOP_*>(_op)) {
      (void)b;
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
  bool is_literal(double val=NOT_VALID) const {
    assert(_op);
    return ::is_literal(_op, val);
  }
  bool is_true() const { untested();
    assert(_op);
    return ::is_literal(_op, true);
  }
  bool is_false() const { untested();
    assert(_op);
    return ::is_literal(_op, false);
  }
  void push() {
    _E->push_back(_op);
    _op = nullptr;
  }
  void pop() {
    _op = nullptr;
  }
  void erase() {
    delete(_op);
    _op = nullptr;
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
static bool is_literal(stash_op const& o)
{
  return o.is_literal();
}
/*--------------------------------------------------------------------------*/
Expression* new_arglist(Expression* E)
{
  assert(!E->is_empty());
  std::stack<Token*> stack; // needed?
  auto arg_exp = new Expression_();
  // replace multiple tokens of a PARLIST with a single token
  for (;;) {
    assert(E->size());
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
  return arg_exp;
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
#if 0
Token_CALL::~Token_CALL()
{ untested();
  detach();
  delete _args;
}
#endif
namespace {
/*--------------------------------------------------------------------------*/
class CD : public TData{
public:
  explicit CD(){
    set_offset();
    set_constant();
  }
}const_deps;
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
TData const* Token_BINOP_::op_deps(Token const* t1, Token const* t2)const
{
  TData const* d1 = dynamic_cast<TData const*>(t1->data());
  TData const* d2 = dynamic_cast<TData const*>(t2->data());
  // op asserts CONSTANT here. otherwise: similar.

  if(!d1){
    assert(dynamic_cast<Token_CONSTANT const*>(t1)
         ||dynamic_cast<Token_OUT_VAR const*>(t1)
         ||dynamic_cast<Token_PAR_REF const*>(t1)
         ||dynamic_cast<Token_VAR_REF const*>(t1) // ARGUMENT?
         ||dynamic_cast<Token_UNARY const*>(t1));
    d1 = &const_deps;
    assert(!d1->is_linear());
  }else if(d1->is_linear()){
  }else{
  }

  if(!d2){
    assert(dynamic_cast<Token_CONSTANT const*>(t2)
         ||dynamic_cast<Token_OUT_VAR const*>(t2)
         ||dynamic_cast<Token_PAR_REF const*>(t2)
         ||dynamic_cast<Token_VAR_REF const*>(t2) // ARGUMENT?
         ||dynamic_cast<Token_UNARY const*>(t2));
    d2 = &const_deps;
    assert(!d2->is_linear());
  }else if(d2->is_linear()){
  }else{
  }

  Base const* b;
  if (name() == "*") {
    b = d2->multiply(d1);
  }else if (name() == "/") {
    b = d2->divide(d1);
  }else if (name() == "+") {
    b = d2->combine(d1);
  }else if (name() == "-") {
    b = d2->combine(d1);
  }else{
    b = d2->combine(d1);
  }

  TData const* ret = nullptr;
  if(b){
    ret = prechecked_cast<TData const*>(b);
    assert(ret);
  }else{ untested();
    ret = const_deps.clone();
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
void Token_UNARY_::stack_op(Expression* E)const
{
  if(op1()){
    op1()->stack_op(E); // clone??
    Token* t1 = E->back();
    E->pop_back();
    TData const* d1 = dynamic_cast<TData const*>(t1->data());
    E->push_back(new Token_UNARY_(name(), t1, copy_deps(d1)));

    return;
  }else{
  }

  assert(E);
  // replace 1 token with 1 (result)
  Token const* t1 = E->back();
  E->pop_back();

  if (is_literal(t1)) {
    assert(!dynamic_cast<TData const*>(t1->data()));
    Token* t = op(t1);
    assert(t);
    if (t->data()) {
      E->push_back(t);
    }else{untested();
      E->push_back(new Token_UNARY_(name(), t1));
      delete t;
    }
  }else{
    TData const* d1 = dynamic_cast<TData const*>(t1->data());
    TData const* deps = nullptr;
    if(d1) {
      deps = d1->clone();
    }else{
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
    op2()->stack_op(E); // clone??

    Token_BINOP_ T(name(), copy_deps(data()));
    T.stack_op(E);
    return;
//    E->push_back(new Token_BINOP_(name(), t1, t2, deps));
//    trace2("BINOP pushed", name(), E->size());
  }else{
    trace2("BINOP1c", name(), E->size());
    assert(!op2());
  }

  assert(E);
  stash_op t1(E);
  stash_op t2(E);

  TData const* deps = op_deps(t1, t2);

  char n = name()[0];
  if(!options().optimize_binop()) {
    E->push_back(new Token_BINOP_(name(), t2, t1, deps));
    t1.pop();
    t2.pop();
  }else if (is_literal(t1)) {
    if (is_literal(t2)) {
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
    }else if(n=='*' && is_literal(t1, 0.)){
      // -ffinite-math?
      t2.erase();
      t1.push();
    }else if(n=='/' && is_literal(t1, 1.)) {
      t1.erase();
      t2.push();
    }else if(n=='*' && is_literal(t1, 1.)) {
      t1.erase();
      t2.push();
    }else if(n=='-' && is_literal(t1, 0.)) {
      t1.erase();
      t2.push();
    }else if(n=='+' && is_literal(t1, 0.)) { untested();
      t1.erase();
      t2.push();
    }else if(name()=="&&" && is_literal(t1, 0.)){
      t2.erase();
      t1.push();
    }else if(auto bb = dynamic_cast<Token_BINOP_*>(t2.op())) {
      char m = bb->name()[0];
      if(is_literal(bb->op2())) {
	auto t3 = bb->op2();
	if(( (m == '+' && n == '+')
	  || (m == '+' && n == '-')
	  || (m == '*' && n == '*')
	  || (m == '*' && n == '/') // (a * B) / C == a * (B/C)
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
	  // incomplete();
	  E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	  t1.pop();
	  t2.pop();
	}
      }else if(is_literal(bb->op1())) {
	auto t3 = bb->op1();
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
	// incomplete();
	E->push_back(new Token_BINOP_(name(), t2, t1, deps));
	t1.pop();
	t2.pop();
      }
    }else{
      E->push_back(new Token_BINOP_(name(), t2, t1, deps));
      t1.pop();
      t2.pop();
    }
//  }else if(t1==t2, '-'){ ...
  }else{
    // t2 is constant?
    if(n=='+' && is_literal(t2, 0.)){ untested();
      t2.erase();
      t1.push();
    }else if(n=='*' && is_literal(t2, 1.)){
      t2.erase();
      t1.push();
    }else if(n=='*' && is_literal(t2, 0.)){
      // -ffinite-math?
      t1.erase();
      t2.push();
    }else if(name()=="&&" && is_literal(t2, 0.)){
      t1.erase();
      t2.push();
    }else if( options().optimize_swap() && is_literal(t2)){
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

  trace1("binop result", E->back()->name());
  if(auto EE=dynamic_cast<Expression_ const*>(E)){
     trace1("binop EE", EE->is_constant());
  }else{ untested();
     trace0("binop Expression");
  }
} // Token_BINOP_::stack_op
/*--------------------------------------------------------------------------*/
void Token_OUT_VAR::stack_op(Expression* E)const
{
  E->push_back(clone());
}
/*--------------------------------------------------------------------------*/
void Token_TERNARY_::stack_op(Expression* E)const
{
  Token const* cond;
  if(_cond){
    _cond->stack_op(E);
  }else{
  }

  assert(E->size());
  cond = E->back();
  E->pop_back();

  assert(true_part());
  assert(false_part());
  if ( is_literal(cond) ) {
    bool select = !is_literal(cond, 0.);
    delete cond;
    cond = nullptr;
    Expression const* sel;

    if(select){
      sel = true_part();
    }else{
      sel = false_part();
    }
    // E->reduce_copy(*sel);
    for (Expression::const_iterator i = sel->begin(); i != sel->end(); ++i) {
      (**i).stack_op(E);
    }

  }else{
    TData* deps = new TData;
    trace1("TERNARY0", is_constant(cond));
    if(is_constant(cond)){
      deps->set_constant();
    }else{
      deps->set_constant(false);
    }
    

    auto SE = prechecked_cast<Expression_*>(E);
    assert(SE);

    Expression_* t = new Expression_;
    t->set_owner(SE->owner());
    for (Expression::const_iterator i = true_part()->begin(); i != true_part()->end(); ++i) {
      trace1("stackop stash arg", (**i).name());
      // already stackopped? just clone..
      (**i).stack_op(t);
    }
    deps->update(t->data());
    trace1("TERNARY1", deps->is_constant());

    Expression_* f = new Expression_;
    f->set_owner(SE->owner());
    for (Expression::const_iterator i = false_part()->begin(); i != false_part()->end(); ++i) {
      trace1("stackop stash arg", (**i).name());
      // already stackopped? just clone..
      (**i).stack_op(f);
    }
    deps->update(f->data());
    trace1("TERNARY", deps->is_constant());

    E->push_back(new Token_TERNARY_(name(), cond, t, f, deps));
  }
}
/*--------------------------------------------------------------------------*/
Token_TERNARY_::~Token_TERNARY_()
{
  delete _cond;
  _cond = nullptr;
}
/*--------------------------------------------------------------------------*/
static TData* new_deps(Base const* data)
{
  if(auto ee = dynamic_cast<Expression const*>(data)){
    auto d = new TData;
    d->set_constant();
    for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
      trace1("stackop get deps", (**i).name());
      if(auto dd=dynamic_cast<TData const*>((*i)->data())){
	trace1("stackop get deps", dd->ddeps().size());
	d->update(*dd);
      }else{
      }
    }
    return d;
  }else{ untested();
    assert(0);
    return new TData;
  }
}
/*--------------------------------------------------------------------------*/
static void stack_op_args(Expression* EE, Expression const* arg_expr, FUNCTION_ const* f)
{
  int ii = 0;
  for (Expression::const_iterator i = arg_expr->begin(); i != arg_expr->end(); ++i) {
    trace2("stackop stash arg", (**i).name(), f->is_output_arg(ii));
    if(!f->is_output_arg(ii)){
      (**i).stack_op(EE);
    }else if(auto tt = dynamic_cast<Token_VAR_REF*>(*i)){
      tt->use_var(); // needed?
      tt->assign_var(); // maybe..
      tt->stack_op(EE);
    }else{
      unreachable();
      (**i).stack_op(EE);
    }
    ++ii;
  }
}
/*--------------------------------------------------------------------------*/
void Token_CALL::stack_op(Expression* e) const
{
  Expression_* E = prechecked_cast<Expression_*>(e);
  assert(E);
  trace1("call stackop", name());
  assert(E);
  Expression const* arg_expr = args();

  Token_PARLIST_* pl = nullptr;
  if (arg_expr) {
  }else if (E->is_empty()) {
  }else if((pl=dynamic_cast<Token_PARLIST_*>(E->back()))) {
    arg_expr = pl->args();
    E->pop_back();
  }else{
    assert(!dynamic_cast<Token_PARLIST const*>(E->back()));
  }

  if (arg_expr) {
    FUNCTION_ const* f = function();

    if(f){
      // this is wrong. need different token...
      try{
	::stack_op(f, *arg_expr, E);
      }catch(Exception const&){
	// incomplete(); later
	f = nullptr;
      }
    }else{ untested();
    }
    if(f){
      // incomplete();
      trace2("CALL stackopped", name(), E->back()->name());
    }else{
      trace1("CALL stash", name());
      auto SE = prechecked_cast<Expression_*>(E);
      auto EE = new Expression_;
      EE->set_owner(SE->owner());
      assert(EE->scope());
      stack_op_args(EE, arg_expr, function());

      // here?
      TData* deps = new_deps(arg_expr);
      trace1("stackop stashed arg", deps->is_constant());
      deps->set_any();

      E->push_back(new Token_CALL(*this, deps, EE));
    }
  }else if (E->is_empty()){
    // SFCALL?
    E->push_back(new Token_CALL(*this, const_deps.clone()));
  }else if(!dynamic_cast<const Token_PARLIST*>(E->back())) {
    // SFCALL
    E->push_back(new Token_CALL(*this, const_deps.clone()));
  }else{ untested();
    trace2("no params?", name(), E->back()->name());
    incomplete();
  }
  delete pl;
}
/*--------------------------------------------------------------------------*/
void Token_FUNCTION::stack_op(Expression* e) const
{
  Expression_* E = prechecked_cast<Expression_*>(e);
  assert(E);
  trace1("call stackop", name());
  assert(E);
  Expression const* arg_expr = NULL;

  if (arg_expr) { untested();
  }else if (E->is_empty()) { untested();
  }else if(auto pl=dynamic_cast<Token_PARLIST_*>(E->back())) {
    // del_args = true;
    arg_expr = pl->args();
    if(arg_expr){
      pl->set_args(nullptr);
      E->pop_back();
      delete(pl);
    }else{ untested();
    }
  }else{
    assert(!dynamic_cast<Token_PARLIST const*>(E->back()));
  }

  if (arg_expr) {
    Token_CALL t(*this, nullptr, arg_expr); // check: does it delete arg_expr?
    return t.stack_op(E);
  }else if (E->is_empty()){ untested();
    incomplete();
    E->push_back(new Token_CALL(*this, const_deps.clone()));
  }else if(!dynamic_cast<const Token_PARLIST*>(E->back())) {
    incomplete();
    E->push_back(new Token_FUNCTION(*this, const_deps.clone()));
  }else{ untested();
    trace2("no params?", name(), E->back()->name());
    incomplete();
  }
} // Token_FUNCTION::stack_op
/*--------------------------------------------------------------------------*/
size_t Token_ACCESS::num_deps() const
{ untested();
  if(auto t=dynamic_cast<TData const*>(data())){ untested();
    return t->ddeps().size();
  }else{ untested();
    // incomplete();
    return 0;
  }
}
/*--------------------------------------------------------------------------*/
// TODO: plug-in.
void Token_ACCESS::stack_op(Expression* e) const
{
  Expression& E = *e;

  if(_prb){
  }else if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else if(auto pl = dynamic_cast<Token_PARLIST_*>(E.back())) {
    Expression const* ee = pl->args();
    if(ee){
      E.pop_back();
      E.push_back(new Token_STOP("fn_stop"));

      // attach args?
      for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
	trace1("xs stack", (*i)->name());
	E.push_back((*i)->clone());
	//      (**i).stack_op(&E);
      }
      E.push_back(new Token_PARLIST("fn_args"));
      delete(pl);
    }else{ untested();
      incomplete();
    }
  }else if(E.back()->data()) { untested();
    unreachable(); // using args
    auto back = E.back();
    E.pop_back();
    Base const* d = back->data();
    auto ee = prechecked_cast<Expression const*>(d);
    assert(ee);
    E.push_back(new Token_STOP("fn_stop"));
    for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) { untested();
      trace1("xs stack", (*i)->name());
      E.push_back((*i)->clone());
      //      (**i).stack_op(&E);
    }
    E.push_back(new Token_PARLIST("fn_args"));
    delete back;
  }else{ untested();
    // repeat elab?
    unreachable();
  }

  auto SE = prechecked_cast<Expression_*>(e);
  assert(SE);
  Block* Scope = SE->scope();

  if(_prb){
    // already resolved
    assert(e->is_empty() || !dynamic_cast<Token_PARLIST const*>(e->back()));
    e->push_back(clone());
  }else{
    assert(!e->is_empty() && dynamic_cast<Token_PARLIST const*>(e->back()));

    delete E.back(); // PARLIST
    E.pop_back();
    assert(!E.is_empty());
    if(dynamic_cast<Token_CONSTANT*>(E.back())) { untested();
      trace1("constant?", E.back()->name());
      assert(0);
      // constant string, possibly.
    }else if(dynamic_cast<Token_PORT_BRANCH*>(E.back())) {
      assert(Scope);

      FUNCTION const* f = function_dispatcher[".port_flow"];
      if(!f){ untested();
	throw Exception("need .port_flow plugin to access port flow\n");
      }else{
      }
      Token* t = Scope->new_token(f, 1);
      t->stack_op(&E);
      delete t;
    }else if(dynamic_cast<Token_STOP*>(E.back())) { untested();
      throw Exception("syntax error");
    }else{
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
	trace2("xs stack again", arg0, arg1);

	delete E.back();
	E.pop_back();
	assert(!E.is_empty());
      }

      delete E.back();
      E.pop_back();
      // BUG: push dep?
      //
      trace4("xs", name(), arg0, arg1, na);
      // bug: upside down
      //  VAMS_ACCESS f(name(), arg0, arg1);
      //    assert(ds.top());
      assert(Scope);
#if 0
      Token* t = SE->owner()->new_token(&f, na);
#else
      // was: Token* VAMS_ACCESS::new_token(Module& m, size_t na)const
      Module* m = to_module(Scope); // dynamic_cast<Module*>(SE->owner());
      assert(m);
      // use na?
      Branch_Ref br = m->new_branch(arg0, arg1);
      trace5("br", name(), arg0, arg1, na, br.has_name());

      //  br->set_owner(this);
      assert(br);
      assert(const_cast<Branch const*>(br.operator->())->owner());
      // Probe const* p = m.new_probe(_name, _arg0, _arg1);
      //
      // install clone?
      FUNCTION_ const* p = m->new_probe(name(), br); // Probe

      Token* t = p->new_token(*m, na); // Token_ACCESS
#endif

      assert(t);
      e->push_back(t);
    }
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
{ untested();
  auto p = prb();
  assert(p);
  unreachable();
  return false;// p->is_reversed();
}
/*--------------------------------------------------------------------------*/
std::string Token_ACCESS::code_name() const
{ untested();
  auto p = prb();
  assert(p);
  return p->code_name();
}
/*--------------------------------------------------------------------------*/
std::string Token_ACCESS::code_name_() const
{
  auto p = prb();
  assert(p);
  return p->code_name_();
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
  Base const* ev = _item->value();
  if(ev) {
    e->push_back(new Token_PAR_REF(*this, ev->clone()));
  }else{
    e->push_back(new Token_PAR_REF(*this, nullptr/*TODO*/));
  }
}
/*--------------------------------------------------------------------------*/
size_t Token_VAR_REF::num_deps() const
{
  if(auto t=dynamic_cast<TData const*>(data())){
    return t->ddeps().size();
  }else{ untested();
    // incomplete();
    return 0;
  }
}
/*--------------------------------------------------------------------------*/
void Token_VAR_REF::stack_op(Expression* e)const
{
  auto E = prechecked_cast<Expression_*>(e);
  assert(E);
//  assert(_item); // or !reachable
//  auto oi = prechecked_cast<Owned_Base const*>(_item);
//  assert(oi);
  auto scope = E->scope(); // prechecked_cast<Block const*>(oi->owner());
  assert(scope);

      if(!E->is_empty() && dynamic_cast<Token_PARLIST*>(E->back())){ untested();
	throw Exception("syntax_error: ...");
      }else{
      }

  Base const* r = scope->lookup(name());
  TData const* more = nullptr;
  if(!r){ untested();
    assert(dynamic_cast<Paramset const*>(scope)); // ?
  }else if(r==this){
  }else if(auto x = dynamic_cast<Token_VAR_REF const*>(r)){
    more = &x->deps();
    assert(more);
  }else{ untested();
  }

  {
    TData* nd = nullptr;
    if(auto a = dynamic_cast<Assignment const*>(_item)){
      // nd = deps().clone();
      nd = a->data().clone();
//      nd->add_sens(_item); not yet.
      trace3("var::stackop a", name(), nd->size(), deps().size());
    }else if(auto dd = dynamic_cast<TData const*>(data())){
      // nd = deps().clone();
      nd = dd->clone();
//      nd->add_sens(_item); not yet.
   //  }else if(!_item){ untested();
   //    assert(prechecked_cast<Token_ARGUMENT const*>(this));
   //    nd = new TData(); // BUG.
    }else{ untested();
      incomplete();
      trace1("var::stackop no assignment", name());
    }
    assert(nd);
    if(more){
      nd->update(*more);
    }else{
    }

    auto nn = new Token_VAR_REF(*this, nd);
    // nn->deps();
    assert(!_item || nn->num_deps() == nd->ddeps().size());
    e->push_back(nn);
    assert(nn->_item == _item);

  }
}
/*--------------------------------------------------------------------------*/
TData* Token_PARLIST_::new_deps()const
{ untested();
  incomplete();
  return ::new_deps(data());
}
/*--------------------------------------------------------------------------*/
void Token_ARRAY_::stack_op(Expression* E) const
{
  assert(E);
  if(auto ee = dynamic_cast<Expression const*>(data())){ untested();
    unreachable();
    auto arg_exp = new Expression_();
    for(auto const& i : *ee){ untested();
      // just clone?
      i->stack_op(arg_exp);
    }
    auto argl = new Token_ARRAY("", arg_exp);
    E->push_back(argl);
  }else if(args()){
    auto arg_exp = new Expression_();
    for(auto const& i : *args()){
      // just clone?
      i->stack_op(arg_exp);
    }
    auto* pl = clone();
    pl->set_args(arg_exp);
    E->push_back(pl);
  }else{
    Expression* arg_exp = new_arglist(E);
    trace0("new PARLIST_");
    Token_ARRAY_* n = clone(); // new Token_PARLIST_("", arg_exp);
    assert(n);
    n->set_args(arg_exp);
    E->push_back(n);
  }
}
/*--------------------------------------------------------------------------*/
Token* Probe::new_token(Module&, size_t na)const
{
  std::string name;
  if(discipline()){
    if(_type==t_pot){
      assert(discipline()->potential());
      name = discipline()->potential()->access().to_string();
    }else if(_type==t_flow){
      assert(discipline()->flow());
      name = discipline()->flow()->access().to_string();
    }else{ untested();
      name = "UNKNOWN";
    }
  }else if(_type==t_pot){ untested();
    name = "potential";
  }else if(_type==t_flow){ untested();
    name = "flow";
  }else{ untested();
    unreachable();
    name = "UNKNOWN";
  }

  trace5("got a probe", name, na, pname(), nname(), _br.has_name());
  name += "(";
  if(_br.has_name()){
    name += _br.name();
  }else if(nname() != ""){
    assert(na==2);
    name += pname() + ", " + nname();
  }else{
    name += pname();
    assert(na==1);
  }
  name += ")";

  TData* deps = new TData;
  if(_type==t_pot){
    deps->ddeps().insert(Dep(branch()->potential_dep(), this, Dep::_LINEAR));
  }else{
    deps->ddeps().insert(Dep(branch()->flow_dep(), this, Dep::_LINEAR));
  }

  trace1("stackop probe, new TA", name);
  Token_ACCESS* nt = new Token_ACCESS(name, deps, this);
  // d.insert(Dep(nt->prb(), Dep::_LINEAR));
  return nt;
} // Probe::new_token
/*--------------------------------------------------------------------------*/
void Token_VAR_REF::assign_var()
{
  if(auto p = dynamic_cast<Variable_Decl*>(_item)){
    trace1("assign_var decl", name());
    p->assign_var();
  }else if(auto a = dynamic_cast<Assignment*>(_item)){
    trace1("assign_var assign", name());
    a->assign_var();
  }else{
    // unreachable(); analogfunction
  }
}
/*--------------------------------------------------------------------------*/
void Token_VAR_REF::use_var()
{
  if(auto p = dynamic_cast<Variable_Decl*>(_item)){
    trace1("use_var decl", name());
    p->use_var();
  }else if(auto a = dynamic_cast<Assignment*>(_item)){
    trace1("use_var assign", name());
    a->use_var();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
TData const& Token_VAR_REF::deps() const
{
  auto d = prechecked_cast<TData const*>(data());
  assert(d);
  return *d;
}
/*--------------------------------------------------------------------------*/
Data_Type const& Token_VAR_REF::type() const
{
  if(auto it=dynamic_cast<Assignment const*>(_item)){
    return it->type();
  }else if(auto p = dynamic_cast<Variable_Decl const*>(_item)){ untested();
    assert(p);
    return p->type();
  }else if(auto af = dynamic_cast<Analog_Function const*>(_item)){
    return af->type();
  }else{ untested();
    unreachable();
    static Data_Type_Real t;
    return t;
  }
}
/*--------------------------------------------------------------------------*/
bool Token_VAR_REF::propagate_deps(Token_VAR_REF const& from)
{
  if(dynamic_cast<Token_ARGUMENT*>(this)){
    // BUG: encapsulation problem.
   auto ad = dynamic_cast<TData*>(_item);
   assert(ad);
   ad->update(from.deps());
  }else if(auto dd=dynamic_cast<TData*>(_item)){ untested();
    unreachable();
    TData const& incoming = from.deps();
    dd->update(incoming);
    assert(deps().ddeps().size() >= incoming.ddeps().size());
  }else if(auto it=dynamic_cast<Assignment*>(_item)){
    trace2("Token_VAR_REF::propagate assign", type(), from.type());
    assert(it->scope());
    assert(from.scope());
    return it->propagate_deps(from);
  }else if(auto p = dynamic_cast<Variable_Decl*>(_item)){
    trace2("Token_VAR_REF::propagate decl", type(), from.type());
    return p->propagate_deps(from);
  }else if(dynamic_cast<Analog_Function*>(_item)){
  }else if(dynamic_cast<Block const*>(_item)){ untested();
  }else{
    unreachable();
    // incomplete();
  }

  return false;
}
/*--------------------------------------------------------------------------*/
Block const* Token_VAR_REF::scope() const
{
  // TODO //
  if(auto b=dynamic_cast<Block*>(_item)){
    return b;
  }else if(auto it=dynamic_cast<Owned_Base*>(_item)){
    return it->scope();
  }else if(auto ex=dynamic_cast<Expression_*>(_item)){
    return ex->scope();
  }else{
    return nullptr;
    unreachable();
    auto p = prechecked_cast<Variable_Decl*>(_item);
    assert(p);
    return p->scope();
  }
}
/*--------------------------------------------------------------------------*/
Data_Type const& Token_ARGUMENT::type() const
{
  if(auto v = dynamic_cast<Token_VAR_DECL const*>(_var)) {
    return v->type();
  }else{ untested();
  unreachable();
    static Data_Type_Real t;
    return t;
  }
}
/*--------------------------------------------------------------------------*/
Data_Type const& Token_VAR_DECL::type() const
{
  assert(_item);
  auto oi = prechecked_cast<Variable_Decl const*>(_item);
  assert(oi);
  if(oi->type()){
  }else{
    // paramset bug?
    assert(0);
  }
  return oi->type();
};
/*--------------------------------------------------------------------------*/
void Token_VAR_DECL::stack_op(Expression* e)const
{
  auto E = prechecked_cast<Expression_*>(e);
  assert(E);
  auto xx = dynamic_cast<Statement*>(E->owner());
  auto mm = dynamic_cast<Module*>(E->owner());
  auto af = dynamic_cast<Analog_Function*>(E->owner());// BUG?
  assert(xx||mm||af);
  (void)(xx||mm||af);
  assert(_item);

  {
    TData* nd = nullptr;
    if(auto a = dynamic_cast<Assignment const*>(_item)){ untested();
      unreachable();
      nd = a->data().clone();
//      nd->add_sens(_item); not yet.
      trace3("var::stackop a", name(), nd->size(), deps().size());
    }else if(auto dd = dynamic_cast<TData const*>(data())){
      nd = dd->clone();
//      nd->add_sens(_item); not yet.
    }else{ untested();
      unreachable();
      incomplete();
    }

    auto nn = new Token_VAR_REF(name(), E->scope(), nd);
    assert(nn->scope());
    e->push_back(nn);

  }
}
/*--------------------------------------------------------------------------*/
void Token_VAR_DECL::dump(std::ostream& o) const
{ untested();
  incomplete();
  o << name();
  if(!options().dump_annotate()){ untested();
  }else if(deps().ddeps().size()){ untested();
    for(Dep const& d : deps().ddeps()){ untested();
      o << "// dep " << probe(d)->code_name();
    }
    o << "\n";
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void Token_ARGUMENT::dump(std::ostream& o) const
{
  o << name();
}
/*--------------------------------------------------------------------------*/
// should Token_ARGUMENT be Base?
void Token_ARGUMENT::stack_op(Expression* e) const
{
  auto E = prechecked_cast<Expression_*>(e);
  assert(E);
//  assert(_item); // or !reachable
//  auto oi = prechecked_cast<Owned_Base const*>(_item);
//  assert(oi);
  auto scope = E->scope(); // prechecked_cast<Block const*>(oi->owner());
  assert(scope);

      if(!E->is_empty() && dynamic_cast<Token_PARLIST*>(E->back())){ untested();
	throw Exception("syntax_error: ...");
      }else{
      }

  // Base const* r = scope->lookup(name()); // VAR_REF
  Base const* r = _var;
  assert(r!=this);
  TData const* more = nullptr;
  if(!r){ untested();
    unreachable();
  }else if(auto x = dynamic_cast<Token_VAR_REF const*>(r)){
    more = &x->deps();
    assert(more);
  }else{ untested();
  }

  {
    TData* nd = new TData();
    nd->insert(Dep(this));

    auto nn = new Token_VAR_REF(*this, nd);
    e->push_back(nn);
  }
}
/*--------------------------------------------------------------------------*/
Token_VAR_REF* Token_VAR_REF::clone()const
{
  return new Token_VAR_REF(*this);
}
/*--------------------------------------------------------------------------*/
std::string Token_NODE::code_name() const
{
  return name();
  return "_n[n_" + name() + "]";
}
/*--------------------------------------------------------------------------*/
void Token_NODE::stack_op(Expression* E) const
{
  if(!E->is_empty() && dynamic_cast<Token_PARLIST*>(E->back())){
    throw Exception("syntax_error: Node " + name() + " does not take args");
  }else{
  }
  E->push_back(clone());
}
/*--------------------------------------------------------------------------*/
std::string Token_PORT_BRANCH::code_name() const
{
  assert(_port);
  Node_Ref const& n = _port->node();
  assert(n);
  return "MOD::"+n->code_name(); //  + ", p";
}
/*--------------------------------------------------------------------------*/
std::string Token_VAR_REF::long_code_name() const
{
  std::string s = scope()->long_code_name();
  return s + "._"+ name();
}
/*--------------------------------------------------------------------------*/
bool Token_VAR_REF::is_state_var() const
{
  // incomplete();
  return true;
}
/*--------------------------------------------------------------------------*/
bool Token_VAR_REF::is_common() const
{
  // incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
bool Token_VAR_REF::is_temporary() const
{
  // incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
inline void Token_PARLIST_::stack_op(Expression* E) const
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
  }else if(args()){ untested();
    unreachable();
    auto arg_exp = new Expression_();
    for(auto const& i : *args()){ untested();
      // just clone?
      i->stack_op(arg_exp);
    }
    auto* pl = clone();
    pl->set_args(arg_exp);
    E->push_back(pl);
  }else{
    Expression* arg_exp = new_arglist(E);
    trace0("new PARLIST_");
    auto parlist = clone(); // new Token_PARLIST_("", arg_exp);
    auto n = prechecked_cast<Token_PARLIST_*>(parlist);
    assert(n);
    n->set_args(arg_exp);
    E->push_back(parlist);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch* Token_FILTER::branch() const
{
  auto func = prechecked_cast<MGVAMS_FILTER const*>(f());
  assert( func);
  assert( func->branch__());
  return func->branch__();
}
/*--------------------------------------------------------------------------*/
static Expression_* clone_args(Base const* e)
{
  if(auto e_ = dynamic_cast<Expression_ const*>(e)) {
    return e_->clone();
  }else{ untested();
    unreachable();
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
void Token_FILTER::stack_op(Expression* e)const
{
  assert(e);
  Token_CALL::stack_op(e);
  assert(!e->is_empty());
  auto cc = dynamic_cast<Token_CALL const*>(e->back());
  if(cc){
    e->pop_back();
  }else if(dynamic_cast<Token_CONSTANT const*>(e->back())){
    return;
  }else{ untested();
    unreachable();
  }
  // assert(!e->is_empty());

  auto func = prechecked_cast<MGVAMS_FILTER const*>(f());
  assert(func);
  Expression_* args = nullptr;

  assert(cc->args()->size());
  if(is_zero(*cc->args())){
    trace2("Token_FILTER::stack_op1", name(), cc->args()->size());
    Float* f = new Float(0.);
    e->push_back(new Token_CONSTANT(f, ""));
    delete cc;
    cc = nullptr;
    func->set_p_to_gnd__();
   //  m->set_to_ground(branch()->p());
  }else if(auto dd = prechecked_cast<TData const*>(cc->data())) {
    trace2("Token_FILTER::stack_op2", name(), cc->args()->size());

    Branch* br = branch();
    assert(br);
    assert(br == func->branch()); // really?

    br->deps().clear();
    br->deps() = *dd; // HACK
    if(func->branch()){
      // yikes. where is m?
      // m->set_to_ground(func->branch()->n());
      Node_Ref n = br->n();
      Node* nn = n.mutable_node();
      nn->set_to_ground(nullptr);
    }else if(1){ untested();
      unreachable(); // encapsulation problem
      func->set_n_to_gnd__();
    }else if(0 /*sth linear*/){ untested();
      // somehow set loss=0 and output ports to target.
    }else{ untested();
    }

    auto d = new TData;
    assert(func->prb__()->branch() == branch());
    d->ddeps().insert(Dep(branch()->potential_dep(), func->prb__(), Dep::_LINEAR)); // BUG?
    args = clone_args(cc->args());
    auto N = new Token_FILTER(*this, d, args);
    assert(N->data());
    assert(dynamic_cast<TData const*>(N->data()));
    e->push_back(N);
    assert(f()==N->f());
    delete(cc);
#ifndef NDEBUG
  }else if(!e->size()) { untested();
    unreachable();
  }else if ( dynamic_cast<Token_PARLIST_ const*>(e->back())) { untested();
    unreachable();
#endif
  }else{ untested();
    unreachable();
  }

  if(args){
    RDeps rr;
    if(func->has_tr_accept()){
      rr.insert(&tr_accept_tag);
    }else{
    }
    trace1("Token_FILTER::stackop", func->prb__()->branch()->code_name());
    rr.insert(func->prb__()->branch());
    trace1("Token_FILTER::stackop 4", args->size());
    args->update(&rr); // bug. more generic path.
  }else{
  }
  // ------------------------
  // branch: function->_br
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
