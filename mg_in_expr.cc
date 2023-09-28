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
#include "m_tokens.h" // $vt
#include "mg_func.h"
#include <stack>
#include <e_cardlist.h>
#include <globals.h>
/*--------------------------------------------------------------------------*/
class DEP_STACK {
  std::stack<Deps*> _stack;
  std::stack<size_t> _args;
public:
  DEP_STACK(){
  }
  ~DEP_STACK(){
    if(_stack.size() == 1){
      delete _stack.top();
    }else if(_stack.empty()) {
      // BUG // possibly undeclared variable?
//      throw Exception("stray dep", _stack.top()->name());
    }else{ untested();
      std::cerr << "BUG stray dep " <<  _stack.top()->size() << "\n";
      assert(_stack.empty());
    }
  }
  void clear(){
    while(_stack.size()){
      delete _stack.top();
      _stack.pop();
    }
  }
  void stop(){
    _stack.push(new Deps());
    _args.push(int(_stack.size()));
  }
  size_t num_args() const{
    assert(_stack.size());
    assert(_args.size());
    return(_stack.size()-_args.top());
  }
  // pRop_args?
  void pop_args(){
    assert(_args.size());
    while(_stack.size() > _args.top()){
      assert(_stack.top());
      Deps* td = _stack.top();
      _stack.pop();
      assert(_stack.size());
#if 0
      _stack.top()->update(*td);
#else
      td->update(*_stack.top());
      *_stack.top() = *td;
#endif
      delete(td);
    }
    _args.pop();
  }
//  void args_pop(){ untested();
//    assert(!_args.empty());
//    _args.pop();
//  }
  void new_constant(){
    _stack.push(new Deps());
  }
  void new_var(){ untested();
    _stack.push(new Deps());
  }
  void new_var(Dep d){ untested();
    _stack.push(new Deps());
    _stack.top()->insert(Dep(d));
  }
  void new_var(Deps const& d){
    _stack.push(new Deps());
    _stack.top()->update(d);
  }
  void set(Deps const& d){ untested();
    assert(_stack.size());
    *_stack.top() = d;
  }
  void set(Dep p){ untested();
    assert(_stack.size());
    assert(_stack.top()->empty());
    _stack.top()->insert(Dep(p));
  }
//  void discard(){ untested();
//    assert(_stack.size());
//    delete _stack.top();
//    _stack.pop();
//  }
  void binop(){
    // incomplete();
    assert(_stack.size()>1);
    Deps* td = _stack.top();
    _stack.pop();
    _stack.top()->update(*td);
    delete(td);
  }
  Deps* top(){
    // BUG: ternary
    assert(_stack.size());
    return _stack.top();
  }
  size_t empty() const{
    return _stack.empty();
  }
  size_t size() const{
    return _stack.size();
  }
};
/*--------------------------------------------------------------------------*/
Token* Expression_::resolve_function(FUNCTION_ const* f, DEP_STACK& ds)
{
  Expression& E = *this;

  size_t na = -1;
  if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())) {
    na = ds.num_args();
    assert(ds.top());
    ds.pop_args();
  }else{
    ds.new_constant();
  }

#if 0
  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else{ untested();
    na = ds.num_args();
    assert(ds.top());
    ds.pop_args();
  }
#endif

  assert(owner());
  Token* t = owner()->new_token(f, *ds.top(), na);
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
static Analog_Function const* is_analog_function_call(std::string const& f, Block const* owner)
{
  Module const* m = to_module(owner);

  for(auto n: m->analog_functions()){
    trace2("is_afcall", n->identifier(), f);
    if(n->identifier().to_string() == f){
      return n;
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
// XS::stack_op?
Token* Expression_::resolve_xs_function(std::string const& n, DEP_STACK& ds)
{
  Expression& E = *this;
  size_t na = ds.num_args();
  ds.pop_args();
  trace2("xsf", n, na);
  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else if(E.back()->data()){
    auto back = E.back();
    E.pop_back();
    Base const* d = back->data();
    auto ee = prechecked_cast<Expression const*>(d);
    assert(ee);
    E.push_back(new Token_STOP("fn_stop"));
    for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
      E.push_back((*i)->clone());
//      (**i).stack_op(&E);
    }
    E.push_back(new Token_PARLIST("fn_args"));
    trace1("restored args", size());
    delete back;
    return new Token_ACCESS(n);
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
    // BUG: push dep?
    //
    VAMS_ACCESS f(n, arg0, arg1);
    assert(ds.top());
    assert(owner());
    Token* t = owner()->new_token(&f, *ds.top(), na);
    assert(t);
    return t;
  }
} // resolve_xs_function
/*--------------------------------------------------------------------------*/
void Token_PARLIST_::stack_op(Expression* E)const
{
  assert(E);
  if(auto ee = dynamic_cast<Expression const*>(data())){
    auto arg_exp = new Expression_();
    for(auto const& i : *ee){
      i->stack_op(arg_exp);
    }
    auto parlist = new Token_PARLIST("", arg_exp);
    E->push_back(parlist);
  }else{
    assert(!E->is_empty());
    std::stack<Token*> stack;
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
    auto parlist = new Token_PARLIST("", arg_exp);
    E->push_back(parlist);
  }
}
/*--------------------------------------------------------------------------*/
void Expression_::resolve_symbols_(Expression const& e, Deps* deps)
{
  Expression& E = *this;
  trace0("resolve symbols ===========");
  DEP_STACK ds;
  assert(ds.size()==0);
  Block* scope = owner();

//   for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) { untested();
//     trace1("resolve symbols", (*ii)->name());
//   }

  // resolve symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* t = *ii;
    trace3("loop top:", t->name(), ds.size(), size());

    auto symbol = dynamic_cast<Token_SYMBOL*>(t);
    std::string const& n = t->name();
    Base const* r = scope->resolve(n);
    trace3("resolve top found:", n, r, symbol);

    if(dynamic_cast<Token_STOP*>(t)) {
      trace0("resolve STOP");
      t->stack_op(&E);
      ds.stop();
      assert(ds.num_args()==0);
    }else if(dynamic_cast<Token_CONSTANT*>(t)) {
      t->stack_op(&E);
      ds.new_constant();
    }else if((E.is_empty() || !dynamic_cast<Token_PARLIST*>(E.back()))
          && symbol && t->name() == "inf") {
      Float* f = new Float(std::numeric_limits<double>::infinity());
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
    }else if(auto pl = dynamic_cast<Token_PARLIST*>(t)){
      Token_PARLIST_ tt(*pl);
      tt.stack_op(&E);
    }else if(dynamic_cast<Token_UNARY*>(t)){
      t->stack_op(&E);
    }else if(auto b = dynamic_cast<Token_BINOP*>(t)){
      Token_BINOP_ bb(*b);
      ds.binop();
      bb.stack_op(&E);
    }else if(auto tt = dynamic_cast<Token_TERNARY const*>(t)){
      auto tp = new Expression_();
      auto fp = new Expression_();
      tp->set_owner(owner());
      fp->set_owner(owner());
      try{
	assert(tt->true_part());
	assert(tt->false_part());
	tp->resolve_symbols_(*tt->true_part(), ds.top());
	fp->resolve_symbols_(*tt->false_part(), ds.top());
      }catch(Exception const& e){ untested();
	delete tp;
	delete fp;
	throw e;
      }

      Token_TERNARY t3(t->name(), tp, fp);
      t3.stack_op(&E);
    }else if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())
	  && is_xs_function(n, scope)) {
      trace3("resolve XS", ds.size(), ds.num_args(), size());

      ds.stop();
      Token* t = resolve_xs_function(n, ds);
      t->stack_op(&E);
      ds.pop_args();

      // not here...
      auto ta = dynamic_cast<Token_ACCESS const*>(t);
      assert(ta);
      ds.top()->insert(Dep(ta->prb()));
      assert(ds.top()->size()==1);
      //

      delete t;

      trace2("resolved XS", ds.size(), size());
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
      ds.new_constant();
      E.push_back(new Token_PAR_REF(n, p));
      trace2("pushed par ref", n, size());
    }else if(auto aa = dynamic_cast<Analog_Function_Arg const*>(r)) {
      E.push_back(new Token_VAR_REF(n, aa));
      ds.new_var(aa->deps());
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      E.push_back(new Token_VAR_REF(n, v));
      ds.new_var(v->deps());
    }else if (strchr("0123456789.", n[0])) {
      // a number
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(n, f, ""));
      ds.new_constant();
    }else if(Analog_Function const* af = is_analog_function_call(n, scope)) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      ds.pop_args();
      Token_AFCALL a(n, af);
      a.stack_op(&E);
    }else if(FUNCTION_ const* vaf = va_function(n)) {
      Token* tt = resolve_function(vaf, ds);
      // trace1("va_function no token?", t->name());
      assert(tt);
      tt->stack_op(&E);
      delete tt;
    }else if(scope->node(t->name())) {
      trace1("unresolved node", t->name());
      // incomplete();
      ds.new_constant();
      E.push_back(t->clone()); // try later?
    }else if(scope->branch(t->name())) {
      trace1("unresolved branch", t->name());
      // incomplete();
      ds.new_constant();
      E.push_back(t->clone()); // try later?
    }else{
      ds.clear();
      throw Exception("unresolved symbol: " + n);
    }
    trace2("loopend", ds.size(), E.size());
  }
  trace1("depstack", ds.size());

  if(!deps){
    if(ds.empty()) {
    }else if(ds.top()->empty()) {
    }else{ untested();
      throw Exception("need constant expression\n");
    }
  }else if(ds.size()==1){
    deps->update(*ds.top());
  }else{
    assert(ds.size()==0);
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
