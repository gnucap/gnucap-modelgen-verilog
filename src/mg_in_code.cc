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
#include "mg_in.h"
#include "mg_out.h"
#include "mg_storage.h"
/*--------------------------------------------------------------------------*/
bool Statement::set_used_in(Base const* b)
{
  return _rdeps.insert(b).second;
}
/*--------------------------------------------------------------------------*/
bool Statement::merge_rdeps(RDeps const& r)
{
  return _rdeps.merge(r);
}
/*--------------------------------------------------------------------------*/
void Statement::set_rdeps(TData const& )
{ untested();
      unreachable(); // still
 //  for(auto x : t.sensitivities()){ untested();
 //    if(auto b = dynamic_cast<Branch*>(x)){ untested();
 //      _rdeps.insert(b);
 //      // _rdeps.push_back(b);
 //    }else{ untested();
 //      _rdeps.insert(b);
 //      // incomplete(); // later
 //    }
 //  }
}
/*--------------------------------------------------------------------------*/
//Statement* Statement::parent_stmt()
//{ untested();
//  Block* b = scope();
//  if(auto x = dynamic_cast<Statement*>(b->owner())){ untested();
//    return x;
//  }else{ untested();
//    incomplete();
//    return nullptr;
//  }
//}
/*--------------------------------------------------------------------------*/
bool Statement::is_ctx_initial() const
{
  if(auto x = dynamic_cast<SeqBlock const*>(scope())) {
    return x->is_ctx_initial();
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
bool Statement::is_ctx_event() const
{
  if(auto x = dynamic_cast<SeqBlock const*>(scope())) {
    return x->is_ctx_event();
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
bool Statement::is_ctx_function() const
{
  if(auto x = dynamic_cast<SeqBlock const*>(scope())) {
    return x->is_ctx_function();
  }else{
    return false;
  }
}
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
bool is_file(Base const*);
bool Statement::propagate_rdep(Base const* b)
{
  bool new_dep = _rdeps.insert(b).second;
  Base* o = owner_();
  if(!new_dep){
  }else if(auto s = dynamic_cast<Statement*>(o)){
    s->propagate_rdep(b);
  }else if(dynamic_cast<Module*>(o)){
  }else if(is_file(o)) {
  }else{ untested();
    unreachable();
  }
  return new_dep;
}
/*--------------------------------------------------------------------------*/
void Variable_Stmt::parse(CS& f)
{
  size_t here = f.cursor();
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
    throw Exception_CS_("What type? " + to_string(t), f);
  }

//  if( f >> "[" ){ untested();
//    incomplete();
//  }else{ untested();
//  }

  _l.set_owner(this);
  try{
    trace1("Variable_Stmt::parse", f.tail().substr(0,30));
    _l.parse(f);
    assert(f);
  }catch (Exception_CS_ const& ee){
    throw ee;
  }catch (Exception const& e){
    // incomplete();
    f.reset_fail(here);
    throw Exception_CS_(e.message(), f);
  }

  attr.move_attributes(tag_t(&f), tag_t(this));

//  update();
} // Variable_Stmt::parse
/*--------------------------------------------------------------------------*/
void Variable_Decl::update()
{
  assert(_token_data);
  _token_data->clear();
  new_var_ref(); // already declared
}
/*--------------------------------------------------------------------------*/
void Assignment::new_token(std::string const& name)
{
  assert(!_token);
  assert(!_token_data);
  _token_data = new TData();
  _token_data->add_sens(this); // here? owner?
  _token = new Token_VAR_DECL(name, this, _token_data); // BUG. REF?
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::parse(CS& f)
{
  if(f >> ',') { untested();
    // ?? BUG.
  }else{
  }
  assert(owner());
  assert(!_token_data);
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

  new_token(name);
  trace1("variable decl", name);

  auto l = prechecked_cast<Variable_Stmt*>(owner());
  assert(l);
  assert(l->type());
  set_type(l->type());

  if(l->scope()->new_var_ref(&token())){
  }else{
    throw Exception_CS_("already declared", f);
  }

  assert(token().item()==this);

  if(attr.has_attributes(tag_t(&f))) {
    ATTRIB_LIST_p const& a = attr.attributes(tag_t(&f));
    attr.set_attributes(tag_t(&token())) = a;
    if(  a->operator[]("_state ") != "0"
      || a->operator[]("desc ") != "0"
      || a->operator[]("units ") != "0" ){
      set_state_override();
      if(a->operator[]("_nostate ") != "0" ){ untested();
	incomplete(); // conflicting request.
      }else if(a->operator[]("_common ") != "0" ){ untested();
	incomplete(); // conflicting request.
      }else{
      }
    }else if(a->operator[]("_common ") != "0" ){ untested();
      set_common_override();
    }else if(a->operator[]("_nostate ") != "0" ){ untested();
      set_temporary_override();
    }else{
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::dump(std::ostream& o)const
{
  o << name() << _dimensions;
  if(Expression_::size()){
    o << " = ";
    Expression_::dump(o);
  }else{
  }
  if(options().dump_annotate()){
    o << "//";
    if(is_state_var()){
      o << " *";
    }else if(is_temporary()){
      o << " +";
    }else if(is_common()){
      o << " -";
    }else{
    }
    if(data().ddeps().size()){
      o << " [";
      std::string sep;
      for(Dep const& d : data().ddeps()){
	o << sep << d.name();
	sep = ",";
      }
      if(is_common()){ untested();
	unreachable();
	o << "-BUG-";
      }else{
      }
      o << "]";
    }else{
    }
    o << '\n';
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool is_output_var(tag_t t)
{
  if(attr.has_attributes(t)) {
    ATTRIB_LIST_p const& a = attr.attributes(t);
    if(   a->operator[](std::string("desc")) != "0"
	||a->operator[](std::string("units")) != "0") {
      return true;
    }else{
    }
  }else{
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_output_var() const
{
  return :: is_output_var(tag_t(_lhsref));
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_state_var() const
{
  assert(_lhsref);
  return _lhsref->is_state_var();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_common() const
{
  assert(_lhsref);
  return _lhsref->is_common();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_temporary() const
{
  assert(_lhsref);
  return _lhsref->is_temporary();
}
/*--------------------------------------------------------------------------*/
TData const& Assignment::data()const
{
  if(_token){
    assert(_token->data() == _token_data);
  }else{ untested();
  }
  assert(_token_data);
  return *_token_data;
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::has_deps() const
{
  return _data.has_deps();
}
/*--------------------------------------------------------------------------*/
// obsolete
void Variable_Decl::new_data()
{
 if (_token_data){
//    delete _token_data;
//    _token_data = nullptr;
 }else{ untested();
 }
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::propagate_deps(Token_VAR_REF const& v)
{
  trace3("Variable_Decl::propagate_deps", type(), data().type(), v.name());
  assert(v.data());

  if(type() != Data_Type_Real()){
  }else{
  }
  TData const& incoming = v.deps();
  assert(&data() != &incoming);

  data().merge_sens(incoming);
  data().merge_flags(incoming);

  if(type().is_int()) {
  }else{
    data().merge_ddeps(incoming);
    assert(data().ddeps().size() >= incoming.ddeps().size());
  }

  token_data().merge(data());
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::propagate_rdeps(RDeps const& incoming)
{
  (void)incoming;
  // incomplete(); // . cf analogprocassign
//  _lhsref->use_var();
  return false;
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::propagate_rdeps(RDeps const& incoming)
{
//  _stt.use();
  return _rdeps.merge(incoming);
}
/*--------------------------------------------------------------------------*/
// bool Variable_Decl::is_used_in(Base const* b) const
// { untested();
//   incomplete();
//   return true;
// }
/*--------------------------------------------------------------------------*/
bool Variable_Stmt::update()
{
  //trace1("Variable_Stmt::update", _rdeps.size());
  for(Variable_Decl* d : *this){
    assert(d);
    d->update();
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool Variable_Stmt::is_used_in(Base const* b) const
{ untested();
  // incomplete

  if(Statement::is_used_in(b)) { untested();
    return true;
  }else{ untested();
    return true; // mg_strobe.0.gc.out 
    return false;
  }
}
/*--------------------------------------------------------------------------*/
    // f >> _variables; ?
static void parse_block_variables(CS& f, Variable_List_Collection& P)
{
  for (;;) {
    trace1("SeqBlock::parse loop", f.tail().substr(0,20));
    parse_attributes(f, &f);
    if( 0 // || ((f >> "parameter ") && (f >> _parameters))
	|| ((f >> "real ") && (f >> P))
	|| ((f >> "integer ") && (f >> P))) {
      if(f.peek() == ';') { untested();
	f.warn(bWARNING, "stray semicolon\n");
	f.skip();
      }else{
      }
    }else if (attr.has_attributes(tag_t(&f))) { untested();
      f.warn(bWARNING, "dangling attributes "
	   + attr.attributes(tag_t(&f))->string(tag_t(nullptr)));
      break;
    }else{
      // break if stuck?
      break;
    }
  }
}
/*--------------------------------------------------------------------------*/
void SwitchBlock::parse(CS&)
{
}
/*--------------------------------------------------------------------------*/
void SeqBlock::parse(CS& f)
{
  _variables.set_owner(this);
  if(f >> ":"){
    parse_identifier(f);
    parse_block_variables(f, _variables);
  }else{
  }

  // TODO? parse contents here.

//  variable_access().collect(this);
}
/*--------------------------------------------------------------------------*/
void SwitchBlock::dump(std::ostream& o)const
{
  Block::dump(o);
}
/*--------------------------------------------------------------------------*/
void SeqBlock::dump(std::ostream& o)const
{
  o << "begin";
  if(identifier() != "" || size()){
    if(identifier() != ""){
      o << " : " << identifier();
    }else{
    }
    if(options().dump_annotate()){
      if(is_ctx_function()){
	o << " // f";
      }else if(is_ctx_event()){
	o << " // e";
      }else if(is_ctx_initial()){
	o << " // i";
      }else if(is_always()){
	o << " // always";
      }else if(is_never()){
	o << " // never";
      }else{
      }
    }else{
    }
    o << "\n";
  }else{
    assert(!variables_().size());
    o << "\n";
  }
  {
    indent x;
    if(options().dump_annotate()){
      for(auto i : variables()){
	if(auto v = dynamic_cast<Token_VAR_REF const*>(i.second)){
	  o__ "// " << v->name() << " : " << v->deps().size() << "\n";
	}else if(dynamic_cast<Block const*>(i.second)){
	  // later.
	}else{
	  o__ "// " << i.first << "\n";
	}
      }
    }else{
    }
    for(auto* i : variables_()) {
      i->dump(o);
    }
    Block::dump(o);
  }
  o__ "end\n";
}
/*--------------------------------------------------------------------------*/
void SeqBlock::parse_identifier(CS& f)
{
  f >> _identifier;
  if(_identifier.to_string() == ""){ untested();
  }else if(scope()) {
    scope()->new_var_ref(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Base* SwitchBlock::lookup(std::string const& k, bool recurse)
{
  Base* v = Block::lookup(k, recurse);
  if(dynamic_cast<Token_VAR_REF*>(v)){
    // bug. not here.
   // _variable_access.push_use(r);
  }else{
  }
  return v;
}
/*--------------------------------------------------------------------------*/
Base* SeqBlock::lookup(std::string const& k, bool recurse)
{
  Base* v = Block::lookup(k, recurse);
  if(dynamic_cast<Token_VAR_REF*>(v)){
    // bug. not here.
   // _variable_access.push_use(r);
  }else{
  }
  return v;
}
/*--------------------------------------------------------------------------*/
bool SwitchBlock::update()
{
  trace1("AnalogSwitchBlock::update", is_reachable());
  int ret = 0;
  // assert(!_variables.size()); ?
  if(is_reachable()){
//      if(auto s = dynamic_cast<Statement*>(i)){
//	ret += s->update();
//	trace1("AnalogSeqBlock::update var", ret);
//      }else{ untested();
//	unreachable(); // comment? later..
//      }
//    }
    for(auto i: *this){
      if(auto s = dynamic_cast<Statement*>(i)){
	ret += s->update();
	trace2("AnalogSwitchBlock::update lst", ret, typeid(*s).name());
      }else{ untested();
	unreachable(); // comment? later..
      }
    }
  }else{
  }
  trace1("AnalogSwitchBlock::update done", ret);

  if(ret){
    // propagate variable deps to parent scope..
  }else{
    // propagate variable xs to parent scop
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
bool SeqBlock::update()
{
  trace1("SeqBlock::update", is_reachable());
  int ret = 0;
  if(is_reachable()){
    for(auto i: _variables){
      if(auto s = dynamic_cast<Statement*>(i)){
	ret += s->update();
	trace1("SeqBlock::update var", ret);
      }else{ untested();
	unreachable(); // comment? later..
      }
    }
    for(auto i: *this){
      if(auto s = dynamic_cast<Statement*>(i)){
	ret += s->update();
	trace2("SeqBlock::update lst", ret, typeid(*s).name());
      }else{ untested();
	unreachable(); // comment? later..
      }
    }
  }else{
  }
  trace1("SeqBlock::update done", ret);

  if(ret){
    // propagate variable deps to parent scope..
  }else{
    // propagate variable xs to parent scop
  }
  return ret;
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
  if(_sens){ untested();
  }else{
    _sens = new Sensitivities;
  }
  _sens->add(s);
}
/*--------------------------------------------------------------------------*/
void SeqBlock::new_variable_access()
{
  _variable_access = new Variable_Access;
}
/*--------------------------------------------------------------------------*/
void SeqBlock::delete_variable_access()
{
  delete _variable_access;
}
/*--------------------------------------------------------------------------*/
SwitchBlock::~SwitchBlock()
{
  delete _sens;
  _sens = nullptr;
}
/*--------------------------------------------------------------------------*/
SeqBlock::~SeqBlock()
{
  delete _sens;
  _sens = nullptr;
 //  delete _variables;
  delete_variable_access();
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
    if(owner()){
      assert(_token_data);
      _token_data->add_sens(owner());
    }else{ untested();
    }

    if(type() == Data_Type_Int()){
    }else{
    }
    {
      assert(_token->data());
      assert(_token->scope());
      trace2("Assignment::parse prop?", _token->name(), data().size());
      trace1("Assignment::parse prop2", typeid(_lhsref).name());
      _lhsref->propagate_deps(*_token);
      assert(_lhsref->name() == _token->name());
      trace2("parsedone", _token->name(), data().size());
    }
    assert(_token);
    assert(scope());
    scope()->new_var_ref(_token);
    if(auto sb = dynamic_cast<SeqBlock*>(scope())) {
      assert(_token->item() == this); // push _token instead?
      // sb->access_assign(this);
      assert(_lhsref);
      if(1){
      }else if(!decl_token()){ untested();
	incomplete();
      }else if(sb->is_ctx_initial()){ untested();
        if(rhs().is_constant()){ untested();
	}else{ untested();
	  // BUG: rdist not considered constant here..
	}
	sb->variable_access().init_variable(decl_token());
      }else if(sb->is_ctx_event()){ untested();
//	sb->variable_access().assign_variable(decl_token());
      }else{ untested();
	sb->variable_access().assign_variable(decl_token(), rhs().is_constant(),
	    scope()->is_always());
      }
    }else{
    }
  }else{
    // possibly not a variable..
  }
} // Assignment::parse
/*--------------------------------------------------------------------------*/
void Assignment::submit_variable_access(Variable_Access& va)const
{
  submit_variable_xs(va);
  auto sb = prechecked_cast<SeqBlock const*>(scope());
  if(sb->is_ctx_initial()){
    va.init_variable(decl_token());
  }else if(sb->is_ctx_event()){
    va.event_variable(decl_token());
  }else{
    va.assign_variable(decl_token(), rhs().is_constant(), true);
  }
}
/*--------------------------------------------------------------------------*/
Token_VAR_REF const* Assignment::decl_token() const
{
  if(!_lhsref){
    return _token;
  }else if(auto p = dynamic_cast<Assignment*>(_lhsref->mutable_item())) {
    assert(p!=this);
    return p->decl_token();
  }else{
    unreachable(); // af??
    // return _token;
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used_in(Base const* b) const
{
  return Expression_::is_used_in(b);
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_used() const
{ untested();
  assert(_token);
  unreachable();
  return false;
  // return _token->is_used();
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
Sensitivities const& Assignment::sensitivities()const
{
  return data().sensitivities();
}
/*--------------------------------------------------------------------------*/
bool Assignment::has_sensitivities()const
{
  return data().has_sensitivities();
}
/*--------------------------------------------------------------------------*/
bool Assignment::update(RDeps const* incoming)
{
  bool ret;
  assert(_lhsref);
  RDeps rdeps(_lhsref->rdeps());
  trace2("Assignment::update0", lhsname(), rdeps.size());
  if(incoming){
    trace2("Assignment::update", incoming->size(), Expression_::data().size());
    rdeps.merge(*incoming);
  }else{
  }

  ret = Expression_::update(&rdeps);

  assert(_token);
  assert(scope());
  trace3("Assignment::update", _lhsref->name(), _token->name(),  Expression_::data().size());
  if (store_deps(Expression_::data())) {
    trace3("Assignment::update0", _token->name(), _token->deps().size(), Expression_::data().size());
    // something new there.. pass it on.
    // TODO: only pass on what's new..
    assert(_lhsref);
    _lhsref->propagate_deps(*_token);
    assert(_token->operator->());
    ret = true;
    assert(_token->data());
  }else if(_lhsref->scope() != _token->scope()){
    ret = false;
    assert(_token->data());
    trace3("Assignment::update1", _token->name(), _token->deps().size(), Expression_::data().size());
    if(type().is_real()){
      assert(_token->deps().size() >= Expression_::data().size());
    }else{
    }
  }else{
    ret = false;
    assert(_token->data());
//    trace2("Assignment::update", _token->deps().size(), Expression_::deps().size());

    if(type().is_real()){
      assert(_token->deps().size() >= Expression_::data().size());
    }else{
    }
  }
  scope()->new_var_ref(_token); // needed in mg4_dep.2 ..
				//
 //  if(!r){ untested();
 //  }else if(auto s = dynamic_cast<Statement*>(owner())){ untested();
 //    ret |= s->propagate_rdeps(*r);
 //  }else{ untested();
 //  }
  trace2("Assignment::update3", _token->name(), ret);
  return ret;
}
/*--------------------------------------------------------------------------*/
bool Assignment::store_deps(TData const& d)
{
  trace1("Assignment::store_deps0", d.type());
  assert(_lhsref);
  size_t ii = 0;
  bool ret = false;

  if(options().optimize_unused() && !scope()->is_reachable()) { untested();
    _token = new Token_VAR_REF(_lhsref->name(), nullptr);
  }else{

    if(_token) {
      assert(_token_data);
      ii = _token_data->ddeps().size();
    }else{
      assert(!_token_data);
      // _token_data = d.clone(); // new TData();
      _token_data = new TData();
      _token_data->set_type(_lhsref->type());
      _token = new Token_VAR_REF(_lhsref->name(), this, _token_data);
      assert(_token->data());
      assert(_token->scope());
    }

    trace1("Assignment::store_deps", _token_data->type());
    _token_data->merge_sens(d);
    _token_data->merge_flags(d);
    if(type().is_int()){
    }else{
      _token_data->merge_ddeps(d);
    }

    assert(ii <= _token_data->ddeps().size());

    if(auto x = dynamic_cast<SeqBlock const*>(scope())) {
      if(x->has_sensitivities()) {
	_token_data->add_sens(*x->sensitivities());
      }else{
      }
    }else{
    }

    for(; ii < _token_data->ddeps().size(); ++ii) {
      ret = true;
      Dep const& dd = _token_data->ddeps()[ii];
     // trace2("inc_use2", (*dd)->code_name(), this);
      dd.set_used_in(this);
    }
//    assert(&deps() == _token_data);
    if(type().is_int()) {
    }else{
      assert(d.ddeps().size() <= _token_data->ddeps().size());
    }
  }

  trace1("Assignment::store_deps done", ret);
  return ret;
} // Assignment::store_deps
/*--------------------------------------------------------------------------*/
bool Assignment::propagate_deps(Token_VAR_REF const& from)
{
  TData const& d = from.deps();
  assert(from.scope());
  bool ret = false;
  if(type()) {
  }else{ untested();
    unreachable();
  }
  assert(_lhsref);
  trace2("Assignment::propagate_deps", from.name(), _lhsref->name());
  if(type().is_int()) {
  }else if(from.type().is_int()) { untested();
  }else if(from.scope() == scope()) {
    ret = _lhsref->propagate_deps(from);
  }else{
    ret |= store_deps(d);
    assert(_lhsref);
    trace0("Assignment::propagate_deps1 lhs?");
    ret |= _lhsref->propagate_deps(*_token);
  }
  // if(auto s = dynamic_cast<Statement*>(owner())) { untested();
  //   s->propagate_deps(from);
  // }else{ untested();
  //   assert(0);
  // }

  return ret;
}
/*--------------------------------------------------------------------------*/
void Assignment::dump(std::ostream& o) const
{
  if(_token){
    o << _token->name() << " = ";
    Expression_::dump(o);
  }else{
//    o << "/// unreachable?\n";
  }
}
/*--------------------------------------------------------------------------*/
Assignment::~Assignment()
{
  if(options().optimize_unused() && !scope()->is_reachable()) {
  }else if(_token_data){
    trace3("~Assignment", _token->name(), this, data().ddeps().size());
    try{
//      for(Dep d : data().ddeps()) { untested();
//	(*d)->unset_used_in(this);
//      }
    }catch(std::logic_error const& e){ untested();
      std::cerr << " logic error in Assignment " << _token->name() << ": ";
      std::cerr << e.what() << "\n";
      assert(0);
    }
  }else{
  }
  delete _token;
  _token = nullptr;
}
/*--------------------------------------------------------------------------*/
void Assignment::parse_rhs(CS& cmd)
{
  assert(owner());
  assert(scope());
  assert(dynamic_cast<Statement*>(owner()));
  trace1("Assignment::parse_rhs", cmd.tail().substr(0,10));
  Expression rhs(cmd);
  assert(Expression_::is_empty());

  assert(!_token_data);
  // assert(deps().ddeps().empty());
  //_rhs.set_owner(owner()); // this? AssignmentStatement?
  resolve_symbols(rhs);
  cmd.reset(cmd.cursor());
  trace1("Assignment::parse_rhs", bool(cmd));
}
/*--------------------------------------------------------------------------*/
RDeps const& Assignment::rdeps() const
{
  if(_lhsref){
//    trace2("Assignment::rdeps", lhsname(), _lhsref->rdeps().size());
    return _lhsref->rdeps();
  }else{ untested();
    static RDeps r;
    return r;
  }
}
/*--------------------------------------------------------------------------*/
void Variable_List_Collection::parse(CS& f)
{
  Collection<Variable_Stmt>::parse(f);
}
/*--------------------------------------------------------------------------*/
void Variable_List_Collection::dump(std::ostream& o)const
{
  for(auto const& i : *this){
    i->dump(o);
  }
  // Collection<Parameter_2_List>::dump(o);
}
/*--------------------------------------------------------------------------*/
bool Statement::propagate_rdeps(RDeps const& r)
{
  trace2("Statement::propagate_rdeps", typeid(*this).name(), r.size());
  assert(owner());
  auto s = prechecked_cast<Statement*>(owner_());
  assert(s);
  bool ret = false;
  for(auto n : r) {
    auto p = _rdeps.insert(n);
    if(p.second){
      ret = s->propagate_rdep(*p.first);
    }else{
    }
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
