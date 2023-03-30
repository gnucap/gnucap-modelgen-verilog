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
#include "m_tokens.h"
#include <gnucap/e_cardlist.h> // TODO: really?
#include <stack>
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
static int is_va_function(std::string const& n)
{
  // stub, need sth.
  if (n == "exp"
   || n == "log"
   || n == "cos"
   || n == "sin") {
    return 1;
  }else if (n == "pow"){
    return 2;
  }else{
    return 0;
  }
}
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
static bool is_filter_function(std::string const& n)
{
  if (n == "ddt"){
    return true;
  }else{
    return false;
  }
}
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
static bool is_xs_function(std::string const& f, Block const* ctx)
{
  assert(ctx);
  while(!dynamic_cast<Module const*>(ctx)){
    ctx = ctx->ctx();
    assert(ctx);
  }
  auto m =dynamic_cast<Module const*>(ctx);
  assert(m);
  File const* file = m->file();
  if(!file){
    // fallback. modelgen_0.cc // incomplete();
    return f=="V" || f=="I" || f=="flow" || f=="potential";
  }else{
  }

  for(auto n: file->nature_list()){
    if(n->access().to_string() == f){
      return true;
    }else{
    }
  }
  // stub, need discipline.h
  return false;
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
      || parse_seq(file)
      ;
    if (!has_begin){
      file.umatch(";");
      break;
    }else if (file.umatch("end ")) {
      break;
    }else if (!file.more()) {
      file.warn(0, "premature EOF (analog)");
      break;
    }else if (file.stuck(&here)) {
      file.warn(0, "bad analog block");
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
CS& AnalogBlock::parse_real(CS& cmd)
{
    trace1("AnalogBlock::parse real", cmd.tail());

    std::string name;
    cmd >> name >> ';';
    Variable* a = new Variable(name);
    push_back(a);
    trace1("AnalogBlock::parse real", name);
    new_var_ref(a);
    return cmd;
}
/*--------------------------------------------------------------------------*/
CS& AnalogBlock::parse_flow_contrib(CS& cmd, std::string const& what)
{
  // what==cmd.last_match?
  FlowContribution* a = new FlowContribution(what);
  a->set_ctx(this);
  cmd >> *a;
  push_back(a);
  return cmd;
}
/*--------------------------------------------------------------------------*/
CS& AnalogBlock::parse_pot_contrib(CS& cmd, std::string const& what)
{
  // what==cmd.last_match?
  PotContribution* a = new PotContribution(what);
  a->set_ctx(this);
  cmd >> *a;
  push_back(a);
  return cmd;
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
  }else if(what == "int") { untested();

  }else if(cmd >> "*=") { untested();
    incomplete();
  }else if(cmd >> "+=") { untested();
    incomplete();
  }else if(cmd >> "=") {
    Assignment* a = new Assignment(what);
    a->set_ctx(this);
    a->parse(cmd);
    push_back(a);
    _var_refs[what] = a;
  }else{
    cmd.reset(here);
  }

  return cmd;
}
/*--------------------------------------------------------------------------*/
Assignment::~Assignment()
{
  // BUG
  // delete _rhs;
}
/*--------------------------------------------------------------------------*/
void Assignment::parse(CS& cmd)
{
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
  trace0("resolve symbols ===========");
  assert(ctx());
  Block const* scope = ctx();
  std::stack<Deps*> depstack;
  depstack.push(&_deps);

  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    trace1("resolve symbols", (*ii)->name());
  }
  // resolve symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* t = *ii;
    trace1("resolve top:", t->name());

    auto s = dynamic_cast<Token_SYMBOL*>(t);
    std::string const& n = t->name();
    Base const* r = scope->resolve(n);
    trace2("resolve top found:", n, r);

    if(dynamic_cast<Token_STOP*>(t)) {
      E.push_back(t->clone());
      trace0("resolve STOP");
      depstack.push(new Deps);
    }else if(auto c = dynamic_cast<Token_CONSTANT*>(t)) { untested();
      Token* cl = c->clone();
      assert(t->name() == cl->name());
      E.push_back(cl);
    }else if(dynamic_cast<Token_PARLIST*>(t)
           ||dynamic_cast<Token_UNARY*>(t)
           ||dynamic_cast<Token_BINOP*>(t)) {
      E.push_back(t->clone());
    }else if(!s) {
      unreachable();
      trace2("huh", name(), t->name());
      E.push_back(t->clone());
    }else if(is_xs_function(n, scope)) {
      trace0("resolve XS");
      Deps* td = depstack.top();
      Token_PROBE* t = resolve_xs_function(E, n, *td);
      E.push_back(t);
      td->insert(t->prb());
      depstack.pop();
      assert(!depstack.empty());
      depstack.top()->update(*td);
      delete(td);
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
      E.push_back(new Token_PAR_REF(n, p));
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      E.push_back(new Token_VAR_REF(n, v));
      depstack.top()->update(v->deps());
    }else if(auto pr = dynamic_cast<Probe const*>(r)) { untested();
//      trace1("resolve: probe dep", pr->name());
      E.push_back(new Token_PROBE(n, pr));
      Deps* td = depstack.top();
      delete(td);
      depstack.pop();
      depstack.top()->insert(pr);
    }else if(r) { untested();
      assert(0);
      incomplete(); // unresolved symbol?
    }else if (strchr("0123456789.", n[0])) {
      // a number
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(t->name(), f, ""));
    }else if(is_va_function(t->name())) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      Deps* td = depstack.top();
      depstack.pop();
      depstack.top()->update(*td);
      delete(td);
      E.push_back(t->clone()); // try later?
    }else if(is_filter_function(n)) {
      assert(dynamic_cast<Token_PARLIST*>(E.back()));
      Deps* td = depstack.top();
      E.push_back(resolve_filter_function(E, n, *td));
      depstack.pop();
      assert(!depstack.empty());
      depstack.top()->update(*td);
      delete(td);
    }else if(is_node(t->name())) {
      E.push_back(t->clone()); // try later?
    }else{
      throw Exception("unresolved symbol: " + n);
    }
  }
  trace1("depstack", depstack.size());
  assert(depstack.size()==1);
}
/*--------------------------------------------------------------------------*/
Token* Variable::resolve_filter_function(Expression& E, std::string const& n, Deps const& cdeps)
{
  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else{
    assert(n=="ddt"); // incomplete.
		      //
    assert(!E.is_empty());

    Filter const* f = _ctx->new_filter(n, cdeps);
    assert(f);

    // arglist
//    delete E.back();
//    E.pop_back();

    return new Token_FILTER(n, f);
  }
}
/*--------------------------------------------------------------------------*/
Token_PROBE* Variable::resolve_xs_function(Expression& E, std::string const& n, Deps const& deps)
{
  if(E.is_empty()) { untested();
    throw Exception("syntax error");
  }else if(!dynamic_cast<Token_PARLIST*>(E.back())) { untested();
    throw Exception("syntax error");
  }else{
    delete E.back();
    E.pop_back();
    assert(!E.is_empty());
    std::string arg1;
    if(dynamic_cast<Token_STOP*>(E.back())) { untested();
      throw Exception("syntax error");
    }else{
    }
    std::string arg0 = E.back()->name();
    delete E.back();
    E.pop_back();
    assert(!E.is_empty());

    while(!dynamic_cast<Token_STOP*>(E.back())) {
      arg1 = E.back()->name();
      delete E.back();
      E.pop_back();
      assert(!E.is_empty());
    }

    delete E.back();
    E.pop_back();
    Probe const* p = _ctx->new_probe(n, arg1, arg0);
    std::string name = n+"("+arg1+", "+arg0+")";

    trace3("got a probe", name, arg1, this);
    return new Token_PROBE(name, p);
    // E.push_back(new Token_PROBE(name, p));
  }
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
void PotContribution::dump(std::ostream& out)const
{
  assert(_branch);
  out << ind << _lhsname << *_branch << " <+ ";
  assert(_rhs);

  ::dump(out, *_rhs);
//  out << "\n";
}
/*--------------------------------------------------------------------------*/
void FlowContribution::dump(std::ostream& out)const
{
  assert(_branch);
  out << ind << _lhsname << *_branch << " <+ ";
  assert(_rhs);

  ::dump(out, *_rhs);
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
