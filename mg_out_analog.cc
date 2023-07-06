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
//#include "m_tokens.h"
/*--------------------------------------------------------------------------*/
static void make_cc_variable(std::ostream& o, Variable const& v)
{
  o << ind << "ddouble _v_" << v.name() << "; // Variable";
  for(auto i : v.deps()) {
    o << ind << " Dep: " << i->code_name();
  }
  o << ind << "\n";
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
  assert(a.rhs());
  Expression const& e = *a.rhs();

  o << ind << "{ // Assignment " << a.type() <<" '" << a.lhsname() << "'.";
  for(auto i : a.deps()) {
    o << " Dep: " << i->code_name();
  }
  o << "\n";

  {
    indent x;
    make_cc_expression(o, e);
    o__ "assert(t0.is_same(t0));\n";
    if(dynamic_cast<Analog_Function_Arg const*>(&a.lhs())){
      o__ a.lhs().code_name() << " = t0; // (*)\n";
    }else if(a.is_int()){
      o__ a.lhs().code_name() << " = int(t0); // (*)\n";
    }else if(within_af(&a)){
      o__ a.lhs().code_name() << " = t0; // (**)\n";
    }else{
#ifdef PASS_UNUSED_DERIV
      untested();
      o__ a.lhs().code_name() << " = t0; // (*)\n";
      for(auto v : a.deps()) { untested();
	o__ "// " << a.lhs().code_name() << "[d" << v->code_name() << "] = " << "t0[d" << v->code_name() << "]; // (2a)\n";
      }
#else
      o__ a.lhs().code_name() << " = t0.value(); // (*)\n";

      for(auto v : a.deps()) {
	if(v->is_reversed()){
	}else{
	}
	assert(v->branch());
	if(v->branch()->is_short()){ untested();
	}else{
	  o__ a.lhs().code_name() << "[d" << v->code_name() << "] = " << "t0[d" << v->code_name() << "]; // (2b)\n";
	}
      }
#endif
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_contrib(std::ostream& o, Contribution const& C)
{
  o__ "{ // Contribution " << C.name() << "\n";
  if(C.branch()->is_short()){
  }else{
    indent x("  ");
    if(C.rhs()) {
      make_cc_expression(o, *C.rhs());
    }else{ untested();
    }

    char sign = C.reversed()?'-':'+';
    std::string bcn = C.branch()->code_name();
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
    o__ "d->_value" << C.branch()->code_name() << " " << sign << "= t0.value();\n";
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
	o__ "d->" << C.branch()->state() << "[1]"
	  " "<<sign<<"= " << "t0[d" << v->code_name() << "];\n";
      }else{
      }
    }
    size_t k = 2;
    for(auto v : C.deps()) {
      o__ "// dep " << v->code_name() << "\n";
      assert(v->branch());
      if(C.branch() == v->branch()){
      }else if(v->branch()->is_short()) { untested();
      }else{
	o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	o__ "assert(MOD::" << C.branch()->state() << "_::dep" << v->code_name() << " < "
	   << C.branch()->num_states() << ");\n";
	o__ "d->" << C.branch()->state() << "[" // << k << "/*"
	  << "MOD::" << C.branch()->state() << "_::dep" << v->code_name() << "] "
	  <<sign<<"= " << "t0[d" << v->code_name() << "]; // (3)\n";
	++k;
      }
    }
  }
  o << ind <<  "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_af_args(std::ostream& o, const Analog_Function& f)
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
static void make_cc_af_body(std::ostream& o, const Analog_Function& f);
/*--------------------------------------------------------------------------*/
void make_cc_analog_functions(std::ostream& o, const Module& m)
{
  for(auto f : m.analog_functions()){
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
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_filter(std::ostream& o, const Module& m)
{
  for(auto f : m.filters()){
    o << "COMMON_" << m.identifier() << "::";
    o << "ddouble COMMON_" << m.identifier() << "::FILTER" << f->code_name() <<
        "::operator()(";
	for(size_t n=0; n<f->num_args(); ++n){
	    o << " ddouble t" << n << ", ";
	}
    o << "MOD_" << m.identifier() << "* d) const\n{\n";
    o______ "// incomplete();\n"; // polarities...

    if(f->has_branch()) {
      char sign = '+'; // TODO
      indent a;
      o__ "// assert(!d->" << f->state() << "[0]);\n";
      o__ "d->" << f->state() << "[0] = " << sign << " " << "t0.value();\n";
//	  o__ "d->" << f->state() << "[1] = 1.;\n";
      size_t k = 2;

      for(auto v : f->deps()) {
	// char sign = f.reversed()?'-':'+';
	o__ "// dep " << v->code_name() << "\n";
	// if(f->branch() == v->branch()){ untested();
	if(v->branch()->is_short()){ untested();
	}else{
	  o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	  o__ "// assert(!d->" << f->state() << "[" << k << "]);\n";
	  o__ "d->" << f->state() << "[" //  << k << "]"
	  << "MOD::" << f->state() << "_::dep" << v->code_name() << "] "
	    " = " << sign << " " << "t0[d" << v->code_name() << "]; // (4)\n";
	  ++k;
	}
      }
    }else{
      o__ "(void) d;\n";
      f->make_cc(o, m);
    }

    if(f->has_branch()){
      std::string cn = f->branch_code_name();
      o__ "d->" << cn << "->do_tr();\n";
//      o__ "d->" << cn << "->q_eval();\n";
      o__ "t0 = d->" << cn << "->tr_amps();\n";
      o__ "trace2(\"filt\", t0, d->"<< cn<<"->tr_outvolts());\n";
      assert(f->prb());
      o__ "t0[d" << f->prb()->code_name() << "] = 1.;\n";
      o__ "return t0;\n";
    }else{
    }

    o << "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  }
}
/*--------------------------------------------------------------------------*/
void AnalogExpression::dump(std::ostream& o)const
{
  assert(_exp);
  _exp->dump(o);
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_cond(std::ostream& o, AnalogConditionalStmt const& s);
static void make_cc_analog_switch(std::ostream& o, AnalogSwitchStmt const& s);
static void make_cc_analog_for(std::ostream& o, AnalogForStmt const& s);
static void make_cc_analog_while(std::ostream& o, AnalogWhileStmt const& s);
static void make_cc_analog_seq(std::ostream& o, AnalogSeqBlock const& s);
static void make_cc_analog_evt(std::ostream& o, AnalogEvtCtlStmt const& s);
static void make_cc_system_task(std::ostream& o, System_Task const& s);
static void make_cc_analog_stmt(std::ostream& o, Base const& ab)
{
  Base const* i = &ab;
  if(auto s = dynamic_cast<AnalogSeqBlock const*>(&ab)){
    make_cc_analog_seq(o, *s);
  }else if(auto fc=dynamic_cast<Contribution const*>(i)) {
    make_cc_contrib(o, *fc);
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
static void make_cc_af_body(std::ostream& o, const Analog_Function& f)
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
  o____ "// incomplete, system task \n#if 0\n";
//  s.dump(o);
  make_cc_expression(o, s.expression().expression());
  o <<  " \n";
  o << "#endif\n";
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

    o<<"\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_analog_loop(std::ostream& o, AnalogWhileStmt const& s)
{
  o__ "while(true){\n";
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
      o__ "{\n";
      {
	indent y;
	make_cc_analog_stmt(o, s.false_part());
      }
      o__ "}\n";
    }else{
    }

    o<<"\n";

  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cond_expressions(std::ostream& o, AnalogConstExpressionList const&l)
{

  std::string paren="";
//  o<< "{\n";
  for(auto e : l){
    make_cc_expression(o, e->expression());

    o__ "if(t0 == s){\n";
    o____ "cond=true;\n";
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
      make_cc_expression(o, s.conditional().expression());
      o__ "s = t0;\n";
    }
    o__ "}\n";
    std::string paren="";

    CaseGen const* def = NULL;
    for(auto& i : s.cases()){
      if(i->cond_or_null()){
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
std::string Analog_Function_Arg::code_name()const
{
  return "af_arg_" + name();
}
/*--------------------------------------------------------------------------*/
static void make_one_branch_contribution(std::ostream& o, Module const& m, const Branch& br)
{
  Branch const* b = &br;
  assert(!br.is_short());
  size_t k = 1;
  o__ "assert(_value" << b->code_name() << " == _value" << b->code_name() << ");\n";
  o__ b->state() << "[0] = _value" << b->code_name() << ";\n";

  // self_admittance
  for(auto d : b->deps()){
    if(d->branch() == b){
      if(b->has_pot_source() && b->has_flow_probe()){

	o__ "if (!_pot"<< b->code_name() << "){ itested();\n";
	o__ "}else if (" << b->state() << "[1] > OPT::shortckt){\n";
	o____ "_pot"<< b->code_name() << " = false;\n";
	o____ b->state() << "[0] /= - " << b->state() << "[1];\n";
	if(br.num_states()<=2){
	}else{
	  o__ "incomplete();\n";
	  std::cerr << "INCOMPLETE" << "_pot"<< b->code_name() << "\n";
	  incomplete(); // the other ones??
	}
	o__ "}else{\n";
	o__ "}\n";
      }else{
      }
      o__ "trace2(\"" <<  b->state() << "self\", " << b->state() << "[1], "<<  d->code_name() <<");\n";
      o__ b->state() << "[0] -= " << b->state() << "[1] * "<< d->code_name() << ";\n";
      break;
    }else{
    }
  }
  k = 2;
  for(auto i : m.branches()){
    if(i->is_short()){
      continue;  // incomplete
    }else{

    }

//    o__ "trace1(\"" <<  b->state() << "\", _value" << b->code_name() << ");\n";
    for(auto d : b->deps()){
      if(d->branch() == b){
      }else if(d->branch() != i){
      }else if(d->is_pot_probe()){
	o__ "// assert(" << k << "  ==  /*MOD::*/" << b->state() << "_::dep" << d->code_name() << ");\n";
	o__ "trace2(\" pot " <<  b->state() << "\", " << b->state() << "["<<k<<"], "<<  d->code_name() <<");\n";
	o__ b->state() << "[0] -= " << b->state() << "["
	 << "/*MOD::*/" << b->state() << "_::dep" << d->code_name()
	 << "] * "<< d->code_name() << ";\n";
	++k;
	break;
      }else if(d->is_filter_probe()){
	o__ "// trace2(\" filter " <<  b->state() << "\", " << b->state() << "["<<k<<"], "<<  d->code_name() <<");\n";
	o__ "// assert(" << k << "  ==  /*MOD::*/" << b->state() << "_::dep" << d->code_name() << ");\n";
	o__ b->state() << "[0] -= " << b->state() << "["
	  << "/*MOD::*/" << b->state() << "_::dep" << d->code_name()
	  << "] * " << d->branch()->code_name() << "->tr_amps();\n";
	++k;
	break;
      }else if(d->is_flow_probe()){
	// nothing, handled below.
      }else{ untested();
	o__ "// bogus probe " << b->state() << " : " << d->code_name() << "\n";
      }
      if(d->branch() == b){
      }else if(d->branch() != i){
      }else if(d->is_flow_probe()){
	// todo.
      }
    }
  }
#if 1
  for(auto i : m.branches()){
    // o__ "trace1(\"" <<  b->state() << "\", _value" << b->code_name() << ");\n";
    if(i->is_short()){
      continue;  // incomplete
    }else{

    }
    for(auto d : b->deps()){
      if(d->branch() == b){
      }else if(d->branch()->is_short()){ untested();
      }else if(d->branch() != i){
      }else if(d->is_flow_probe()){
	o__ "trace2(\"flow " <<  b->state() << "\", " << b->state() << "["<<k<<"], "<<  d->code_name() <<");\n";
	o__ b->state() << "[0] -= " << b->state() << "["
	 << "/*MOD::*/" << b->state() << "_::dep" << d->code_name()
	// << k
	 << "] * "<< d->code_name() << "; // (5)\n";
	// BUG? scaling before convcheck?
	o__ b->state() << "["
	 << "/*MOD::*/" << b->state() << "_::dep" << d->code_name()
	// << k
	 <<"] *= " << d->branch()->code_name() <<"->_loss0; // BUG?\n";
	++k;
	break;
      }else if(d->is_pot_probe()){
	// nothing, handled above
      }else if(d->is_filter_probe()){
	// nothing, handled above
      }else{ untested();
      }
    }
  }
#endif
}
/*--------------------------------------------------------------------------*/
void make_set_branch_contributions(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::set_branch_contributions()\n{\n";
  for(auto i : m.branches()){
    Branch const* b = i;
    // if(b->has_pot_source() && b->has_flow_source()){
    //  ...
    // }else

    if(b->is_short()) {
    }else if(b->has_flow_source() || b->has_pot_source()) {
      for(auto D : b->deps()){
	o__ "// DEP " << D->code_name() << "\n";
      }
      make_one_branch_contribution(o, m, *b);
    }else if(b->has_flow_probe()) {
      o__ "// flow prb " << b->name() << "\n";
      o__ "if(" << b->code_name() << "){\n";
      o____ b->code_name() << "->_loss0 = 1./OPT::shortckt;\n";
      o__ "}else{\n";
      o__ "}\n";
    }else{
    }

    if(b->is_short()) {
    }else if(b->has_pot_source()) {
      o__ "// pot src " << b->name() << "\n";
      o__ "if(!" << b->code_name() << "){\n";
      o__ "}else if(_pot" << b->code_name() << "){\n";
	o____ b->code_name() << "->_loss0 = 1./OPT::shortckt;\n";
      o__ "}else{\n";
	o____ b->code_name() << "->_loss0 = 0.;\n";
      o__ "}\n";
    }else if(b->has_flow_source()) {
      assert(!b->has_pot_source());
      o__ "// flow src " << b->name() << "\n";
    }else{
    }
  }

  o__ "// filters\n";
  for(auto x : m.filters()){
    o__ "// filter " << x->code_name() << "\n";
  }

  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_tr_eval(std::ostream& o, const Module& m)
{
  o << "inline void COMMON_" << m.identifier() << 
    "::tr_eval_analog(MOD_" << m.identifier() << "* d) const\n{\n";

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
      o << ind << "{\n";
      {
	indent a;
	make_analog_construct(o, *ab);
      }
      o << ind << "}\n";
    }else{ untested();
    }
  }
  o << "}\n";
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
      }else if(x->has_pot_source()){ untested();
	o____ "_pot" << x->code_name() << " = true;\n";
      }else{
      }
      o____ "_value" << x->code_name() << " = 0.;\n";
      o____ "std::fill_n(_st" << x->code_name() << "+1, " << x->num_states()-1 << ", 0.);\n";
    }else{
    }
  }
  for(auto x : m.filters()){
    assert(x);
    if(x->has_branch()){
      o____ "// std::fill_n(_st" << x->code_name() << ", " << x->num_states() << ", 0.);\n";
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
  make_set_branch_contributions(o, m);
  make_clear_branch_contributions(o, m);
  make_common_tr_eval(o, m);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
