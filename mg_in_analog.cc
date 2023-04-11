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
- analog_construct ::=
- analog analog_statement
- | analog initial analog_function_statement// from A.6.2
- analog_statement ::=
- { attribute_instance } analog_loop_generate_statement
- | { attribute_instance } analog_loop_statement
- | { attribute_instance } analog_case_statement
- | { attribute_instance } analog_conditional_statement
- | { attribute_instance } analog_procedural_assignment
- | { attribute_instance } analog_seq_block
- | { attribute_instance } analog_system_task_enable
- | { attribute_instance } contribution_statement
- | { attribute_instance } indirect_contribution_statement
- | { attribute_instance } analog_event_control_statement// from A.6.4
- analog_statement_or_null ::=
- analog_statement
- | { attribute_instance } ;
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
static bool is_pot_xs(std::string const& n)
{
  // stub, need discipline.h
  return n == "V" || n == "potential";
}
/*--------------------------------------------------------------------------*/
static bool is_flow_xs(std::string const& n)
{
  // stub, need discipline.h
  return n == "I" || n == "flow";
}
/*--------------------------------------------------------------------------*/
void AnalogBlock::parse(CS& file)
{
  assert(ctx());
  bool has_begin = (file >> "begin ");
  trace2("AB parse", file.tail(), has_begin);

  for (;;) {
    // TODO: move parse_seq code here.
    size_t here = file.cursor();
    ONE_OF	// module_item
      || file.umatch(";")
      || ((file >> "real ") && parse_real(file))
      || parse_assignment(file)
      || parse_seq(file)
      ;
    if (!has_begin){
      file.umatch(";");
      break;
    }else if (file.umatch("end ")) {
      break;
    }else if (!file.more()) { untested();
      file.warn(0, "premature EOF (analog)");
      break;
    }else if (file.stuck(&here)) {
      throw Exception_CS("bad analog block", file);
    }else{
    }
  }
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
  o__ name();
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
CS& AnalogBlock::parse_real(CS& file)
{
    trace1("AnalogBlock::parse real", file.tail());

    std::string name;
    // Variable* a = new Variable(name);
    ListOfBlockRealIdentifiers* l = new ListOfBlockRealIdentifiers();
    l->set_owner(this);
    file >> *l;
    push_back(l);
    return file;
}
/*--------------------------------------------------------------------------*/
CS& AnalogBlock::parse_flow_contrib(CS& cmd, std::string const& what)
{
  // what==cmd.last_match?
  FlowContribution* a = new FlowContribution(what);
  a->set_owner(this);
  cmd >> *a;
  push_back(a);
  return cmd;
}
/*--------------------------------------------------------------------------*/
CS& AnalogBlock::parse_pot_contrib(CS& cmd, std::string const& what)
{
  // what==cmd.last_match?
  PotContribution* a = new PotContribution(what);
  a->set_owner(this);
  cmd >> *a;
  push_back(a);
  return cmd;
}
/*--------------------------------------------------------------------------*/
bool AnalogBlock::parse_assignment(CS& file)
{
  std::string what;
  size_t here = file.cursor();
  file >> what;
  if(what == ""){
    throw Exception_CS("need name", file);
  }else{
  }

  Base const* b = resolve(what);
  Variable const* v = dynamic_cast<Variable const*>(b);
  if(!v){ untested();
    file.reset(here);
    return false;
  }else if(file >> "=") { untested();
    Assignment* a = new Assignment(what);
    a->set_owner(this);
    a->parse(file);
    push_back(a);
    _var_refs[what] = a;
    return true;
  }else{ untested();
    // += etc.
    incomplete();
    throw Exception_CS("need assign op", file);
  }
}
/*--------------------------------------------------------------------------*/
//    _var_refs[name] = a;
// analog sequential block
CS& AnalogBlock::parse_seq(CS& cmd)
{
  assert(ctx());
  std::string what;
  size_t here = cmd.cursor();
  cmd >> what;
  if(is_pot_xs(what)) {
    parse_pot_contrib(cmd, what);
  }else if(is_flow_xs(what)) {
    parse_flow_contrib(cmd, what);
//  }else if(is_variable(what)) {
//    unreachable();
//    incomplete();
#if 0
  }else if(what == "int") { untested();
    incomplete();
  }else if(cmd >> "*=") { untested();
    incomplete();
  }else if(cmd >> "+=") { untested();
    incomplete();
  }else if(cmd >> "=") {
    Assignment* a = new Assignment(what);
    a->set_owner(this);
    a->parse(cmd);
    push_back(a);
    _var_refs[what] = a;
#endif
  }else{
    cmd.reset(here);
  }

  return cmd;
}
/*--------------------------------------------------------------------------*/
void AnalogBlock::dump(std::ostream& o)const
{
  o__ "analog begin\n";
  for(auto i: *this){
    indent x;
    o << *i;
  }
  o << ind << "end\n";
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
void Assignment::parse(CS& cmd)
{
  // TODO: rhs is an analog expression
  assert(ctx());
  trace1("Assignment::parse", cmd.tail());
  Expression rhs(cmd);
  assert(!_rhs);
#if 1
  Expression tmp;
  assert(ctx());
  resolve_symbols(rhs, tmp);
  trace1("Assignment::parse resolved", rhs.size());
  _rhs = new Expression(tmp, &CARD_LIST::card_list);
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
void PotContribution::parse(CS& cmd)
{
  assert(ctx());

  cmd >> "(";
  std::string pp = cmd.ctos(",)");
  std::string pn = cmd.ctos(",)");
  _name = _lhsname + pp + pn;
  Branch_Ref nb = new_branch(pp, pn);
  _branch = nb;
  assert(_branch);
  cmd >> ")";
  cmd >> "<+";
  Assignment::parse(cmd);

  for(auto d : deps()) {
    nb->add_probe(d);
  }
  nb->set_pot_source();
}
/*--------------------------------------------------------------------------*/
void FlowContribution::parse(CS& cmd)
{
  assert(ctx());

  cmd >> "(";
  std::string pp = cmd.ctos(",)");
  std::string pn = cmd.ctos(",)");
  _name = _lhsname + pp + pn;
  Branch_Ref nb = new_branch(pp, pn);
  _branch = nb;
  assert(_branch);
  cmd >> ")";
  cmd >> "<+";
  Assignment::parse(cmd);

  for(auto d : deps()) {
    nb->add_probe(d);
  }
  nb->set_flow_source();
}
/*--------------------------------------------------------------------------*/
void Branch_Map::parse(CS&)
{
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
// common Contribution Base?
void PotContribution::dump(std::ostream& o)const
{
  assert(_branch);
  o__ lhsname() << *_branch << " <+ ";
  assert(_rhs);

  ::dump(o, *_rhs);
  o << ";\n";
}
/*--------------------------------------------------------------------------*/
void FlowContribution::dump(std::ostream& o)const
{
  assert(_branch);
  o__ lhsname() << *_branch << " <+ ";
  assert(_rhs);

  ::dump(o, *_rhs);
  o << ";\n";
//  out << "\n";
}
/*--------------------------------------------------------------------------*/
std::string Branch::name()const
{
  return "(" + _p->name()+", "+_n->name()+")";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
