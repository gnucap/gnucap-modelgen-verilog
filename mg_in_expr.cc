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
  void new_var(Probe const* d){ untested();
    _stack.push(new Deps());
    _stack.top()->insert(Dep(d));
  }
  void new_var(Deps const& d){
    _stack.push(new Deps(d));
  }
  void set(Deps const& d){ untested();
    assert(_stack.size());
    *_stack.top() = d;
  }
  void set(Probe const* p){ untested();
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
Token* Symbolic_Expression::resolve_function(FUNCTION_ const* f,
    DEP_STACK& ds, Block* o)
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

  Token* t = o->new_token(f, *ds.top(), na);
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
class VAMS_ACCESS : public FUNCTION_ {
  std::string _name, _arg0, _arg1;
public:
  VAMS_ACCESS(std::string n, std::string a0, std::string a1)
    : _name(n), _arg0(a0), _arg1(a1) {
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override {unreachable(); return "";}
  Token* new_token(Module& m, size_t na, Deps& d)const override {
    // use na?
    Branch_Ref br = m.new_branch(_arg0, _arg1);
  //  br->set_owner(this);
    assert(br);
    assert(const_cast<Branch const*>(br.operator->())->owner());
    // Probe const* p = m.new_probe(_name, _arg0, _arg1);
    //
     // install clone?
    Probe const* p = m.new_probe(_name, br);

    std::string name = _name + "(" + _arg0;
    if(_arg1 != ""){
      assert(na==2);
      name += ", " + _arg1;
    }else{
      assert(na==1);
    }
    name += ")";

    trace3("got a probe", name, _arg1, _arg0);
    Token_ACCESS* nt = new Token_ACCESS(name, p);
    assert(d.empty());
    d.insert(Dep(nt->prb()));
    return nt;
  }
  void make_cc_common(std::ostream&)const override { unreachable(); }
};
/*--------------------------------------------------------------------------*/
Token* Symbolic_Expression::resolve_xs_function(std::string const& n, DEP_STACK& ds, Block* o)
{
  Expression& E = *this;
  size_t na = ds.num_args();
  ds.pop_args();
  trace2("xsf", n, na);
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
    // BUG: push dep?
    //
    VAMS_ACCESS f(n, arg0, arg1);
    assert(ds.top());
    Token* t = o->new_token(&f, *ds.top(), na);
    assert(t);
    return t;
  }
} // resolve_xs_function
/*--------------------------------------------------------------------------*/
void Symbolic_Expression::resolve_symbols(Expression const& e, Block* scope, Deps* deps)
{
  Expression& E = *this;
  trace0("resolve symbols ===========");
  DEP_STACK ds;
  assert(ds.size()==0);

//   for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) { untested();
//     trace1("resolve symbols", (*ii)->name());
//   }

  // resolve symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* t = *ii;
    trace2("loop top:", t->name(), ds.size());

    auto symbol = dynamic_cast<Token_SYMBOL*>(t);
    std::string const& n = t->name();
    Base const* r = scope->resolve(n);
    trace3("resolve top found:", n, r, symbol);

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
          && symbol && t->name() == "inf") {
      Float* f = new Float(std::numeric_limits<double>::infinity());
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
    }else if(dynamic_cast<Token_PARLIST*>(t)){
      E.push_back(t->clone());
    }else if(dynamic_cast<Token_UNARY*>(t)){
      E.push_back(t->clone());
    }else if(dynamic_cast<Token_BINOP*>(t)){
      ds.binop();
#if 0
      t->stack_op(&E); // ?
#else
      E.push_back(t->clone());
#endif
    }else if(auto tt = dynamic_cast<Token_TERNARY const*>(t)){
      auto tp = new Symbolic_Expression();
      auto fp = new Symbolic_Expression();
      try{
	assert(tt->true_part());
	assert(tt->false_part());
	tp->resolve_symbols(*tt->true_part(), scope, ds.top());
	fp->resolve_symbols(*tt->false_part(), scope, ds.top());
      }catch(Exception const& e){ untested();
	delete tp;
	delete fp;
	throw e;
      }

      E.push_back(new Token_TERNARY(t->name(), tp, fp));
    }else if(!symbol) { untested();
      unreachable();
      trace1("huh", t->name());
      E.push_back(t->clone());
    }else if(!E.is_empty() && dynamic_cast<Token_PARLIST*>(E.back())
	  && is_xs_function(n, scope)) {
      trace2("resolve XS", ds.size(), ds.num_args());
      Token* t = resolve_xs_function(n, ds, scope);
      E.push_back(t);
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
      ds.new_constant();
      E.push_back(new Token_PAR_REF(n, p));
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
      E.push_back(new Token_AFCALL(n, af));
    }else if(FUNCTION_ const* vaf = va_function(n)) {
      Token* tt = resolve_function(vaf, ds, scope);
      // trace1("va_function no token?", t->name());
      assert(tt);
      E.push_back(tt);
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
  Symbolic_Expression tmp;
  assert(_owner);
  tmp.resolve_symbols(E, _owner);
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
Symbolic_Expression& Symbolic_Expression::operator=(Symbolic_Expression const& Proto)
{
  _scope = &CARD_LIST::card_list;
  { // reduce_copy(Proto);
    for (const_iterator i = Proto.begin(); i != Proto.end(); ++i) {
      (**i).stack_op(this);
    }
    if (is_empty()) {untested();
      assert(Proto.is_empty());
    }else{
    }
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
// vim:ts=8:sw=2:noet
