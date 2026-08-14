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
#include "mg_.h" // Node BUG
/*--------------------------------------------------------------------------*/
#if 1
void Expression__::parse(CS& file)
{
  trace1("AnalogExpression::parse", file.tail().substr(0,100));

  {
    Expression rhs(file);
    file >> ","; // LiSt??
    assert(owner());
    // Expression_::set_owner(scope());
    //
    {
    resolve_symbols(rhs);
    }
  }
}
/*--------------------------------------------------------------------------*/
bool Expression__::is_true() const
{
  return ::is_true(expression());
}
/*--------------------------------------------------------------------------*/
bool Expression__::is_false() const
{
  return ::is_false(expression());
}
#endif
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
bool Statement::is_ctx_final() const
{
  if(auto x = dynamic_cast<SeqBlock const*>(scope())) {
    return x->is_ctx_final();
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

  if(mod){
  }else{
  }
  attr.move_attributes(tag_t(&f), tag_t(this));
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
{ untested();
  return _data.has_ddeps();
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
void Variable_Stmt::submit_variable_access(Variable_Access& va) const
{
  if(dynamic_cast<Module const*>(scope())) {
    for(auto i : _l) {
      assert(i);
      va.init_variable(i->decl_token());
    }
  }else{ untested();
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
{ untested();
}
/*--------------------------------------------------------------------------*/
void SeqBlock::parse_seq(CS& f)
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
      }else if(is_ctx_initial()){
	o << " // i";
      }else if(is_ctx_final()){
	o << " // F";
      }else if(is_ctx_event()){
	o << " // e";
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
	}else{ untested();
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
//      if(auto s = dynamic_cast<Statement*>(i)){ untested();
//	ret += s->update();
//	trace1("SeqBlock::update var", ret);
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
  }else{ untested();
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
{ untested();
  if(_sens){ untested();
  }else{ untested();
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
void SeqBlock::parse(CS& f)
{
  assert(owner());
  bool begin = f >> "begin ";
  if(begin){
    // f.reset(here);
    parse_seq(f); // _variables
  }else{
  }
  if(is_ctx_initial()){
  }else if(dynamic_cast<Module const*>(owner())) { untested();
    unreachable();
    set_always();
  }else if(dynamic_cast<Module const*>(scope())) {
    set_always();
  }else if(prechecked_cast<SeqBlock*>(scope())) {
  }else if(prechecked_cast<Statement const*>(owner())) {
  }else{ untested();
    unreachable();
  }
  while (begin) {
    if(f >> "end "){
      if(f.peek() == ';') {
	f.warn(bWARNING, "stray semicolon\n");
	f.skip();
      }else{
      }
      break;
    }else{
    }
    Base* s = parse_stmt(f, this);
    if(!s){
      throw Exception_CS_("bad analog block", f);
    }else{
      push_back(s);
    }
  }
  if(!begin){
    Base* b = parse_stmt_or_null(f, this);
    if(!f) {
      assert(!b);
    }else if(b){
      push_back(b);
    }else{
      delete b;
    }
  }else{
  }

  variable_access().collect(this);
} // SeqBlock::parse
/*--------------------------------------------------------------------------*/
// void Lhs_Ref::parse()
static Token_VAR_REF* parse_variable(CS& f, Block* o)
{
  size_t here = f.cursor();
  std::string what;
  f >> what;
  trace1("parse_variable", what);
  Base* b = o->lookup(what);
  if(dynamic_cast<Node*>(b)) { untested();
    incomplete();
    return nullptr;
  }else if(auto v = dynamic_cast<Token_VAR_REF*>(b)) {
    // assert(v->data()); no. unreachable?
    return v;
  }else if(auto n = dynamic_cast<Token_NODE*>(b)) { untested();
    // assert(v->data()); no. unreachable?
    return n;
  }else if (b) { untested();
    unreachable();
    f.reset_fail(here);
    trace1("not a variable", f.tail().substr(0,10));
    return nullptr;
  }else{
    f.reset_fail(here);
    trace1("not found", f.tail().substr(0,10));
    return nullptr;
  }
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
#if 0
  }else if(_lhsref && !l->data()) { untested();
    incomplete();
    assert(dynamic_cast<Token_NODE*>(l));
    assert(!_token);
    store_deps(Expression_::data());
    assert(_token);

    if(owner()){ untested();
      assert(_data);
      _data->add_sens(owner());
    }else{ untested();
    }
#endif
  }else if(_lhsref) {
    assert(f);
//    assert(l->data());
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
  Expression_::submit_variable_xs(va);
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
  }else if(dynamic_cast<Token_ARGUMENT const*>(_lhsref)) {
    // something af?
  }else if(dynamic_cast<Token_VAR_REF const*>(_lhsref)) {
    // something af?
  }else{ untested();
    assert(0);
    unreachable();
    // return _token;
  }
  return nullptr;
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
{
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

  assert(scope());
  trace3("Assignment::update", _lhsref->name(), _token->name(),  Expression_::data().size());
  if(!_token){ untested();
  }else if (store_deps(Expression_::data())) {
    trace3("Assignment::update0", _token->name(), _token->deps().size(), Expression_::data().size());
    // incomplete(); something new there.. pass it on.
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
  if(_token){
    scope()->new_var_ref(_token); // always needed?
  }else{ untested();
  }
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
  }else{ untested();
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
  assert(dynamic_cast<Statement*>(owner())
       ||dynamic_cast<Module*>(owner())); // NetAssignment
  trace1("Assignment::parse_rhs", cmd.tail().substr(0,10));
  Expression rhs(cmd);
  assert(Expression_::is_empty());

  assert(!_token_data);
  // assert(deps().ddeps().empty());
  //_rhs.set_owner(owner()); // this? AssignmentStatement?
  resolve_symbols(rhs);
  cmd.reset(cmd.cursor());
  trace2("Assignment::parse_rhs", bool(cmd), size());
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
CtrlStmt::~CtrlStmt()
{
  delete _block;
  _block = nullptr;
}
/*--------------------------------------------------------------------------*/
void InitialStmt::parse(CS& f)
{
  new_block();

  assert(owner());
  assert(_block);
  _block->set_owner(this);
  _block->set_ctx_initial();
  assert(_block->is_ctx_initial());

  if(f >> *_block){
    scope()->add_block(_block); //?
  }else{ untested();
    throw Exception_CS_("expecting statement", f);
  }
  assert(_block->is_ctx_initial());
}
/*--------------------------------------------------------------------------*/
void InitialStmt::dump(std::ostream& o) const
{
  o__ "initial ";
  assert(_block);
  _block->dump(o);
}
/*--------------------------------------------------------------------------*/
bool InitialStmt::update()
{
  bool ret = CtrlStmt::update();
  ret |= propagate_rdep(&tr_begin_tag);
  return ret;
}
/*--------------------------------------------------------------------------*/
bool WhileStmt::update()
{
  bool ret = false;
  while(true){
    trace0("WhileStmt::update");
    body().clear_vars();
    if (body().update()){
      ret = true;
      trace1("WhileStmt::update1", ret);
    }else{
      break;
    }
  }
  _cond.update(&rdeps()); // CtrlStmt?
  return // propagate_rdeps(_rdeps) ||
     CtrlStmt::update() || ret;
}
/*--------------------------------------------------------------------------*/
void WhileStmt::parse(CS& file)
{
  new_block();
  //_cond.set_owner(scope());
  _cond.set_owner(this);
  file >> "(" >> _cond >> ")";
  if(_cond.is_true()) {
    if(is_always()) {
      body().set_always();
    }else{ untested();
    }
  }else{
  }
  if(file >> ";"){
  }else{
    body().set_owner(this);
    file >> body();
    scope()->add_block(&body());
  }

  update();
}
/*--------------------------------------------------------------------------*/
void WhileStmt::dump(std::ostream& o)const
{
  o__ "while (" << _cond << ")";
  CtrlStmt::dump(o);
}
/*--------------------------------------------------------------------------*/
void WhileStmt::submit_variable_access(Variable_Access& va) const
{
  conditional().submit_variable_xs(va);
  Variable_Access a = body().variable_access();

  if(body().is_always()) {
    va &= a;
    va &= a;
  }else if(body().is_reachable()) {
    Variable_Access b;
    va &= a | b;
    va &= a;
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
System_Task::System_Task(CS& f, Block* o) : Statement()
{
  set_owner(o);
  parse(f);
}
/*--------------------------------------------------------------------------*/
void System_Task::parse(CS& f)
{
  assert(owner());
  _e.set_owner(this);
  Expression rhs(f);
  _e.resolve_symbols(rhs);
  f >> ";";

  assert(function());

  // add_rdeps(function()->rdeps()); /// TODO // 
  if(function()->has_tr_begin()){
    add_rdep(&tr_begin_tag);
  }else{
  }
  if(function()->has_tr_restore()){
    add_rdep(&tr_restore_tag);
  }else{
  }
  if(function()->has_tr_review()){
    add_rdep(&tr_eval_tag);
  }else{
  }
  if(function()->has_tr_review()){
    add_rdep(&tr_review_tag);
  }else{
  }
  if(function()->has_tr_accept()){
    add_rdep(&tr_accept_tag);
  }else{
  }
  if(function()->has_tr_advance()){
    add_rdep(&tr_advance_tag);
  }else{
  }
  if(function()->has_final()){
    add_rdep(&final_tag);
  }else{
  }

  // assert(rdeps());
  trace1("System_Task::parse2", rdeps().size());
  update(); // rdeps?
  trace0("System_Task::update1");
}
/*--------------------------------------------------------------------------*/
void System_Task::dump(std::ostream&o)const
{
  o__ _e << ";";

  if(options().dump_annotate()){
    dump_annotate(o, *this);
  }else{
  }
  o << "\n";
}
/*--------------------------------------------------------------------------*/
bool System_Task::is_used_in(Base const*b) const
{
  if(_e.is_used_in(b)){ untested();
    return true;
  }else if( Statement::is_used_in(b)) {
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
FUNCTION_ const* System_Task::function() const
{
  assert(_e.size());
  Token const* t = *_e.begin();
  assert(t);
  Token_CALL const* c = prechecked_cast<Token_CALL const*>(t);
  assert(c);
  return c->f();
}
/*--------------------------------------------------------------------------*/
bool System_Task::update()
{
  bool ret = _e.update(&_rdeps);
  return propagate_rdeps(_rdeps) || Statement::update() || ret;
}
/*--------------------------------------------------------------------------*/
void System_Task::submit_variable_access(Variable_Access& va) const
{
  expression().submit_variable_xs(va);
}
/*--------------------------------------------------------------------------*/
Base* SeqBlock::parse_stmt(CS& file, Block* owner) const
{
  size_t here = file.cursor();
  Base* a = parse_stmt_or_null(file, owner);
  if(file.stuck(&here)) {
    delete a;
    trace1("what?", file.tail().substr(0,20));
    throw Exception_CS_("what's this?", file);
    file.reset_fail(here);
    return nullptr;
  }else{
    return a;
  }
}
/*--------------------------------------------------------------------------*/
void SeqBlock::init_context(Statement* s)
{
  // reachability here?
  if(s->is_ctx_event()){
    set_ctx_event();
  }else if(s->is_ctx_function()){
    set_ctx_function();
  }else{
  }
  if(s->is_ctx_initial()){
    set_ctx_initial();
  }else{
  }
  if(s->is_ctx_final()){ untested();
    set_ctx_final();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void CtrlStmt::parse(CS& f)
{
  assert(_block);
  _block->set_owner(this);
  f >> *_block;
  scope()->add_block(_block);
}
/*--------------------------------------------------------------------------*/
void CtrlStmt::dump(std::ostream& o) const
{
  if(!*_block){
    o << ";\n";
  }else{
    o << " ";
    _block->dump(o);
  }
}
/*--------------------------------------------------------------------------*/
void CtrlStmt::submit_variable_access(Variable_Access& va) const
{
  va &= _block->variable_access();
}
/*--------------------------------------------------------------------------*/
void ConditionalStmt::parse(CS& f)
{
  new_block();
  assert(owner());
  //_cond.set_owner(owner());
  _cond.set_owner(this);
  body().set_owner(this);
  assert(!body().is_always());
  assert(!body().is_never());
  false_part().set_owner(this); // !!!

  if(f >> "(" >> _cond >> ")"){
  }else{ untested();
    throw Exception_CS_("expecting conditional", f);
  }

  {
    if(is_never()) {
      body().set_never();
      false_part().set_never();
    }else if(_cond.is_true()) {
      if(is_always()) {
	body().set_always();
      }else{
      }
      false_part().set_never();
    }else if(_cond.is_false()) {
      if(is_always()) {
	false_part().set_always();
      }else{
      }
      body().set_never();
    }else{
    }

    if(f >> body()){
      scope()->add_block(&body());
    }else{
      throw Exception_CS_("expecting statement", f);
    }
    size_t here = f.cursor();
    if(f >> "else "){
      f >> false_part();
      scope()->add_block(&false_part());
    }else{
      f.reset(here);
    }
  }
} // ConditionalStmt::parse
/*--------------------------------------------------------------------------*/
void ConditionalStmt::dump(std::ostream& o) const
{
  bool omit_true = !options().dump_unreachable() && _cond.is_false();
  bool omit_false = !options().dump_unreachable() && _cond.is_true();
  bool omit_cond = omit_true || omit_false;

  if(omit_cond) {
  }else{
    o__ "if (" << _cond << ") ";
  }

  if(omit_true) {
  }else if(omit_cond){
    o__ "";
    body().dump(o);
  }else{
    body().dump(o);
  }

  if(omit_false){
  }else if(false_part()){
    if(omit_true){
    }else{
      o__ "else ";
    }
    if(omit_cond){
      o__ "";
    }else{
    }
    false_part().dump(o);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void ConditionalStmt::submit_variable_access(Variable_Access& va) const
{
  cond().submit_variable_xs(va);
//  trace2("AnalogConditionalStmt::submit_variable_access",
//      false_part().is_reachable(), true_part().is_reachable());

  if(false_part().is_reachable() && true_part().is_reachable()) {
    Variable_Access a = false_part().variable_access() | true_part().variable_access();
    va &= a;
  }else{
    if(true_part().is_reachable()) {
      va &= true_part().variable_access();
    }else if(false_part().is_reachable()) {
      va &= false_part().variable_access();
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
bool ConditionalStmt::is_used_in(Base const* b) const
{
  if (_cond.is_used_in(b)){ untested();
    return true;
  }else{
    return CtrlStmt::is_used_in(b);
  }
}
/*--------------------------------------------------------------------------*/
bool ConditionalStmt::update()
{
  bool ret = false_part().update();
  _cond.update(&rdeps());
  return CtrlStmt::update() || ret;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
