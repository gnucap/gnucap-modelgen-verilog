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
#include "mg_error.h"
#include "mg_options.h"
#include "m_tokens.h"
#include <e_cardlist.h> // TODO: really?
#include <u_opt.h>
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
static Base* parse_real(CS& f, Block* o)
{
  trace1("AnalogBlock::parse real", f.tail().substr(0,10));
  return new ListOfBlockRealIdentifiers(f, o);
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
  if(ONE_OF
    || f.umatch("$display ")
    || f.umatch("$finish ")
    || f.umatch("$monitor ")
    || f.umatch("$strobe ")
    || f.umatch("$write ")
    || f.umatch("$debug ")
  ){
    f.reset(here);
    try{
      return new System_Task(f, o);
    }catch(Exception const& e){ untested();
      return NULL;
    }
  } else{
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
// Variable::Variable
static Variable* parse_variable(CS& f, Block* o)
{
  size_t here = f.cursor();
  std::string what;
  f >> what;
  trace1("parse_variable", what);
  Base* b = o->lookup(what);
  Variable* v = dynamic_cast<Variable*>(b);
  if(!v){
    f.reset_fail(here);
    trace1("not a variable", f.tail().substr(0,10));
    return NULL; // BUG
  }else{
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
  // _lhs.set_owner(o);
  // TODO: LhsRef?
  // f >> _lhs;
  Variable* l = parse_variable(f, owner());
  // assert(l->name() == name());?

  if(!f){
    assert(!_lhs);
    f.reset_fail(here);
  }else if(f >> "="){
    _lhs = l;
    assert(_lhs);
    parse_rhs(f);

    if(!is_reachable()) {
    }else if(l){
      l->update_deps(deps());
    }else{ untested();
      unreachable(); // loop statement??
    }

  }else{ untested();
    f.reset_fail(here);
//    throw Exception_CS_("no assignment", f);
  }
}
/*--------------------------------------------------------------------------*/
// void Assignment::parse(CS& f)?
void AnalogProceduralAssignment::parse(CS& f)
{
  assert(_a.owner());
  size_t here = f.cursor();

  trace2("Proc assignment?", f.tail().substr(0,20), _a.owner());
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
    }else{ itested();
      f.warn(bWARNING, "missing semicolon?");
    }
  }else{
  }
  return;

  {

#if 0
    f >> _a;
#else

    Base* b = _a.owner()->lookup(what);
    Variable* v = dynamic_cast<Variable*>(b);
    if(!v){
      f.reset(here);
      trace1("not a variable", f.tail().substr(0,10));
      throw Exception_No_Match("what's this: " + what);
    }else if(f >> "=") {
      assert(v->name() != "");
      Variable const* cv=v;
      trace4("got a variable", what, f.tail().substr(0,10), v->name(), cv->owner());
      _a.set_lhs(v);
      trace1("pA", f.tail().substr(0,10));
      _a.parse_rhs(f);
      f >> ";";

      // if reachable?
      if(_a.is_reachable()) {
	v->update_deps(_a.deps());
      }else{
      }

    }else{ untested();
      throw Exception_CS_("need assign op", f);
    }
#endif
  }
}
/*--------------------------------------------------------------------------*/
void AnalogProceduralAssignment::dump(std::ostream& o)const
{
  o__ "";
  _a.dump(o);
  o << ";\n";
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
static Base* parse_analog_stmt_or_null(CS& file, Block* owner)
{
  assert(owner);
  Base* ret = NULL;

  trace1("parse_analog_stmt_or_null", file.tail().substr(0,30));
//  size_t here = file.cursor();
  ONE_OF	// module_item
    || (file >> ";")
    || ((file >> "begin") && (ret = pArse_seq(file, owner)))
    || ((file >> "real ") && (ret = parse_real(file, owner)))
    || ((file >> "integer ") && (ret = parse_int(file, owner)))
    || ((file >> "if ") && (ret = parse_cond(file, owner)))
    || ((file >> "case ") && (ret = parse_switch(file, owner)))
    || ((file >> "while ") && (ret = new AnalogWhileStmt(file, owner)))
    || ((file >> "for ") && (ret = new AnalogForStmt(file, owner)))
    || ((file >> "@ ") && (ret = new_evt_ctl_stmt(file, owner)))
    || (ret = parse_proc_assignment(file, owner))
    || (ret = parse_system_task(file, owner))
    || (ret = parse_contribution(file, owner))
    ;

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
void AnalogConditionalStmt::parse(CS& file)
{
  _body.set_owner(owner());
  _false_part.set_owner(owner());

  file >> "(" >> _cond >> ")";
  // set reachabilities here!
  if(file >> ";"){
  }else{

    if(owner()->is_never()) {
      _body.set_never();
      _false_part.set_never();
    }else if(_cond.is_true()) {
      if(owner()->is_always()) { untested();
	_body.set_always();
      }else{
      }
      _false_part.set_never();
    }else if(_cond.is_false()) {
      if(owner()->is_always()) { untested();
	_false_part.set_always();
      }else{
      }
      _body.set_never();
    }else{
    }

    file >> _body;
    if(file >> "else "){
      trace1("got else branch", file.tail().substr(0, 1));
      file >> _false_part;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void AnalogConditionalStmt::dump(std::ostream& o)const
{
  // TODO revisit indentation logic.
  static bool cont; // yikes. recurse.

  bool omit_true = !options().dump_unreachable() && _cond.is_false(); // _body->is_reachable?
  bool omit_false = !options().dump_unreachable() && _cond.is_true();
  bool omit_cond = omit_true || omit_false;

  if(omit_cond) {
  }else if(cont){ untested();
  }else{
    o__ "";
  }

  if(omit_cond) {
  }else{
    o << "if (" << _cond << ") ";
    cont = false;
  }
  {
    if(omit_true) {
    }else if(omit_cond){
       o__ "";
      _body.dump(o);
    }else{
      _body.dump(o);
    }
  }
  if(omit_false){
  }else if(_false_part){
    if(omit_true){
      cont = false;
    }else{
      o__ "else ";
    }
    // indent?
    if(omit_cond){
      o__ "";
    }else{
     // o << "X";
    }
    _false_part.dump(o);
  }else{
  }
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
bool AnalogWhileStmt::update()
{
  bool ret = false;
  while(true){
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
  file >> "(" >> _cond >> ")";
  if(file >> ";"){
  }else{
    _body.set_owner(owner());
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
//    }else if(_cond.update()){
    }else if (_body.update()){
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
  Assignment* tail = parse_assignment_or_null(f, owner());
  _tail = tail;

  f >> ")";
  if(f >> ";"){ untested();
  }else{
    _body.set_owner(owner()); // needed?
    f >> _body;
  }

  update();
}
/*--------------------------------------------------------------------------*/
bool AnalogSeqBlock::update()
{
  bool ret = false;
  for(auto i: _block){
    if(auto s = dynamic_cast<AnalogStmt*>(i)){
      ret |= s->update();
    }else{
    }
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
  }else{ untested();
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
    for(auto j : *_cond){
      trace2("==", ctrl.back()->name(), j->expression().back()->name());
      Token_BINOP_ b("==", ctrl.back(), j->expression().back(), NULL);
      b.stack_op(&result);
      b.pop();
      assert(result.size());

      if(is_false(result)) { untested();
      }else{
	all_never = false;
      }
      if(is_true(result)) {
	set_always();
	break;
      }else{
      }
    }
    if(all_never){ untested();
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

    o << ' ' << _code;
  }
}
/*--------------------------------------------------------------------------*/
CaseGen::CaseGen(CS& f, Block* owner, Expression const& ctrl, bool have_r, bool have_a)
{
  trace1("CaseGen", f.tail().substr(0,20));
  AnalogConstExpressionList* c = new AnalogConstExpressionList();
  c->set_owner(owner);

//  parse(f); 

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

//  size_t here = f.cursor();
//  if(f >> "endcase") { untested();
//    assert(!_cond);
//    f.reset_fail(here);
//  }else
  if(have_a){
    set_never();
  }else if(!have_r){
    // default?
    calc_reach(ctrl);
  }else{
    calc_reach(ctrl);
  }
  if(f){
    _code.set_owner(owner);
    f >> _code;
    if(f){
    }else{ untested();
      incomplete();
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void AnalogSwitchStmt::parse(CS& f)
{
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
      }else{
      }

      size_t here = f.cursor();
      CaseGen* g = new CaseGen(f, owner(), _ctrl.expression(),
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
      }else if(def){ untested();
	f.reset_fail(here);
	delete g;
	throw Exception_CS_("multiple default: not allowed", f);
      }else{
	def = g;
      }

      _cases.push_back(g);
    }
  }
}
/*--------------------------------------------------------------------------*/
void AnalogSwitchStmt::dump(std::ostream& o)const
{
  o__ "case (" << _ctrl << ")\n";

  {
    indent x;
    o << _cases;
  }
  o__ "endcase\n";
}
/*--------------------------------------------------------------------------*/
void AnalogConstruct::parse(CS& f)
{
  assert(!_stmt);
  _stmt = new AnalogCtrlBlock(f, owner());
}
/*--------------------------------------------------------------------------*/
void AnalogSeqBlock::parse(CS& f)
{
  if(f >> ":"){
    _block.parse_identifier(f);
  }else{
  }
  for (;;) {
    if(f >> "end "){
      if(f.peek() == ';') {
	// error(bWARNING, "// stray semicolon?");
	std::cerr << "stray semicolon\n";
	f.skip();
      }else{
      }
      break;
    }else{
    }
    Base* s = parse_analog_stmt(f, &_block);
    if(!s){ untested();
      throw Exception_CS_("bad analog block", f);
    }else{
      _block.push_back(s);
    }
  }
}
/*--------------------------------------------------------------------------*/
void AnalogCtrlBlock::parse(CS& f)
{
  assert(block().owner());
  if(f >> ";"){ untested();
  }else if(f >> "begin"){
    AnalogSeqBlock::parse(f);
  }else{
    Base* b = parse_analog_stmt_or_null(f, &_block);
    if(b) {
      _block.push_back(b);
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void SeqBlock::dump(std::ostream& o)const
{
  Block::dump(o);
}
/*--------------------------------------------------------------------------*/
void BlockVarIdentifier::parse(CS& file)
{
  file >> _name;
  new_var_ref();
}
/*--------------------------------------------------------------------------*/
void BlockVarIdentifier::dump(std::ostream& o)const
{
  o << name();
}
/*--------------------------------------------------------------------------*/
void ListOfBlockIntIdentifiers::dump(std::ostream& o) const
{ untested();
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
//CS& ListOfBlockVarIdentifiers::parse(CS& file)
//{ untested();
//    trace1("AnalogBlock::parse real", name);
//}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void AnalogConstruct::dump(std::ostream& o)const
{
  o__ "analog ";
  _stmt->dump(o);
}
/*--------------------------------------------------------------------------*/
void Assignment::dump(std::ostream& o) const
{
  o << lhsname() << " = " << _rhs;
}
/*--------------------------------------------------------------------------*/
Assignment::~Assignment()
{
  delete _deps;
  _deps = NULL;
}
/*--------------------------------------------------------------------------*/
void Assignment::parse_rhs(CS& cmd)
{
  assert(owner());
  trace1("Assignment::parse_rhs", cmd.tail().substr(0,10));
  Expression rhs(cmd);
  assert(_rhs.is_empty());

  assert(owner());
  assert(deps().empty());
  _rhs.set_owner(owner());
  _rhs.resolve_symbols(rhs);
  cmd.reset(cmd.cursor());
  trace1("Assignment::parse_rhs", bool(cmd));
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::parse(CS& f)
{
  trace1("Branch_Ref::parse", f.tail().substr(0,10));
  if(f >> "("){
  }else{
    throw Exception_No_Match("not a branch");
  }
  std::string pp = f.ctos(",)");
  std::string pn = f.ctos(",)");
  f >> ")";

  assert(owner());
  trace3("Branch_Ref::parse", pp, pn, _br);
  assert(!_br);
  Branch_Ref b;
  assert(!b._br);
  b = owner()->new_branch(pp, pn);

  assert(b._br);

  *this = b;
  assert(owner());
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::dump(std::ostream& o)const
{
  assert(_br);
  if(has_name()){
    o << "(" << *_name << ")";
  }else if(_br->n()==&mg_ground_node){
    o << "(" << pname() << ")";
  }else{
    o << "(" << pname() << ", " << nname() << ")";
  }
}
/*--------------------------------------------------------------------------*/
void Contribution::parse(CS& cmd)
{
  size_t here = cmd.cursor();
  cmd >> _name;

  _branch.set_owner(owner());
  if(!owner()->is_reachable()){
    // set_never();
  }else{
  }

  try{
    cmd >> _branch;
  }catch (Exception_No_Match const&){
    cmd.reset(here);
    trace2("not a Contribution", _name, cmd.tail().substr(0,20));
    throw Exception_No_Match("not a contribution");
  }
  trace1("Contribution", _name);
  if(_branch->deps().is_linear()){
  }else{
  }


  if(!_branch->discipline()) {
    throw Exception_CS_("bad discipline", cmd);
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

  // TODO: parse branch_ref?
  //std::string pp = cmd.ctos(",)");
  //std::string pn = cmd.ctos(",)");
  // _name = _lhsname + pp + pn;
  // Branch_Ref nb = new_branch(pp, pn);
  // _branch = nb;
  // assert(_branch);
  // cmd >> ")";
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
    trace1("Assignment::parse", cmd.tail().substr(0,20));
    Expression rhs_(cmd);
    assert(_rhs.is_empty());

    assert(owner());
    _rhs.set_owner(owner());
    _rhs.resolve_symbols(rhs_);
  }
  {

    trace1("Assignment::parse", rhs().back()->name());
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

    if(_branch->deps().is_linear()){
    }else{
    }

    for(const Dep& d : deps()) {
      trace3("contrib dep", d->code_name(), d.is_linear(), _branch->deps().is_linear());
      _branch->add_dep(d);

      if(_branch->deps().is_linear()){
      }else{
      }

      if(!is_direct()){
      }else if(d->branch() != _branch){
      }else if(is_flow_contrib() && d->is_flow_probe()){ untested();
	_branch->set_selfdep();
      }else if(is_pot_contrib() && d->is_pot_probe()){
	_branch->set_selfdep();
      }else{
      }
    }
  }
  cmd >> ";";

  if(is_zero(_rhs)){
  }else if(_nature == disc->flow()){
    set_flow_contrib();
  }

  if(_branch->deps().is_linear()){
  }else{
  }

} // Contribution::parse
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
/*--------------------------------------------------------------------------*/
void Branch::dump(std::ostream& o)const
{ untested();
  o << "(" << _p->name() << ", " << _n->name() << ")";
}
/*--------------------------------------------------------------------------*/
static void dump(std::ostream& out, Expression const& e)
{
//   out << "/* RPN ";
//   for (auto i = e.begin(); i != e.end(); ++i) { untested();
//     out << "" << (*i)->full_name() << " ";
//   }
//  out << "*/\n";
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
  o << ";\n";
}
/*--------------------------------------------------------------------------*/
std::string Branch::name()const
{
  return "(" + _p->name()+", "+_n->name()+")";
}
/*--------------------------------------------------------------------------*/
void Assignment::set_lhs(Variable* v)
{
  _lhs = v;
  assert(v);
  _name = v->name(); // BUG?
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
    o << "\n";
    {
      indent x;
      _block.dump(o);
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
  if(_block.identifier() != ""){
    o << " : " << _block.identifier();
  }else{
  }
  o << "\n";
  {
    indent x;
    _block.dump(o);
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
  file >> _ctl;
  assert(owner());
  // _stmt.set_owner(owner);
  // f >> _stmt;
  _stmt = parse_analog_stmt(file, owner());
}
/*--------------------------------------------------------------------------*/
void AnalogEvtCtlStmt::dump(std::ostream& o) const
{
  o__ "@" << _ctl;
  if(dynamic_cast<AnalogSeqBlock const*>(_stmt)){
    o << " " << *_stmt;
  }else if(_stmt){
#if 0
    o << " " << *_stmt;
#else
    o << "\n";
    {
      indent x;
      o << *_stmt;
    }
#endif
  }else{ untested();
    o << ";";
  }
}
/*--------------------------------------------------------------------------*/
// void Variable_Decl::parse(CS& f)
// { untested();
//   f >> _name;
// }
/*--------------------------------------------------------------------------*/
void make_cc_af_body(std::ostream& o, const Analog_Function& f); // BUG
namespace{
/*--------------------------------------------------------------------------*/
Module const* to_module(Block const* owner)
{
  assert(owner);
  while(true){
    if(auto m = dynamic_cast<Module const*>(owner)){
      return m;
    }else{ untested();
    }
    owner = owner->owner();
    assert(owner);
  }
  unreachable();
  return NULL;
}
/*--------------------------------------------------------------------------*/
void make_cc_af_args(std::ostream& o, const Analog_Function& f)
{
  std::string sep = "";
  std::string qual = "";
  for (auto coll : f.args()){
      if(coll->is_output()){
	qual = "&";
      }else{
	qual = "";
      }
    for(auto i : *coll){
      o << sep << "ddouble " << qual;
      o << " af_arg_" << i->identifier();
      sep = ", ";
    }
  }
}
/*--------------------------------------------------------------------------*/
// analog_in? analog_out?
class AF : public MGVAMS_FUNCTION {
  Analog_Function const* _af{NULL};
public:
  explicit AF(Analog_Function const* af) : _af(af) {
    assert(_af);
    set_label(af->identifier().to_string());
  }

#if 1
  Token* new_token(Module& m, size_t /*na*/) const override {
    trace1("AF::new_token", label());
    m.install(this);
    if(code_name() != ""){
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
    auto mp = to_module(_af->owner());
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
  f >> _name;
  assert(owner());
  owner()->new_var_ref(this);
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
// update_deps?
bool Assignment::update()
{
  incomplete();
  size_t n = _rhs.deps().size();

  auto i=_rhs.begin();
  for(size_t n=_rhs.size(); n--;){
    (*i)->stack_op(&_rhs);
    i = _rhs.erase(i);
  }

  if(n == _rhs.deps().size()){
    return false;
  }else{
    update_deps(deps());
    return true;
  }
}
/*--------------------------------------------------------------------------*/
// propagate_deps?
void Assignment::update_deps(Deps const& d)
{
  // TODO: attrib.
  // TODO: only if reachable.
  assert(_lhs);
  _lhs->update_deps(d);
  if(_deps) {
  }else{
    _deps = rhs().deps().clone();
  }
  _deps->update(d);
//  assert(&deps() != &d);
//  deps().update(d);
}
/*--------------------------------------------------------------------------*/
CS& Module::parse_analog(CS& f)
{
  if(f >> "function "){
     f >> _analog_functions;
  }else{
    AnalogConstruct* ab = new AnalogConstruct();
    ab->set_owner(this);
    ab->parse(f);
    _analog_list.push_back(ab);
  }
  return f;
}
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
    _branch->set_pot_source();
  }
  _type = t_pot;
}
/*--------------------------------------------------------------------------*/
void Contribution::set_flow_contrib()
{
  assert(_branch);
  if(options().optimize_unused() && !owner()->is_reachable()) {
  }else{
    _branch->set_flow_source();
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
  if(!_branch) {
    incomplete();
  }else if(options().optimize_unused() && !owner()->is_reachable()) {
  }else if (is_flow_contrib()) {
    _branch->dec_flow_source();
  }else if(is_pot_contrib()) {
    _branch->dec_pot_source();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
