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
    }else{
      std::cerr << "BUG stray dep " <<  _stack.top()->size() << "\n";
      assert(_stack.empty());
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
      Deps* td = _stack.top();
      _stack.pop();
      assert(_stack.size());
      _stack.top()->update(*td);
      delete(td);
    }
    _args.pop();
  }
//  void args_pop(){
//    assert(!_args.empty());
//    _args.pop();
//  }
  void new_constant(){
    _stack.push(new Deps());
  }
  void new_var(){
    _stack.push(new Deps());
  }
  void new_var(Probe const* d){
    _stack.push(new Deps());
    _stack.top()->insert(d);
  }
  void new_var(Deps const& d){
    _stack.push(new Deps(d));
  }
  void set(Deps const& d){
    assert(_stack.size());
    *_stack.top() = d;
  }
  void set(Probe const* p){
    assert(_stack.size());
    assert(_stack.top()->empty());
    _stack.top()->insert(p);
  }
//  void discard(){
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
static Token_SYMBOL* resolve_va_function(Expression& E, MGVAMS_FUNCTION const* t,
    DEP_STACK& ds, Block* o)
{
//  size_t na = ds.num_args(); ??
  Token* nt = o->new_token(t, *ds.top(), 0);
  delete nt; // TODO;
  return NULL;
}
/*--------------------------------------------------------------------------*/
static Token_TASK* resolve_system_task(Expression& E, MGVAMS_TASK const* t,
    DEP_STACK& ds, Block* o)
{
  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else{
    size_t na = ds.num_args();
    trace2("Token_TASK num_args", na, t->label());
    ds.pop_args();
    assert(ds.top());

    Token* nt = o->new_token(t, *ds.top(), na);
    assert(nt);

    auto token = dynamic_cast<Token_TASK*>(nt);
    assert(token);
    token->set_num_args(na);

    Deps outdeps;
//    outdeps.insert((*t)->prb());
    ds.set(outdeps);
    return token;
  }
}
/*--------------------------------------------------------------------------*/
static Token* resolve_filter_function(Expression& E, MGVAMS_FILTER const* filt,
    DEP_STACK& ds, Block* o)
{
  std::string const& n = filt->label();

  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else if(n=="ddt" || n=="idt") {
    size_t na = ds.num_args();
    ds.pop_args();
    assert(ds.top());

    Token* t_ = o->new_token(filt, *ds.top(), na);
    Token_FILTER * t=dynamic_cast<Token_FILTER*>(t_);
    assert(t);

//    Deps outdeps;
//    outdeps.insert((*t)->prb());
//    ds.set(outdeps);
    return t;
  }else if(n=="ddx") {
    size_t na = ds.num_args();
    // delete(E.back());
    // E.pop_back();

    trace1("ddx0", E.back()->name());
    for(auto d : *ds.top()){
      trace1("ddx0 dep", d->code_name());
    }

#if 0
    incomplete();
    Filter const* f = NULL; // o->new_filter(n, *ds.top());
    auto t = new Token_FILTER(n, f);
#else
    Token* t_ = o->new_token(filt, *ds.top(), na);
    Token_FILTER * t=dynamic_cast<Token_FILTER*>(t_);
    //assert(t);
#endif

//    Token* f = o->new_token(n, *ds.top());

    ds.pop_args();
    Deps outdeps;
    ds.set(outdeps);

    return t;
  }else{
    unreachable();
    incomplete();
    return NULL;
  }
      // depstack.top()->update(*td);
}
/*--------------------------------------------------------------------------*/
static MGVAMS_FILTER const* filter_function(std::string const& n)
{
  FUNCTION const* f = function_dispatcher[n];
  return dynamic_cast<MGVAMS_FILTER const*>(f);
}
/*--------------------------------------------------------------------------*/
static bool is_system_function_call(std::string const& n)
{
  // TODO: see is_va_function_call
  if(n=="$param_given"
    || n=="$abstime"
    || n=="$monitor"
//    || n=="$finish"
    || n=="$simparam"
    || n=="$temperature"
    || n=="$vt"){
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static MGVAMS_TASK const* system_task(std::string const& n)
{
  FUNCTION const* f = function_dispatcher[n];
  return dynamic_cast<MGVAMS_TASK const*>(f);
}
/*--------------------------------------------------------------------------*/
static MGVAMS_FUNCTION const* va_function(std::string const& n)
{
  FUNCTION const* f = function_dispatcher[n];
  return dynamic_cast<MGVAMS_FUNCTION const*>(f);
}
/*--------------------------------------------------------------------------*/
static Module const* to_module(Block const* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<Module const*>(owner)){
      return m;
    }
    owner = owner->owner();
    assert(owner);
  }
  unreachable();
  return NULL;
}
/*--------------------------------------------------------------------------*/
static bool is_analog_function_call(std::string const& f, Block const* owner)
{
  Module const* m = to_module(owner);

  for(auto n: m->analog_functions()){
    trace2("is_afcall", n->identifier(), f);
    if(n->identifier().to_string() == f){
      return true;
    }else{
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
static bool is_xs_function(std::string const& f, Block const* owner)
{
  assert(owner);
  while(true){
    if(dynamic_cast<Analog_Function const*>(owner)){
      return false;
    }else if(dynamic_cast<Module const*>(owner)){
      break;
    }
    owner = owner->owner();
    assert(owner);
  }
  auto m = to_module(owner);
  assert(m);
  File const* file = m->file();
  if(!file){
    file = dynamic_cast<File const*>(m->owner());
  }else{
  }

  assert(file);
  if(!file){ untested();
    // fallback. modelgen_0.cc // incomplete();
    return f=="V" || f=="I" || f=="flow" || f=="potential" || f=="Pwr";
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
Token_PROBE* resolve_xs_function(Expression& E, std::string const& n, Block* o)
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
    // BUG: push dep?
    Probe const* p = o->new_probe(n, arg0, arg1);
    std::string name = n + "(" + arg0;
    if(arg1 != ""){
      name += ", " + arg1;
    }else{
    }
    name += ")";

    trace3("got a probe", name, arg1, arg0);
    return new Token_PROBE(name, p);
  }
} // resolve_xs_function
/*--------------------------------------------------------------------------*/
void resolve_symbols(Expression const& e, Expression& E, Block* scope, Deps* deps)
{
  trace0("resolve symbols ===========");
  DEP_STACK ds;
  assert(ds.size()==0);

  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    trace1("resolve symbols", (*ii)->name());
  }

  // resolve symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* t = *ii;
    trace2("loop top:", t->name(), ds.size());

    auto s = dynamic_cast<Token_SYMBOL*>(t);
    std::string const& n = t->name();
    Base const* r = scope->resolve(n);
    trace3("resolve top found:", n, r, s);

    if(dynamic_cast<Token_STOP*>(t)) {
      E.push_back(t->clone());
      trace0("resolve STOP");
      ds.stop();
      assert(ds.num_args()==0);
      // depstack.push(new Deps);
    }else if(auto c = dynamic_cast<Token_CONSTANT*>(t)) {
      Token* cl = c->clone();
      assert(t->name() == cl->name());
      E.push_back(cl);
      ds.new_constant();
    }else if((E.is_empty() || !dynamic_cast<Token_PARLIST*>(E.back()))
          &&  dynamic_cast<Token_SYMBOL*>(t)
          && t->name() == "inf") {
      Float* f = new Float(std::numeric_limits<double>::infinity());
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
    }else if(dynamic_cast<Token_PARLIST*>(t)){
      E.push_back(t->clone());
    }else if(dynamic_cast<Token_UNARY*>(t)){
      E.push_back(t->clone());
    }else if(dynamic_cast<Token_BINOP*>(t)){
      // t->stack_op(E); // ?
      E.push_back(t->clone());
      // merge operand deps? depends on operator..
      ds.binop();

    }else if(auto tt = dynamic_cast<Token_TERNARY const*>(t)){
      Expression* tp = new Expression();
      Expression* fp = new Expression();
      try{
	assert(tt->true_part());
	assert(tt->false_part());
	resolve_symbols(*tt->true_part(), *tp, scope, ds.top());
	resolve_symbols(*tt->false_part(), *fp, scope, ds.top());
      }catch(Exception const& e){ untested();
	delete tp;
	delete fp;
	throw e;
      }

      E.push_back(new Token_TERNARY(t->name(), tp, fp));
    }else if(!s) { untested();
      unreachable();
      trace1("huh", t->name());
      E.push_back(t->clone());
    }else if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())
	  && is_xs_function(n, scope)) {
      trace2("resolve XS", ds.size(), ds.num_args());
      Token_PROBE* t = resolve_xs_function(E, n, scope);
      E.push_back(t);
      ds.pop_args();
      ds.set(t->prb());
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
      ds.new_constant();
      E.push_back(new Token_PAR_REF(n, p));
    }else if(auto aa = dynamic_cast<Analog_Function_Arg const*>(r)) {
      E.push_back(new Token_VAR_REF(n, aa));
      ds.new_var(aa->deps());
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      E.push_back(new Token_VAR_REF(n, v));
      ds.new_var(v->deps());
#if 0
    }else if(auto pr = dynamic_cast<Probe const*>(r)) { untested();
//      trace1("resolve: probe dep", pr->name());
      E.push_back(new Token_PROBE(n, pr));
      Deps* td = depstack.top();
      delete(td);
      depstack.pop();
      depstack.top()->insert(pr);
#endif
    }else if(r) { untested();
      incomplete(); // unresolved symbol?
      std::cerr << "run time error. what is " << t->name() << "?\n";
      assert(0);
    }else if (strchr("0123456789.", n[0])) {
      // a number
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
      ds.new_constant();
    }else if(is_system_function_call(t->name())) {
      if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())){
	trace1("is_system_function_call w args", t->name());
	ds.pop_args();
// :	Deps* td = depstack.top();
// ://	TODO:: td must be empty??
// :	depstack.pop();
// :	depstack.top()->update(*td);
// :	delete(td);
      }else{
	ds.new_constant();
	E.push_back(new Token_STOP(".."));
	E.push_back(new Token_PARLIST("...", NULL));
      }
      if(MGVAMS_FUNCTION const* vaf = va_function(t->name())) {
	// ... incomplete();
	/* Token* t = */ resolve_va_function(E, vaf, ds, scope);
      }else{ untested();
      }
      E.push_back(new Token_SFCALL(t->name()));
    }else if(is_analog_function_call(t->name(), scope)) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      ds.pop_args();
      E.push_back(new Token_AFCALL(t->name()));
///////////////////
    }else if(MGVAMS_FUNCTION const* vaf = va_function(t->name())) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      ds.pop_args();
      /* Token* t = */ resolve_va_function(E, vaf, ds, scope);
      E.push_back(t->clone()); // try later?
    }else if(auto ff = filter_function(n)) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      Token* t = resolve_filter_function(E, ff, ds, scope);
      E.push_back(t);
    }else if(MGVAMS_TASK const* st = system_task(n)) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      Token* tt = resolve_system_task(E, st, ds, scope);
      E.push_back(tt);
///////////////////
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
      throw Exception("unresolved symbol: " + n);
    }
    trace1("loopend", ds.size());
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
  }else{ untested();
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
