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
/*--------------------------------------------------------------------------*/
#include "mg_out.h"
//#include "m_tokens.h"
/*--------------------------------------------------------------------------*/
static void make_cc_variable(std::ostream& o, Variable const& v)
{
  o << ind << "ddouble _v_" << v.name() << "; // Variable";
  for(auto i : v.deps()) { untested();
    o << ind << " Dep: " << i->code_name();
  }
  o << ind << "\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_block_real_identifier_list(std::ostream& o, ListOfBlockRealIdentifiers const& rl)
{
  for(Variable const* v : rl){
    assert(v);
    make_cc_variable(o, *v);
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_assignment(std::ostream& o, Assignment const& a)
{
  assert(a.rhs());
  Expression const& e = *a.rhs();

  o << ind << "{ // Assignment '" << a.lhsname() << "'.";
  for(auto i : a.deps()) {
    o << " Dep: " << i->code_name();
  }
  o << "\n";

  {
    indent x;
    make_cc_expression(o, e);
    std::string prefix;
    if(a.is_module_variable()){
      prefix = "/*m*/ d->_v_";
    }else{
      prefix = "_v_";
    }
    o__ prefix << a.lhsname() << ".value() = t0.value();\n";
    for(auto v : a.deps()) {
      if(v->is_reversed()){ untested();
	o__ "// reversed??\n";
	unreachable(); // ?
      }else{
      }
      o__ prefix << a.lhsname() << "[d" << v->code_name() << "] = " << "t0[d" << v->code_name() << "];\n";
      o__ "assert(" << prefix << a.lhsname() << " == " << prefix << a.lhsname() << ");\n";
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_contrib(std::ostream& o, Contribution const& f)
{
  o__ "{ // Contribution " << f.name() << "\n";
  {
    indent x("  ");
    if(f.rhs()) {
      make_cc_expression(o, *f.rhs());
    }else{ untested();
    }

    char sign = f.reversed()?'-':'+';

    assert(f.branch());
    o__ "d->_value" << f.branch()->code_name() << " "<<sign<<"= t0.value();\n";
    for(auto v : f.deps()) {
      if(f.branch() == v->branch()){
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "d->" << f.branch()->state() << "[1]"
	  " "<<sign<<"= " << "t0[d" << v->code_name() << "];\n";
      }else{
      }
    }
    size_t k = 2;
    for(auto v : f.deps()) {
      o__ "// dep " << v->code_name() << "\n";
      if(f.branch() == v->branch()){
      }else{
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "d->" << f.branch()->state() << "[" << k << "]"
	  " "<<sign<<"= " << "t0[d" << v->code_name() << "];\n";
	++k;
      }
    }
  }
  o << ind <<  "}\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_filter(std::ostream& o, const Module& m)
{
  for(auto f : m.filters()){
    std::string cn = f->branch_code_name();
    o << "COMMON_" << m.identifier() << "::";
    o << "ddouble COMMON_" << m.identifier() << "::FILTER" << f->code_name() <<
        "::operator()(ddouble t0, MOD_" << m.identifier() << "* d) const\n{\n";
    o______ "// incomplete();\n"; // polarities...
    {
      char sign = '+'; // TODO
      indent a;
      o__ "// assert(!d->" << f->state() << "[0]);\n";
      o__ "d->" << f->state() << "[0] = " << sign << " " << "t0.value();\n";
//	  o__ "d->" << f->state() << "[1] = 1.;\n";
      size_t k = 2;

      for(auto v : f->deps()) {
	// char sign = f.reversed()?'-':'+';
	o__ "// dep " << v->code_name() << "\n";
	// if(f->branch() == v->branch()){
	// }else{
	  o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	  o__ "// assert(!d->" << f->state() << "[" << k << "]);\n";
	  o__ "d->" << f->state() << "[" << k << "]"
	    " = " << sign << " " << "t0[d" << v->code_name() << "];\n";
	  ++k;
	//}
      }
    }

    o__ "d->" << cn << "->do_tr();\n";
    o__ "t0 = d->" << cn << "->tr_amps();\n";
    o__ "trace2(\"filt\", t0, d->"<< cn<<"->tr_outvolts());\n";
    assert(f->prb());
    o__ "t0[d" << f->prb()->code_name() << "] = 1.;\n";
    o__ "return t0;\n";

    o << "}\n";
  }
}
/*--------------------------------------------------------------------------*/
void AnalogConditionalExpression::dump(std::ostream& o)const
{
  assert(_exp);
  _exp->dump(o);
  //    o << ")))";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_cond(std::ostream& o, AnalogConditionalStmt const& s);
static void make_cc_analog_seq(std::ostream& o, AnalogSeqBlock const& s);
static void make_cc_analog_stmt(std::ostream& o, Base const& ab)
{
  Base const* i = &ab;
  if(auto s = dynamic_cast<AnalogSeqBlock const*>(&ab)){
    make_cc_analog_seq(o, *s);
  }else if(auto fc=dynamic_cast<FlowContribution const*>(i)) {
    make_cc_contrib(o, *fc);
  }else if(auto pc=dynamic_cast<PotContribution const*>(i)) {
    make_cc_contrib(o, *pc);
  }else if(auto a=dynamic_cast<Assignment const*>(i)) {
    make_cc_assignment(o, *a);
  }else if(auto rl=dynamic_cast<ListOfBlockRealIdentifiers const*>(i)) {
    make_cc_block_real_identifier_list(o, *rl);
  }else if(auto v=dynamic_cast<Variable const*>(i)) {
    unreachable();
    make_cc_variable(o, *v);
  }else if(auto v=dynamic_cast<AnalogConditionalStmt const*>(i)) {
    make_cc_analog_cond(o, *v);
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_cond(std::ostream& o, AnalogConditionalStmt const& s)
{
  o__ "{\n";
  {
    indent x;
    make_cc_expression(o, s.conditional().expression());
    o__ "if (t0) {\n";
    if(s.true_part_or_null()) {
      indent y;
      make_cc_analog_stmt(o, s.true_part());
    }else{
    }
    o__ "}";
    if(s.false_part_or_null()) {
      o << "else ";
      indent y;
      make_cc_analog_stmt(o, s.false_part());
    }else{
    }

    o<<"\n";

  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_seq(std::ostream& o, AnalogSeqBlock const& s)
{
  for(auto i : s.block()) {
    make_cc_analog_stmt(o, *i);
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_analog(std::ostream& o, AnalogConstruct const& ab)
{
  if(ab.statement_or_null()){
    make_cc_analog_stmt(o, *ab.statement_or_null());
  }else{ untested();
    o << ";\n";
  }
}
/*--------------------------------------------------------------------------*/
std::string const& Branch::omit() const
{
  static std::string const n = "";
  return n;
//  return !(_has_contibutions || _has_iprobe);
}
/*--------------------------------------------------------------------------*/
std::string Probe::code_name() const
{
    return "_" + _xs + _br->code_name();
  }
/*--------------------------------------------------------------------------*/
std::string Branch::code_name() const
{
  return "_b_" + p()->name() + "_" + n()->name();
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
