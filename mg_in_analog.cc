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
static bool is_pot_function(std::string const& n)
{
  // stub, need discipline.h
  return n == "V";
}
/*--------------------------------------------------------------------------*/
static bool is_flow_function(std::string const& n)
{
  // stub, need discipline.h
  return n == "I";
}
/*--------------------------------------------------------------------------*/
static bool is_xs_function(std::string const& n)
{
  // stub, need discipline.h
  return n == "V" || n == "I";
}
/*--------------------------------------------------------------------------*/
void AnalogBlock::parse(CS& file)
{
  assert(_ctx);
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
  untested();
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
//    _var_refs[name] = a;
// analog sequential block
CS& AnalogBlock::parse_seq(CS& cmd)
{
  assert(_ctx);
  std::string what;
  size_t here = cmd.cursor();
  cmd >> what;
  if(is_pot_function(what)) {
    assert(_ctx);
    PotContribution* a = new PotContribution(what);
    a->set_ctx(this);
    cmd >> *a;
    push_back(a);
  }else if(is_flow_function(what)) {
    assert(_ctx);
    FlowContribution* a = new FlowContribution(what);
    a->set_ctx(this);
    cmd >> *a;
    push_back(a);
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
  untested();
#else
  Expression tmp(rhs, &CARD_LIST::card_list);
  _rhs = new Expression();
  resolve_symbols(tmp, *_rhs);
#endif
}
/*--------------------------------------------------------------------------*/
void Variable::resolve_symbols(Expression const& e, Expression& E)
{
  assert(ctx());
  // resolve symbols
  for(List_Base<Token>::const_iterator ii = e.begin(); ii!=e.end(); ++ii) {
    Token* i = *ii;
    trace1("resolve top:", i->name());

    auto s = dynamic_cast<Token_SYMBOL*>(i);
    std::string const& n = i->name();
    Base const* r = ctx()->resolve(n);
    trace2("resolve top found:", n, r);

    if(dynamic_cast<Token_STOP*>(i)) {
      trace1("resolve STOP?", i->name());
      Token* cl = i->clone();
      assert(i->name() == cl->name());
      E.push_back(cl);
    }else if(auto c = dynamic_cast<Token_CONSTANT*>(i)) { untested();
      Token* cl = c->clone();
      assert(i->name() == cl->name());
      E.push_back(cl);
    }else if(!s) {
      trace2("huh", name(), i->name());
      E.push_back(i->clone());
    }else if(is_xs_function(i->name())) {
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
	_deps.insert(p);
	std::string name = n+"("+arg1+", "+arg0+")";
	E.push_back(new Token_PROBE(name, p));

	trace3("got a probe", n, arg1, this);
      }
    }else if(auto p = dynamic_cast<Parameter_Base const*>(r)) {
      trace2("resolve: param", name(), p->name());
      E.push_back(new Token_PAR_REF(n, p));
    }else if(auto v = dynamic_cast<Variable const*>(r)) {
      trace2("resolve: variable", name(), v->name());
      E.push_back(new Token_VAR_REF(n, v));
      for(auto d : v->deps()) {
	_deps.insert(d);
      }
    }else if(auto pr = dynamic_cast<Probe const*>(r)) { untested();
      trace1("resolve: probe dep", pr->name());
      E.push_back(new Token_PROBE(n, pr));
      _deps.insert(pr);
    }else if(r) { untested();
      assert(0);
      incomplete(); // unresolved symbol?
    }else if (strchr("0123456789.", n[0])) {
      // a number
      Float* f = new Float(n);
      E.push_back(new Token_CONSTANT(i->name(), f, ""));
    }else if(is_va_function(i->name())) {
      E.push_back(i->clone()); // try later?
    }else if(is_node(i->name())) {
      E.push_back(i->clone()); // try later?
    }else{
      throw Exception("unresolved symbol: " + n);
    }
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
  _branch = new_branch(pp, pn);
  assert(_branch);
  cmd >> ")";
  cmd >> "<+";
  Assignment::parse(cmd);
}
/*--------------------------------------------------------------------------*/
void FlowContribution::parse(CS& cmd)
{
  assert(ctx());

  cmd >> "(";
  std::string pp = cmd.ctos(",)");
  std::string pn = cmd.ctos(",)");
  _name = _lhsname + pp + pn;
  _branch = new_branch(pp, pn);
  assert(_branch);
  cmd >> ")";
  cmd >> "<+";
  Assignment::parse(cmd);
}
/*--------------------------------------------------------------------------*/
Node const* Module::new_node(std::string const& p)
{
  Node*& cc = _nodes[p];
  if(cc) { untested();
  }else{
    cc = new Node(p);
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Node const* Module::node(std::string const& p) const
{
  auto i = _nodes.find(p);
  if(i != _nodes.end()) {
    return i->second;
  }else{
    return NULL;
    throw Exception("no such node " + p );
  }
}
/*--------------------------------------------------------------------------*/
void Branch::dump(std::ostream& o)const
{
  // objects? _nname ground?
  o << "(" << _pname << ", " << _nname << ")";
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
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
