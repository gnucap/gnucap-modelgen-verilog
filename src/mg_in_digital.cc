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
#include <globals.h>
#include <e_cardlist.h> // TODO: really?
#include <u_opt.h>
#include "mg_digital.h"
#include "mg_out.h"
#include "mg_in.h"
#include "mg_error.h"
#include "mg_options.h"
#include "mg_discipline.h"
#include "mg_token.h"
#include "mg_event.h"
#include <e_cardlist.h> // TODO: really?
#include <u_opt.h>
#include "globals.h"
#include "mg_storage.h"
/*--------------------------------------------------------------------------*/
#if 0
static Base* parse_initial(CS& file, Block* o)
{ untested();
  return new InitialStmt(o, file);
}
#endif
/*--------------------------------------------------------------------------*/
static Base* parse_cond(CS& file, Block* o)
{
  return new DigitalConditionalStmt(o, file);
}
/*--------------------------------------------------------------------------*/
static Base* parse_switch(CS& file, Block* o)
{ untested();
  (void)file;
  assert(o);
  (void) o;
  assert(!dynamic_cast<Module const*>(o));
  incomplete();
  return nullptr;
 // return new DigitalSwitchStmt(o, file);
}
/*--------------------------------------------------------------------------*/
static Base* new_evt_ctl_stmt(CS& file, Block* o)
{
  auto cb = new DigitalEvtCtlStmt();
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
  size_t here = f.cursor();
  if(f >> "begin ") { untested();
    f.reset(here);
    return new DigitalSeqStmt(f, owner);
  }else{
    return nullptr;
  }
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
      return nullptr;
    }
  }else{
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
void DigitalProceduralAssignment::parse(CS& f)
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
Statement* DigitalProceduralAssignment::deep_copy(Base* owner) const
{ untested();
//  return new DigitalProceduralAssignment(this);
  auto b = prechecked_cast<Block*>(owner);
  assert(b);
  std::stringstream s;
  dump(s);
  CS f(CS::_STRING, s.str());
  trace1("AP::deep_cp", s.str());
  auto a = new DigitalProceduralAssignment(f, b);
  assert(a->scope());
  //a->dump(s);
  return a;
}
/*--------------------------------------------------------------------------*/
void DigitalProceduralAssignment::dump(std::ostream& o)const
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
  }else{ untested();
    // optimised out. BUG: ';' or not ';'?
  }
}
/*--------------------------------------------------------------------------*/
static Base* parse_proc_assignment(CS& f, Block* o)
{
  assert(o);
  f.skipbl();
  trace1("parse_proc_assignment", f.tail().substr(0,30));
  try{
    auto n = new DigitalProceduralAssignment(f, o);
    if(f){
      return n;
    }else{
      delete n;
    }
  }catch(Exception_No_Match const&){ untested();
  }
  return nullptr;
}
/*--------------------------------------------------------------------------*/
DigitalProceduralAssignment::DigitalProceduralAssignment(CS& file, Block* o)
  : DigitalStmt()
{
  set_owner(o);
  _a.set_owner(this);
  parse(file);
}
/*--------------------------------------------------------------------------*/
static Base* parse_stmt_or_null(CS& file, Block* scope)
{
  size_t here = file.cursor();
  assert(scope);
  Base* ret = nullptr;

  trace1("parse_digital_stmt_or_null", file.tail().substr(0,30));
  ONE_OF	// module_item
    || (file >> ";")
    || (ret = parse_seq(file, scope))
    || ((file >> "if ") && (ret = parse_cond(file, scope)))
    || ((file >> "case ") && (ret = parse_switch(file, scope)))
    || ((file >> "while ") && (ret = new DigitalWhileStmt(file, scope)))
    || ((file >> "for ") && (ret = new DigitalForStmt(file, scope)))
    || ((file >> "@ ") && (ret = new_evt_ctl_stmt(file, scope)))
    || (ret = parse_proc_assignment(file, scope))
    || (ret = parse_system_task(file, scope))
    ;

  if(file.stuck(&here)) {
    trace1("parse_digital_stmt_or_null? stuck", file.tail().substr(0,30));
    file.reset_fail(here);
  }else{
    trace1("parse_digital_stmt_or_null? cont", file.tail().substr(0,30));
  }

  return ret;
}
/*--------------------------------------------------------------------------*/
Base* DigitalSeqBlock::parse_stmt_or_null(CS& f, Block* b) const
{
  return ::parse_stmt_or_null(f, b);
}
/*--------------------------------------------------------------------------*/
void DigitalCtrlStmt::parse(CS& f)
{ untested();
  _body.set_owner(this);
  f >> _body;
  scope()->add_block(&_body);
}
/*--------------------------------------------------------------------------*/
void DigitalCtrlStmt::dump(std::ostream& o) const
{
  if(!_body){
    o << ";\n";
  }else{
    o << " ";
    _body.dump(o);
  }
}
/*--------------------------------------------------------------------------*/
void DigitalCtrlStmt::submit_variable_access(Variable_Access& va) const
{ untested();
  va &= _body.variable_access();
}
/*--------------------------------------------------------------------------*/
bool DigitalCtrlStmt::propagate_rdeps(RDeps const& r)
{
  // incomplete(); // remove?
  return Statement::propagate_rdeps(r);
}
/*--------------------------------------------------------------------------*/
DigitalWhileStmt::DigitalWhileStmt(CS& file, Block* o)
{ untested();
  set_owner(o);
  parse(file);
}
/*--------------------------------------------------------------------------*/
#if 0
static Assignment* parse_assignment_or_null(CS& f, Statement* owner)
{ untested();
  auto a = new Assignment(f, owner);
  if(f) { untested();
  }else{ untested();
    delete a;
    a = nullptr;
  }
  return a;
}
#endif
/*--------------------------------------------------------------------------*/
bool DigitalProceduralAssignment::update()
{
  bool ret;
  if(options().optimize_unused() && !scope()->is_reachable()) { untested();
    ret = false;
  }else{
    RDeps r(rdeps());
    assert(r.size()==rdeps().size());
  // TODO copy from _rdeps to _a._rdeps..?
    ret = _a.update(&r);
    if(r.size()==rdeps().size()){
    }else{ untested();
    }
    ret |= merge_rdeps(r); // into Statement
  }

  ret |= propagate_rdep(&tr_begin_tag); // BUG. propagates across event block boundaries.
  // ret |= propagate_rdep(&tr_restore_tag);
  if(is_state_var()){
    ret |= propagate_rdep(&tr_advance_tag);
    ret |= propagate_rdep(&tr_accept_tag);
  }else{ untested();
  }

  return DigitalStmt::update() || ret;
} // AnalogProceduralAssignment::update()
/*--------------------------------------------------------------------------*/
#if 0 // code?
void CaseGen::calc_reach(Expression const& ctrl)
{ untested();
  Expression_ result;
  result.set_owner(this);

  if(!ctrl.size()){ untested();
    incomplete();
  }else if(_cond && _cond->size()){ untested();
    bool all_never = true;
    for(auto const& j : *_cond){ untested();
      trace2("==", ctrl.back()->name(), j->expression().back()->name());
      Token_BINOP_ b("==", ctrl.back(), j->expression().back(), nullptr);
      b.stack_op(&result);
      b.pop();
      assert(result.size());

      if(is_false(result)) { untested();
      }else{ untested();
	all_never = false;
      }
      if(is_true(result)) { untested();
	_body.set_always();
	break;
      }else{ untested();
      }
    }
    if(all_never){ untested();
      set_never();
    }else{ untested();
    }
  }else if(_cond){ untested();
    set_never();
  }else{ untested();
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
{ untested();
  if(!options().dump_unreachable() && is_never()){ untested();
  }else{ untested();
    if(_cond){ untested();
      o__ *_cond;
  //    o << " :";
    }else{ untested();
      o__ "default:";
    }
//    o << ' ';
    DigitalCtrlStmt::dump(o);
  }
}
/*--------------------------------------------------------------------------*/
CaseGen::CaseGen(CS& f, Block* o, Expression const& ctrl, bool have_r, bool have_a)
{ untested();
  trace2("CaseGen", f.tail().substr(0,20), have_a);
  assert(!dynamic_cast<Module const*>(o));
  DigitalConstExpressionList* c = new DigitalConstExpressionList();
  assert(o);
  c->set_owner(o);
  set_owner(o);
  assert(owner());
  trace2("CaseGen", is_always(), is_never());

  if (f >> "default"){ untested();
    delete c;
    f >> ":"; // is optional..
    size_t here = f.cursor();
    if(f >> "endcase") { untested();
      f.reset_fail(here);
    }else{ untested();
      f.reset(here);
    }
  }else if(f >> *c) { untested();
    trace1("CaseGen2", f.tail().substr(0,20));
    _cond = c;
  }else{ untested();
    delete c;
    throw Exception_CS_("bad switch statement", f);
  }

  if(have_a) { untested();
    set_never();
    assert(is_never());
  }else if(!have_r){ untested();
    // default?
    calc_reach(ctrl);
  }else{ untested();
    calc_reach(ctrl);
  }
  if(f){ untested();
    DigitalCtrlStmt::parse(f);
    if(f){ untested();
    }else{ untested();
      incomplete();
    }
  }else{ untested();
  }
  trace2("CaseGen done", have_a, is_never());

  assert(owner());
}
/*--------------------------------------------------------------------------*/
void DigitalSwitchStmt::parse(CS& f)
{ untested();
  assert(owner());
  assert(!dynamic_cast<Module const*>(owner()));
  //_ctrl.set_owner(scope());
  _ctrl.set_owner(this);
  _body.set_owner(this);
  auto sb = dynamic_cast<SeqBlock*>(scope());
  assert(sb);
  sb->add_block(&_body); // re-use var_ref?

  f >> "(" >> _ctrl >> ")";
  CaseGen* def = nullptr;
  bool have_reachable = false;
  bool have_always = false;

  while(true){ untested();
    if(f >> "endcase"){ untested();
      break;
    }else{ untested();
      if(def){ untested();
	// default is out of order. reachability messed up.
	// CS.warn ...
	// use update() to fix reachability?
      }else{ untested();
      }

      size_t here = f.cursor();
      CaseGen* g = new CaseGen(f, &_body, _ctrl.expression(),
	  have_reachable, have_always);
//      g->set_owner(owner());
//      g->set_ctrl(&_ctrl.expression());
//      f >> *g;
//
      if(g->is_always()){ untested();
	have_always = true;
      }else if(g->is_reachable()){ untested();
	have_reachable = true;
      }else{ untested();
      }

      if(!g->is_default()) { untested();
      }else if(def){ untested();
	f.reset_fail(here);
	delete g;
	throw Exception_CS_("multiple default: not allowed", f);
      }else{ untested();
	def = g;
      }

      _body.push_back(g);
    }
  }
}
/*--------------------------------------------------------------------------*/
void DigitalSwitchStmt::dump(std::ostream& o)const
{ untested();
  o__ "case (" << _ctrl << ")\n";
  { untested();
    indent x;
    o << _body;
  }
  o__ "endcase\n";
}
/*--------------------------------------------------------------------------*/
#endif
bool AlwaysConstruct::is_used_in(Base const*) const
{
  // incomplete.
  return true;
}
/*--------------------------------------------------------------------------*/
void AlwaysConstruct::new_block()
{ untested();
  unreachable();
  _block.set_owner(this);
}
/*--------------------------------------------------------------------------*/
void AlwaysConstruct::push_back(Statement*s)
{ untested();
  // assert(0); // no longer used?
  assert(_block);
  _block.push_back(s);
}
/*--------------------------------------------------------------------------*/
void AlwaysConstruct::parse(CS& f)
{
  assert(owner());
  assert(!_block);
  _block.set_owner(this);
//  auto ab = new DigitalSeqBlock(f, this);
  f >> _block;
}
/*--------------------------------------------------------------------------*/
void AlwaysConstruct::dump(std::ostream& o)const
{
  Base const* b = &_block;
  b->dump(o);
}
/*--------------------------------------------------------------------------*/
bool AlwaysConstruct::update()
{
  bool ret = false;
  while(_block.update()){
    ret = true;
    trace0("AnalogConstruct update");
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
void DigitalSeqStmt::parse(CS& f)
{ untested();
  _block.set_owner(this);

  if(is_never()) { untested();
    _block.set_never();
  }else if(is_always()) { untested();
    _block.set_always();
  }else{ untested();
  }

  f >> _block;
  // _block.update();
  // TODO: add block.
}
/*--------------------------------------------------------------------------*/
void DigitalSeqStmt::submit_variable_access(Variable_Access& va) const
{ untested();
  va &= block().variable_access();
}
/*--------------------------------------------------------------------------*/
#if 1
void DigitalSeqBlock::set_owner(Statement* st)
{
  Block* o = st->scope();
  if(auto x = dynamic_cast<SeqBlock const*>(o)) {
    if(x->has_sensitivities()){ untested();
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
void AlwaysConstruct::submit_variable_access(Variable_Access& va) const
{
  va &= _block.variable_access();
}
/*--------------------------------------------------------------------------*/
#if 0 // analog?
Branch_Ref parse_branch(Block* owner, CS& f)
{ untested();
  size_t here = f.cursor();
  if(f >> "("){ untested();
  }else{ untested();
    f.reset_fail(here);
    return Branch_Ref();
    f.reset_fail(here);
    throw Exception_No_Match("not a branch");
  }
  std::string pp = f.ctos(",)");
  std::string pn = f.ctos(",)");
  if(f >> ")"){ untested();
  }else{ untested();
    f.reset_fail(here);
    throw Exception_No_Match("not a branch");
  }

  trace2("a branch", pp, pn);
  assert(owner);
  Branch_Ref b = owner->new_branch(pp, pn);
  trace2("still a branch", pp, pn);
  if(b){ untested();
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
{ untested();
  assert(_br);
  if(auto x = dynamic_cast<Named_Branch const*>(_br)) { untested();
    o << "(" << x->name() << ")";
  }else if(_br->n()->is_ground()) { untested();
    o << "(" << pname() << ")";
  }else{ untested();
    o << "(" << pname() << ", " << nname() << ")";
  }
}
/*--------------------------------------------------------------------------*/
std::string const& Branch_Ref::pname() const
{ untested();
  assert(_br);
  assert(_br);
  if(_r){ untested();
    assert(_br->n());
    return _br->n()->name();
  }else{ untested();
    assert(_br->p());
    return _br->p()->name();
  }
}
/*--------------------------------------------------------------------------*/
std::string const& Branch_Ref::nname() const
{ untested();
  assert(_br);
  if(_r){ untested();
    assert(_br->n());
    return _br->p()->name();
  }else{ untested();
    assert(_br->p());
    return _br->n()->name();
  }
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::set_used_in(Base const* b) const
{ untested();
  _br->set_used_in(b);
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::unset_used_in(Base const* b) const
{ untested();
  return _br->unset_used_in(b);
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
{ untested();
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
  _deps = nullptr;

  if(_use){ untested();
    unreachable();
    std::cerr << "logic error. " << name() << " still used.\n";
    assert(false);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
//bool Branch::propagate_rdeps(RDeps const& b)
//{ untested();
//  return _rdeps.merge(b);
//}
/*--------------------------------------------------------------------------*/
bool Branch::set_used_in(Base const* b)
{ untested();
  for(auto& i : _used_in){ untested();
    if(i == b){ untested();
      return false;
    }else{ untested();
    }
  }
  _used_in.push_back(b);
  return true;
}
/*--------------------------------------------------------------------------*/
void Branch::unset_used_in(Base const* b)
{ untested();
  int found = 0;
  for(auto& i : _used_in){ untested();
    if(i == b){ untested();
      i = nullptr;
      ++found;
#ifndef NDEBUG
      return;
#endif
    }else{ untested();
    }
  }
  if(!found){ untested();
    // can't seem to avoid. cyclic deps...
    // unreachable();
//    throw std::logic_error("cleanup " + code_name());
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void Branch::dump(std::ostream& o)const
{ untested();
  if(_n->is_ground()){ untested();
    o << "(" << _p->name() << ")";
  }else{ untested();
    o << "(" << _p->name() << ", " << _n->name() << ")";
  }
}
/*--------------------------------------------------------------------------*/
#if 0
static void dump(std::ostream& out, Expression const& e)
{ untested();
  e.dump(out);
}
#endif
/*--------------------------------------------------------------------------*/
std::string Branch::name() const
{ untested();
  return "(" + _p->name()+", "+_n->name()+")";
}
/*--------------------------------------------------------------------------*/
bool Branch::is_short() const
{ untested();
  assert(_p);
  assert(_n);
  return _p->number() == _n->number();
}
/*--------------------------------------------------------------------------*/
bool Branch::req_short() const
{ untested();
  if(has_flow_source()){ untested();
    return false;
  }else if(_has_short){ untested();
    return _has_pot_src == _has_short;
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
#endif
#if 0
void DigitalCtrlBlock::dump(std::ostream& o)const
{ untested();
  // base?
//  o << "BBbegin";
  if(size() || identifier() != ""){ untested();
    if(identifier() != ""){ untested();
      o << " : " << identifier();
    }else{ untested();
    }
    if(!options().dump_annotate()){ untested();
    }else if(is_always()){ untested();
      o << " // always";
    }else if(is_never()){ untested();
      o << " // never";
    }else{ untested();
    }
    o << "\n";
    { untested();
//      indent x;
      if(options().dump_annotate()){ untested();
	for(auto i : variables()){ untested();
	  if(auto v = dynamic_cast<Token_VAR_REF const*>(i.second)){ untested();
	    o__ "// " << v->name();
	    if(v->data()){ untested();
	      o << " : " << v->deps().size() << "\n";
	    }else{ untested();
	      o << "???\n";
	    }
	  }else if(dynamic_cast<Block const*>(i.second)){ untested();
	    // later.
	  }else{ untested();
	    o__ "// " << i.first << "\n";
	  }
	}
      }else{ untested();
      }
      for(auto* i : variables_()) { untested();
	i->dump(o);
      }
      if(size()){ untested();
	SeqBlock::dump(o);
      }else{ untested();
      }
    }
  }else{ untested();
    o<< "\n";
    // (why not) annotate?
  }
//  o__ "end\n";
}
#endif
/*--------------------------------------------------------------------------*/
void DigitalSeqBlock::dump(std::ostream& o)const
{
  // SeqBlock::dump, code?
  o << "begin";
  if(identifier() != ""){
    o << " : " << identifier() << "\n";
    indent x;
    for(auto* i : variables_()) { untested();
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
void DigitalExpression::parse(CS& file)
{ untested();
  trace1("DigitalExpression::parse", file.tail().substr(0,100));

  { untested();
    Expression rhs(file);
    file >> ","; // LiSt??
    assert(owner());
    // Expression_::set_owner(scope());
    //
    { untested();
    resolve_symbols(rhs);
    }
  }
}
/*--------------------------------------------------------------------------*/
bool DigitalExpression::is_true() const
{ untested();
  return ::is_true(expression());
}
/*--------------------------------------------------------------------------*/
bool DigitalExpression::is_false() const
{ untested();
  return ::is_false(expression());
}
#endif
/*--------------------------------------------------------------------------*/
void DigitalEvtCtlStmt::parse(CS& file)
{
  _ctrl.set_owner(owner()); // ?
  if(file >> '('){
  }else{ untested();
    file.warn(bDANGER, "expecting '('");
  }
  file >> _ctrl;
  if(file >> ')'){
  }else{ untested();
    file.warn(bDANGER, "need ')'");
  }
  assert(owner());
  _body.set_ctx_event();
  if(_ctrl.is_final()){ untested();
    _body.set_ctx_final();
  }else{
  }
  _body.set_owner(this);
  _body.set_sens(this); // BUG
  file >> _body;
  scope()->add_block(&_body);
}
/*--------------------------------------------------------------------------*/
void DigitalEvtCtlStmt::dump(std::ostream& o) const
{
  o__ _ctrl;
  DigitalCtrlStmt::dump(o);
#if 0
  if(dynamic_cast<DigitalSeqBlock const*>(_stmt)){ untested();
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
bool DigitalEvtCtlStmt::update()
{

 // bool rdd = _rhs.update(&_deps->rdeps());
  bool ret = propagate_rdeps(_ctrl.rdeps());
  while(true){
    _body.clear_vars();
    if ( _ctrl.update(nullptr) ){ untested();
      ret = true;
    }else if (_body.update()){
      ret = true;
    }else{
      break;
    }
  }

  // set_rdeps(_ctrl.rdeps());
  return CtrlStmt::update() || ret;
}
/*--------------------------------------------------------------------------*/
bool DigitalEvtCtlStmt::is_used_in(Base const* b)const
{
  // store rdeps in Statement::_rdeps?

  // o__ "// DigitalEvtCtlStmt2 " << cond().data().rdeps().size() << "\n";

  if( _ctrl.is_used_in(b)){
    return true;
  }else{
    return DigitalCtrlStmt::is_used_in(b);
  }
}
/*--------------------------------------------------------------------------*/
void DigitalEvtCtlStmt::submit_variable_access(Variable_Access& va) const
{
  cond().submit_variable_xs(va);
  Variable_Access b;
  Variable_Access a = b | body().variable_access();
  va &= a;
}
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
bool DigitalEvtExpression::is_used_in(Base const* b)const
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
void DigitalEvtExpression::parse(CS& file)
{
  assert(!size());
 // assert(!function());

  trace1("DigitalEvtExpression::parse0", file.tail().substr(0,10));

  Expression rhs;
  rhs.push_back(new Token_STOP("("));
  file >> rhs;

  // TODO:
  // if(dynamic_cast<EVENT_FUNCTION const*>(rhs.back()){ untested();
  // }else{ untested();
  //   error.
  // }

  trace1("DigitalEvtExpression::parse1", file.tail().substr(0,10));
  assert(owner());
  // Expression_::set_owner(scope());

  while(file >> "or" || file >> ","){ untested();
    file >> rhs;
    // TODO:
    // if(dynamic_cast<EVENT_FUNCTION const*>(rhs.back()){ untested();
    // }else{ untested();
    //   error.
    // }
  }

  FUNCTION const* f = function_dispatcher["@"];
  auto f_ = prechecked_cast<FUNCTION_ const*>(f);
  assert(f_);
  rhs.push_back(new Token_PARLIST(")"));
  rhs.push_back(new Token_CALL("@", f_));

  resolve_symbols(rhs);
  update(nullptr); // BUG?

  set_rdeps();
}
/*--------------------------------------------------------------------------*/
void DigitalEvtExpression::dump(std::ostream& o) const
{
  Expression_::dump(o);
}
/*--------------------------------------------------------------------------*/
// TODO // dup in SystemTask
// TODO // dup in AnalogEvtExpression
void DigitalEvtExpression::set_rdeps()
{
  assert(size());
  Token const* t = back();
  assert(t);
  Token_CALL const* c = prechecked_cast<Token_CALL const*>(t);
  assert(c);

  Expression const* a = c->args();
  assert(a);

  for(auto i : *a) {
    if(dynamic_cast<Token_NODE const*>(i)) {
      add_rdep(&tr_accept_tag);
      add_rdep(&tr_advance_tag);
    }else if(auto call = dynamic_cast<Token_CALL const*>(i)) {
      auto& f = *call;
      //     add_rdeps(call)
      //  auto e = prechecked_cast<MGVAMS_EVENT const*>(f.f());
      //  assert(e);
      if(f->has_tr_begin()){
	add_rdep(&tr_begin_tag);
      }else{
      }
      if(f->has_tr_restore()){
	add_rdep(&tr_restore_tag);
      }else{
      }
      if(f->has_tr_review()){
	add_rdep(&tr_eval_tag);
      }else{ untested();
      }
      if(f->has_tr_review()){
	add_rdep(&tr_review_tag);
      }else{ untested();
      }
      if(f->has_tr_accept()){
	add_rdep(&tr_accept_tag);
      }else{ untested();
      }
      if(f->has_tr_advance()){
	add_rdep(&tr_advance_tag);
      }else{ untested();
      }
      if(f->has_final()){ untested();
	add_rdep(&final_tag);
      }else{
      }
    }else{ untested();
      incomplete();
    }
  }
}
/*--------------------------------------------------------------------------*/
// incomplete. expressions...
FUNCTION_ const* DigitalEvtExpression::function() const
{ untested();
  assert(size());
  Token const* t = back();
  assert(t);
  Token_CALL const* c = prechecked_cast<Token_CALL const*>(t);
  assert(c);
  return c->f();
}
/*--------------------------------------------------------------------------*/
DigitalExpression::~DigitalExpression()
{ untested();
}
/*--------------------------------------------------------------------------*/
void Module::new_always()
{
  assert(!_always);
  _always = new Always;
}
/*--------------------------------------------------------------------------*/
void Module::new_assign()
{
  assert(!_assign);
  _assign = new Assign;
}
/*--------------------------------------------------------------------------*/
void Module::delete_always()
{
  delete _always;
  _always = nullptr;
}
/*--------------------------------------------------------------------------*/
void Module::delete_assign()
{
  delete _assign;
  _assign = nullptr;
}
/*--------------------------------------------------------------------------*/
bool Module::has_always_block() const
{
  if(::always(*this).has_block()){
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
Always::Always()
{
 // new_probe_map();
}
/*--------------------------------------------------------------------------*/
Always::~Always()
{
  _list.clear();
  // delete _probes;
}
/*--------------------------------------------------------------------------*/
bool Always::has_block() const
{
  return !list().is_empty();
}
/*--------------------------------------------------------------------------*/
void Always::dump(std::ostream& o) const
{
  Module const* m = to_module(owner());
  assert(m);
  (void) m;

  for(auto const& i: blocks()){
    o__ "always ";
    o << *i << "\n";
//    o__ "end\n";
  }
}
/*--------------------------------------------------------------------------*/
#if 0 // code?
static File const* to_file(Block const* owner)
{ untested();
  assert(owner);
  while(true){ untested();
    if(auto m = dynamic_cast<File const*>(owner)){ untested();
      return m;
    }else{ untested();
    }
    owner = owner->scope();
    if(!owner){ untested();
      return nullptr;
    }else{ untested();
    }
  }
  unreachable();
  return nullptr;
}
#endif
/*--------------------------------------------------------------------------*/
void Always::push_back(Base* ab)
{
  if(auto c = dynamic_cast<AlwaysConstruct*>(ab)){
    _list.push_back(c);
  }else if(auto i = dynamic_cast<DigitalInitialStmt*>(ab)){
    _list.push_back(i);
  }else{
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
void Always::parse(CS& f)
{
  if (f.last_match() == "initial "){
    Module* m = to_module(owner());
    assert(m);
    m->set_tr_begin_digital();
    DigitalInitialStmt* is = new DigitalInitialStmt(owner(), f);
    is->set_owner(owner());
    is->update(); // here??
    push_back(is);
  }else if(1||f >> "always "){
    assert(owner());
    AlwaysConstruct* ab = new AlwaysConstruct();

    assert(owner());
    ab->set_owner(owner());
    ab->parse(f);
    _list.set_owner(owner()); // needed?
    push_back(ab);
    ab->update();
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
static Token_NODE* parse_net(CS& f, Block* o)
{
  size_t here = f.cursor();
  std::string what;
  f >> what;
  trace1("parse_variable", what);
  Base* b = o->lookup(what);
  Node* v = nullptr;
  if((v = dynamic_cast<Node*>(b))) { untested();
    assert(f);
    // assert(v->data()); no. unreachable?
  }else if(auto t = dynamic_cast<Token_NODE*>(b)) {
    return t;
    v = prechecked_cast<Node*>(t->item());
    assert(v);
  }else if (b) { untested();
    f.reset_fail(here);
    trace1("not a net", f.tail().substr(0,10));
    assert(0);

  }else{ untested();
    f.reset_fail(here);
    trace1("not found", f.tail().substr(0,10));
  }
  unreachable(); incomplete();
  return nullptr;
//  return v;
}
/*--------------------------------------------------------------------------*/
static int _nacount;
NetAssignment::NetAssignment(CS& f, Base* o)
  : Assignment(), _seq(_nacount++)
{
  set_owner(o);
  parse(f);
}
/*--------------------------------------------------------------------------*/
void NetAssignment::parse(CS& f)
{
  assert(owner());
  assert(scope());
  size_t here = f.cursor();
  _lhsref = parse_net(f, scope());
  // assert(l->name() == name());?

  if(f && f >> "="){
    parse_rhs(f);
  }else{ untested();
    //assert(!_lhsref);
    f.reset_fail(here);
  }
}
/*--------------------------------------------------------------------------*/
void NetAssignment::dump(std::ostream& o)const
{
  if(_lhsref){
    o << _lhsref->name()
      << " = ";
    Expression_::dump(o);
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
void ContinuousAssign::push_back(NetAssignment* n)
{
  Block::push_back(n);
}
/*--------------------------------------------------------------------------*/
void ContinuousAssign::dump(std::ostream& o)const
{
  o__ "assign ";
  if(_delay.size()){
    o << "#";
    _delay.dump(o);
    o << " ";
  }else{
  }
  std::string sep;
  for(Base const* b : *this){
    o << sep;
    assert(b);
    b->dump(o);
    sep = ", ";
  }
  o << ";";
}
/*--------------------------------------------------------------------------*/
Assign::Assign()
{
}
/*--------------------------------------------------------------------------*/
Assign::~Assign()
{
}
/*--------------------------------------------------------------------------*/
void Assign::parse(CS& f)
{
  if(1||f >> "assign ") {
    assert(owner());
    ContinuousAssign* ab = new ContinuousAssign();

    assert(owner());
    ab->set_owner(owner());
    ab->parse(f);
    _list.set_owner(owner()); // needed?
    push_back(ab);
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
void Assign::push_back(Base* ab)
{
  if(auto c = dynamic_cast<ContinuousAssign*>(ab)){
    _list.push_back(c);
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
void Assign::dump(std::ostream& o) const
{
  Module const* m = to_module(owner());
  assert(m);

  for(auto const& i: list()){
    o << *i << "\n";
  }
}
/*--------------------------------------------------------------------------*/
bool DigitalProceduralAssignment::is_used_in(Base const*b)const
{
  if (b == &tr_begin_tag){
    return true;
  }else if (_a.is_used_in(b)) { untested();
    return true;
  }else if (DigitalStmt::is_used_in(b)) {
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
DigitalStmt::~DigitalStmt()
{
}
/*--------------------------------------------------------------------------*/
bool DigitalProceduralAssignment::propagate_rdep(Base const* incoming)
{
  return Statement::propagate_rdep(incoming);
}
/*--------------------------------------------------------------------------*/
bool DigitalProceduralAssignment::propagate_rdeps(RDeps const& r)
{
  assert(owner());
  auto s = dynamic_cast<Statement*>(owner_());
  if(s){
  }else{ untested();
  }
  bool ret = false;
  for(Base const* n : r) {
    ret |= propagate_rdep(n);
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
DigitalForStmt::DigitalForStmt(CS& f, Block* o) : ForStmt()
{
  options().disable_optimize_common(); // for now.
  set_owner(o);
  parse(f);
}
/*--------------------------------------------------------------------------*/
#if 0
// Analog::new_probe?
Probe* new_Probe(std::string const& xs, Branch_Ref const& br);
Probe const* Always::new_probe(std::string const& xs, Branch_Ref const& br, Module* m)
{ untested();
    trace1("new_probe", br.has_name());
  std::string flow_xs;
  std::string pot_xs;

  if(br->discipline()){ untested();
    trace2("new_probe", xs, br->discipline()->identifier());
    flow_xs = br->discipline()->flow()->access().to_string();
    pot_xs = br->discipline()->potential()->access().to_string();
  }else{ untested();
    // huh?
  }

  std::string nn = xs;
  if(xs == flow_xs || xs == "flow"){ untested();
    nn = "flow";
  }else if( xs == pot_xs || xs == "potential" ){ untested();
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
  if(br.has_name()){ untested();
    k = nn + "_" + br.name();
  }else{ untested();
  }

  assert(_probes);
  Probe*& prb = (*_probes)[k];

  if(prb) { untested();
  }else{ untested();
    prb = ::new_Probe(nn, br);
//    prb->set_label(k);
    m->install(prb); // duplicate reference..?
  }

  trace1("new_probe", br.has_name());
  return prb;
}
#endif
/*--------------------------------------------------------------------------*/
void Always::setup_storage(Variable_Access& va) const
{
//  SeqBlock::variable_access().collect(this);
  for(Statement* bb : _list){
    if(dynamic_cast<InitialStmt const*>(bb)){
      incomplete(); // later
      bb->submit_variable_access(va);
    }else if(prechecked_cast<AlwaysConstruct const*>(bb)){
    }else{ untested();
      incomplete();
    }
  }
//  va.prune_dynamic();

  for(Statement* bb : _list){
    if(dynamic_cast<AlwaysConstruct const*>(bb)){
      bb->submit_variable_access(va);
    }else if(prechecked_cast<InitialStmt const*>(bb)){
    }else{ untested();
      incomplete();
    }
  }

  if(dynamic_cast<Module const*>(scope())){
  }else{ untested();
    unreachable();
  }

//  va.sift_locals(scope());
//  assert(!va.size());
}
/*--------------------------------------------------------------------------*/
// BUG? should an always block be a Statement
void always_setup_storage(Base* b, Variable_Access& va)
{
  auto a = prechecked_cast<Always*>(b);
  assert(a);
  return a->setup_storage(va);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
