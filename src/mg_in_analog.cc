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
#include "mg_analog.h"
#include "mg_out.h"
#include "mg_in.h"
#include "mg_error.h"
#include "mg_options.h"
#include "mg_discipline.h"
#include "m_tokens.h"
#include <e_cardlist.h> // TODO: really?
#include <u_opt.h>
#include "globals.h"
/*--------------------------------------------------------------------------*/

/* So analog initial block shall not contain the fol-
lowing statements:
—statements with access functions or analog operators;
—contribution statements;
—event control statements.
*/

// A module definition may have multiple analog blocks. The simulator shall internally combine the multiple
// analog blocks into a single analog block in the order that the analog blocks appear in the module description.
// In other words, the analog blocks shall execute in the order that they are specified in the module.
//
//
// 6.2
// A module definition may have multiple analog blocks. The behavior of multiple analog blocks shall be
// defined by assuming that the multiple analog blocks internally combine into a single analog block in the
// order that the analog blocks appear in the module description. In other words, they are concatenated in the
// order they appear in the module. Concurrent evaluation of the multiple analog blocks is implementation
// dependent as long as the behavior in that case is similar to what would happen if they had been concate-
// nated.
//
//
/*
+ analog_construct ::=
+ analog analog_statement
- | analog initial analog_function_statement// from A.6.2
+ analog_statement ::=
- { attribute_instance } analog_loop_generate_statement
- | { attribute_instance } analog_loop_statement
+ | { attribute_instance } analog_case_statement
+ | { attribute_instance } analog_conditional_statement
+ | { attribute_instance } analog_procedural_assignment
+ | { attribute_instance } analog_seq_block
- | { attribute_instance } analog_system_task_enable
+ | { attribute_instance } contribution_statement
+ | { attribute_instance } indirect_contribution_statement
- | { attribute_instance } analog_event_control_statement// from A.6.4
+ analog_statement_or_null ::=
+ analog_statement
+ | { attribute_instance } ;
- Syntax 5-1—Syntax for analog procedural block
*/
/*--------------------------------------------------------------------------*/
/*
- analog_filter_function_call ::=               // from A.8.2
-   ddt ( analog_expression [ , abstol_expression ] )
- | ddx ( analog_expression , branch_probe_function_call )
- | idt ( analog_expression [ , analog_expression [ , analog_expression [ , abstol_expression ] ] ] )
- | idtmod ( analog_expression [ , analog_expression [ , analog_expression [ , analog_expression
- [ , abstol_expression ] ] ] ] )
- | absdelay ( analog_expression , analog_expression [ , constant_expression ] )
- | transition ( analog_expression [ , analog_expression [ , analog_expression
- [ , analog_expression [ , constant_expression ] ] ] ] )
- | slew ( analog_expression [ , analog_expression [ , analog_expression ] ] )
- | last_crossing ( analog_expression [ , analog_expression ] )
- | limexp ( analog_expression )
- | laplace_filter_name ( analog_expression , [ analog_filter_function_arg ] ,
- [ analog_filter_function_arg ] [ , constant_expression ] )
- | zi_filter_name ( analog_expression , [ analog_filter_function_arg ] ,
- [ analog_filter_function_arg ] , constant_expression
- [ , analog_expression [ , constant_expression ] ] )
*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/* The potential and flow access functions can also be used to contribute to the potential or flow of a
named or unnamed branch. The example below demonstrates the potential access functions being used
to contribute to a branch and the flow and potential access functions being used to probe branches.
Note V and I cannot be used as access functions because there are parameters called V and I declared in the
module. */
/*--------------------------------------------------------------------------*/
static Base* parse_cond(CS& file, Block* o)
{
  return new AnalogConditionalStmt(o, file);
}
/*--------------------------------------------------------------------------*/
static Base* parse_switch(CS& file, Block* o)
{
  assert(o);
  return new AnalogSwitchStmt(o, file);
}
/*--------------------------------------------------------------------------*/
static Base* new_evt_ctl_stmt(CS& file, Block* o)
{
  auto cb = new AnalogEvtCtlStmt();
  cb->set_owner(o);
  try{
    file >> *cb;
    return cb;
  }catch(Exception_No_Match const& e){ untested();
    delete cb;
    throw e;
  }
}
/*--------------------------------------------------------------------------*/
static Base* pArse_seq(CS& f, Block* owner)
{
  return new AnalogSeqBlock(f, owner);
}
/*--------------------------------------------------------------------------*/
static Base* parse_int(CS& f, Block* o)
{
  trace1("AnalogBlock::parse int", f.tail().substr(0,10));
  return new ListOfBlockIntIdentifiers(f, o);
}
/*--------------------------------------------------------------------------*/
static Base* parse_system_task(CS& f, Block* o)
{
  trace1("parse_system_task", f.tail().substr(0,10));
  size_t here = f.cursor();
  std::string name;
  f >> name;
  f.reset(here);

  if(dynamic_cast<MGVAMS_TASK const*>(function_dispatcher[name])){
    try{
      return new System_Task(f, o);
    }catch(Exception const& e){ untested();
      return NULL;
    }
  }else{
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
System_Task::System_Task(CS& f, Block* o)
{
  set_owner(o);
  parse(f);
}
/*--------------------------------------------------------------------------*/
void System_Task::parse(CS& f)
{
  assert(owner());
  _e.set_owner(owner());
  trace1("====", f.tail().substr(0,10));
  f >> _e;
  trace1("/====", f.tail().substr(0,10));
  f >> ";";
}
/*--------------------------------------------------------------------------*/
void System_Task::dump(std::ostream&o)const
{
  o__ _e << ";\n";
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
    assert(v->data());
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
Assignment::Assignment(CS& f, Block* o)
{
  // unreachable(); // reached from for condition
  set_owner(o);
  parse(f);
} // Assignment::Assignment
/*--------------------------------------------------------------------------*/
void Assignment::parse(CS& f)
{
  assert(owner());
  size_t here = f.cursor();
  Token_VAR_REF* l = parse_variable(f, owner());
  // assert(l->name() == name());?

  if(f && f >> "="){
    _lhsref = l;
    assert(_lhsref);
    parse_rhs(f);

  }else{
    assert(!_lhsref);
    f.reset_fail(here);
//    throw Exception_CS_("no assignment", f);
  }

  if(_lhsref) {
    assert(l->data());
    assert(!_token);
    store_deps(_rhs.deps());
    assert(_token);
    if(options().optimize_unused() && !is_reachable()) {
    }else{
      assert(_token->data());
//      trace1("parse prop", name());
      _lhsref->propagate_deps(*_token);
      // _name = l->name();
      assert(_lhsref->name() == _token->name());
    }
    assert(_token);
    owner()->new_var_ref(_token);
    // trace2("parsedone", _token->name(), data().size());
  }else{
    // possibly not a variable..
  }
}
/*--------------------------------------------------------------------------*/
// void Assignment::parse(CS& f)?
void AnalogProceduralAssignment::parse(CS& f)
{
//  assert(_a.owner());
  size_t here = f.cursor();

//  trace2("Proc assignment?", f.tail().substr(0,20), _a.owner());
  std::string what;
  f >> what;
//  f >> what;
  if(what == ""){ untested();
    f.reset_fail(here);
    throw Exception_No_Match("need name");
  }else{
    f.reset(here);
  }
  if(f >> _a){
    if(f >> ";"){
    }else{ untested();
      f.warn(bWARNING, "missing semicolon?");
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
template<class A>
void dump_annotate(std::ostream& o, A const& _a)
{
  if(!_a.is_reachable()){
    o << " // --\n";
  }else{
    if(_a.data().ddeps().size()){
      o << " //";
    }else if(_a.has_sensitivities()){
      o << " //";
    }else{
    }
    for(const Dep& d : _a.data().ddeps()) {
      o << " dep: ";
      o << d->code_name();
    }
    if(_a.has_sensitivities()){
      o << " s";
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void AnalogProceduralAssignment::dump(std::ostream& o)const
{
  o__ "";
  _a.dump(o);
  o << ";";
  if(options().dump_annotate()){
    dump_annotate(o, _a);
  }else{
  }
  o << "\n";
}
/*--------------------------------------------------------------------------*/
static Base* parse_proc_assignment(CS& f, Block* o)
{
  f.skipbl();
  trace1("parse_proc_assignment", f.tail().substr(0,30));
  try{
    auto n = new AnalogProceduralAssignment(f, o);
    if(f){
      return n;
    }else{
      delete n;
    }
  }catch(Exception_No_Match const&){ untested();
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
AnalogProceduralAssignment::AnalogProceduralAssignment(CS& file, Block* o)
  : AnalogStmt()
{
  _a.set_owner(o);
  parse(file);
}
/*--------------------------------------------------------------------------*/
static Base* parse_contribution(CS& f, Block* owner)
{
  try{
    return new Contribution(f, owner);
  }catch(Exception_No_Match const& x){
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
static Base* parse_analog_stmt_or_null(CS& file, Block* scope)
{
  size_t here = file.cursor();
  assert(scope);
  Base* ret = NULL;

  trace1("parse_analog_stmt_or_null", file.tail().substr(0,30));
//  size_t here = file.cursor();
  ONE_OF	// module_item
    || (file >> ";")
    || ((file >> "begin") && (ret = pArse_seq(file, scope)))
    || ((file >> "real ") && (ret = new AnalogRealDecl(file, scope)))
    || ((file >> "integer ") && (ret = parse_int(file, scope)))
    || ((file >> "if ") && (ret = parse_cond(file, scope)))
    || ((file >> "case ") && (ret = parse_switch(file, scope)))
    || ((file >> "while ") && (ret = new AnalogWhileStmt(file, scope)))
    || ((file >> "for ") && (ret = new AnalogForStmt(file, scope)))
    || ((file >> "@ ") && (ret = new_evt_ctl_stmt(file, scope)))
    || (ret = parse_proc_assignment(file, scope))
    || (ret = parse_system_task(file, scope))
    || (ret = parse_contribution(file, scope))
    ;

  trace1("parse_analog_stmt_or_null?", file.tail().substr(0,30));
  if(file.stuck(&here)) {
    file.reset_fail(here);
  }else{
  }

  return ret;
}
/*--------------------------------------------------------------------------*/
static Base* parse_analog_stmt(CS& file, Block* owner)
{
  size_t here = file.cursor();
  Base* a = parse_analog_stmt_or_null(file, owner);
  if(file.stuck(&here)) {
    throw Exception_CS_("what's this?", file);
    file.reset_fail(here);
    return NULL;
  }else{
    return a;
  }
}
/*--------------------------------------------------------------------------*/
void AnalogConditionalStmt::parse(CS& f)
{
  assert(owner());
  _cond.set_owner(owner());
  _body.set_owner(this);
  assert(!_body.is_always());
  assert(!_body.is_never());
  _false_part.set_owner(this);

  if(f >> "(" >> _cond >> ")"){
  }else{ untested();
    throw Exception_CS_("expecting conditional", f);
  }

  {
    if(owner()->is_never()) {
      _body.set_never();
      _false_part.set_never();
    }else if(_cond.is_true()) {
      if(owner()->is_always()) {
	_body.set_always();
      }else{untested();
      }
      _false_part.set_never();
    }else if(_cond.is_false()) {
      if(owner()->is_always()) {
	_false_part.set_always();
      }else{untested();
      }
      _body.set_never();
    }else{
    }

    if(f >> _body){
    }else{
      throw Exception_CS_("expecting statement", f);
    }
    size_t here = f.cursor();
    if(f >> "else "){
      f >> _false_part;
    }else{
      f.reset(here);
    }
  }
}
/*--------------------------------------------------------------------------*/
AnalogConditionalStmt::AnalogConditionalStmt(Block* o, CS& file)
{
  set_owner(o);
  parse(file);
}
/*--------------------------------------------------------------------------*/
void AnalogConditionalStmt::dump(std::ostream& o)const
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
    _body.dump(o);
  }else{
    _body.dump(o);
  }

  if(omit_false){
  }else if(_false_part){
    if(omit_true){
    }else{
      o__ "else ";
    }
    if(omit_cond){
      o__ "";
    }else{
    }
    _false_part.dump(o);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void AnalogCtrlStmt::parse(CS& f)
{
  _body.set_owner(this);
  f >> _body;
}
/*--------------------------------------------------------------------------*/
void AnalogCtrlStmt::dump(std::ostream& o)const
{
  if(!_body){
    o << ";\n";
  }else{
    o << " ";
    _body.dump(o);
  }
}
/*--------------------------------------------------------------------------*/
AnalogWhileStmt::AnalogWhileStmt(CS& file, Block* o)
{
  set_owner(o);
  parse(file);
}
/*--------------------------------------------------------------------------*/
bool AnalogWhileStmt::update()
{
  bool ret = false;
  while(true){
    _body.clear_vars();
    if (_body.update()){
      ret = true;
    }else{
      break;
    }
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
void AnalogWhileStmt::parse(CS& file)
{
  _cond.set_owner(scope());
  file >> "(" >> _cond >> ")";
  if(file >> ";"){
  }else{
    _body.set_owner(this);
    file >> _body;
  }

  update();
}
/*--------------------------------------------------------------------------*/
void AnalogWhileStmt::dump(std::ostream& o)const
{
  o__ "while (" << _cond << ")";
  AnalogCtrlStmt::dump(o);
}
/*--------------------------------------------------------------------------*/
static Assignment* parse_assignment_or_null(CS& f, Block* owner)
{
  auto a = new Assignment(f, owner);
  if(f) {
  }else{
    delete a;
    a = NULL;
  }
  return a;
}
/*--------------------------------------------------------------------------*/
bool AnalogForStmt::update()
{
  auto init_ = dynamic_cast<Assignment*>(_init);
  auto tail_ = dynamic_cast<Assignment*>(_tail);
  bool ret = false;

  while(true){
    if ( init_ && init_->update() ){ untested();
      ret = true;
    }else if (_body.update()){ untested();
      ret = true;
    }else if ( tail_ && tail_->update() ) { untested();
      ret = true;
    }else{
      break;
    }
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
void AnalogForStmt::parse(CS& f)
{
  assert(owner());
  _cond.set_owner(owner());
  f >> "(";
  Assignment* init = parse_assignment_or_null(f, owner());
  _init = init;
  if(!f){
    delete _init;
    _init = NULL;
  }else{
  }
  f >> ";";
  f >> _cond;
  f >> ";";

  _tail = parse_assignment_or_null(f, owner());

  f >> ")";
  if(f >> ";"){ untested();
  }else{
    _body.set_owner(this); // needed?
    f >> _body;
  }

  update();
}
/*--------------------------------------------------------------------------*/
bool AnalogSeqBlock::update()
{
  bool ret = false;
  if(is_reachable()){
    for(auto i: *this){
      if(auto s = dynamic_cast<AnalogStmt*>(i)){
	ret |= s->update();
      }else{ untested();
      }
    }
  }else{ untested();
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
bool AnalogProceduralAssignment::update()
{
  return _a.update();
}
/*--------------------------------------------------------------------------*/
void AnalogForStmt::dump(std::ostream& o)const
{
  o__ "for (" ;
  if(has_init()){
    o << init();
  }else{
  }
  o << "; " << _cond << "; ";
  if(has_tail()){
    o << tail();
  }else{
  }
  o << ")";
  AnalogCtrlStmt::dump(o);
}
/*--------------------------------------------------------------------------*/
void CaseGen::calc_reach(Expression const& ctrl)
{
  Expression_ result;

  if(!ctrl.size()){ untested();
    incomplete();
  }else if(_cond && _cond->size()){
    bool all_never = true;
    for(auto const& j : *_cond){
      trace2("==", ctrl.back()->name(), j->expression().back()->name());
      Token_BINOP_ b("==", ctrl.back(), j->expression().back(), NULL);
      b.stack_op(&result);
      b.pop();
      assert(result.size());

      if(is_false(result)) {
      }else{
	all_never = false;
      }
      if(is_true(result)) {
	_body.set_always();
	break;
      }else{
      }
    }
    if(all_never){
      set_never();
    }else{
    }
  }else if(_cond){ untested();
    set_never();
  }else{
    // default?
  }
}
/*--------------------------------------------------------------------------*/
void CaseGen::parse(CS&)
{ untested();
  assert(0);
}
/*--------------------------------------------------------------------------*/
void CaseGen::dump(std::ostream& o)const
{
  if(!options().dump_unreachable() && is_never()){
  }else{
    if(_cond){
      o__ *_cond;
  //    o << " :";
    }else{
      o__ "default:";
    }
//    o << ' ';
    AnalogCtrlStmt::dump(o);
  }
}
/*--------------------------------------------------------------------------*/
CaseGen::CaseGen(CS& f, Block* o, Expression const& ctrl, bool have_r, bool have_a)
{
  trace2("CaseGen", f.tail().substr(0,20), have_a);
  assert(!dynamic_cast<Module const*>(o));
  AnalogConstExpressionList* c = new AnalogConstExpressionList();
  assert(o);
  c->set_owner(o);
  set_owner(o);
  assert(owner());
  trace2("CaseGen", is_always(), is_never());

  if (f >> "default"){
    delete c;
    f >> ":"; // is optional..
    size_t here = f.cursor();
    if(f >> "endcase") {
      f.reset_fail(here);
    }else{
      f.reset(here);
    }
  }else if(f >> *c) {
    trace1("CaseGen2", f.tail().substr(0,20));
    _cond = c;
  }else{ untested();
    delete c;
    throw Exception_CS_("bad switch statement", f);
  }

  if(have_a) {
    set_never();
    assert(is_never());
  }else if(!have_r){
    // default?
    calc_reach(ctrl);
  }else{
    calc_reach(ctrl);
  }
  if(f){
    AnalogCtrlStmt::parse(f);
    if(f){
    }else{ untested();
      incomplete();
    }
  }else{
  }
  trace2("CaseGen done", have_a, is_never());

  assert(owner());
}
/*--------------------------------------------------------------------------*/
AnalogSwitchStmt::AnalogSwitchStmt(Block* o, CS& file)
{
  set_owner(o);
  parse(file);
}
/*--------------------------------------------------------------------------*/
void AnalogSwitchStmt::parse(CS& f)
{
  assert(owner());
  assert(!dynamic_cast<Module const*>(owner()));
  _ctrl.set_owner(owner());
  _body.set_owner(this);

  f >> "(" >> _ctrl >> ")";
  CaseGen* def = NULL;
  bool have_reachable = false;
  bool have_always = false;

  while(true){
    if(f >> "endcase"){
      break;
    }else{
      if(def){
	// default is out of order. reachability messed up.
	// CS.warn ...
	// use update() to fix reachability?
      }else{
      }

      size_t here = f.cursor();
      CaseGen* g = new CaseGen(f, &_body, _ctrl.expression(),
	  have_reachable, have_always);
//      g->set_owner(owner());
//      g->set_ctrl(&_ctrl.expression());
//      f >> *g;
//
      if(g->is_always()){
	have_always = true;
      }else if(g->is_reachable()){
	have_reachable = true;
      }else{
      }

      if(!g->is_default()) {
      }else if(def){
	f.reset_fail(here);
	delete g;
	throw Exception_CS_("multiple default: not allowed", f);
      }else{
	def = g;
      }

      _body.push_back(g);
    }
  }
}
/*--------------------------------------------------------------------------*/
void AnalogSwitchStmt::dump(std::ostream& o)const
{
  o__ "case (" << _ctrl << ")\n";

  {
    indent x;
    o << _body;
  }
  o__ "endcase\n";
}
/*--------------------------------------------------------------------------*/
void AnalogConstruct::parse(CS& f)
{
  assert(owner());
  assert(!_stmt);
  auto ab = new AnalogCtrlBlock(f, this);
  // ab->update();
  _stmt = ab;
}
/*--------------------------------------------------------------------------*/
void AnalogSeqBlock::parse(CS& f)
{
  assert(owner());
  if(f >> ":"){
    parse_identifier(f);
  }else{
  }
  if(dynamic_cast<Module const*>(owner())) {
    set_always();
  }else if(dynamic_cast<Module const*>(scope())) {
    set_always();
  }else{
  }
  for (;;) {
    if(f >> "end "){
      if(f.peek() == ';') {
	f.warn(bWARNING, "stray semicolon\n");
	f.skip();
      }else{
      }
      break;
    }else{
    }
    Base* s = parse_analog_stmt(f, this);
    if(!s){ untested();
      throw Exception_CS_("bad analog block", f);
    }else{
      push_back(s);
    }
  }
}
/*--------------------------------------------------------------------------*/
//void AnalogSeqBlock::clear_vars()
//{
//  _block.clear_vars();
//}
/*--------------------------------------------------------------------------*/
#if 1
void AnalogCtrlBlock::set_owner(Statement* st)
{
  Block* o = st->scope();
  if(auto x = dynamic_cast<SeqBlock const*>(o)) {
    if(x->has_sensitivities()){
      merge_sens(*x->sensitivities());
    }else{
    }
  }else{
  }
  set_owner_raw(st);
//  Statement::set_owner(st);
  assert(owner());
}
#endif
/*--------------------------------------------------------------------------*/
void AnalogCtrlBlock::parse(CS& f)
{
  if(dynamic_cast<Module const*>(owner())) {
    set_always();
  }else{
  }
  // _block.set_owner(owner());
  assert(owner());
//  size_t here = f.cursor();
  if(f >> "begin"){
    AnalogSeqBlock::parse(f);
  }else{
    Base* b = parse_analog_stmt_or_null(f, this);
    if(!f) {
      assert(!b);
    }else if(b){
      push_back(b);
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
// Variable_Decl::parse?
void BlockVarIdentifier::parse(CS& file)
{
  assert(owner());
  assert(!_data);
  assert(!_token);
  std::string name;
  file >> name;

  _data = new TData();
  _token = new Token_VAR_REF(name, this, _data);
  trace1("variable decl", name);
  owner()->new_var_ref(_token);
}
/*--------------------------------------------------------------------------*/
void BlockVarIdentifier::dump(std::ostream& o)const
{
  o << name();
}
/*--------------------------------------------------------------------------*/
void ListOfBlockIntIdentifiers::dump(std::ostream& o) const
{
  o__ "integer ";
  LiSt<BlockVarIdentifier, '\0', ',', ';'>::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void ListOfBlockRealIdentifiers::dump(std::ostream& o) const
{
  o__ "real ";
  LiSt<BlockVarIdentifier, '\0', ',', ';'>::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void AnalogConstruct::dump(std::ostream& o)const
{
  o__ "analog ";
  _stmt->dump(o);
}
/*--------------------------------------------------------------------------*/
void Assignment::dump(std::ostream& o) const
{
  if(_token){
    o << _token->name() << " = " << _rhs;
  }else{ untested();
    o << "/// unreachable?\n";
  }
}
/*--------------------------------------------------------------------------*/
Assignment::~Assignment()
{
  if(_token){
    // trace3("~Assignment", _token->name(), this, data().ddeps().size());
  }else{
  }
  if(options().optimize_unused() && !owner()->is_reachable()) {
  }else if(_data){
    try{
      for(Dep d : data().ddeps()) {
	(*d)->unset_used_in(this);
      }
    }catch(std::logic_error const& e){ untested();
      std::cerr << " logic error in Assignment " << _token->name() << ": ";
      std::cerr << e.what() << "\n";
      assert(0);
    }
  }else{
  }
  delete _token;
  _token = NULL;
}
/*--------------------------------------------------------------------------*/
void Assignment::parse_rhs(CS& cmd)
{
  assert(owner());
  trace1("Assignment::parse_rhs", cmd.tail().substr(0,10));
  Expression rhs(cmd);
  assert(_rhs.is_empty());

  assert(owner());
  assert(!_data);
  // assert(deps().ddeps().empty());
  _rhs.set_owner(owner()); // this? AssignmentStatement?
  _rhs.resolve_symbols(rhs);
  cmd.reset(cmd.cursor());
  trace1("Assignment::parse_rhs", bool(cmd));
}
/*--------------------------------------------------------------------------*/
Branch_Ref parse_branch(Block* owner, CS& f)
{
  size_t here = f.cursor();
  if(f >> "("){
  }else{
    f.reset_fail(here);
    return Branch_Ref();
    throw Exception_No_Match("not a branch");
  }
  std::string pp = f.ctos(",)");
  std::string pn = f.ctos(",)");
  f >> ")";

  assert(owner);
  Branch_Ref b = owner->new_branch(pp, pn);
  // assert(b._br);
  return b;
}
/*--------------------------------------------------------------------------*/
// in_module?
void Branch_Ref::parse(CS& f)
{ untested();
  assert(!_br);

  trace1("Branch_Ref::parse", f.tail().substr(0,10));
  if(f >> "("){ untested();
  }else{ untested();
    throw Exception_No_Match("not a branch");
  }
  std::string pp = f.ctos(",)");
  std::string pn = f.ctos(",)");
  f >> ")";

  assert(0); // incomplete. parse Branch_Decl?
#if 0
  assert(owner());
  trace3("Branch_Ref::parse", pp, pn, _br);
  Branch_Ref b;
  b = owner()->new_branch(pp, pn);
  assert(b._br);
  *this = b;
#endif
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::dump(std::ostream& o)const
{
  assert(_br);
  if(auto x = dynamic_cast<Named_Branch const*>(_br)) {
    o << "(" << x->name() << ")";
  }else if(_br->n()->is_ground()) {
    o << "(" << pname() << ")";
  }else{
    o << "(" << pname() << ", " << nname() << ")";
  }
}
/*--------------------------------------------------------------------------*/
std::string const& Branch_Ref::pname() const
{
  assert(_br);
  assert(_br);
  if(_r){
    assert(_br->n());
    return _br->n()->name();
  }else{
    assert(_br->p());
    return _br->p()->name();
  }
}
/*--------------------------------------------------------------------------*/
std::string const& Branch_Ref::nname() const
{
  assert(_br);
  if(_r){
    assert(_br->n());
    return _br->p()->name();
  }else{
    assert(_br->p());
    return _br->n()->name();
  }
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::set_used_in(Base const* b) const
{
  return _br->set_used_in(b);
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::unset_used_in(Base const* b) const
{
  return _br->unset_used_in(b);
}
/*--------------------------------------------------------------------------*/
void Contribution::parse(CS& cmd)
{
  size_t here = cmd.cursor();
  cmd >> _name;

  _branch = parse_branch(owner(), cmd);

  if(_branch){
  }else{
    cmd.reset(here);
    trace2("not a Contribution", _name, cmd.tail().substr(0,20));
    throw Exception_No_Match("not a contribution");
  }
  trace1("Contribution", _name);
  if(_branch->deps().is_linear()){
  }else{
  }

  if(_branch.is_reversed()){
  }else{
  }

  if(!_branch->discipline()) {
    cmd.reset(here);
    throw Exception_CS_("bad discipline.", cmd);
  }else{
  }
  Discipline const* disc = _branch->discipline();
  assert(disc);

  if(disc->flow() &&
      (_name == "flow" || disc->flow()->access().to_string() == _name)) {
    _nature = disc->flow();
  }else if(disc->potential() &&
      (_name == "potential" || disc->potential()->access().to_string() == _name)) {
    _nature = disc->potential();
    set_pot_contrib();
  }else{
    throw Exception_CS_("bad access", cmd);
  }

  if(cmd >> "<+"){
    set_direct();
  }else if(cmd >> ":"){
    set_direct(false);
    // std::string xs;
    // Branch_Ref bb;
    // bb.set_owner(owner());
    // trace1("bb?", cmd.tail().substr(0,20));
    // cmd >> xs >> bb >> "==";
  }else{
    throw Exception_CS_("expecting \"<+\" or \":\"", cmd);
  }


  { // cmd >> _rhs;
    assert(owner());
    trace1("Contrib::parse", cmd.tail().substr(0,20));
    Expression rhs_(cmd);
    assert(_rhs.is_empty());

    assert(owner());
    _rhs.set_owner(owner());
    _rhs.resolve_symbols(rhs_);
  }
  {

    trace1("Contrib::parse", rhs().back()->name());
    if(is_direct()){
    }else if(rhs().is_empty()){ untested();
      throw Exception_CS_("syntax error", cmd);
    }else if(rhs().back()->name()!="=="){ untested();
      throw Exception_CS_("syntax error", cmd);
    }else{
      auto b = dynamic_cast<Token_BINOP_*>(rhs().back());
      assert(b);
      auto dd = deps().clone();
      rhs().pop_back();
      rhs().push_back(new Token_BINOP_("-", b->op1(), b->op2(), dd));
      b->pop();
      delete b;
    }
    assert(owner());

    for(const Dep& d : deps().ddeps()) {
      add_dep(d);
    }
    assert(!_deps);
    _deps = _rhs.deps().clone();
  }
  cmd >> ";";

  if(_nature == disc->potential()){
    if(is_zero(_rhs)) {
      set_short();
    }else if(owner()->is_always()) {
      set_always_pot();
    }
  }else if(_nature == disc->flow()){
    if(is_zero(_rhs)) {
    }else{
      set_flow_contrib();
    }
    _branch->set_source();
  }

  if(_branch->deps().is_linear()){
  }else{
  }

  if(owner()->is_reachable()){
    update();
  }else{
  }

  if(options().optimize_unused() && !owner()->is_reachable()) {
  }else{
    trace2("inc_use0", name(), branch()->name());
    _branch->inc_use(); // ??
    _branch->reg_stmt(this);
    set_used_in(_branch);
  }
} // Contribution::parse
/*--------------------------------------------------------------------------*/
void Contribution::add_dep(Dep const& d)
{
  trace3("contrib dep", d->code_name(), d.is_linear(), _branch->deps().is_linear());
  _branch->add_dep(d);

  if(!is_direct()){
  }else if(d->branch() != _branch){
  }else if(is_flow_contrib() && d->is_flow_probe()){ untested();
    _branch->set_selfdep();
  }else if(is_pot_contrib() && d->is_pot_probe()){
    _branch->set_selfdep();
  }else{
  }


  if(owner()->is_reachable()){
    d->branch()->inc_use();
    trace2("inc_use1", name(), branch()->name());
    (*d)->set_used_in(this);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
DDeps const& Contribution::ddeps() const
{
  return data().ddeps();
}
/*--------------------------------------------------------------------------*/
TData const& Contribution::deps()
{
  if(_deps){
    return *_deps;
  }else{
    return _rhs.deps();
  }
}
/*--------------------------------------------------------------------------*/
bool Contribution::update()
{
  assert(owner()->is_reachable());
  trace1("Contribution::update", name());

  _rhs.update();
  TData const* D = &_rhs.deps();
  if(!_deps){ untested();
    _deps = new TData;
    _deps->rdeps().push_back(_branch);
  }else{
  }
  size_t s = _deps->ddeps().size();

  _deps->update(*D);
  bool ret = (s != _deps->ddeps().size());

  if(options().optimize_unused() && !owner()->is_reachable()) { untested();
  }else{
    for(; s < _deps->ddeps().size(); ++s) {
      Dep const& d = deps().ddeps()[s];
      add_dep(d);
    }

    _rhs.propagate_rdeps(*_deps);
  }
  _sens.merge(_deps->sensitivities());

  return ret || (_deps->ddeps().size() != D->ddeps().size());
}
/*--------------------------------------------------------------------------*/
void Branch_Map::parse(CS& f)
{ untested();
  trace1("Branch_Map::parse", f.tail().substr(0,20));
  incomplete();
}
/*--------------------------------------------------------------------------*/
void Branch_Map::dump(std::ostream&)const
{ untested();
  incomplete();
}
/*--------------------------------------------------------------------------*/
Branch::~Branch()
{
  // no, shutting down, not all Refs tidied up.
  assert(!_refs.size());

  // Contributions tidied up
  assert(!_has_pot_src);
  assert(!_has_flow_src);
  // assert(!_has_always_pot);

  // Probes tidied up
  assert(!_has_pot_probe);
  assert(!_has_flow_probe);

  delete _deps;
  _deps = NULL;

  for(auto i : _used_in){
    if(i){ untested();
      std::cerr << "logic error. " << name() << " still used in. " << i << "\n";
      assert(0);
    }else{
    }
    assert(!i);
  }
  if(_use){ untested();
    unreachable();
    std::cerr << "logic error. " << name() << " still used.\n";
    assert(false);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Branch::set_used_in(Base const* b)
{
  for(auto& i : _used_in){
    if(i == b){ untested();
      return;
    }else{
    }
  }
  _used_in.push_back(b);
}
/*--------------------------------------------------------------------------*/
void Branch::unset_used_in(Base const* b)
{
  int found = 0;
  for(auto& i : _used_in){
    if(i == b){
      i = NULL;
      ++found;
#ifndef NDEBUG
      return;
#endif
    }else{
    }
  }
  if(!found){ untested();
    unreachable();
    throw std::logic_error("cleanup " + code_name());
  }else if(found>1){ untested();
    throw std::logic_error("cleanup1 " + code_name());
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void Branch::dump(std::ostream& o)const
{
  if(_n->is_ground()){ untested();
    o << "(" << _p->name() << ")";
  }else{
    o << "(" << _p->name() << ", " << _n->name() << ")";
  }
}
/*--------------------------------------------------------------------------*/
static void dump(std::ostream& out, Expression const& e)
{
  e.dump(out);
}
/*--------------------------------------------------------------------------*/
void Contribution::dump(std::ostream& o)const
{
  assert(_branch);
  o__ _name << _branch << " ";
  if(is_direct()){
    o << "<+";
  }else{
    o << ":";
  }
  o << " ";

  /// assert(_rhs);
  ::dump(o, _rhs);
  o << ";";
  if(options().dump_annotate()){
    dump_annotate(o, *this);
  }else{
  }
  o << "\n";
}
/*--------------------------------------------------------------------------*/
std::string Branch::name() const
{
  return "(" + _p->name()+", "+_n->name()+")";
}
/*--------------------------------------------------------------------------*/
bool Branch::is_short() const
{
  assert(_p);
  assert(_n);
  return _p->number() == _n->number();
}
/*--------------------------------------------------------------------------*/
bool Branch::req_short() const
{
  if(has_flow_source()){
    return false;
  }else if(_has_short){
    return _has_pot_src == _has_short;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
void AnalogCtrlBlock::dump(std::ostream& o)const
{
  if(size() || identifier() != ""){
    o << "begin";
    if(identifier() != ""){
      o << " : " << identifier();
    }else{
    }
    if(!options().dump_annotate()){
    }else if(is_always()){
      o << " // always";
    }else if(is_never()){
      o << " // never";
    }else{
    }
    o << "\n";
    {
      indent x;
      if(options().dump_annotate()){
	for(auto i : variables()){
	  if(auto v = dynamic_cast<Token_VAR_REF const*>(i.second)){
	    o__ "// " << v->name();
	    if(v->data()){
	      o << " : " << v->deps().size() << "\n";
	    }else{
	      o << "???\n";
	    }
	  }else{ untested();
	    o__ "// " << i.first << "\n";
	  }
	}
      }else{
      }
      SeqBlock::dump(o);
    }
    o__ "end\n";
  }else{
    o << ";\n";
  }
}
/*--------------------------------------------------------------------------*/
void AnalogSeqBlock::dump(std::ostream& o)const
{
  o__ "begin";
  if(identifier() != ""){
    o << " : " << identifier();
  }else{
  }
  o << "\n";
  if(options().dump_annotate()){
    for(auto i : variables()){
      if(auto v = dynamic_cast<Token_VAR_REF const*>(i.second)){
	o__ "// " << v->name() << " : " << v->deps().size() << "\n";
      }else{ untested();
	o__ "// " << i.first << "\n";
      }
    }
  }else{
  }
  {
    indent x;
    Block::dump(o);
  }
  o__ "end\n";
}
/*--------------------------------------------------------------------------*/
#if 1
void AnalogExpression::parse(CS& file)
{
  trace1("AnalogExpression::parse", file.tail().substr(0,40));

  Expression rhs(file);
  file >> ","; // LiSt??
  assert(owner());
  _exp.set_owner(owner());
  _exp.resolve_symbols(rhs);
}
/*--------------------------------------------------------------------------*/
bool AnalogExpression::is_true() const
{
  return ::is_true(_exp);
}
/*--------------------------------------------------------------------------*/
bool AnalogExpression::is_false() const
{
  return ::is_false(_exp);
}
#endif
/*--------------------------------------------------------------------------*/
void AnalogEvtCtlStmt::parse(CS& file)
{
  _ctrl.set_owner(owner()); // ?
  file >> _ctrl;
  assert(owner());
  _body.set_owner(this);
  _body.set_sens(this);
  file >> _body;
}
/*--------------------------------------------------------------------------*/
void AnalogEvtCtlStmt::dump(std::ostream& o) const
{
  o__ "@" << _ctrl << "";
  AnalogCtrlStmt::dump(o);
#if 0
  if(dynamic_cast<AnalogSeqBlock const*>(_stmt)){ untested();
    o << " " << *_stmt;
  }else if(_stmt){ untested();
#if 0
    o << " " << *_stmt;
#else
    o << "\n";
    { untested();
      indent x;
      o << *_stmt;
    }
#endif
  }else{ untested();
    o << ";";
  }
#endif
}
/*--------------------------------------------------------------------------*/
void make_cc_af_body(std::ostream& o, const Analog_Function& f); // BUG
void make_cc_af_args(std::ostream& o, const Analog_Function& f); // BUG
namespace{
/*--------------------------------------------------------------------------*/
static Module const* to_module(Block const* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<Module const*>(owner)){
      return m;
    }else if(auto b = dynamic_cast<Block const*>(owner->owner())){
      owner = b;
    }else if(auto st = dynamic_cast<Statement const*>(owner->owner())){
      owner = st->scope();
    }else{
      assert(false);
      return NULL;
    }
  }
  unreachable();
  return NULL;
}
/*--------------------------------------------------------------------------*/
class AF : public MGVAMS_FUNCTION {
  Analog_Function const* _af{NULL};
public:
  explicit AF(Analog_Function const* af) : _af(af) {
    assert(_af);
    set_label(af->identifier());
  }

#if 1
  Token* new_token(Module& m, size_t /*na*/) const override { untested();
    trace1("AF::new_token", label());
    m.install(this);
    if(code_name() != ""){ untested();
      return new Token_CALL(label(), this);
    }else if(label() != ""){ untested();
      return new Token_CALL(label(), this);
    }else{ untested();
      incomplete();
      return NULL;
    }
  }
#endif
  std::string code_name()const override {
    return "af_" + label();
  }
  void make_cc_impl(std::ostream& o)const override {
    o << "//incomplete: af impl\n";
    assert(_af);
    auto mp = to_module(_af->scope());
    assert(mp);
    auto& m = *mp;
#if 1
    auto f = _af;
    assert(f);
    o << "COMMON_" << m.identifier() << "::";
    o << "ddouble COMMON_" << m.identifier() << "::" << f->code_name() << "(\n";
    o << "            ";
    make_cc_af_args(o, *f);
    o << ") const\n{\n";
    {
      indent x;
      o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
      o__ "(void)pc;\n";
      make_cc_af_body(o, *f);
    }
    o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
#endif
  }

  void make_cc_common(std::ostream& o)const override {
    o << "//incomplete: af common\n";
    assert(_af);
    auto& F = *_af;
    o__ "ddouble " << F.code_name() << "(";
    trace1("af::make_cc_common", label());
    // BUG? make_cc_af_args
    std::string sep = "";
    std::string qual = "";
    for (auto coll : F.args()){
      if(coll->is_output()){
	qual = "/*output*/ &";
      }else{
	qual = "";
      }
      for(auto i : *coll){
	o << sep << "ddouble " << qual;
	o << "/*" << i->identifier() << "*/";
	sep = ", ";
      }
    }
    o << ") const;\n";
  }

};
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
void Analog_Function::parse(CS& f)
{
//  f >> _type >> _identifier; // VARIABLE
  _args.set_owner(this);
  _variable.set_owner(this);
  f >> _type >> _variable;
//  new_var_ref(&_variable); // !
  size_t here = f.cursor();
  bool end = false;
  Base* s = NULL;
  for (;;) {
    ONE_OF	// module_item
      || f.umatch(";")
      || ((f >> "input ") && (f >> _args))
      || ((f >> "output ") && (f >> _args))
      || ((f >> "inout ") && (f >> _args))
//      || (f >> _attribute_dummy)
      || ((f >> "endfunction ") && (end = true))
      || (s = parse_analog_stmt_or_null(f, this))
      ;
    if(s){
      push_back(s);
      s = NULL;
    }else{
    }
    if (end){
      break;
    }else if (!f.more()) { untested();
      f.warn(bWARNING, "premature EOF (analog function)");
      break;
    }else if (f.stuck(&here)) {
      f.warn(bWARNING, "bad analog function");
      break;
    }else{
    }
  }

  assert(!_function);
  _function = new AF(this);
}
/*--------------------------------------------------------------------------*/
Analog_Function::~Analog_Function()
{
  for(auto x : _var_refs){
    trace1("~AF", x.first);
  }
  delete _function;
  _function = NULL;
}
/*--------------------------------------------------------------------------*/
void Analog_Function::dump(std::ostream& o) const
{
  o__ "analog function " << _type << " " << _variable << ";\n";
  {
    indent x;
    o__ _args;
    Block::dump(o); // TODO: indentation
  }
  o__ "endfunction\n";
}
/*--------------------------------------------------------------------------*/
void Analog_Function::new_var_ref(Base* b)
{
  auto t = prechecked_cast<Token*>(b);
  assert(t);
  trace1("AF::nvr", t->name());
  return Block::new_var_ref(b);
}
/*--------------------------------------------------------------------------*/
void AnalogEvtExpression::parse(CS& file)
{
  _expression = new Expression(file);
}
/*--------------------------------------------------------------------------*/
void AnalogEvtExpression::dump(std::ostream& o) const
{
  o << "(";
  if(_expression) {
    o << *_expression;
  }else{ untested();
  }
  o << ")";
}
/*--------------------------------------------------------------------------*/
AnalogExpression::~AnalogExpression()
{
}
/*--------------------------------------------------------------------------*/
void AF_Arg_List_Collection::dump(std::ostream& o)const
{
  for(auto const& i : *this){
    i->dump(o);
  }
  // Collection<Parameter_2_List>::dump(o);
}
/*--------------------------------------------------------------------------*/
void Analog_Function_Arg::parse(CS& f)
{
  trace1("Analog_Function_Arg::parse", f.tail().substr(0,10));
  Variable_Decl::parse(f);
}
/*--------------------------------------------------------------------------*/
void Analog_Function_Arg::dump(std::ostream& o)const
{
  o << name();
}
/*--------------------------------------------------------------------------*/
void AF_Arg_List::parse(CS& file)
{
  std::string dir = file.last_match().substr(0,3);
//  file >> _type;
  if(dir=="inp"){
    _direction = a_input;
  }else if(dir=="out"){ untested();
    _direction = a_output;
  }else if(dir=="ino"){
    _direction = a_inout;
  }else{ untested();
    trace2("AF_Arg_List::parse", file.tail().substr(0,10), dir);
    unreachable();
  }
  LiSt<Analog_Function_Arg, '\0', ',', ';'>::parse(file);
//  for(auto& i : *this){ untested();
//    i->set_type(type);
//  }
}
/*--------------------------------------------------------------------------*/
void AF_Arg_List::dump(std::ostream& o)const
{
  static std::string names[] = {"input", "output", "inout"};
  o << names[_direction] << " ";
  LiSt<Analog_Function_Arg, '\0', ',', ';'>::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
bool Assignment::update()
{
  TData const* D = &_rhs.deps();
  assert(D);
//  size_t s = D->ddeps().size();
  bool ret;

//  trace5("Assignment::update1", name(), s, D->ddeps().size(), this, _data->size());
  _rhs.update();
  D = &_rhs.deps();
//  trace4("Assignment::update2", name(), s, D->ddeps().size(), this);

  assert(_token);
  if (store_deps(_rhs.deps())) {
    assert(_lhsref);
//    trace2("Assignment::update prop", _rhs.deps().ddeps().size(), _lhsref->deps().ddeps().size());
    _lhsref->propagate_deps(*_token);
    assert(_token->operator->());
    ret = true;
    assert(_token->data());
  }else{
//    trace4("Assignment::update no prop", name(), _rhs.deps().size(), _lhsref->deps().size(), _token->deps().size());
    ret = false;
    assert(_token->data());
    assert(_token->deps().size() >= _rhs.deps().size());
  }
  owner()->new_var_ref(_token); // ??

  // new_var_ref();
  return ret;
}
/*--------------------------------------------------------------------------*/
bool AnalogStmt::propagate_rdeps(TData const&)
{
  incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
bool Assignment::propagate_deps(Token_VAR_REF const& from)
{
  TData const& d = from.deps();
  if(from.scope() == scope()) {
    return _lhsref->propagate_deps(from);
  }else{
    bool ret = store_deps(d);
    assert(_lhsref);
    return _lhsref->propagate_deps(*_token) || ret;
  }
}
/*--------------------------------------------------------------------------*/
// bool Assignment::sync_data(TData const& d)
bool Assignment::store_deps(TData const& d)
{
  assert(_lhsref);
  size_t ii = 0;
  bool ret = false;

  if(options().optimize_unused() && !owner()->is_reachable()) {
    _token = new Token_VAR_REF(_lhsref->name(), NULL);
  }else{

    if(_token) {
      assert(_data);
      ii = _data->ddeps().size();
    }else{
      assert(!_data);
      _data = new TData();
      _token = new Token_VAR_REF(_lhsref->name(), this, _data);
      assert(_token->data());
    }
    _data->update(d);

    assert(ii <= _data->ddeps().size());

    if(auto x = dynamic_cast<SeqBlock const*>(owner())) {
      if(x->has_sensitivities()) {
	_data->add_sens(*x->sensitivities());
      }else{
      }
    }else{ untested();
    }

    for(; ii < _data->ddeps().size(); ++ii) {
      ret = true;
      Dep const& dd = _data->ddeps()[ii];
      trace2("inc_use2", (*dd)->code_name(), this);
      (*dd)->set_used_in(this);
    }
//    assert(&deps() == _data);
    assert(d.ddeps().size() <= _data->ddeps().size());
  }

  return ret;
} // Assignment::store_deps
/*--------------------------------------------------------------------------*/
void Contribution::set_direct(bool d)
{
  assert(_branch);
  _branch->set_direct(d);
}
/*--------------------------------------------------------------------------*/
bool Contribution::is_direct() const
{
  assert(_branch);
  return _branch->is_direct();
}
/*--------------------------------------------------------------------------*/
void Contribution::set_pot_contrib()
{
  assert(_branch);
  assert(owner());
  if(options().optimize_unused() && !owner()->is_reachable()) {
  }else{
    _branch->inc_pot_source();
  }
  _type = t_pot;
}
/*--------------------------------------------------------------------------*/
void Contribution::set_always_pot()
{
  assert(_type == t_pot);
  _branch->inc_always_pot();
}
/*--------------------------------------------------------------------------*/
void Contribution::set_short()
{
  assert(_branch);
  if(!options().optimize_nodes()) { untested();
  }else if(owner()->is_always()) {
    _branch->inc_short();
  }else{
  }
  assert(_type == t_pot);
  _short = true;
}
/*--------------------------------------------------------------------------*/
void Contribution::set_flow_contrib()
{
  assert(_branch);
  if(options().optimize_unused() && !owner()->is_reachable()) {
  }else{
    _branch->inc_flow_source();
  }
  _type = t_flow;
}
/*--------------------------------------------------------------------------*/
bool Contribution::is_flow_contrib() const
{
  return _type == t_flow;
}
/*--------------------------------------------------------------------------*/
bool Contribution::is_pot_contrib() const
{
  return _type == t_pot;
}
/*--------------------------------------------------------------------------*/
Contribution::~Contribution()
{
  assert(_branch); //?
  if(!options().optimize_nodes()){ untested();
  }else if(!owner()->is_always()) {
  }else if(is_short()) {
    _branch->dec_short();
  }else{
  }

  if(options().optimize_unused() && !owner()->is_reachable()) {
  }else{
    trace2("dec_use0", name(), branch()->name());
    _branch->dec_use();
    if (is_flow_contrib()) {
      _branch->dec_flow_source();
    }else if(is_pot_contrib()) {
      _branch->dec_pot_source();
    }else{
    }
//    }

    for(Dep const& i : deps().ddeps()){
      assert(i->branch());
      i->branch()->dec_use();
      try{
	(*i)->unset_used_in(this);
      }catch(std::logic_error const& e){ untested();
	unreachable();
	std::cerr << " logic error in " << name() << ": ";
	std::cerr << e.what() << "\n";
      }
    }

    _branch->dereg_stmt(this);
    unset_used_in(_branch);
  }
  delete _deps;
  _deps = NULL;
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::dump(std::ostream& o)const
{
  o__ name();
  if(options().dump_annotate()){
    for(auto d : deps().ddeps()){
      o << "// dep " << d->code_name();
    }
    o << "\n";
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
bool AnalogRealDecl::update()
{ untested();
  for(BlockVarIdentifier* i : _l){ untested();
    assert(i);
    i->update();
  }
  return false;
}
/*--------------------------------------------------------------------------*/
Probe* new_Probe(std::string const& xs, Branch_Ref const& br)
{
  return new Probe(xs, br);
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_nodes() const
{
  size_t ret=1;

  for(auto i : ddeps()){
    if(i->branch()->is_short()){ untested();
    }else if(i->branch() == this){
      // self conductance
    }else if(i->is_pot_probe()){
      ++ret;
//     }else if(i->is_filter_probe()){ untested();
//       assert(i->is_pot_probe());
//       unreachable();
//       ++ret;
    }else{
    }
  }
  return 2*ret;
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_states() const
{
  size_t k = 2;
  // TODO: cleanup
  for(auto i : ddeps()){
    assert(i);
    // if(i->is_reversed()){ untested();
    //}else
    if(i->branch() == this){
    }else if(i->branch()->is_short()){ untested();
    }else{
      ++k;
    }
  }
  return k;
}
/*--------------------------------------------------------------------------*/
Probe::Probe(std::string const& xs, Branch_Ref br) : _br(br)
{
  trace3("::Probe", xs, code_name(), br.has_name());
  // TODO: disciplines.h
  if( (xs == "V") || (xs == "potential") ){
    _type = t_pot;
    _br->inc_pot_probe();
  }else if( (xs == "I") || (xs == "flow") ){
    _type = t_flow;
    _br->inc_flow_probe();
    _br->set_probe(); // shadow
  }else{ untested();
    unreachable();
  }

  std::string k = xs + "_" + br.pname() + "_" + br.nname();
  if(br.has_name()){
    k = xs + "_" + br.name();
  }else{
  }

  set_label(k);
}
/*--------------------------------------------------------------------------*/
Probe::~Probe()
{
  if(is_flow_probe()){
    _br->dec_flow_probe();
  }else{
  }
  if(is_pot_probe()){
    _br->dec_pot_probe();
  }else{
  }

//  assert(!_use);
}
/*--------------------------------------------------------------------------*/
bool Probe::is_reversed() const
{
  return _br.is_reversed();
}
/*--------------------------------------------------------------------------*/
Discipline const* Probe::discipline() const
{
  return _br->discipline();
}
/*--------------------------------------------------------------------------*/
Nature const* Probe::nature() const
{ untested();
  return _br->nature();
}
/*--------------------------------------------------------------------------*/
void Module::new_analog()
{
  assert(!_analog);
  _analog = new Analog;
}
/*--------------------------------------------------------------------------*/
void Module::delete_analog()
{
  delete _analog;
  _analog = NULL;
}
/*--------------------------------------------------------------------------*/
bool Module::has_analog_block() const
{
  return ::analog(*this).has_block();
}
/*--------------------------------------------------------------------------*/
Analog::Analog()
{
}
/*--------------------------------------------------------------------------*/
Analog::~Analog()
{
  _list.clear();
  _functions.clear();
}
/*--------------------------------------------------------------------------*/
bool Analog::has_block() const
{
  return !list().is_empty();
}
/*--------------------------------------------------------------------------*/
void Module::new_probe_map()
{
  _probes = new Probe_Map;
}
/*--------------------------------------------------------------------------*/
Probe_Map::~Probe_Map()
{
  for(auto i: *this) {
    delete i.second;
  }
}
/*--------------------------------------------------------------------------*/
void Module::install(Probe const* f)
{
  _funcs.insert(f);
}
/*--------------------------------------------------------------------------*/
bool Branch::is_shadow_source()const
{
  if (_source && _probe && !has_pot_source() && !has_flow_source()) {
    return true;
  }else if (_probe && has_flow_probe() && !is_source()){
    if(has_pot_probe()){
      // BUG: not the right place. need another pass in Module::parse?
      throw Exception("cannot have both flow and potential probes on a probe branch\n");
    }else{
    }
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static void dump_shadow_src(std::ostream& o, Module const& m)
{
  if(!options().dump_unreachable()){
    for(auto i: m.circuit()->branches()) {
      if(i->is_shadow_source()){
	o__ "analog begin\n";
	o____ "";
	if(i->is_source()) {
	  if(i->discipline() && i->discipline()->flow()){
	    o << i->discipline()->flow()->access();
	  }else{ untested();
	    o << "I";
	  }
	}else{
	  // shadow flow probe
	  if(i->discipline() && i->discipline()->potential()){
	    o << i->discipline()->potential()->access();
	  }else{ untested();
	    o << "V";
	  }
	}
	i->dump(o);
	o << " <+ 0.;\n";
	o__ "end\n";
      }else{
      }
    }
  }else{
//    o__ "// no shadow src\n";
  }
}
/*--------------------------------------------------------------------------*/
void Analog::dump(std::ostream& o) const
{
  for(auto const& i: functions()){
    o << *i << "\n";
  }

  Module const* m = to_module(owner());
  assert(m);
  if(!options().dump_unreachable()) {
    dump_shadow_src(o, *m);
  }else if (options().optimize_unused()) {
    dump_shadow_src(o, *m);
  }else{ untested();
  }

  for(auto const& i: blocks()){
    o << *i << "\n";
  }
}
/*--------------------------------------------------------------------------*/
FUNCTION_ const* analog_function_call(std::string const& f, Block const* scope)
{
  Module const* m = to_module(scope);
  assert(m);
  for(auto n: ::analog(*m).functions()){
    trace2("is_afcall", n->identifier(), f);
    if(n->identifier() == f){
      assert(n->function());
      return n->function();
    }else{
    }
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
static File const* to_file(Block const* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<File const*>(owner)){
      return m;
    }else{
    }
    owner = owner->scope();
    if(!owner){ untested();
      return NULL;
    }else{
    }
  }
  unreachable();
  return NULL;
}
/*--------------------------------------------------------------------------*/
FUNCTION_ const* xs_function_call(std::string const& f, Block const* owner)
{
  Module const* m = to_module(owner);
  assert(m);
  File const* file = to_file(owner);
  if(!file){ untested();
    file = dynamic_cast<File const*>(m->owner());
  }else{
  }

  assert(file);
  if(f=="flow" || f=="potential") { untested();
    // TODO: return FUNCTION_*, VAMS_XS* from nature
    return (FUNCTION_*)(1); // TODO true;
  }else if(file){
    // use actual disciplines
    // auto const& nl = file->nature_list();
    // return find_ptr(nl.begin(), nl.end(), f);
  }else if(f=="V") { untested();
    // fallback. modelgen_0.cc // incomplete();
    return (FUNCTION_*)(1); // TODO true;
  }else if(f=="I") { untested();
    // fallback. modelgen_0.cc // incomplete();
    return (FUNCTION_*)(1); // TODO true;
  }else if(f=="Pwr") { untested();
    // fallback. modelgen_0.cc // incomplete();
    return (FUNCTION_*)(1); // TODO true;
  }

  /// TODO ///
  for(auto n: file->nature_list()){
    if(n->access().to_string() == f){
      // TODO: return FUNCTION_*, VAMS_XS* from nature
      return (FUNCTION_*)(1); // TODO true;
    }else{
    }
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
void Analog::parse(CS& f)
{
  if(f >> "function "){
    _functions.set_owner(owner());
    f >> _functions;
  }else{
    AnalogConstruct* ab = new AnalogConstruct();
    // AnalogConstruct* ab = new AnalogConstruct(f, owner());

    assert(owner());
    ab->set_owner(owner());
    ab->parse(f);
    _list.set_owner(owner()); // needed?
    _list.push_back(ab);

//    if(auto s = dynamic_cast<AnalogStmt*>(ab->statement_or_null())){ untested();
//      s->update();
//    }else{ untested();
//    }
  }
//  return f;
}
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::lookup(std::string const& n)const
{
  auto it = _names.find(n);
  if(it != _names.end()) {
    return Branch_Ref(*it);
  }else{
    return Branch_Ref(NULL, false);
  }
}
// Branch_Ref Branch_Names::lookup(std::string const&) const
// { untested();
//   ...
// }
/*--------------------------------------------------------------------------*/
void AnalogStmt::set_used_in(Base const* b)
{
  for(auto& i : _used_in){ untested();
    if(i == b){ untested();
      return;
    }else{ untested();
    }
  }
  _used_in.push_back(b);
}
/*--------------------------------------------------------------------------*/
void AnalogStmt::unset_used_in(Base const* b)
{
  for(auto& i : _used_in){
    if(i == b){
      i = NULL;
      return;
    }else{ untested();
    }
  }
  unreachable();
}
/*--------------------------------------------------------------------------*/
bool AnalogStmt::is_used_in(Base const* b)const
{
  for(auto& i : _used_in){
    if(i == b){
      return true;
    }else{
    }
  }
  for(auto& i : _rdeps){ untested();
    if(i == b){ untested();
      return true;
    }else{ untested();
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
AnalogStmt::~AnalogStmt()
{
  for(auto n :_used_in){
    assert(!n);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
