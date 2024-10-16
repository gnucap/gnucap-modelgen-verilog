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
  }else if(auto data = dynamic_cast<const TData*>(t->data())){
    return data->is_constant();
  }else{
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
      return f->value();
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
  }else{
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
static bool is_literal(stash_op const& o)
{
  return o.is_literal();
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
  }else{
    b = d2->combine(d1);
  }

  TData const* ret = NULL;
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
    TData const* deps = NULL;
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
    cond = NULL;
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
  _cond = NULL;
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
void Token_CALL::stack_op(Expression* e) const
{
  Expression_* E = prechecked_cast<Expression_*>(e);
  assert(E);
  trace1("call stackop", name());
  assert(E);
  Expression const* arg_expr = args();

  Token const* T1 = NULL;
  bool del_args = false;
  if (arg_expr) {
  }else if (E->is_empty()) {
  }else if(auto pl=dynamic_cast<Token_PARLIST_*>(E->back())) {
    del_args = true;
    arg_expr = pl->args();
    if(arg_expr){
      pl->set_args(NULL);
      E->pop_back();
      delete(pl);
    }else{ untested();
    }
  }else{
    assert(!dynamic_cast<Token_PARLIST const*>(E->back()));
  }

  if (arg_expr) {
    trace2("CALL got parlist", name(), arg_expr->size());
    FUNCTION_ const* f = _function;

    if(f){
      // this is wrong. need different token...
      try{
	f->stack_op(*arg_expr, E);
      }catch(Exception const&){
	// incomplete(); later
	f = NULL;
      }
    }else{ untested();
    }
    if(f){
      // incomplete();
      trace2("CALL stackopped", name(), E->back()->name());
    }else{
      auto SE = prechecked_cast<Expression_*>(E);
      auto EE = new Expression_;
      EE->set_owner(SE->owner());
      assert(EE->scope());
      for (Expression::const_iterator i = arg_expr->begin(); i != arg_expr->end(); ++i) {
	trace1("stackop stash arg", (**i).name());
	(**i).stack_op(EE);
      }

      // here?
      TData* deps = new_deps(arg_expr);
      trace1("stackop stashed arg", deps->is_constant());
      deps->set_any();

      E->push_back(new Token_CALL(*this, deps, EE));
    }
    delete T1;
    if(del_args){
      delete(arg_expr); // really?
    }else{
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
}
/*--------------------------------------------------------------------------*/
Token_CALL::~Token_CALL()
{
  detach();
  delete _args;
}
/*--------------------------------------------------------------------------*/
// BUG
static Module* to_module(Block* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<Module *>(owner)){
      return m;
    }else{
    }
    owner = owner->scope();
    assert(owner);
  }
  unreachable();
  return NULL;
}
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
  return p->is_reversed();
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
  double ev = _item->eval();
  trace2("stack_op PAR_REF", name(), ev);
  if(ev!=::NOT_INPUT) {
    Float* f = new Float(ev);
    // e->push_back(new Token_CONSTANT(name(), f, ""));
    e->push_back(new Token_PAR_REF(*this, f));
  }else{
    e->push_back(new Token_PAR_REF(*this, NULL/*TODO*/));
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
  if(!r){ untested();
    assert(dynamic_cast<Paramset const*>(scope)); // ?
  }else if(r==this){
  }else if(auto x = dynamic_cast<Token_VAR_REF const*>(r)){
    return x->stack_op(e); // BUG.
  }else{ untested();
  }

  {
    TData* nd = NULL;
    if(auto a = dynamic_cast<Assignment const*>(_item)){
      nd = deps().clone();
      nd = a->data().clone();
//      nd->add_sens(_item); not yet.
      trace3("var::stackop a", name(), nd->size(), deps().size());
    }else if(auto dd = dynamic_cast<TData const*>(data())){ untested();
      nd = deps().clone();
      nd = dd->clone();
//      nd->add_sens(_item); not yet.
    }else{ untested();
      incomplete();
      trace1("var::stackop no assignment", name());
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
Token_PARLIST_::~Token_PARLIST_()
{
  delete _args;
  _args = NULL;
}
/*--------------------------------------------------------------------------*/
static Expression* new_arglist(Expression* E)
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
void Token_PARLIST_::stack_op(Expression* E) const
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
Token* VAMS_ACCESS::new_token(Module& m, size_t na)const
{ untested();
  unreachable(); // obsolete.
  // use na?
  Branch_Ref br = m.new_branch(_arg0, _arg1);
  //  br->set_owner(this);
  assert(br);
  assert(const_cast<Branch const*>(br.operator->())->owner());
  // Probe const* p = m.new_probe(_name, _arg0, _arg1);
  //
  // install clone?
  FUNCTION_ const* p = m.new_probe(_name, br);

  return p->new_token(m, na);
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
  deps->insert(Dep(this, Dep::_LINEAR));

  Token_ACCESS* nt = new Token_ACCESS(name, deps, this);
  // d.insert(Dep(nt->prb(), Dep::_LINEAR));
  return nt;
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
  if(auto dd=dynamic_cast<TData*>(_item)){ untested();
    TData const& incoming = from.deps();
    dd->update(incoming);
    assert(deps().ddeps().size() >= incoming.ddeps().size());
  }else if(auto it=dynamic_cast<Assignment*>(_item)){
    assert(it->scope());
    assert(from.scope());
    return it->propagate_deps(from);
  }else if(auto p = dynamic_cast<Variable_Decl*>(_item)){
    return p->propagate_deps(from);
  }else if(dynamic_cast<Analog_Function*>(_item)){
  }else{ untested();
    unreachable();
  }

  return false;
}
/*--------------------------------------------------------------------------*/
Block const* Token_VAR_REF::scope() const
{
  // TODO //
  if(auto b=dynamic_cast<Block*>(_item)){
    return b;
  }else if(auto it=dynamic_cast<Owned_Base*>(_item)){ untested();
    return it->scope();
  }else if(auto ex=dynamic_cast<Expression_*>(_item)){
    return ex->scope();
  }else{ untested();
    return NULL;
    unreachable();
    auto p = prechecked_cast<Variable_Decl*>(_item);
    assert(p);
    return p->scope();
  }
}
/*--------------------------------------------------------------------------*/
#if 0
void Token_VAR_REAL::clear_deps()
{ untested();
  if(auto it=dynamic_cast<TData*>(_item)){ untested();
    it->clear();
  }else{ untested();
    assert(false);
  }
}
/*--------------------------------------------------------------------------*/
Data_Type const& Token_VAR_REAL::type() const
{ untested();
  static Data_Type_Real t;
  return t;
};
#endif
/*--------------------------------------------------------------------------*/
Data_Type const& Token_VAR_DECL::type() const
{
  assert(_item);
  auto oi = prechecked_cast<Variable_Decl const*>(_item);
  assert(oi);
  assert(oi->type());
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
  assert(_item);
  auto oi = prechecked_cast<Variable_Decl const*>(_item);
  assert(oi);

  {
    TData* nd = NULL;
    if(auto a = dynamic_cast<Assignment const*>(_item)){ untested();
      nd = a->data().clone();
//      nd->add_sens(_item); not yet.
      trace3("var::stackop a", name(), nd->size(), deps().size());
    }else if(auto dd = dynamic_cast<TData const*>(data())){
      nd = dd->clone();
//      nd->add_sens(_item); not yet.
    }else{ untested();
      incomplete();
      trace1("var::stackop no assignment", name());
    }

    auto nn = new Token_VAR_REF(name(), E->scope(), nd);
    assert(nn->scope());
    e->push_back(nn);

  }
}
/*--------------------------------------------------------------------------*/
#if 0
void Token_VAR_REAL::stack_op(Expression* e)const
{ untested();
  auto E = prechecked_cast<Expression_*>(e);
  assert(E);
  auto xx = dynamic_cast<Statement*>(E->owner());
  auto mm = dynamic_cast<Module*>(E->owner());
  auto af = dynamic_cast<Analog_Function*>(E->owner());// BUG?
  assert(xx||mm||af);
  assert(_item);
  auto oi = prechecked_cast<TData const*>(_item);
  assert(oi);

  { untested();
    TData* nd = NULL;
    if(auto a = dynamic_cast<Assignment const*>(_item)){ untested();
      nd = a->data().clone();
//      nd->add_sens(_item); not yet.
      trace3("var::stackop a", name(), nd->size(), deps().size());
    }else if(auto dd = dynamic_cast<TData const*>(data())){ untested();
      nd = dd->clone();
//      nd->add_sens(_item); not yet.
    }else{ untested();
      incomplete();
      trace1("var::stackop no assignment", name());
    }

    auto nn = new Token_VAR_REF(name(), E->scope(), nd);
    assert(nn->scope());
    e->push_back(nn);

  }
}
/*--------------------------------------------------------------------------*/
// same as VAR_REAL::stack_op?
void Token_VAR_INT::stack_op(Expression* e)const
{ untested();
  auto E = prechecked_cast<Expression_*>(e);
  assert(E);
  auto xx = dynamic_cast<Statement*>(E->owner());
  auto mm = dynamic_cast<Module*>(E->owner());
  auto af = dynamic_cast<Analog_Function*>(E->owner());// BUG?
  assert(xx||mm||af);
  assert(_item);
  auto oi = prechecked_cast<TData const*>(_item);
  assert(oi);
  auto nd = deps().clone();

  { untested();
    if(auto a = dynamic_cast<Assignment const*>(_item)){ untested();
      nd = a->data().clone();
      trace3("var::stackop a", name(), nd->size(), deps().size());
    }else if(auto dd = dynamic_cast<TData const*>(data())){ untested();
      nd = dd->clone();
    }else{ untested();
      incomplete();
      trace1("var::stackop no assignment", name());
    }

    auto nn = new Token_VAR_REF(name(), E->scope(), nd);
    assert(nn->scope());
    e->push_back(nn);
  }
}
#endif
/*--------------------------------------------------------------------------*/
void Token_VAR_DECL::dump(std::ostream& o) const
{ untested();
  incomplete();
  o << name();
  if(!options().dump_annotate()){ untested();
  }else if(deps().ddeps().size()){ untested();
    for(auto d : deps().ddeps()){ untested();
      o << "// dep " << d->code_name();
    }
    o << "\n";
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
#if 0
void Token_VAR_REAL::dump(std::ostream& o) const
{ untested();
  o << name();
  if(!options().dump_annotate()){ untested();
  }else if(deps().ddeps().size()){ untested();
    for(auto d : deps().ddeps()){ untested();
      o << "// dep " << d->code_name();
    }
    o << "\n";
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Data_Type const& Token_VAR_INT::type() const
{ itested();
  static Data_Type_Int t;
  return t;
};
/*--------------------------------------------------------------------------*/
void Token_VAR_INT::dump(std::ostream& o) const
{ untested();
  o << name();
}
#endif
/*--------------------------------------------------------------------------*/
void Token_ARGUMENT::dump(std::ostream& o) const
{
  o << name();
}
/*--------------------------------------------------------------------------*/
// bool Token_VAR_REAL::propagate_deps(Token_VAR_REF const& from)
// { untested();
//     assert(!(options().optimize_unused() && !scope()->is_reachable()));
//   // TODO //
//   if(auto it=dynamic_cast<Assignment*>(_item)){ untested();
//     return it->propagate_deps(from);
//   }else{ untested();
//     auto p = prechecked_cast<Variable_Decl*>(_item);
//     assert(p);
//     return p->propagate_deps(from);
//   }
// 
//   return false;
// }
/*--------------------------------------------------------------------------*/
Token_VAR_REF* Token_VAR_REF::clone()const
{
  return new Token_VAR_REF(*this);
}
/*--------------------------------------------------------------------------*/
Token_VAR_REF* Token_VAR_REF::deep_copy(Base* /*owner*/, std::string prefix)const
{ untested();
  assert(0); // not needed.
  if(dynamic_cast<TData const*>(_item)) { untested();
    auto cl = new TData;
    auto n = new Token_VAR_REF(prefix + name(), cl, cl);
    attr.set_attributes(tag_t(n)) = attr.attributes(tag_t(this));
    return n;
  }else{ untested();
    unreachable();
    return new Token_VAR_REF(*this);
  }
}
/*--------------------------------------------------------------------------*/
#if 0
Token_VAR_REAL* Token_VAR_REAL::deep_copy(Base* /*owner*/, std::string prefix)const
{ untested();
  if(dynamic_cast<TData const*>(_item)) { untested();
    auto cl = new TData;
    auto n = new Token_VAR_REAL(prefix + name(), cl, cl);
    attributes(n) = attributes(this);
    return n;
  }else{ untested();
    unreachable();
    return new Token_VAR_REAL(*this);
  }
}
#endif
/*--------------------------------------------------------------------------*/
// Token_VAR_DECL* Token_VAR_DECL::deep_copy(Variable_Decl* owner, std::string prefix)const
// { untested();
//   unreachable();
//   if(dynamic_cast<TData const*>(_item)) { untested();
//     auto cl = new TData;
//     auto n = new Token_VAR_DECL(prefix + name(), owner, cl);
//     attributes(n) = attributes(this);
//     return n;
//   }else{ untested();
//     unreachable();
//     return new Token_VAR_DECL(*this);
//   }
// }
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
  return "_v_" + s + "._"+ name();
}
/*--------------------------------------------------------------------------*/
bool Token_VAR_REF::is_state_var() const
{
  return true; // TODO
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
