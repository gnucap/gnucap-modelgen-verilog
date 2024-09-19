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
#include "mg_token.h"
#include <e_cardlist.h> // TODO: really?
#include <u_opt.h>
#include "globals.h"
/*--------------------------------------------------------------------------*/

/* So analog initial block shall not contain the fol-
lowing statements:untested();
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
  assert(!dynamic_cast<Module const*>(o));
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
static Statement* parse_seq(CS& f, Block* owner)
{
  return new AnalogSeqStmt(f, owner);
}
/*--------------------------------------------------------------------------*/
template<class A>
void dump_annotate(std::ostream& o, A const& a)
{
  if(!a.scope()->is_reachable()){
    o << " // --\n";
  }else if(a.data().is_constant()){ untested();
    o << " // c\n";
  }else{
    if(a.data().ddeps().size()){
      o << " //";
    }else if(a.has_sensitivities()){
      o << " //";
    }else if(a.rdeps_().size()){
      o << " //";
    }else{
    }
    for(const Dep& d : a.data().ddeps()) {
      o << " dep: ";
      o << d->code_name();
    }
#if 1
    if(a.has_sensitivities()){
      o << " s" << a.sensitivities().size();
    }else{
    }
#endif
    if(a.rdeps_().size()){
      o << " r" << a.rdeps_().size();
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
// static Base* parse_int(CS& f, Block* o)
// { untested();
//   trace1("AnalogBlock::parse int", f.tail().substr(0,10));
//   return new ListOfBlockIntIdentifiers(f, o);
// }
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
  trace1("System_Task::parse0", f.tail().substr(0,10));
  f >> _e;
  trace1("System_Task::parse1", f.tail().substr(0,10));
  f >> ";";

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
// code?
bool System_Task::update()
{
  assert(function());

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
  if(function()->has_tr_advance()){ untested();
    add_rdep(&tr_advance_tag);
  }else{
  }
  // Expression_::update(rdeps());
 //  auto& rd = _e.data().rdeps(); // also collect from control block?
 //  size_t rs = rd.size();
 // //  _rdeps.merge(rd); // needed?
 //  trace2("System_Task::update2", rd.size(), rdeps().size());
  bool ret = _e.update(&_rdeps);
//  assert(rs<=_e.data().rdeps().size()); // WIP

 // trace3("System_Task::update3", ret, rd.size(), rdeps().size());
  return propagate_rdeps(_rdeps) || Statement::update() || ret;
}
/*--------------------------------------------------------------------------*/
void AnalogProceduralAssignment::parse(CS& f)
{
  // assert(owner()); ?
  _a.set_owner(this);
  size_t here = f.cursor();

  trace2("Proc assignment?", f.tail().substr(0,20), _a.owner());
  std::string what;
  f >> what;
  if(what == ""){ untested();
    f.reset_fail(here);
    throw Exception_No_Match("need name");
  }else{
    f.reset(here);
  }
  if(f >> _a){
    trace1("preupdate", _a);
    update(); // hmm, analysis?
   // _a.data().add_sens(this);
    trace1("postupdate", _a);
    if(f >> ";"){
    }else{ untested();
      f.warn(bWARNING, "missing semicolon?");
    }
  }else{
  }

}
/*--------------------------------------------------------------------------*/
Statement* AnalogProceduralAssignment::deep_copy(Base* owner) const
{
//  return new AnalogProceduralAssignment(this);
  auto b = prechecked_cast<Block*>(owner);
  assert(b);
  std::stringstream s;
  dump(s);
  CS f(CS::_STRING, s.str());
  trace1("AP::deep_cp", s.str());
  auto a = new AnalogProceduralAssignment(f, b);
  assert(a->scope());
  //a->dump(s);
  return a;
}
/*--------------------------------------------------------------------------*/
void AnalogProceduralAssignment::dump(std::ostream& o)const
{
  if(_a){
    o__ "";
    _a.dump(o);
    o << ";";
    if(options().dump_annotate()){
      dump_annotate(o, *this);
      // dump_annotate(o, _a);
    }else{
    }
    o << "\n";
  }else{
    // optimised out. BUG: ';' or not ';'?
  }
}
/*--------------------------------------------------------------------------*/
Base* parse_proc_assignment(CS& f, Block* o)
{
  assert(o);
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
  set_owner(o);
  _a.set_owner(this);
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
  ONE_OF	// module_item
    || (file >> ";")
    || ((file >> "begin") && (ret = parse_seq(file, scope)))
    || ((file >> "if ") && (ret = parse_cond(file, scope)))
    || ((file >> "case ") && (ret = parse_switch(file, scope)))
    || ((file >> "while ") && (ret = new AnalogWhileStmt(file, scope)))
    || ((file >> "for ") && (ret = new AnalogForStmt(file, scope)))
    || ((file >> "@ ") && (ret = new_evt_ctl_stmt(file, scope)))
    || (ret = parse_proc_assignment(file, scope))
    || (ret = parse_system_task(file, scope))
    || (ret = parse_contribution(file, scope))
    ;

  if(file.stuck(&here)) {
    trace1("parse_analog_stmt_or_null? stuck", file.tail().substr(0,30));
    file.reset_fail(here);
  }else{
    trace1("parse_analog_stmt_or_null? cont", file.tail().substr(0,30));
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
  //_cond.set_owner(owner());
  _cond.set_owner(this);
  _body.set_owner(this);
  assert(!_body.is_always());
  assert(!_body.is_never());
  _false_part.set_owner(this);

  if(f >> "(" >> _cond >> ")"){
  }else{ untested();
    throw Exception_CS_("expecting conditional", f);
  }

  {
    if(is_never()) {
      _body.set_never();
      _false_part.set_never();
    }else if(_cond.is_true()) {
      if(is_always()) {
	_body.set_always();
      }else{
      }
      _false_part.set_never();
    }else if(_cond.is_false()) {
      if(is_always()) {
	_false_part.set_always();
      }else{ untested();
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
void AnalogConditionalStmt::dump(std::ostream& o) const
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
bool AnalogConditionalStmt::is_used_in(Base const* b) const
{
  if (_cond.is_used_in(b)){ untested();
    return true;
  }else{
    return AnalogStmt::is_used_in(b);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void AnalogCtrlStmt::parse(CS& f)
{
  _body.set_owner(this);
  f >> _body;
}
/*--------------------------------------------------------------------------*/
void AnalogCtrlStmt::dump(std::ostream& o) const
{
  if(!_body){
    o << ";\n";
  }else{
    o << " ";
    _body.dump(o);
  }
}
/*--------------------------------------------------------------------------*/
bool AnalogCtrlStmt::propagate_rdeps(RDeps const& r)
{
  // incomplete(); // remove?
  return Statement::propagate_rdeps(r);
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
//  _ctrl?
  bool ret = false;
  while(true){
    trace0("AnalogWhileStmt::update");
    _body.clear_vars();
    if (_body.update()){
      ret = true;
    }else{
      break;
    }
  }
  return // propagate_rdeps(_rdeps) ||
     AnalogStmt::update() || ret;
}
/*--------------------------------------------------------------------------*/
// duplicate?
bool AnalogSwitchStmt::update()
{
//  _ctrl?
  bool ret = false;
  while(true){
    _body.clear_vars();
    if (_body.update()){
      ret = true;
    }else{
      break;
    }
  }
  return // propagate_rdeps(_rdeps) ||
     AnalogStmt::update() || ret;
}
/*--------------------------------------------------------------------------*/
void AnalogWhileStmt::parse(CS& file)
{
  //_cond.set_owner(scope());
  _cond.set_owner(this);
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
static Assignment* parse_assignment_or_null(CS& f, Statement* owner)
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
  trace0("AnalogForStatement::update");
  auto init_ = dynamic_cast<Assignment*>(_init);
  auto tail_ = dynamic_cast<Assignment*>(_tail);
  bool ret = false;

  while(true){
    if ( init_ && init_->update() ){ untested();
      ret = true;
    }else if (_body.update()){
      ret = true;
    }else if ( tail_ && tail_->update() ) { untested();
      ret = true;
    }else{
      break;
    }
  }
  return // propagate_rdeps(_rdeps) ||
     AnalogStmt::update() || ret;
}
/*--------------------------------------------------------------------------*/
void AnalogForStmt::parse(CS& f)
{
  assert(owner());
  _cond.set_owner(owner());
  f >> "(";
  Assignment* init = parse_assignment_or_null(f, this);
  _init = init;
  if(!f){
    delete _init;
    _init = NULL;
  }else{
  }
  f >> ";";
  f >> _cond;
  f >> ";";

  _tail = parse_assignment_or_null(f, this);

  f >> ")";
  if(f >> ";"){ untested();
  }else{
    _body.set_owner(this); // needed?
    f >> _body;
  }

  update();
}
/*--------------------------------------------------------------------------*/
bool AnalogProceduralAssignment::update()
{
  trace1("AnalogProceduralAssignment::update",  deps().size());
//  trace1("AnalogProceduralAssignment::update",  _a.data().size());
  trace1("AnalogProceduralAssignment::update",  rdeps().size());

  bool ret;
  if(options().optimize_unused() && !scope()->is_reachable()) {
    ret = false;
  }else{
    RDeps r(rdeps());
    assert(r.size()==rdeps().size());
    ret = _a.update(&r);
    assert(r.size()==rdeps().size());
    //_rdeps.merge(_a.data().rdeps());
  }

  trace1("AnalogProceduralAssignment::update1",  rdeps().size());
 // trace1("AnalogProceduralAssignment::update1",  _a.data().size());
  trace1("AnalogProceduralAssignment::update1",  deps().size());
  if(is_state_var()){ untested();
    ret |= propagate_rdep(&tr_advance_tag);
  }else{ untested();
  }
  return AnalogStmt::update() || ret;
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
  result.set_owner(this);

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
  //_ctrl.set_owner(scope());
  _ctrl.set_owner(this);
  _body.set_owner(this);
  auto sb = dynamic_cast<SeqBlock*>(scope());
  assert(sb);
  sb->add_block(&_body); // re-use var_ref?

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
bool AnalogConstruct::is_used_in(Base const*) const
{
  // incomplete.
  return true;
}
/*--------------------------------------------------------------------------*/
void AnalogConstruct::new_block()
{
  assert(!_block);
  _block = new AnalogCtrlBlock();
  _block->set_owner(this);
}
/*--------------------------------------------------------------------------*/
void AnalogConstruct::push_back(Statement*s)
{
  // assert(0); // no longer used?
  assert(_block);
  _block->push_back(s);
}
/*--------------------------------------------------------------------------*/
void AnalogConstruct::parse(CS& f)
{
  assert(owner());
  assert(!_block);
  auto ab = new AnalogCtrlBlock(f, this);
  _block = ab;
  ab->update();
}
/*--------------------------------------------------------------------------*/
void AnalogSeqStmt::parse(CS& f)
{
  _block.set_owner(this);

  if(is_never()) { untested();
    _block.set_never();
  }else if(is_always()) {
    _block.set_always();
  }else{ untested();
  }

  f >> _block;
  // _block.update();
}
/*--------------------------------------------------------------------------*/
    // f >> _variables; ?
static void parse_block_variables(CS& f, Variable_List_Collection& P)
{
  for (;;) {
    trace1("AnalogSeqBlock::parse loop", f.tail().substr(0,20));
    if( 0 // || ((f >> "parameter ") && (f >> _parameters))
	|| ((f >> "real ") && (f >> P))
	|| ((f >> "integer ") && (f >> P))) {
      if(f.peek() == ';') { untested();
	f.warn(bWARNING, "stray semicolon\n");
	f.skip();
      }else{
      }
    }else{
      break;
    }
  }
}
/*--------------------------------------------------------------------------*/
void AnalogSeqBlock::parse(CS& f)
{
  assert(owner());
  if(f >> ":"){
    parse_identifier(f);
    _variables.set_owner(this);
    parse_block_variables(f, _variables);
  }else{
  }
  if(dynamic_cast<Module const*>(owner())) { untested();
    set_always();
  }else if(dynamic_cast<Module const*>(scope())) {
    set_always();
  }else if(auto sb = dynamic_cast<SeqBlock*>(scope())) {
    sb->add_block(this); // re-use var_ref?
  }else if(dynamic_cast<Statement const*>(owner())) { untested();
  }else{ untested();
    assert(0);
    unreachable();
  }
  for (;;) {
    trace1("AnalogSeqBlock::parse loop", f.tail().substr(0,20));
    if(f >> "end "){
      trace0("AnalogSeqBlock::parse, end");
      if(f.peek() == ';') {
	f.warn(bWARNING, "stray semicolon\n");
	f.skip();
      }else{
      }
      break;
    }else{
    }
    Base* s = parse_analog_stmt(f, this);
    if(!s){
      throw Exception_CS_("bad analog block", f);
    }else{
      push_back(s);
    }
  }
}
/*--------------------------------------------------------------------------*/
//void AnalogSeqBlock::clear_vars()
//{ untested();
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
  Base* b = st;
  set_owner_raw(b);
//  Statement::set_owner(st);
  assert(owner());
}
#endif
/*--------------------------------------------------------------------------*/
void AnalogCtrlBlock::parse(CS& f)
{
  if(dynamic_cast<Module const*>(owner())) { untested();
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
void AnalogConstruct::dump(std::ostream& o)const
{
  o__ "analog ";
  Base* b = _block;
  b->dump(o);
}
/*--------------------------------------------------------------------------*/
Branch_Ref parse_branch(Block* owner, CS& f)
{
  size_t here = f.cursor();
  if(f >> "("){
  }else{
    f.reset_fail(here);
    return Branch_Ref();
    f.reset_fail(here);
    throw Exception_No_Match("not a branch");
  }
  std::string pp = f.ctos(",)");
  std::string pn = f.ctos(",)");
  if(f >> ")"){
  }else{ untested();
    f.reset_fail(here);
    throw Exception_No_Match("not a branch");
  }

  trace2("a branch", pp, pn);
  assert(owner);
  Branch_Ref b = owner->new_branch(pp, pn);
  trace2("still a branch", pp, pn);
  if(b){
  }else{ untested();
  //   throw Exception_No_Match("not a branch");
    f.reset_fail(here);
  }
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
  _br->set_used_in(b);
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
    trace2("got branch", _name, cmd.tail().substr(0,20));
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

  if(!_branch->discipline()) { untested();
    cmd.reset(here);
    unreachable();
    assert(0);
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
    _rhs.set_owner(this);
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
    _deps = _rhs.data().clone();
    assert(_branch);
    // _deps->add_rdep(_branch);
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
    // update(); // later. need return value
  }else{
  }

  if(options().optimize_unused() && !owner()->is_reachable()) {
  }else{
    trace2("inc_use0", name(), branch()->name());
    _branch->inc_use(); // ??
    _branch->reg_stmt(this);
    set_used_in(_branch);
//    _deps->add_rdep(this);
//    _deps->add_rdep(_branch);
  }

  assert(_deps);
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
TData const& Contribution::deps() const
{
  if(_deps){
    return *_deps;
  }else{
    return _rhs.data();
  }
}
/*--------------------------------------------------------------------------*/
bool Contribution::update()
{
  assert(owner()->is_reachable());

  if(!_deps){ untested();
    _deps = new TData;
  }else{
  }
  _rdeps.insert(_branch);
  _rdeps.insert(&tr_accept_tag);

  size_t s = _deps->ddeps().size();
  bool rdd = _rhs.update(&_rdeps);
  TData const* D = &_rhs.data();
  s = _deps->ddeps().size();

  _deps->update(*D);
  bool ret = (s != _deps->ddeps().size());

  if(options().optimize_unused() && !owner()->is_reachable()) { untested();
  }else{
    for(; s < _deps->ddeps().size(); ++s) {
      Dep const& d = deps().ddeps()[s];
      add_dep(d);
    }
  }
  _sens.merge(_deps->sensitivities());

  trace4("Contribution::update C", rdd, ret, _deps->ddeps().size(),  D->ddeps().size());
  return AnalogStmt::update() || rdd || ret || (_deps->ddeps().size() != s);
} // Contribution::update
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

  if(_use){ untested();
    unreachable();
    std::cerr << "logic error. " << name() << " still used.\n";
    assert(false);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
//bool Branch::propagate_rdeps(RDeps const& b)
//{ untested();
//  return _rdeps.merge(b);
//}
/*--------------------------------------------------------------------------*/
bool Branch::set_used_in(Base const* b)
{
  for(auto& i : _used_in){
    if(i == b){
      return false;
    }else{
    }
  }
  _used_in.push_back(b);
  return true;
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
  if(!found){
    // can't seem to avoid. cyclic deps...
    // unreachable();
//    throw std::logic_error("cleanup " + code_name());
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
  // base?
  o << "begin";
  if(size() || identifier() != ""){
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
	    }else{ untested();
	      o << "???\n";
	    }
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
      if(size()){
	SeqBlock::dump(o);
      }else{
      }
    }
  }else{
    o<< "\n";
    // (why not) annotate?
  }
  o__ "end\n";
}
/*--------------------------------------------------------------------------*/
void AnalogSeqBlock::dump(std::ostream& o)const
{
  // SeqBlock::dump, code?
  o__ "begin";
  if(identifier() != ""){
    o << " : " << identifier() << "\n";
    indent x;
    for(auto* i : variables_()) {
      i->dump(o);
    }
  }else{
    assert(!variables_().size());
    o << "\n";
  }
  if(options().dump_annotate()){
    for(auto i : variables()){
      if(auto v = dynamic_cast<Token_VAR_REF const*>(i.second)){
	o__ "// " << v->name() << " : " << v->deps().size() << "\n";
      }else{
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
  // Expression_::set_owner(scope());
  resolve_symbols(rhs);
}
/*--------------------------------------------------------------------------*/
bool AnalogExpression::is_true() const
{
  return ::is_true(expression());
}
/*--------------------------------------------------------------------------*/
bool AnalogExpression::is_false() const
{
  return ::is_false(expression());
}
#endif
/*--------------------------------------------------------------------------*/
void AnalogEvtCtlStmt::parse(CS& file)
{
  _ctrl.set_owner(owner()); // ?
  file >> _ctrl;
  assert(owner());
  _body.set_owner(this);
  _body.set_sens(this); // BUG
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
bool AnalogEvtCtlStmt::update()
{
  trace1("AnalogEvtCtlStmt::update", rdeps_size());

 // bool rdd = _rhs.update(&_deps->rdeps());
  bool ret = propagate_rdeps(_ctrl.rdeps());
  while(true){
    _body.clear_vars();
    if ( _ctrl.update() ){ untested();
      ret = true;
    }else if (_body.update()){ untested();
      ret = true;
    }else{
      break;
    }
  }
  trace1("AnalogEvtCtlStmt::update done", rdeps_size());

  // set_rdeps(_ctrl.rdeps());
  return AnalogStmt::update() || ret;
}
/*--------------------------------------------------------------------------*/
bool AnalogEvtCtlStmt::is_used_in(Base const* b)const
{
  // store rdeps in Statement::_rdeps?

  // o__ "// AnalogEvtCtlStmt2 " << cond().data().rdeps().size() << "\n";

  if( _ctrl.is_used_in(b)){
    return true;
  }else{
    return AnalogCtrlStmt::is_used_in(b);
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_af(std::ostream& o, const Analog_Function& f); // BUG
namespace{
/*--------------------------------------------------------------------------*/
static Module const* to_module(Block const* owner)
{
  while(true){
    assert(owner);
    if(auto m = dynamic_cast<Module const*>(owner)){
      return m;
    }else if(auto b = dynamic_cast<Block const*>(owner->owner())){ untested();
      owner = b;
    }else if(auto st = dynamic_cast<Statement const*>(owner->owner())){
      owner = st->scope();
    }else{ untested();
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
    set_label(af->variable()->name());
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
#if 1
    assert(_af);
    make_cc_af(o, *_af);
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
    for (Base const* x : F.header()){
      auto coll = prechecked_cast<AF_Arg_List const*>(x);
      assert(coll);

      if(coll->is_output()){
	qual = "/*output*/ &";
      }else{
	qual = "";
      }
      for(auto i : *coll){
	o << sep << "ddouble " << qual;
	o << "/*" << i->name() << "*/";
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
  _args.set_owner(this);
  std::string name;
  f >> _type >> name;
  assert(!_variable);
  auto data = new TData;
  _variable = new Token_VAR_REF(name, this, data);
  set_identifier(name); // parse_identifier?
  trace1("PAF", name);
  _block.new_var_ref(_variable);

 // parse_block_variables(f, _block._variables);
  _args._variables.set_owner(&_args); // ..

  for (;;) {
    size_t here = f.cursor();
    Variable_Stmt* s = NULL;
    trace1("af variables...", f.tail().substr(0,30));
    ONE_OF	// module_item
      || f.umatch(";")
      || ((f >> "input ") && (f >> _args))
      || ((f >> "output ") && (f >> _args))
      || ((f >> "inout ") && (f >> _args))
      || ((f >> "real ") && (s = new Variable_Stmt(f, &_args)))
      || ((f >> "integer ") && (s = new Variable_Stmt(f, &_args)))
      ;
    if (!s){
      trace1("done?", _args.lookup("x"));
    }
    if (s){
      _args._variables.push_back(s);
    }else if (!f.more()) { untested();
      f.warn(bWARNING, "premature EOF (analog function)");
      break;
    }else if (f.stuck(&here)) {
      break;
    }else if (!f){ untested();
      // bug? handle in "stuck" below?
      throw Exception_CS_("error parsing variables\n", f);
    }else{
    }
  }
  trace1("body?", f.tail().substr(0,20));
  _block.set_owner(this);

  f >> _block; // actually a Statement, but always a CtrlBlock, for simplicity

  if(f >> "endfunction"){
  }else{
    throw Exception_CS_("need 'endfunction'\n", f);
  }

  assert(!_function);
  _function = new AF(this);
}
/*--------------------------------------------------------------------------*/
Block* AnalogFunctionBody::scope()
{
  auto o = prechecked_cast<Analog_Function*>(owner());
  return &o->_args;
}
/*--------------------------------------------------------------------------*/
Base* AnalogFunctionArgs::lookup(std::string const& k, bool recurse)
{
  trace2("AnalogFunctionArgs::lookup", k, recurse);
  Base* b = Block::lookup(k, false);
  if(dynamic_cast<Token_VAR_DECL const*>(b)){
    return b;
  }else{
  }

  b = Block::lookup(k, recurse);
  if(auto n = dynamic_cast<Token_ARGUMENT const*>(b)){
    trace2("AnalogFunctionArgs::lookup1 arg", k, n->_var);
    if(n->_var){
      return n->_var;
    }else{
      return b;
    }
  }else if(dynamic_cast<Token_VAR_DECL const*>(b)){
    return NULL;
  }else if(dynamic_cast<Token_VAR_REF const*>(b)){ untested();
    return NULL;
  }else if(dynamic_cast<Token_NODE const*>(b)){ untested();
      // nodes not allowed here.
    return NULL;
  }else{
    return b;
  }
}
/*--------------------------------------------------------------------------*/
Base* AnalogFunctionBody::lookup(std::string const& k, bool recurse)
{
  Base* b = AnalogCtrlBlock::lookup(k, false);
  if(b){
    return b;
  }else if(recurse){
    assert(scope());
    assert(scope() != this);
    // b = scope()->lookup(k, true);

    if(dynamic_cast<Token_NODE const*>(b)){ untested();
      // nodes not allowed here.
      return NULL;
    }else if(b){ untested();
      trace2("AnalogFunctionBody::lookup1 arg", k, b);
      return b;
    }else{
      return scope()->lookup(k, recurse);
      // return b;
    }
  }else{ untested();
    return scope()->lookup(k, false);
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
void AnalogFunctionBody::dump(std::ostream& o) const
{
 AnalogCtrlBlock::dump(o);
 return;
  for(auto* i : variables_()) { untested();
    i->dump(o);
  }
  o__ "begin\n";
  { untested();
    indent x;
    Block::dump(o);
  }
  o__ "end\n";
}
/*--------------------------------------------------------------------------*/
Analog_Function::~Analog_Function()
{
  delete _variable;
  _variable = NULL;
  delete _function;
  _function = NULL;
}
/*--------------------------------------------------------------------------*/
void Analog_Function::dump(std::ostream& o) const
{
  assert(_variable);
  o__ "analog function " << _type << " " << _variable->name() << ";\n";
  {
    indent x;
    o << _args;
    incomplete();
   // o << _vars;
    o__ "";
    _block.dump(o);
  }
  o__ "endfunction\n";
}
/*--------------------------------------------------------------------------*/
void AnalogFunctionArgs::parse(CS& f)
{
  auto n = new AF_Arg_List(f, this);
  push_back(n);
}
/*--------------------------------------------------------------------------*/
void AnalogFunctionArgs::dump(std::ostream& o) const
{
  for(auto* i : _variables) {
    i->dump(o);
  }
  for(auto const& i : *this){
    o__ *i; // ->dump(o);
  }
}
/*--------------------------------------------------------------------------*/
bool AnalogFunctionArgs::new_var_ref(Base* b)
{
  auto t = prechecked_cast<Token*>(b);
  assert(t);
  trace1("AF_args::new_var_ref", t->name());
  Base* ex = NULL;
  if(auto T = dynamic_cast<Token const*>(b)){
    ex = lookup(T->name(), false);
    trace1("AF_args::new_var_ref0", ex);
  }else{ untested();
  }

  Token_ARGUMENT* arg = NULL;
  Token_VAR_DECL* decl = NULL;
  if((arg = dynamic_cast<Token_ARGUMENT*>(b))){
    trace1("AF_args::new_var_ref ARG", t->name());
    if(dynamic_cast<Token_ARGUMENT*>(ex)){ untested();
      throw Exception("duplicate argument");
    }else if(auto dd = dynamic_cast<Token_VAR_DECL*>(ex)){
      trace1("AF_args::new_var_ref ARG + DECL", t->name());
      arg->_var = dd;
      return Block::new_var_ref(arg);
    }else if(auto tt = dynamic_cast<Token*>(ex)){ untested();
      return Block::new_var_ref(tt);
    }else if(ex){ untested();
      assert(0);
      unreachable();
    }else{
      return Block::new_var_ref(b);
    }
  }else if((decl = dynamic_cast<Token_VAR_DECL*>(b))){
    trace2("AF_args::new_var_ref DECL", t->name(), ex);
    if(dynamic_cast<Token_VAR_DECL*>(ex)){
      throw Exception("duplicate variable name " + t->name());
    }else if(auto aa=dynamic_cast<Token_ARGUMENT*>(ex)){
      trace2("AF_args::new_var_ref DECL + ARG", t->name(), aa->_var);
      if(aa->_var){ untested();
	throw Exception("duplicate variable name");
      }else{
	aa->_var = decl;
      }
      Block::new_var_ref(aa);
      return true; // OK
    }else if(auto tt = dynamic_cast<Token*>(ex)){ untested();
      return Block::new_var_ref(tt);
    }else if(ex){ untested();
      assert(0);
      unreachable();
    }else{
      return Block::new_var_ref(b);
    }
  }else if(dynamic_cast<Token_VAR_REF*>(b)) { untested();
    return Block::new_var_ref(b);
  }else{ untested();
    assert(0);
    return Block::new_var_ref(b);
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool AnalogFunctionBody::new_var_ref(Base* b)
{
  return Block::new_var_ref(b);
}
/*--------------------------------------------------------------------------*/
// bool AnalogEvtExpression::update(RDeps const* r=NULL);
// {
//   trace2("EvtExpression::update", _rdeps.size(), r);
//   Expression_::update(r);
// }
/*--------------------------------------------------------------------------*/
bool AnalogEvtExpression::is_used_in(Base const* b)const
{
  // propagate to owner instead??
  for(auto& i : _rdeps){
    if(i == b){
      return true;
    }else{
    }
  }

  return false;
}
/*--------------------------------------------------------------------------*/
void AnalogEvtExpression::parse(CS& file)
{
  Expression rhs(file);
  file >> ","; // LiSt??
  assert(owner());
  // Expression_::set_owner(scope());
  resolve_symbols(rhs);

  set_rdeps();
}
/*--------------------------------------------------------------------------*/
void AnalogEvtExpression::dump(std::ostream& o) const
{
  o << "("; // here?
  Expression_::dump(o);
  o << ")"; // here?
}
/*--------------------------------------------------------------------------*/
void AnalogEvtExpression::set_rdeps()
{
  if(function()->has_tr_begin()){
    add_rdep(&tr_begin_tag);
  }else{
  }
  if(function()->has_tr_eval()){
    add_rdep(&tr_eval_tag);
  }else{
  }
  if(function()->has_tr_review()){ untested();
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
}
/*--------------------------------------------------------------------------*/
// incomplete. expressions...
FUNCTION_ const* AnalogEvtExpression::function() const
{
  assert(size());
  Token const* t = *begin();
  assert(t);
  Token_CALL const* c = prechecked_cast<Token_CALL const*>(t);
  assert(c);
  return c->f();
}
/*--------------------------------------------------------------------------*/
AnalogExpression::~AnalogExpression()
{
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void AF_Arg_List::parse(CS& f)
{
  std::string dir = f.last_match().substr(0,3);
//  f >> _type;
  if(dir=="inp"){
    _direction = a_input;
  }else if(dir=="out"){
    _direction = a_output;
  }else if(dir=="ino"){
    _direction = a_inout;
  }else{ untested();
    trace2("AF_Arg_List::parse", f.tail().substr(0,10), dir);
    unreachable();
  }
  Analog_Function* o = prechecked_cast<Analog_Function*>(scope()->owner());
  assert(o);
  AnalogFunctionArgs* oo = prechecked_cast<AnalogFunctionArgs*>(owner());
  assert(oo);

  // Can't parse Tokens, cf AnalogDeclareVars::parse
  LiSt<String_Arg, '\0', ',', ';'> l;
  l.parse(f);
  for(auto i : l){
    trace1("AF_Arg_List::parse alreacy declared?", i->to_string());
    auto b = owner()->lookup(i->to_string(), false);
    auto v = dynamic_cast<Token_VAR_DECL*>(b);

    if(dynamic_cast<Token_ARGUMENT const*>(b)){ untested();
      throw Exception_CS_("already declared: " + i->to_string() + "\n", f);
    }else if(dynamic_cast<Token_VAR_DECL const*>(b)){
      incomplete();
    }else if(v){ untested();
//      v->set_arg? set_used_in?
    }else{
    }

    auto t = new Token_ARGUMENT(i->to_string());
    t->_var = v;
    trace1("stash", t->name());
    try{
      owner()->new_var_ref(t);
    }catch(Exception const& e){ untested();
      throw Exception_CS_(e.message(), f);
    }

    if(v){
//      v->deps().add_sens(t);
    }else{
    }

    _l.push_back(t);
  }

}
/*--------------------------------------------------------------------------*/
void AF_Arg_List::dump(std::ostream& o)const
{
  static std::string names[] = {"input", "output", "inout"};
  o << names[_direction] << " ";
 // LiSt<Analog_Function_Arg, '\0', ',', ';'>::dump(o);
  _l.dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
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
  assert(owner());
  assert(dynamic_cast<Block const*>(owner()));
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
Probe* new_Probe(std::string const& xs, Branch_Ref const& br)
{
  return new Probe(xs, br);
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_nodes() const
{
  size_t ret=1;

  for(auto i : ddeps()){
    if(i->branch()->is_short()){
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
    }else if(i->branch()->is_short()){
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

  for(auto p : _rdeps){
    _br->unset_used_in(p);
  }

//  if(_rdeps.size()){ untested();
//    _br->unset_used_in(this);
//  }

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
  new_probe_map();
}
/*--------------------------------------------------------------------------*/
Analog::~Analog()
{
  _list.clear();
  _functions.clear();

  // name clash, Gnucap probes vs Verilog-AMS "probe branch"
  delete _probes; // .clear();
}
/*--------------------------------------------------------------------------*/
bool Analog::has_block() const
{
  return !list().is_empty();
}
/*--------------------------------------------------------------------------*/
void Analog::new_probe_map()
{
  assert(!_probes);
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
  }else{
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
    if(n->key() == f){
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
  if(f=="flow" || f=="potential") {
    // TODO: return FUNCTION_*, VAMS_XS* from nature
    return (FUNCTION_*)(1); // TODO true;
  }else{
  }

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
void Analog::push_back(Base* ab)
{
  if(auto c = dynamic_cast<AnalogConstruct*>(ab)){
    _list.push_back(c);
  }else{ untested();
    unreachable();
  }
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
    push_back(ab);
  }
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
/*--------------------------------------------------------------------------*/
bool AnalogProceduralAssignment::is_used_in(Base const*b)const
{
 // return AnalogStmt::is_used_in(b);
  if (b == &tr_begin_tag){
    return true;
  }else if (_a.is_used_in(b)) { untested();
    return true;
  }else if (AnalogStmt::is_used_in(b)) {
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
AnalogStmt::~AnalogStmt()
{
}
/*--------------------------------------------------------------------------*/
// propagate individually?
bool AnalogProceduralAssignment::propagate_rdeps(RDeps const& r)
{
  assert(owner());
  auto s = dynamic_cast<Statement*>(owner_());
  if(s){
  }else{
  }
  bool ret = false;
  for(auto n : r) {
   // auto p = _rdeps.insert(n);
    //if(p.second)
    {
      ret |= propagate_rdep(n);
      // ret = propagate_rdep(*p.first);
      //ret = s->propagate_rdep(*p.first);
    }
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
bool Probe::propagate_rdeps(RDeps const& r) const
{
  bool ret = false;
  for(auto n : r) {
    auto p = _rdeps.insert(n);
    if(p.second){
      ret = _br->set_used_in(*p.first); // unset?
    }else{
    }
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
Probe* new_Probe(std::string const& xs, Branch_Ref const& br);
Probe const* Analog::new_probe(std::string const& xs, Branch_Ref const& br, Module* m)
{
    trace1("new_probe", br.has_name());
  std::string flow_xs;
  std::string pot_xs;

  if(br->discipline()){
    trace2("new_probe", xs, br->discipline()->identifier());
    flow_xs = br->discipline()->flow()->access().to_string();
    pot_xs = br->discipline()->potential()->access().to_string();
  }else{
    // huh?
  }

  std::string nn = xs;
  if(xs == flow_xs || xs == "flow"){
    nn = "flow";
  }else if( xs == pot_xs || xs == "potential" ){
    nn = "potential";
//  }else if( xs == "_filter"){ untested();
//   // obsolete?
//    br->set_filter();
  }else{ untested();
    trace1("new_probe", xs);
    incomplete(); // port branch?
    nn = xs;
  }

  std::string k = nn + "_" + br.pname() + "_" + br.nname();
  if(br.has_name()){
    k = nn + "_" + br.name();
  }else{
  }

  assert(_probes);
  Probe*& prb = (*_probes)[k];

  if(prb) {
  }else{
    prb = ::new_Probe(nn, br);
//    prb->set_label(k);
    m->install(prb); // duplicate reference..?
  }

  trace1("new_probe", br.has_name());
  return prb;
}
/*--------------------------------------------------------------------------*/
// todo: detach
Probe const* Module::new_probe(std::string const& xs, Branch_Ref const& br)
{
  auto a = prechecked_cast<Analog*>(_analog);
  assert(a);
  FUNCTION_ const* f = a->new_probe(xs, br, this);

  auto pr = prechecked_cast<Probe const*>(f);
  assert(pr);
  return pr;
}
/*--------------------------------------------------------------------------*/
// mg_code?
void Statement::set_rdeps(TData const& t)
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
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
