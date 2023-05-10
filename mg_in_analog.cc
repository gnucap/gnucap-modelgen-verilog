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
#include "mg_.h"
#include "mg_out.h"
#include "m_tokens.h"
#include <gnucap/e_cardlist.h> // TODO: really?
#include <gnucap/u_opt.h>
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
- | { attribute_instance } analog_case_statement
+ | { attribute_instance } analog_conditional_statement
+ | { attribute_instance } analog_procedural_assignment
+ | { attribute_instance } analog_seq_block
- | { attribute_instance } analog_system_task_enable
+ | { attribute_instance } contribution_statement
- | { attribute_instance } indirect_contribution_statement
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
static bool is_pot_xs(CS& file)
{
  trace1("is_pot_xs", file.tail().substr(0,10));
  // stub, need discipline.h
  return file >> "V " || file >> "potential ";
}
/*--------------------------------------------------------------------------*/
static bool is_flow_xs(CS& file)
{
  trace1("is_flow_xs", file.tail().substr(0,10));
  // stub, need discipline.h
  return file >> "I " || file >> "flow ";
}
/*--------------------------------------------------------------------------*/
static Base* parse_flow_contrib(CS& cmd, std::string const& what, Block* owner)
{
  auto a = new FlowContribution(what);
  a->set_owner(owner);
  cmd >> *a >> ";";
  return a;
}
/*--------------------------------------------------------------------------*/
static Base* parse_pot_contrib(CS& cmd, std::string const& what, Block* owner)
{
  trace1("parse_pot_contrib", what);
  // what==cmd.last_match?
  auto a = new PotContribution(what);
  a->set_owner(owner);
  cmd >> *a >> ";";
  return a;
}
/*--------------------------------------------------------------------------*/
static Base* parse_cond(CS& file, Block* o)
{
  auto cb = new AnalogConditionalStmt();
  cb->set_owner(o);
  file >> *cb;
  return cb;
}
/*--------------------------------------------------------------------------*/
static Base* new_evt_ctl_stmt(CS& file, Block* o)
{
  auto cb = new AnalogEvtCtlStmt();
  cb->set_owner(o);
  try{
    file >> *cb;
    return cb;
  }catch(Exception const& e){
    delete cb;
    throw e;
  }
}
/*--------------------------------------------------------------------------*/
static AnalogStmt* parse_seq(CS& file, Block* owner)
{
  auto b = new AnalogSeqBlock();
  b->set_owner(owner);
  file >> *b;
  return b;
}
/*--------------------------------------------------------------------------*/
static Base* parse_real(CS& file, Block* owner)
{
  trace1("AnalogBlock::parse real", file.tail());

  std::string name;
  // Variable* a = new Variable(name);
  ListOfBlockRealIdentifiers* l = new ListOfBlockRealIdentifiers();
  l->set_owner(owner);
  file >> *l;
  return l;
}
/*--------------------------------------------------------------------------*/
static Base* parse_assignment(CS& file, Block* o)
{
  size_t here = file.cursor();
  std::string what;
  file >> what;
  trace2("assignment", what, file.tail().substr(0,10));
//  file >> what;
  if(what == ""){ untested();
    throw Exception_CS("need name", file);
  }else{
  }

  Base const* b = o->resolve(what);
  Variable const* v = dynamic_cast<Variable const*>(b);
  if(!v){
    file.reset(here);
    return NULL;
    throw Exception_CS("what's this: " + what, file);
  }else if(file >> "=") {
    Assignment* a = new Assignment();
    a->set_lhs(v);
    a->set_owner(o);
    a->parse(file);
    file >> ";";
    trace1("got semicolon", (bool)file);
//    _var_refs[what] = a;
    return a;
  }else{ untested();
    throw Exception_CS("need assign op", file);
  }
}
/*--------------------------------------------------------------------------*/
static Base* parse_contribution(CS& file, Block* owner)
{
  if(is_pot_xs(file)) {
    return parse_pot_contrib(file, file.last_match(), owner);
  }else if(is_flow_xs(file)) {
    trace2("is_flow_xs", file.last_match(), OPT::case_insensitive);
    return parse_flow_contrib(file, file.last_match(), owner);
  }else{
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
static Base* parse_analog_stmt(CS& file, Block* owner)
{
  assert(owner);
  Base* ret = NULL;

  trace1("parse_analog_stmt", file.tail().substr(0,10));
//  size_t here = file.cursor();
  ONE_OF	// module_item
    || file.umatch(";")
    || ((file >> "begin ") && (ret = parse_seq(file, owner)))
    || ((file >> "real ") && (ret = parse_real(file, owner)))
    || ((file >> "if ") && (ret = parse_cond(file, owner)))
    || ((file >> "@ ") && (ret = new_evt_ctl_stmt(file, owner)))
    || (ret = parse_contribution(file, owner))
    || (ret = parse_assignment(file, owner))
//    || parse_seq(file)
    ;

  return ret;
}
/*--------------------------------------------------------------------------*/
void AnalogConditionalStmt::parse(CS& file)
{
  file >> "(" >> _cond >> ")";
  if(file >> ";"){
  }else{
    _true_part = parse_analog_stmt(file, owner());
    if(file >> "else "){
      trace1("got else branch", file.tail().substr(0, 1));
      _false_part = parse_analog_stmt(file, owner());
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void AnalogConditionalStmt::dump(std::ostream& o)const
{
  static bool cont;
  if(cont){
  }else{
    o__ "";
  }
  o << "if (" << _cond << ")";
  cont = false;
  if(!_true_part){
    o << ";\n";
  }else{
    if(dynamic_cast<AnalogSeqBlock const*>(_true_part)){
      o << " ";
      _true_part->dump(o);
    }else{
      indent m;
      o << "\n";
      _true_part->dump(o);
    }
  }
  if(_false_part){
    o__ "else";
    if(dynamic_cast<AnalogConditionalStmt const*>(_false_part)){
      o << " ";
      cont = true;
      _false_part->dump(o);
    }else if(dynamic_cast<AnalogSeqBlock const*>(_false_part)){
      o << " ";
      _false_part->dump(o);
    }else{
      o << "\n";
      _false_part->dump(o);
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void AnalogConstruct::parse(CS& file)
{
  _stmt = parse_analog_stmt(file, owner());
  assert(_stmt); // throw?
}
/*--------------------------------------------------------------------------*/
void AnalogSeqBlock::parse(CS& file)
{
  for (;;) {
//    size_t here = file.cursor();
    trace1("AnalogSeqBlock::parse", file.tail().substr(0,10));
    if(file >> "end "){
      break;
    }else{
    }
    Base* s = parse_analog_stmt(file, &_block);
    if(!s){
      throw Exception_CS("bad analog block", file);
      break;
    }else{
      _block.push_back(s);
    }
  }

#if 0
    if (!file.more()) { untested();
      file.warn(0, "premature EOF (analog)");
      break;
    }
#endif
}
/*--------------------------------------------------------------------------*/
void SeqBlock::dump(std::ostream& o)const
{
  Block::dump(o);
}
/*--------------------------------------------------------------------------*/
void BlockRealIdentifier::parse(CS& file)
{
  file >> _name;
  new_var_ref();
}
/*--------------------------------------------------------------------------*/
void BlockRealIdentifier::dump(std::ostream& o)const
{
  o << name();
}
/*--------------------------------------------------------------------------*/
void ListOfBlockRealIdentifiers::dump(std::ostream& o) const
{
  o__ "real ";
  LiSt<BlockRealIdentifier, '\0', ',', ';'>::dump(o);
  o << "\n";
}
//CS& ListOfBlockRealIdentifiers::parse(CS& file)
//{
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
  assert(_rhs);
  o__ lhsname() << " = " << *_rhs << ";\n";
}
/*--------------------------------------------------------------------------*/
Assignment::~Assignment()
{
  delete _rhs;
}
/*--------------------------------------------------------------------------*/
// void AnalogExpression::parse(CS& cmd)
// {
// }
/*--------------------------------------------------------------------------*/
// hijacked from m_base_in.
void Name_String::parse(CS& File)
{
  File.skipbl();
  _data = "";
  if (File.is_pfloat()) {
    while (File.is_pfloat()) {
      _data += File.ctoc();
    }
    if (File.match1("eE")) {
      _data += File.ctoc();
      if (File.match1("+-")) {
	_data += File.ctoc();
      }else{
      }
      while (File.is_digit()) {
	_data += File.ctoc();
      }
    }else{
    }
    while (File.is_alpha()) {
      _data += File.ctoc();
    }
  }else{
    while (File.is_alpha() || File.is_pfloat() || File.match1("_[]$")) {
      _data += File.ctoc();
    }
  }
  File.skipbl();
}
/*--------------------------------------------------------------------------*/
void Assignment::parse(CS& cmd)
{
  // TODO: rhs is an analog expression
  assert(owner());
  trace1("Assignment::parse", cmd.tail());
  Expression rhs(cmd);
  assert(!_rhs);
#if 1
  Expression tmp;
  assert(owner());
  resolve_symbols(rhs, tmp);
  trace1("Assignment::parse resolved", rhs.size());
  _rhs = new Expression(tmp, &CARD_LIST::card_list);
  trace1("Assignment::parse gotit", rhs.size());
#else
  Expression tmp(rhs, &CARD_LIST::card_list);
  _rhs = new Expression();
  resolve_symbols(tmp, *_rhs);
#endif
}
/*--------------------------------------------------------------------------*/
void Variable::resolve_symbols(Expression const& e, Expression& E)
{
  Block* scope = owner();
  assert(scope);
  return ::resolve_symbols(e, E, scope, &_deps);
}
/*--------------------------------------------------------------------------*/
void Contribution::parse(CS& cmd)
{
  // TODO: parse branch_ref?
  cmd >> "(";
  std::string pp = cmd.ctos(",)");
  std::string pn = cmd.ctos(",)");
  // _name = _lhsname + pp + pn;
  Branch_Ref nb = new_branch(pp, pn);
  _branch = nb;
  assert(_branch);
  cmd >> ")";
  cmd >> "<+";


  {// Assignment::parse(cmd);
    // TODO: rhs is an analog expression
    assert(owner());
    trace1("Assignment::parse", cmd.tail());
    Expression rhs(cmd);
    assert(!_rhs);
#if 1
    Expression tmp;
    assert(owner());
    resolve_symbols(rhs, tmp, owner(), &deps());
    trace1("Assignment::parse resolved", rhs.size());
    _rhs = new Expression(tmp, &CARD_LIST::card_list);
#else
    Expression tmp(rhs, &CARD_LIST::card_list);
    _rhs = new Expression();
    resolve_symbols(tmp, *_rhs);
#endif
    assert(owner());

    for(auto d : deps()) {
      nb->add_probe(d);
    }
  }
}
/*--------------------------------------------------------------------------*/
void PotContribution::parse(CS& cmd)
{
  Contribution::parse(cmd);
  set_pot_source();
}
/*--------------------------------------------------------------------------*/
void FlowContribution::parse(CS& cmd)
{
  Contribution::parse(cmd);
  set_flow_source();
}
/*--------------------------------------------------------------------------*/
void Branch_Map::parse(CS& f)
{
  trace1("Branch_Map::parse", f.tail());
  incomplete();
}
/*--------------------------------------------------------------------------*/
void Branch_Map::dump(std::ostream&)const
{
  incomplete();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Branch::dump(std::ostream& o)const
{
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
  o__ _name << *_branch << " <+ ";
  assert(_rhs);

  ::dump(o, *_rhs);
  o << ";\n";
}
/*--------------------------------------------------------------------------*/
std::string Branch::name()const
{
  return "(" + _p->name()+", "+_n->name()+")";
}
/*--------------------------------------------------------------------------*/
void Assignment::set_lhs(Variable const* v)
{
  _lhs = v;
  assert(v);
  _name = v->name(); // BUG?
}
/*--------------------------------------------------------------------------*/
void AnalogSeqBlock::dump(std::ostream& o)const
{
  o << "begin\n";
  {
    indent x;
    _block.dump(o);
  }
  o__ "end\n";
}
/*--------------------------------------------------------------------------*/
void AnalogConditionalExpression::parse(CS& file)
{
  Expression rhs(file);
  Expression tmp;
  assert(owner());
//  resolve_symbols(rhs, tmp);
  assert(owner());
  Deps ignore;
  resolve_symbols(rhs, tmp, owner(), &ignore);
  _exp = new Expression(tmp, &CARD_LIST::card_list);
}
/*--------------------------------------------------------------------------*/
void AnalogEvtCtlStmt::parse(CS& file)
{
  file >> _ctl;
  assert(owner());
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
  }else{
    o << ";";
  }
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
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
