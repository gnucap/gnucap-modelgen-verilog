/*                       -*- C++ -*-
 * Copyright (C) 2023, 2024 Felix Salfelder
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
#include "mg_code.h"
#include "mg_module.h"
#include "mg_error.h"
#include "mg_token.h"
#include "mg_attrib.h"
#include "mg_options.h"
/*--------------------------------------------------------------------------*/
bool Statement::update()
{
  trace0("Statement::update");
//  if(dynamic_cast<Block*>(parent_stmt())){ untested();
//    incomplete();
//  }else{ untested();
//    incomplete();
//  }
  return false;
}
/*--------------------------------------------------------------------------*/
//Statement* Statement::parent_stmt()
//{ untested();
//  Block* b = scope();
//  if(auto x = dynamic_cast<Statement*>(b->owner())){ untested();
//    return x;
//  }else{ untested();
//    incomplete();
//    return NULL;
//  }
//}
/*--------------------------------------------------------------------------*/
bool Statement::is_reachable() const
{ untested();
  assert(scope());
  return scope()->is_reachable();
}
/*--------------------------------------------------------------------------*/
bool Statement::is_always() const
{
  assert(scope());
  return scope()->is_always();
}
/*--------------------------------------------------------------------------*/
bool Statement::is_never() const
{
  assert(scope());
  return scope()->is_never();
}
/*--------------------------------------------------------------------------*/
void Variable_Stmt::parse(CS& f)
{
  assert(owner());
  Module* mod = prechecked_cast<Module*>(owner());
  if(mod){
  }else{
  }

  char t = f.last_match()[0];
  if(t=='r') {
    _type = Data_Type_Real();
  }else if(t=='i') {
    _type = Data_Type_Int();
  }else{ untested();
    throw Exception_CS_("What type? " + t, f);
  }

//  if( f >> "[" ){ untested();
//    incomplete();
//  }else{ untested();
//  }

  _l.set_owner(this);
  _l.parse(f);

  attr.move_attributes(tag_t(&f), tag_t(this));
//  update();
} // Variable_Stmt::parse
/*--------------------------------------------------------------------------*/
void Variable_Decl::update()
{
  assert(_token);
  _data->clear();
  new_var_ref_(); // already declared
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::parse(CS& f)
{
  f >> ','; // ?? BUG.
  assert(owner());
  assert(!_data);
  assert(!_token);
  std::string name;

  name = f.ctos(",=(){};[]");
  trace1("Variable_Decl::parse", name);
  if(f.peek() == '['){
    f >> _dimensions;
  }else{
  }
  if(f >> "="){
    Expression init(f);
    resolve_symbols(init);
  }else{
  }

  // new_data();
  _data = new TData();
  _data->add_sens(this); // here? owner?
			 //
  _token = new Token_VAR_DECL(name, this, _data);
  trace1("variable decl", name);

  auto l = prechecked_cast<Variable_Stmt*>(owner());
  assert(l);
  assert(l->type());
  set_type(l->type());

  if(l->scope()->new_var_ref(_token)){
  }else{
    throw Exception_CS_("already declared", f);
  }

  attr.set_attributes(tag_t(_token)) = attr.attributes(tag_t(&f));
}
void Variable_Decl::dump(std::ostream& o)const
{
  o << name() << _dimensions;
  if(Expression_::size()){
    o << " = ";
    Expression_::dump(o);
  }else{
  }
  if(options().dump_annotate()){
    std::string nl;
    for(Dep const& d : deps().ddeps()){
      o << "// dep " << d.code_name();
      nl = "\n";
    }
    o << nl;
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::propagate_deps(Token_VAR_REF const& v)
{
  if(!v.data()){ untested();
    incomplete();
    return false;
  }else{
  }
  TData const& incoming = v.deps();
  assert(&deps() != &incoming);
  data().update(incoming);
  assert(deps().ddeps().size() >= incoming.ddeps().size());
  return false;
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::propagate_rdeps(RDeps const& r)
{
  incomplete();
  deps().rdeps().merge(r);
  return false;
}
/*--------------------------------------------------------------------------*/
bool Variable_Stmt::update()
{
  trace0("AnalogDeclareVars::update");
  for(Variable_Decl* d : *this){
    assert(d);
    d->update();
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool Variable_Stmt::is_used_in(Base const* b) const
{
  for(auto v : _l){
    if(v->is_used_in(b)){ untested();
      return true;
    }else{
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool SeqBlock::update()
{
  trace0("AnalogSeqBlock::update");
  bool ret = false;
  if(is_reachable()){
    for(auto i: *this){
      if(auto s = dynamic_cast<Statement*>(i)){
	ret |= s->update();
      }else{
      }
    }
  }else{
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::is_used_in(Base const*) const
{
  // return _token->is_used_in(b);
  // incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
void SeqBlock::merge_sens(Sensitivities const& s)
{
  if(_sens){ untested();
  }else{
    _sens = new Sensitivities;
  }
  _sens->merge(s);
}
/*--------------------------------------------------------------------------*/
void SeqBlock::set_sens(Base* s)
{
  if(_sens){
  }else{
    _sens = new Sensitivities;
  }
  _sens->add(s);
}
/*--------------------------------------------------------------------------*/
SeqBlock::~SeqBlock()
{
  delete _sens;
  _sens = NULL;
}
/*--------------------------------------------------------------------------*/
// void Lhs_Ref::parse()
static Token_VAR_REF* parse_variable(CS& f, Block* o)
{
  size_t here = f.cursor();
  std::string what;
  f >> what;
  trace1("parse_variable", what);
  Base* b = o->lookup(what);
  Token_VAR_REF* v = dynamic_cast<Token_VAR_REF*>(b);
  if(v){
    assert(f);
    // assert(v->data()); no. unreachable?
  }else if (b) { untested();
    f.reset_fail(here);
    trace1("not a variable", f.tail().substr(0,10));
    assert(0);
  }else{
    f.reset_fail(here);
    trace1("not found", f.tail().substr(0,10));
  }
  return v;
}
/*--------------------------------------------------------------------------*/
Assignment::Assignment(CS& f, Base* o)
{
  // unreachable(); // reached from for condition
  set_owner(o);
  parse(f);
} // Assignment::Assignment
/*--------------------------------------------------------------------------*/
void Assignment::parse(CS& f)
{
  assert(owner());
  assert(scope());
  size_t here = f.cursor();
  Token_VAR_REF* l = parse_variable(f, scope());
  // assert(l->name() == name());?

  if(f && f >> "="){
    _lhsref = l;
    assert(_lhsref);
    parse_rhs(f);
  }else{
    assert(!_lhsref);
    f.reset_fail(here);
  }

  if(options().optimize_unused() && !scope()->is_reachable()) {
  }else if(_lhsref) {
    assert(f);
    assert(l->data());
    assert(!_token);
    store_deps(Expression_::data());
    assert(_token);
    if(options().optimize_unused() && !scope()->is_reachable()) { untested();
      unreachable();
    }else{
      assert(_token->data());
      assert(_token->scope());
      _lhsref->propagate_deps(*_token);
      assert(_lhsref->name() == _token->name());
      trace2("parsedone", _token->name(), data().size());
    }
    assert(_token);
    assert(scope());
    scope()->new_var_ref(_token);
  }else{
    // possibly not a variable..
  }
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used_in(Base const* b) const
{ untested();
  if(auto p = dynamic_cast<Statement const*>(owner())) { untested();
    return p->is_used_in(b);
  }else{ untested();
  }
  // incomplete(); // later
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used() const
{ untested();
  assert(_token);
  return _token->is_used();
}
/*--------------------------------------------------------------------------*/
std::string Assignment::code_name() const
{ untested();
  assert(_lhsref);
  return _lhsref->code_name();
}
/*--------------------------------------------------------------------------*/
Data_Type const& Assignment::type() const
{
  //assert(_lhs->is_int() == _type.is_int());
  assert(_lhsref);
  return _lhsref->type();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_int() const
{
  return type().is_int();
}
/*--------------------------------------------------------------------------*/
bool Assignment::has_sensitivities()const
{
  return data().has_sensitivities();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
