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
#include "mg_options.h"
#include <stack>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
static bool is_constant(Token const* t, bool val)
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
static bool is_constant(Token const* t, double val=NOT_VALID)
{
  if(auto f = dynamic_cast<const Float*>(t->data())){
    assert(dynamic_cast<Token_CONSTANT const*>(t)
	  ||dynamic_cast<Token_PAR_REF const*>(t));
    if(val == NOT_VALID){
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
namespace {
/*--------------------------------------------------------------------------*/
class CD : public Deps{
public:
  explicit CD(){
    set_offset();
  }
}const_deps;
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
Deps const* Token_BINOP_::op_deps(Token const* t1, Token const* t2)const
{
  Deps const* d1 = dynamic_cast<Deps const*>(t1->data());
  Deps const* d2 = dynamic_cast<Deps const*>(t2->data());
  // op asserts CONSTANT here. otherwise: similar.

  if(!d1){
    assert(dynamic_cast<Token_CONSTANT const*>(t1)
         ||dynamic_cast<Token_PAR_REF const*>(t1)
         ||dynamic_cast<Token_UNARY const*>(t1));
    d1 = &const_deps;
    assert(!d1->is_linear());
  }else if(d1->is_linear()){
  }else{
  }

  if(!d2){
    assert(dynamic_cast<Token_CONSTANT const*>(t2)
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

  Deps const* ret = NULL;
  if(b){
    ret = prechecked_cast<Deps const*>(b);
    assert(ret);
  }else{
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

  if (is_constant(t1)) {
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

  Deps const* deps = op_deps(t1, t2);

  char n = name()[0];
  if(!options().optimize_binop()) {
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
    }else if(name()=="&&" && is_constant(t1, 0.)){
      t2.erase();
      t1.push();
    }else if(auto bb = dynamic_cast<Token_BINOP_*>(t2.op())) {
      char m = bb->name()[0];
      if(is_constant(bb->op2())) {
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
      }else if(is_constant(bb->op1())) {
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

      }else{ itested();
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
    }else if(name()=="&&" && is_constant(t2, 0.)){
      t1.erase();
      t2.push();
    }else if( options().optimize_swap() && is_constant(t2)){
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
    E->push_back(new Token_CALL(*this, const_deps.clone()));
  }else if(!dynamic_cast<const Token_PARLIST*>(E->back())) {
    // SFCALL
    E->push_back(new Token_CALL(*this, const_deps.clone()));
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
// BUG
static Module* to_module(Block* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<Module *>(owner)){
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
void Token_ACCESS::stack_op(Expression* e) const
{
  Expression& E = *e;

  if(_prb){
  }else if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else if(E.back()->data()) {
    auto back = E.back();
    E.pop_back();
    Base const* d = back->data();
    auto ee = prechecked_cast<Expression const*>(d);
    assert(ee);
    E.push_back(new Token_STOP("fn_stop"));
    for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
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
    assert(SE->owner());
#if 0
    Token* t = SE->owner()->new_token(&f, na);
#else
     // was: Token* VAMS_ACCESS::new_token(Module& m, size_t na)const
    Module* m = to_module(SE->owner()); // dynamic_cast<Module*>(SE->owner());
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
  double ev = _item->eval();
  trace2("stack_op PAR_REF", name(), ev);
  if(ev!=NOT_INPUT) {
    Float* f = new Float(ev);
    // e->push_back(new Token_CONSTANT(name(), f, ""));
    e->push_back(new Token_PAR_REF(*this, f));
  }else{
    e->push_back(new Token_PAR_REF(*this, NULL/*TODO*/));
  }
}
/*--------------------------------------------------------------------------*/
void Token_VAR_REF::stack_op(Expression* e)const
{
  assert(_item);
  double ev = _item->eval();
  if(0&&ev!=NOT_INPUT) { untested();
    Float* f = new Float(ev);
    e->push_back(new Token_CONSTANT(name(), f, ""));
  }else{
    auto deps = _item->deps().clone();
    trace2("var::stack_op", name(), _item->deps().size());
    e->push_back(new Token_VAR_REF(*this, deps));
  }
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
