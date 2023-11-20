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
#include "mg_analog.h"
#include "mg_options.h"
/*--------------------------------------------------------------------------*/
static void make_cc_variable(std::ostream& o, Variable const& v)
{
  o << ind << "ddouble _v_" << v.name() << "; // Variable";

  for(auto const& i : v.deps()) {
    o__ " Dep: " << i->code_name() << " lin: " << i.is_linear();
  }
  o << "\n";
  //if(options().fpi()){
  //}else
  o << ind << "_v_" << v.name() << ".set_all_deps();\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_block_int_identifier_list(std::ostream& o, ListOfBlockIntIdentifiers const& rl)
{
  for(Variable const* v : rl){
    assert(v);
    make_cc_variable(o, *v);
  }
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
static bool within_af(Base const* what)
{
  while(what){
    if(dynamic_cast<Analog_Function const*>(what)){
      return true;
    }else if(auto b = dynamic_cast<Block const*>(what)){
      what = b->owner();
    }else if(auto ob = dynamic_cast<Owned_Base const*>(what)){
      what = ob->owner();
    }else{ untested();
      unreachable();
      return false;
    }
  }
  {
    return false;
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_assignment(std::ostream& o, Assignment const& a)
{
  Expression_ const& e = a.rhs();

  o__ "{ // Assignment " << a.type() <<" '" << a.lhsname() << "'.\n";
  std::string lhsname = a.lhs().code_name();

  {
    indent x;
    make_cc_expression(o, e);
    o__ "assert(t0.is_same(t0));\n";
    if(dynamic_cast<Analog_Function_Arg const*>(&a.lhs())){
      o__ lhsname << " = t0; // (*)\n";
    }else if(a.is_int()){
      o__ lhsname << " = int(t0); // (*)\n";
    }else if(within_af(&a)){
      o__ lhsname << " = t0; // (1a)\n";
    }else if(!options().optimize_deriv()) { untested();
      o__ lhsname << " = t0; // (*)\n";
      for(auto v : a.deps()) {
	o__ "// " << a.lhs().code_name() << "[d" << v->code_name() << "] = " << "t0[d" << v->code_name() << "]; // (2a)\n";
	o__ "assert(" << a.lhs().code_name() << "[d" << v->code_name() << "] == " << "t0[d" << v->code_name() << "]); // (2a2)\n";
      }
    }else{
      o__ lhsname << " = t0.value(); // (*)\n";
//      o__ lhsname << ".set_no_deps(); // (42)\n";
#ifdef TRACE_ASSIGN
      o__ "trace1(\"assign\", " << lhsname << ");\n";
#endif

      for(auto v : a.deps()) {
	assert(v->branch());
	if(v.is_linear()){
	  // TODO incomplete();
	}else if(v.is_quadratic()){
	  incomplete();
	}else{
	  // TODO incomplete();
	}

	if(a.deps().is_linear()) {
//	}else if(v.is_linear()){ incomplete(); // later.
	}else{
	}

	if(v->branch()->is_short()) { untested();
	// }else if(a.lhs().is_module_variable()){
	}else{
	  o__ lhsname << "[d" << v->code_name() << "] = " << "t0[d" << v->code_name() << "]; // (2b)\n";
	  o__ "assert(" << lhsname << "[d" << v->code_name() << "] == " << "t0[d" << v->code_name() << "]); // (2b2)\n";
	}
#ifdef TRACE_ASSIGN
	o__ "trace1(\"assign\", " << lhsname << "[d" << v->code_name() << "]);\n";
#endif
      }
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_contrib(std::ostream& o, Contribution const& C)
{
  Expression const& e = C.rhs();

  o__ "{ // Contribution " << C.name() << C.branch_ref() << " lin: " << C.deps().is_linear() << "\n";
  if(!C.is_pot_contrib() && is_zero(e)){
    // TODO incomplete(); // optimize out?
  }else if(C.branch()->is_short()){
  }else{
    indent x("  ");
    make_cc_expression(o, e);

    char sign = C.reversed()?'-':'+';
    std::string bcn = C.branch_ref().code_name();
    if(C.branch()->has_pot_source()){
      if(C.is_pot_contrib()){
	o__ "if (!d->_pot" << bcn << "){\n";
	o____ "d->_pot" << bcn << " = true;\n";
      }else{
	o__ "if (d->_pot" << bcn << "){\n";
	o____ "d->_pot" << bcn << " = false;\n";
      }

      // DUP, clear.
      o____ "d->_value" << bcn << " = 0.;\n";
      o____ "std::fill_n(d->_st" << bcn << "+1, " << C.branch()->num_states()-1 << ", 0.);\n";
      o__ "}else{\n";
      o__ "}\n";
    }else if(C.branch()->has_flow_probe()){


    }else{
      // always flow.
    }

    assert(C.branch());
    if(C.branch()->deps().is_linear()) {
    }else{
      trace1("nonlinear branch deps", C.branch()->code_name());
    }

    if(C.deps().is_linear()) {
      trace1("linear C deps", C.branch()->code_name());
    }else{
    }

    o__ "d->_value" << bcn << " " << sign << "= t0.value();\n";
    if(C.branch()->has_pot_source()) {
      // incomplete? //
    }else if(C.branch()->has_flow_probe()) {
      // BUG? what does the standard say? cf. mg3_iprobe.
      o__ "d->_flow" << C.branch()->code_name() << " = " <<
	  "d->_value" << C.branch()->code_name() << "; // (8)\n";
    }
    for(auto v : C.deps()) {
      if(C.branch() == v->branch()){
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "d->_st" << bcn << "[1]"
	  " " << sign << "= " << "t0[d" << v->code_name() << "];\n";
      }else{
      }
    }
    for(auto v : C.deps()) {
      o__ "// dep " << v->code_name() << "\n";
      assert(v->branch());
      if(C.branch() == v->branch()){
      }else if(v->branch()->is_short()) { untested();
      }else{
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "assert(MOD::" << C.branch()->state() << "_::dep" << v->code_name() << " < "
	   << C.branch()->num_states() << ");\n";
	o__ "d->" << C.branch()->state() << "["
	   << "MOD::" << C.branch()->state() << "_::dep" << v->code_name() << "] "
	   << sign << "= " << "t0[d" << v->code_name() << "]; // (3)\n";
      }
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void AnalogExpression::dump(std::ostream& o)const
{
  _exp.dump(o);
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_cond(std::ostream& o, AnalogConditionalStmt const& s);
static void make_cc_analog_switch(std::ostream& o, AnalogSwitchStmt const& s);
static void make_cc_analog_for(std::ostream& o, AnalogForStmt const& s);
static void make_cc_analog_while(std::ostream& o, AnalogWhileStmt const& s);
static void make_cc_analog_seq(std::ostream& o, AnalogSeqBlock const& s);
static void make_cc_analog_ctrl(std::ostream& o, AnalogCtrlBlock const& s);
static void make_cc_analog_evt(std::ostream& o, AnalogEvtCtlStmt const& s);
static void make_cc_system_task(std::ostream& o, System_Task const& s);
static void make_cc_analog_stmt(std::ostream& o, Base const& ab)
{
  Base const* i = &ab;
  if(auto s = dynamic_cast<AnalogSeqBlock const*>(&ab)){
    make_cc_analog_seq(o, *s);
  }else if(auto s = dynamic_cast<AnalogCtrlBlock const*>(&ab)){
    make_cc_analog_ctrl(o, *s);
  }else if(auto fc=dynamic_cast<Contribution const*>(i)) {
    make_cc_contrib(o, *fc);
  }else if(auto a=dynamic_cast<AnalogProceduralAssignment const*>(i)) {
    make_cc_assignment(o, a->assignment());
  }else if(auto a=dynamic_cast<Assignment const*>(i)) {
    make_cc_assignment(o, *a);
  }else if(auto rl=dynamic_cast<ListOfBlockRealIdentifiers const*>(i)) {
    make_cc_block_real_identifier_list(o, *rl);
  }else if(auto rl=dynamic_cast<ListOfBlockIntIdentifiers const*>(i)) {
    make_cc_block_int_identifier_list(o, *rl);
  }else if(auto v=dynamic_cast<Variable const*>(i)) { untested();
    unreachable();
    make_cc_variable(o, *v);
  }else if(auto v=dynamic_cast<AnalogConditionalStmt const*>(i)) {
    make_cc_analog_cond(o, *v);
  }else if(auto ss=dynamic_cast<AnalogSwitchStmt const*>(i)) {
    make_cc_analog_switch(o, *ss);
  }else if(auto ww=dynamic_cast<AnalogForStmt const*>(i)) {
    make_cc_analog_for(o, *ww);
  }else if(auto ww=dynamic_cast<AnalogWhileStmt const*>(i)) {
    make_cc_analog_while(o, *ww);
  }else if(auto ev=dynamic_cast<AnalogEvtCtlStmt const*>(i)) {
    make_cc_analog_evt(o, *ev);
    //throw Exception("analogevtctl unsupported");
  }else if(auto t=dynamic_cast<System_Task const*>(i)) {
    make_cc_system_task(o, *t);
  }else{ untested();
    incomplete();
    assert(false);
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_af_body(std::ostream& o, const Analog_Function& f)
{
  std::string me = f.variable().code_name();
  o__ "ddouble " << me << "(0.);\n";

  // is-a-block, has-a-block...?
  for(auto i : f) {
    make_cc_analog_stmt(o, *i);
  }
  o__ "return " << me << ";\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_system_task(std::ostream& o, System_Task const& s)
{
  o__ "{\n";
  make_cc_expression(o, s.expression().expression());
  o <<  "\n";
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_evt(std::ostream& o, AnalogEvtCtlStmt const& s)
{
  o__ "{\n";
  {
    indent x;
    make_cc_event_cond(o, s.cond());
    o__ "if (evt) {\n";
    if(s.stmt_or_null()) {
      indent y;
      make_cc_analog_stmt(o, *s.stmt_or_null());
    }else{
    }
    o__ "}else{\n";
    o__ "}\n";

    o << "\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_loop(std::ostream& o, AnalogWhileStmt const& s)
{
  o__ "while(true) {\n";
  {
    indent x;
    make_cc_expression(o, s.conditional().expression());
    o__ "if (t0) {\n";
    if(s.has_body()) {
      indent y;
      make_cc_analog_stmt(o, s.body());
    }else{ untested();
    }

    if(s.has_tail()){
      make_cc_analog_stmt(o, s.tail());
    }else{
    }

    o__ "}else{\n";
    o____ "break;\n";
    o__ "}\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_while(std::ostream& o, AnalogWhileStmt const& s)
{
  make_cc_analog_loop(o, s);
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_for(std::ostream& o, AnalogForStmt const& s)
{
  if(s.has_init()){
    make_cc_analog_stmt(o, s.init());
  }else{
  }
  make_cc_analog_loop(o, s);
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_cond(std::ostream& o, AnalogConditionalStmt const& s)
{
  o__ "{\n";
  if(s.conditional().is_true()) {
    if(s.true_part()) {
      indent y;
      make_cc_analog_stmt(o, s.true_part());
    }else{ untested();
    }
  }else if(s.conditional().is_false()){
    if(s.false_part()) {
      indent y;
      make_cc_analog_stmt(o, s.false_part());
    }else{
    }
  }else{
    indent x;
    make_cc_expression(o, s.conditional().expression());
    o__ "if (t0) {\n";
    if(s.true_part()) {
      indent y;
      make_cc_analog_stmt(o, s.true_part());
    }else{
    }
    o__ "}";
    if(s.false_part()) {
      o << "else {\n";
      {
	indent y;
	make_cc_analog_stmt(o, s.false_part());
      }
      o__ "}\n";
    }else{
    }
    o << "\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cond_expressions(std::ostream& o, AnalogConstExpressionList const&l)
{
  std::string paren="";
  for(auto e : l){
    make_cc_expression(o, e->expression());

    o__ "if(t0 == s){\n";
    o____ "cond = true;\n";
    o__ "}else{\n";
    paren += "}";
  }

  o << paren << "\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_switch(std::ostream& o, AnalogSwitchStmt const& s)
{
  // TODO: indent properly
  o__ "{\n";
  {
    indent x;
    o__ "double s;\n";
    o__ "{\n";
    {
      indent y;
      make_cc_expression(o, s.control().expression());
      o__ "s = t0;\n";
    }
    o__ "}\n";
    std::string paren="";

    CaseGen const* def = NULL;
    for(auto& i : s.cases()){
      if(i->is_never()){
      }else if(i->cond_or_null()){
	o << "{\n";

	o << "bool cond = false;\n";
	make_cond_expressions(o, *i->cond_or_null());

	o__ "if (cond) {\n";

	if(i->code_or_null()){
	  indent y;
	  make_cc_analog_stmt(o, *i->code_or_null());
	}else{ untested();
	}

	o__ "}else";
	paren += "}";

      }else{
	def = &*i;
      }
    }

    o << "{\n";
    if(!def){
    }else if(def->code_or_null()){
      indent y;
      make_cc_analog_stmt(o, *def->code_or_null());
    }else{ untested();
    }
    o<<paren;
    o__ "\n";
    o__ "}\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_ctrl(std::ostream& o, AnalogCtrlBlock const& s)
{
  o__ "{ // " << s.identifier() << "\n";
  for(auto i : s.block()) {
    indent x;
    make_cc_analog_stmt(o, *i);
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_seq(std::ostream& o, AnalogSeqBlock const& s)
{
  o__ "{ // " << s.identifier() << "\n";
  for(auto i : s.block()) {
    indent x;
    make_cc_analog_stmt(o, *i);
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void make_analog_construct(std::ostream& o, AnalogConstruct const& ab)
{
  if(ab.statement_or_null()){
    make_cc_analog_stmt(o, *ab.statement_or_null());
  }else{ untested();
    unreachable();
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
  if ( _type == t_flow ){
    return "_flow" + _br->code_name();
  }else if (_type == t_pot){
    return "_potential" + _br->code_name();
  }else{
    unreachable();
    return("unreachable_probe");
  }
}
/*--------------------------------------------------------------------------*/
std::string Branch::code_name() const
{
  return "_b_" + p()->name() + "_" + n()->name();
}
/*--------------------------------------------------------------------------*/
std::string Analog_Function_Arg::code_name()const
{
  return "af_arg_" + name();
}
/*--------------------------------------------------------------------------*/
static void make_set_one_branch_contribution(std::ostream& o, const Branch& br)
{
  Branch const* b = &br;
  assert(!br.is_short());
  o__ "assert(_value" << b->code_name() << " == _value" << b->code_name() << ");\n";

  o__ b->state() << "[0] = _value" << b->code_name() << ";\n";
  if(b->has_pot_source()){
    for(auto n: b->names()){
	o__ "if(_pot" << b->code_name() << "){\n";
	o__ "}else if(_pot_br_" << n << "){\n";
	o____ "_pot" << b->code_name() << " = true;\n";
	o____ b->state() << "[0] = _value_br_" << n << ";\n";
	o____ "notstd::copy_n(_st_br_" << n << "+1, " << b->num_states()-1 << ", " << b->state() <<" + 1);\n";
	o__ "}else if (!_pot" << b->code_name() << "){itested();\n";
	o____ b->state() << "[0] += _value_br_" << n << ";\n";
	o____ "notstd::add_n(_st_br_" << n << "+1, " << b->num_states()-1 << ", " << b->state() <<" + 1);\n";
	o__ "}else{untested();\n";
	o__ "}\n";
    }
  }else{
    o__ "// collect currents from named branches\n";
    for(auto n: b->names()){
      o__ b->state() << "[0] += _value_br_" << n << ";\n";
      o__ "for(int k=1; k<" << b->num_states() << "; ++k){\n";
	o__ b->state() << "[k] += _st_br_" << n << "[k];\n";
      o__ "}\n";
    }
  }

  if(b->deps().is_linear()){
    // TODO incomplete();
    // o__ b->state() << "[0] = 0.;\n";
  }else{
  }

  for(auto const& d : b->deps()){
    if(d->branch() == b){
      o__ "// self_admittance lin: " << d.is_linear() << "\n";
      if(b->has_pot_source() && b->has_flow_probe()){
	if(br.num_states()<=2){
	}else{
	  incomplete(); // the other ones??
	}
      }else{
      }
      o__ "trace2(\"" <<  b->state() << "self\", " << b->state() << "[1], "<<  d->code_name() <<");\n";
      o__ b->state() << "[0] -= " << b->state() << "[1] * " << d->code_name() << "; // (4)\n";

      break;
    }else{
    }
  }

  // TODO: does it work in current mode?
  if(!br.is_generic()){
  }else if(br.is_direct()){
  }else{
    o__ "if (_pot"<< b->code_name() << "){\n";
    o____ b->state() << "[1] += 1.; // (4c)\n";
    o__ "}else{\n";
    o__ "}\n";
  }
  if (b->deps().size()) {
//    o__ "typedef long double D;\n";
    o__ "long double sp = 0.;\n";
  }else{
  }
  o__ "// sources...\n";
  {
    indent ii;

    for(auto const& d : b->deps()){
      o__ "// " << d->code_name() << " lin: " <<  d.is_linear() << "\n";
      if(d->branch() == b){
      }else if(d->branch()->is_short()) {
      }else if(d->is_pot_probe()){
	o__ "sp += (long double)(" << b->state() << "["
	 << "/*MOD::*/" << b->state() << "_::dep" << d->code_name()
	 << "] * "<< d->code_name() << ");\n";
      }else if(d->is_flow_probe()){
	o__ "sp += (long double)(" << b->state() << "["
	 << "/*MOD::*/" << b->state() << "_::dep" << d->code_name()
	 << "] * "<< d->code_name() << "); // (5)\n";
	o__ b->state() << "["
	 << "/*MOD::*/" << b->state() << "_::dep" << d->code_name()
	 <<"] *= " << d->branch()->code_name() <<"->_loss0; // BUG?\n";
      }else{ untested();
	o__ "// bogus probe " << b->state() << " : " << d->code_name() << "\n";
      }
      if(d->branch() == b){
      }else if(d->is_flow_probe()){
	// todo?
      }
    }
  }

  if (b->deps().size()) {
    o__ b->state() << "[0] = double(" << b->state() << "[0] - sp);\n";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
// some filters are not reached in do_tr. set output is zero.
void make_zero_filter_readout(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::zero_filter_readout()\n{\n";
  for(auto x : m.branches()){
    Branch const* b = x;
    assert(b);
    if(!b->is_filter()){
    }else{
      if(b->is_short()){
      }else if(b->has_pot_probe()){
	o__ "_potential" << b->code_name() << " = 0.;\n";
      }
    }
  }

  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_set_branch_contributions(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::set_branch_contributions()\n{\n";
  for(auto i : m.branches()){
    Branch const* b = i;

    if(b->is_short()) {
    }else if(b->has_flow_source() || b->has_pot_source()) {
      indent ii;
      o__ "{\n";
      make_set_one_branch_contribution(o, *b);
      o__ "}\n";
    }else if(b->has_flow_probe()) {
      o__ "// flow prb " << b->name() << "\n";
      o__ "if(" << b->code_name() << "){\n";
      o____ b->code_name() << "->_loss0 = 1./OPT::shortckt; // (L0)\n";
      o__ "}else{\n";
      o__ "}\n";
    }else{
    }

    if(b->is_short()) {
    }else if(b->has_pot_source()) {
      o__ "// pot src " << b->name() << "\n";
      o__ "if(!" << b->code_name() << "){ untested();\n";
      o__ "}else if(_pot" << b->code_name() << "){\n";
      o____ b->code_name() << "->_loss0 = 1./OPT::shortckt;\n";
      for(auto n: b->names()){
	o__ "// }else if(_pot_br_" << n << "){\n";
	o____ "//" << b->code_name() << "->_loss0 = 1./OPT::shortckt;\n";
      }
      o__ "}else{\n";
      o____ b->code_name() << "->_loss0 = 0.;\n";
      o__ "}\n";


    }else if(b->has_flow_source()) {
      assert(!b->has_pot_source());
      o__ "// flow src " << b->name() << "\n";
      if(b->has_flow_probe()) {
	o__ "// not a flow prb? " << b->name() << "\n";
	o__ "if(" << b->code_name() << "){\n";
	o____ b->code_name() << "->_loss0 = 0.; // (L1);\n";
	o__ "}else{\n";
	o__ "}\n";
      }else{
      }
    }else{
    }
  }

  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_one_variable_proxy(std::ostream& o, const Variable_Decl& V, Module const& m)
{
  o__ "class _V_" << V.name() << " : public ddouble {\n";
  o____ "MOD_" << m.identifier() << " * const _m;\n";
  o__ "public:\n";
  o____ "typedef ddouble base;\n";
  o____ "typedef va::ddouble_tag base_tag;\n";
  o____ "_V_" << V.name() << "(ddouble const& p) : ddouble(p), _m(NULL) { itested(); }\n";
  o____ "_V_" << V.name() << "(double const& p) : ddouble(p), _m(NULL) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(PARAMETER<double> const& p) : ddouble(p), _m(NULL) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(_V_" << V.name() << " const& p) : ddouble(p), _m(NULL) {}\n";
  o____ "explicit _V_" << V.name() << "() : ddouble(), _m(NULL) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(MOD_" << m.identifier() << " * m) : "
    << "ddouble(m->" << V.code_name() << "), _m(m) {}\n";
  o____ "~_V_" << V.name() << "() {\n"
    << "	if(_m){\n";
  o______ "_m->" << V.code_name() << " = value();\n";
  o____ "}else{itested(); }}\n";
  o____ "ddouble& operator=(double t){\n";
  o______ "ddouble::operator=(t);\n";
  o______ "return *this;\n";
  o____ "}\n";
  o____ "ddouble& operator=(ddouble t){\n";
  o______ "ddouble::operator=(t);\n";
  o______ "return *this;\n";
  o____ "}\n";
  o__ "}";
}
/*--------------------------------------------------------------------------*/
void make_one_variable_load(std::ostream& o, const Variable_Decl& V, Module const& m)
{
  if(V.type().is_real()) {
    if(!V.is_module_variable()){ untested();
    }else if(V.deps().size() == 0){
      o__ "double& " << V.code_name() << "(d->" << V.code_name() << ");\n";
    }else if(options().optimize_deriv()) {
      make_one_variable_proxy(o, V, m);
      o << V.code_name() << "(d);\n";
    }else{ untested();
      o__ "ddouble " << V.code_name() << "(d->" << V.code_name() << ");\n";
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void make_one_variable_store(std::ostream& o, const Variable_Decl& V)
{
  if(!V.type().is_real()) {
  }else if(!V.is_module_variable()){ untested();
    unreachable();
  }else if(V.deps().size() == 0){ itested();
    // it's a reference.
  }else if(options().optimize_deriv()) {
    // use destructor
  }else{ untested();
    o__ "d->" << V.code_name() << " = " << V.code_name() << ".value();\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_load_variables(std::ostream& o, const
    Variable_List_Collection& P, const Module& m)
{
  // not sure if these are actually unused. GCC bug?
  o << "#pragma GCC diagnostic push\n";
  o << "#pragma GCC diagnostic ignored \"-Wunused-local-typedefs\"\n";

  for (auto q = P.begin(); q != P.end(); ++q) {
    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      Variable_Decl const* V = *p;
      assert(V);
      if(!V->is_module_variable()){
      }else{
	make_one_variable_load(o, *V, m);
      }
    }
  }

  o << "#pragma GCC diagnostic pop\n";
}
/*--------------------------------------------------------------------------*/
static void make_store_variables(std::ostream& o, const Variable_List_Collection& P)
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      Variable_Decl const* V = *p;
      assert(V);
      if(!V->is_module_variable()) { untested();
      }else{
	make_one_variable_store(o, *V);
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_common_tr_eval(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() << 
    "::tr_eval_analog(MOD_" << m.identifier() << "* d) const\n{\n";
  o__ "trace1(\"" << m.identifier() <<"::tr_eval_analog\", d);\n";
  o__ "trace1(\"" << m.identifier() <<"::tr_eval_analog\", d->long_label());\n";

//  make_variable_decl(o, m.variables());
  make_load_variables(o, m.variables(), m);

  // parameters are here.
  o__ "MOD_" << m.identifier() << " const* p = d;\n";
  o__ "assert(p);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";

  for(auto bb : m.analog_list()){
    assert(bb);
//    if(auto ab = dynamic_cast<AnalogStmt const*>(bb)){
//    }else
    if(auto ab = dynamic_cast<AnalogConstruct const*>(bb)){
      o__ "{\n";
      {
	indent a;
	make_analog_construct(o, *ab);
      }
      o__ "}\n";
    }else{ untested();
    }
  }
  make_store_variables(o, m.variables());
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_clear_branch_contributions(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::clear_branch_contributions()\n{\n";
  for(auto x : m.branches()){
    assert(x);
    if(x->has_element()){
      if(x->has_pot_source()){
	o____ "_pot" << x->code_name() << " = false;\n";
	for(auto n: x->names()){
	  o____ "_pot_br_" << n << " = false;\n";
	}
      }else{
      }
      o____ "_value" << x->code_name() << " = 0.;\n";
      o____ "std::fill_n(_st" << x->code_name() << "+1, " << x->num_states()-1 << ", 0.);\n";
      for(auto n: x->names()){
	o____ "_value_br_" << n << " = 0.;\n";
	o____ "std::fill_n(_st_br_" << n << "+1, " << x->num_states()-1 << ", 0.);\n";
      }
    }else{
    }
  }
  o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_analog(std::ostream& o, const Module& m)
{
  make_zero_filter_readout(o, m);
  make_set_branch_contributions(o, m);
  make_clear_branch_contributions(o, m);
  make_common_tr_eval(o, m);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
