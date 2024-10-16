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
#include <typeinfo>
/*--------------------------------------------------------------------------*/
class OUT_ANALOG {
public:
  enum mode{
    modePRECALC = 0,
    modeSTATIC = 1,
    modeDYNAMIC = 2,
    modePROBE = 3,
    modeTR_BEGIN = 4,
    modeTR_RESTORE = 5,
    modeTR_ADVANCE = 6,
    modeTR_REGRESS = 7,
    modeTR_REVIEW = 8,
    modeTR_ACCEPT = 9,
    modeNUM = 10
  }_mode;
  Base const* _src{NULL};
  std::string ctx()const {
    char const* names[modeNUM] = { //
      "precalc", "static", "tr_eval", "probe", "tr_begin", "tr_restore",
      "tr_advance", "tr_regress", "tr_review", "tr_accept"
    };
    return names[_mode];
  }

public:
  explicit OUT_ANALOG(mode m, Base const* src=NULL)
    : _mode(m),
      _src(src){}

public:
  bool is_dynamic()const { return _mode==modeDYNAMIC; }
  bool is_static()const { return _mode==modeSTATIC || _mode==modeTR_BEGIN || _mode==modeTR_REVIEW ; } // || ...?
  bool is_precalc()const { return _mode==modePRECALC; }
  bool is_probe()const   { untested(); return _mode==modePROBE; }
  bool is_tr_begin()const  { untested(); return _mode==modeTR_BEGIN; }
  bool is_tr_review()const  { untested(); return _mode==modeTR_REVIEW; }
  bool is_tr_accept()const  { return _mode==modeTR_ACCEPT; }
  bool is_tr_advance()const  { untested(); return _mode==modeTR_ADVANCE; }
public:
  void make_analog_list(std::ostream& o, const Module& m)const;
  void make_construct  (std::ostream& o, AnalogConstruct const& ab)const;
  void make_af         (std::ostream& o, const Analog_Function& f)const;
  void make_load_variables(std::ostream& o, const Module& m)const;
 // void make_store_variables(std::ostream& o, const Variable_List_Collection& P)const;
private:
  void make_load_block_variables(std::ostream& o, const Variable_List_Collection& P)const;
  void make_stmt       (std::ostream& o, Statement const& a)const;
  void make_block      (std::ostream& o, Block const& s)const;
private:
  void make_af_tparam  (std::ostream& o, const Analog_Function& f)const;
  void make_af_args    (std::ostream& o, const Analog_Function& f)const;
  void make_af_body    (std::ostream& o, const Analog_Function& f)const;
  void make_cond       (std::ostream& o, AnalogConditionalStmt const& s)const;
  void make_switch     (std::ostream& o, AnalogSwitchStmt const& s)const;
  void make_for        (std::ostream& o, AnalogForStmt const& s)const;
  void make_while      (std::ostream& o, AnalogWhileStmt const& s)const;
  void make_seq        (std::ostream& o, AnalogSeqStmt const& s)const;
  void make_ctrl       (std::ostream& o, AnalogCtrlBlock const& s)const;
  void make_assignment (std::ostream& o, Assignment const& a)const;
  void make_contrib    (std::ostream& o, Contribution const& C)const;
  void make_evt        (std::ostream& o, AnalogEvtCtlStmt const& s)const;
  void make_loop       (std::ostream& o, AnalogWhileStmt const& s) const;
  void make_system_task(std::ostream& o, System_Task const& s)const;
  void make_task       (std::ostream& o, System_Task const& s)const;
  void make_variable   (std::ostream& o, Token_VAR_REF const& v)const;
  void make_variable   (std::ostream& o, Variable_Decl const& v)const;
private:
  void make_block_variables(std::ostream& o, Variable_Stmt const&)const;
  void make_real_variable  (std::ostream& o, Token_VAR_DECL const&)const;
  void make_seq_block      (std::ostream& o, AnalogSeqBlock const&)const;
private:
  void make_one_local_var(std::ostream& o, Token_VAR_REF const& V)const;
  void make_one_variable_load(std::ostream& o, Token_VAR_REF const& V)const;
  void make_one_variable_store(std::ostream& o, Token_VAR_REF const& V)const;

  void make_cc_expression(std::ostream& o, Expression const& e, bool b=false)const {
    (void)b;
    return ::make_cc_expression(o, e, _mode!=modePRECALC, ctx());
  }
}; // OUT_ANALOG
/*--------------------------------------------------------------------------*/
static void make_int_variable(std::ostream& o, Token_VAR_DECL const& v)
{ untested();
  o__ "/*int_variable*/ int _v_" << v.name() << ";\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_real_variable(std::ostream& o, Token_VAR_DECL const& v) const
{
  // type?
  //
  if(_mode==modeSTATIC){ untested();
    o__ "double _v_" << v.name() << "; // Token_VAR_DECL";
  }else{
    o__ "ddouble _v_" << v.name() << "; // Token_VAR_DECL";

    for(auto const& i : v.deps().ddeps()) { untested();
      o__ " Dep: " << i->code_name() << " lin: " << i.is_linear();
    }
    o << "\n";
    //if(options().fpi()){ untested();
    //}else
    o << ind << "_v_" << v.name() << ".set_all_deps();\n";
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_variable(std::ostream& o, Token_VAR_REF const& v) const
{
  auto r = dynamic_cast<Token_VAR_DECL const*>(&v);
  assert(r);
  if(r->type().is_real()){
    make_real_variable(o, *r);
  }else if(r->type().is_int()){ untested();
    make_int_variable(o, *r);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
// obsolete.?
void OUT_ANALOG::make_variable(std::ostream& o, Variable_Decl const& v) const
{
  if(v.size()){ untested();
    incomplete();
  }else{
  }
  // type?
  //
  if(_mode==modeSTATIC){ untested();
    o__ "double _v_" << v.name() << "; // Variable_Decl";
  }else{
    o__ "ddouble _v_" << v.name() << "; // Variable_Decl";

    for(auto const& i : v.deps().ddeps()) { untested();
      o__ " Dep: " << i->code_name() << " lin: " << i.is_linear();
    }
    o << "\n";
    //if(options().fpi()){ untested();
    //}else
    o << ind << "_v_" << v.name() << ".set_all_deps();\n";
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_block_variables(std::ostream& o,
    Variable_Stmt const& rl) const
{
  o << "// make_block_variables\n";
  for(Variable_Decl const* v : rl){
    if(v->size()){ untested();
      incomplete();
    }else{
    }
    assert(v);
    if(_src && !v->is_used_in(_src)){ untested();
      o << "// omit Var decl?\n";
    }else{
    }

    make_variable(o, *v);
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
    }else if(auto ex = dynamic_cast<Expression_ const*>(what)){
      what = ex->scope();
    }else{ untested();
      unreachable();
      assert(0);
      return false;
    }
  }
  {
    return false;
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_assignment(std::ostream& o, Assignment const& a) const
{
  Expression_ const& e = a.rhs();

  std::string lhsname = a.lhs().code_name();
  std::string name = a.lhs().name();
  o__ "{ // Assignment " << a.type() << " '" << name << "'.\n";

  // wrong place?
//   if(!a.is_used()){ untested();
//     o__ "// not used\n";
//   }else
  {
    indent x;
    make_cc_expression(o, e);
    if(a.is_int()){
      o__ lhsname << " = int(t0); // (*)\n";
    }else if(within_af(&a)){
      o__ lhsname << " = t0; // (1a)\n";
    }else if(!options().optimize_deriv()) { untested();
      o__ lhsname << " = t0; // (*)\n";
      for(auto v : a.data().ddeps()) { untested();
	o__ "// " << a.lhs().code_name() << "[d" << v->code_name() << "] = " << "t0[d" << v->code_name() << "]; // (2a)\n";
	o__ "assert(" << a.lhs().code_name() << "[d" << v->code_name() << "] == " << "t0[d" << v->code_name() << "]); // (2a2)\n";
      }
    }else if(_mode==modePRECALC){
      o__ lhsname << " = t0; // (prec)\n";
    }else if(is_static()){
      o__ lhsname << " = t0.value(); // (s)\n";
    }else if(_mode==modeTR_ADVANCE){
      o__ lhsname << " = t0.value(); // (s)\n";
    }else if(_mode==modeTR_REGRESS){
      o__ lhsname << " = t0.value(); // (s)\n";
    }else if(_mode==modeTR_ACCEPT){
      o__ lhsname << " = t0.value(); // (s)\n";
    }else{
      o__ lhsname << " = t0.value(); // (*)\n";
      // o__ lhsname << ".set_no_deps(); // (42)\n";
#ifdef TRACE_ASSIGN
      o__ "trace1(\"assign\", " << lhsname << ");\n";
#endif

      for(auto v : a.data().ddeps()) {
	assert(v->branch());
	if(v.is_linear()){
	  // TODO incomplete();
	}else if(v.is_quadratic()){ untested();
	  incomplete();
	}else{
	  // TODO incomplete();
	}

	if(a.data().is_linear()) {
//	}else if(v.is_linear()){ incomplete(); // later.
	}else{
	}

	if(v->branch()->is_short()) {
	  o__ "// " << lhsname << "[d" << v->code_name() << "] short\n";
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
void OUT_ANALOG::make_contrib(std::ostream& o, Contribution const& C) const
{
  Expression const& e = C.rhs();

  for(auto v : C.data().ddeps()) {
    trace2("contrib dep", C.name(), v->code_name());
  }

  o__ "{ // Contribution " << C.name() << C.branch_ref() << " lin: " << C.data().is_linear() << "\n";
//  if(!C.is_used()){ untested();
//    o__ "// not used\n";
//  }else
  if(!C.is_pot_contrib() && is_zero(e)){
    // TODO incomplete(); // optimize out?
  }else if(C.branch()->is_short()){
  }else{
    indent x;
    make_cc_expression(o, e);

    char sign = C.reversed()?'-':'+';
    std::string bcn = C.branch_ref().code_name();
    if(!is_dynamic()) {
    }else if(C.branch()->has_pot_source()){
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

    if(C.data().is_linear()) {
      trace1("linear C deps", C.branch()->code_name());
    }else{
    }

    if(is_dynamic()) {
      o__ "d->_value" << bcn << " /* contrib sign: */ " << sign << "= t0.value();\n";
    }else{
     //  o__ "d->_value" << bcn << " " << sign << "= t0;\n";
    }
    if(!is_dynamic()) {
    }else if(C.branch()->has_pot_source()) {
      // incomplete? //
    }else if(C.branch()->has_flow_probe()) {
      // BUG? what does the standard say? cf. mg3_iprobe.
      o__ "d->_flow" << C.branch()->code_name() << " = " <<
	  "d->_value" << C.branch()->code_name() << "; // (8)\n";
    }

    if(is_dynamic()) {
      for(auto v : C.data().ddeps()) {
	if(C.branch() == v->branch()){
	  o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	  o__ "d->_st" << bcn << "[1]"
	    " " << sign << "= " << "t0[d" << v->code_name() << "];\n";
	}else{
	}
      }
      for(auto v : C.data().ddeps()) {
	o__ "// dep " << v->code_name() << "\n";
	assert(v->branch());
	if(C.branch() == v->branch()){
	  o__ "// same2 " << v->code_name() << "\n";
	}else if(v->branch()->is_short()) {
	  o__ "// short: " << v->code_name() << "\n";
#if 1
	}else if(v->is_flow_probe() && v->branch()->has_flow_source()) {
	  // incomplete untested(); mg4_current2,3
	  // copy dep values from source?
	  o__ "// source " << v->code_name() << "\n";
#endif
	}else{
	  o__ "assert(" << "t0[d" << v->code_name() << "] == t0[d" << v->code_name() << "]" << ");\n";
	  // o__ "d->" << C.branch()->state() << "["
	  o__ "d->_st" << bcn << "["
	     << "MOD::" << C.branch()->state() << "_::dep" << v->code_name() << "] "
	     << sign << "= " << "t0[d" << v->code_name() << "]; // (3)\n";
	}
      }
    }else if(is_precalc()) {
      for(auto v : C.data().ddeps()) {
	assert(v->branch());
	if(C.branch() == v->branch()) {
	  o__ "// same2 " << v->code_name() << "\n";
	}else if(v->branch()->is_detached_filter()){
	  o__ "m->" << v->branch()->state() << "[1] = " << sign << "t0[d" << v->code_name() << "]; // (3p)\n";
	}else if(v->branch()->is_filter()){
	  o__ "m->" << v->branch()->state() << "[1] = " << sign << "1; // (3q)\n"; // mfactor hack
	}else if(v->branch()->is_short()) {
	  o__ "// short: " << v->code_name() << "\n";
	}else if(v->branch()->has_element()){
	  o__ "// elt? " << v->code_name() << "\n";
	}else{
	  o__ "//noelt: " << v->code_name() << "\n";
	}
      }
      if(C.data().ddeps().empty()) {
	  o__ "//noddeps..\n";
      }else{
      }
    }else{
    }
  }
  o__ "}\n";
} // make_contrib
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_block(std::ostream& o, Block const& ab) const
{
  if(auto s = dynamic_cast<AnalogSeqBlock const*>(&ab)){
    return make_seq_block(o, *s);
  }else{ untested();
    assert(0);
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_stmt(std::ostream& o, Statement const& ab) const
{
  if(_src && !ab.is_used_in(_src)){
    o << "// omit Statement " << typeid(ab).name() << "\n";
    return;
    o << "#if 0 // omit Statement " << typeid(ab).name() << "\n";
  }else{
  }
#if 0
    if(dynamic_cast<Contribution const*>(&ab)){ untested();
      o << "// omit Contibution..\n";
    }else if(dynamic_cast<AnalogProceduralAssignment const*>(&ab)) { untested();
      o << "// omit Assignment..\n";
    }else if(auto sb = dynamic_cast<AnalogSeqBlock const*>(&ab)){ untested();
      // incomplete();
      make_seq(o, *sb);
    }else{ untested();
      o << "// omit statement1\n";
      o << "//" << st->rdeps_size() << "\n";
    }
#endif

  if(auto fc=dynamic_cast<Contribution const*>(&ab)) {
    make_contrib(o, *fc);
  }else if(auto a=dynamic_cast<AnalogProceduralAssignment const*>(&ab)) {
    make_assignment(o, a->expression());
  }else if(auto assign=dynamic_cast<Assignment const*>(&ab)) { untested();
    // incomplete.
    make_assignment(o, *assign);
  }else if(auto cs=dynamic_cast<AnalogConditionalStmt const*>(&ab)) {
    make_cond(o, *cs);
  }else if(auto ss=dynamic_cast<AnalogSwitchStmt const*>(&ab)) {
    make_switch(o, *ss);
  }else if(auto ww=dynamic_cast<AnalogForStmt const*>(&ab)) {
    make_for(o, *ww);
  }else if(auto aws=dynamic_cast<AnalogWhileStmt const*>(&ab)) {
    make_while(o, *aws);
  }else if(auto ev=dynamic_cast<AnalogEvtCtlStmt const*>(&ab)) {
    make_evt(o, *ev);
    //throw Exception("analogevtctl unsupported");
  }else if(auto ct = dynamic_cast<AnalogCtrlStmt const*>(&ab)){
    make_ctrl(o, ct->body());
  }else if(auto t=dynamic_cast<System_Task const*>(&ab)) {
    make_system_task(o, *t);
  }else if(auto ass=dynamic_cast<AnalogSeqStmt const*>(&ab)) {
    make_seq(o, *ass);
  }else{ untested();
    incomplete();
    assert(false);
  }

  if(_src && !ab.is_used_in(_src)){ untested();
    o << "#endif // omit\n";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_af(std::ostream& o, const Analog_Function& f) const
{
  auto mp = prechecked_cast<Module const*>(f.owner());
  assert(mp);
  auto& m = *mp;
  o << "template<";
  make_af_tparam(o, f);
  o << "class X>\n";
  o << "COMMON_" << m.identifier() << "::";
  o << "ddouble COMMON_" << m.identifier() << "::" << f.code_name() << "(\n";
  o << "            ";
  make_af_args(o, f);
  o << ") const\n{\n";
  {
    indent x;
    o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
    o__ "(void)pc;\n";
    make_af_body(o, f);
  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_af_tparam(std::ostream& o, const Analog_Function& f) const
{
  int n = 0;
  for (Base const* x : f.header()){
    auto coll =  prechecked_cast<AF_Arg_List const*>( x);
    assert(coll);
    for(auto i : *coll){
      (void) i;
      if(coll->is_output()){
	o << "class D"<<++n<<", ";
      }else{
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_af_args(std::ostream& o, const Analog_Function& f) const
{
  std::string sep = "";
  std::string qual = "";
  int n = 0;
  for (Base const* x : f.header()){
    auto coll =  prechecked_cast<AF_Arg_List const*>( x);
    assert(coll);
    if(coll->is_output()){
      qual = "&";
    }else{
      qual = "";
    }
    for(auto i : *coll){
      if(coll->is_output()){
	o << sep << "D" << ++n << "& ";
      }else{
	qual = "";
	o << sep << "ddouble " << qual;
      }
      //o << " af_arg_" << i->identifier();
      o << " _v_" << i->name(); // code_name?
      sep = ", ";
    }
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_af_body(std::ostream& o, const Analog_Function& f) const
{
  auto vv = prechecked_cast<Token_VAR_REF const*>( f.variable() );
  assert(vv);
  std::string me = vv->code_name();
  o__ "ddouble " << me << "(0.);\n"; // type??
  o << "// header w/o dir\n";
  for(auto const& v : f.header().var_refs()) {
    if(auto ard = dynamic_cast<Token_VAR_DECL const*>(v.second)){
      make_variable(o, *ard);
    }else{
      trace1("non-decl vr", v.first);
      // ignore ARGS
    }
  }
  o << "// /header w/o dir\n";

  for(Base const* s : f.body().variables_()) {
     if(auto ard = dynamic_cast<Variable_Stmt const*>(s)){
       make_block_variables(o, *ard);
     }else{ untested();
     }
  }
  for(Base const* i : f.body()) {
    if(dynamic_cast<Variable_Stmt const*>(i)){ untested();
      unreachable();
      // make_stmt(o, *i);
     // make_block_variables(o, *ard);
    }else if(auto st = dynamic_cast<AnalogStmt const*>(i)){
      make_stmt(o, *st);
    }else{ untested();
      unreachable();
    }
  }
  o__ "return " << me << ";\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_af(std::ostream& o, const Analog_Function& f)
{
  OUT_ANALOG oo(OUT_ANALOG::modeDYNAMIC);
  oo.make_af(o, f);
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_system_task(std::ostream& o, System_Task const& s) const
{
  o__ "{\n";
  make_cc_expression(o, s.expression());
  o << "\n";
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
// TODO: use FUNCTION_?
void make_cc_event_cond(std::ostream& o, Expression const& e)
{ untested();
  typedef Expression::const_iterator const_iterator;
  // TODO: var stack.
  o__ "bool evt = false\n;";
  o__ "{\n";
  for (const_iterator i = e.begin(); i != e.end(); ++i) { untested();
    if((*i)->name()=="initial_step"){ untested();
      o__ "evt = _sim->_phase == p_INIT_DC;\n";
//    o__ "evt = _sim->vams_initial_step();\n"; // TODO
    }else if((*i)->name()=="initial_model"){ untested();
      std::cerr << "WARNING: ADMS style keyword encountered\n";
      o__ "evt = _sim->is_initial_step();\n";
    }else{ untested();
      incomplete();
      o << "--> " << (*i)->name() << " <--\n";
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_evt(std::ostream& o, AnalogEvtCtlStmt const& s) const
{
  o__ "{ // AnalogEvtCtlStmt\n";
  {
    indent x;
    make_cc_expression(o, s.cond());
    o__ "if (t0 /*?*/) {\n";
    {
      indent y;
      make_ctrl(o, s.code());
    }
    o__ "}else{\n";
    o__ "}\n";

    o << "\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_loop(std::ostream& o, AnalogWhileStmt const& s) const
{
  o__ "while(true) {\n";
  {
    indent x;
    make_cc_expression(o, s.conditional());
    o__ "if (t0) {\n";
    if(s.has_body()) {
      indent y;
      if(auto bb = dynamic_cast<AnalogCtrlBlock const*>(&s.body())){
	make_ctrl(o, *bb);
      }else{ untested();
	assert(0);
      }
    }else{ untested();
    }

    if(s.has_tail()){
      if(auto bb = dynamic_cast<Assignment const*>(&s.tail())){
	make_assignment(o, *bb);
      }else{ untested();
	assert(0);
      }
    }else{
    }

    o__ "}else{\n";
    o____ "break;\n";
    o__ "}\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_while(std::ostream& o, AnalogWhileStmt const& s) const
{
  make_loop(o, s);
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_for(std::ostream& o, AnalogForStmt const& s) const
{
  if(s.has_init()){
    make_assignment(o, s.init());
  }else{ untested();
  }
  make_loop(o, s);
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_cond(std::ostream& o, AnalogConditionalStmt const& s) const
{
  o__ "{\n";
  if(s.conditional().is_true()) {
    if(s.true_part()) {
      indent y;
      make_ctrl(o, s.true_part());
    }else{ untested();
    }
  }else if(s.conditional().is_false()){
    if(s.false_part()) {
      indent y;
      make_ctrl(o, s.false_part());
    }else{
    }
  }else{
    indent x;
    make_cc_expression(o, s.conditional());
    o__ "if (t0) {\n";
    if(s.true_part()) {
      indent y;
      make_ctrl(o, s.true_part());
    }else{
    }
    o__ "}";
    if(s.false_part()) {
      o << "else {\n";
      {
	indent y;
	make_ctrl(o, s.false_part());
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
    assert(e);
    make_cc_expression(o, *e, false);

    o__ "if(t0 == s){\n";
    o____ "cond = true;\n";
    o__ "}else{\n";
    paren += "}";
  }

  o << paren << "\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_switch(std::ostream& o, AnalogSwitchStmt const& s) const
{
  // TODO: indent properly
  o__ "{\n";
  {
    indent x;
    o__ "double s;\n";
    o__ "{\n";
    {
      indent y;
      make_cc_expression(o, s.control(), false);
      o__ "s = t0;\n";
    }
    o__ "}\n";
    std::string paren="";

    CaseGen const* def = NULL;
    for(auto c : s.cases()){
      auto i = prechecked_cast<CaseGen*>(c);
      assert(i);
      if(i->is_never()){
      }else if(i->cond_or_null()){
	o << "{\n";

	o << "bool cond = false;\n";
	make_cond_expressions(o, *i->cond_or_null());

	o__ "if (cond) {\n";

//	if(i->code_or_null()){ untested();
	  indent y;
	  make_stmt(o, *i); // ->code_or_null());
//	}else{ untested();
//	}

	o__ "}else";
	paren += "}";

      }else{
	def = &*i;
      }
    }

    o << "{\n";
    if(!def){
    }else{ // } if(def->code_or_null()){ untested();
      indent y;
      make_stmt(o, *def);
//    }else{ untested();
    }
    o<<paren;
    o__ "\n";
    o__ "}\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_ctrl(std::ostream& o, AnalogCtrlBlock const& s) const
{
  make_seq_block(o, s);
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_seq(std::ostream& o, AnalogSeqStmt const& s) const
{
  return make_seq_block(o, s.block());
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_seq_block(std::ostream& o, AnalogSeqBlock const& s) const
{
  if(s.has_identifier()) {
    o__ "{ // : " << s.identifier() << "\n";
  }else{
    o__ "{ // anonymous block\n";
  }
  make_load_block_variables(o, s.variables_());
#if 0
  for(auto i : s.variables_()) { untested();
    assert(s.identifier() != "");

    if(auto st = dynamic_cast<Variable_Stmt const*>(i)){ untested();
      make_block_variables(o, *st);
    }else{ untested();
      unreachable();
    }

  }
#endif
  for(auto i : s.block()) {
    indent x;
    if(auto st = dynamic_cast<Statement const*>(i)){
      make_stmt(o, *st);
    }else if(auto as = dynamic_cast<AnalogSeqBlock const*>(i)){ untested();
      // incomplete(); // later.
      make_seq_block(o, *as);
    }else{ untested();
      incomplete();
      unreachable();
    }
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_construct(std::ostream& o, AnalogConstruct const& ab) const
{
  if(_src && !ab.is_used_in(_src)){ untested();
    o << "// omit construct...\n";
  }else if(ab.block_or_null()){
    make_block(o, *ab.block_or_null());
  }else{ untested();
    unreachable();
    o << ";\n";
  }
}
/*--------------------------------------------------------------------------*/
std::string Branch::state()const
{
  return "_st" + code_name();
}
/*--------------------------------------------------------------------------*/
std::string Branch::state(std::string const& n)const
{
  if(n == ""){
    return state();
  }else{ untested();
    return "_st_br_" + n;
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
std::string Branch::code_name() const
{
  return "_b_" + p()->name() + "_" + n()->name();
}
/*--------------------------------------------------------------------------*/
static void make_set_self_contribution(std::ostream& o, Dep const& d)
{
  // geting here if V(br) <+ .. I(br) or
  //                I(br) <+ .. V(br)
  //    .. what if both?
  Branch const* b = d->branch();
  o__ "trace2(\"" <<  b->state() << "self\", " << b->state() << "[1], "<<  d->code_name() <<");\n";
  o__ "// generic: " << b->is_generic() << "\n";
  bool both = b->has_flow_source() && b->has_pot_source();

  if(both && d->is_pot_probe()) {
    o__ "// self pot\n";
    o__ "if (_pot"<< b->code_name() << "){\n";
    o__ "}else{\n";
    o____ b->state() << "[0] -= " << b->state() << "[1] * " << d->code_name() << "; // (4)\n";
    o__ "}\n";
  }else if(both && d->is_flow_probe()) { untested();
    o__ "// self flow\n";
    o__ "if (_pot"<< b->code_name() << "){\n";
    o____ b->state() << "[0] -= " << b->state() << "[1] * " << d->code_name() << "; // (4)\n";
    o__ "}else{\n";
    o__ "}\n";
  }else{
    o__ "// self\n";
    o__ b->state() << "[0] -= " << b->state() << "[1] * " << d->code_name() << "; // (4)\n";
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_set_state(std::ostream& o, Branch const& b, std::string cn)
{
  o__ "{ // set state " << cn << "\n";
  if (b.deps().ddeps().size()) {
    //    o__ "typedef long double D;\n";
    o__ "long double sp = 0.;\n";
  }else{
  }
  for(auto const& d : b.deps().ddeps()){
    o__ "// " << d->code_name() << " lin: " <<  d.is_linear() << "\n";
    if(d->branch() == &b){
      // move make_set_self_contribution here?
    }else if(d->branch()->is_short()) {
    }else if(d->is_pot_probe()){
      o__ "sp += (long double)(" << b.state(cn) << "["
	<< b.state() << "_::dep" << d->code_name()
	<< "] * "<< d->code_name() << ");\n";
    }else if(d->is_flow_probe()){
      o__ "sp += (long double)(" << b.state(cn) << "["
	<< b.state() << "_::dep" << d->code_name()
	<< "] * "<< d->code_name() << "); // (5)\n";
      o__ b.state(cn) << "["
	<< b.state() << "_::dep" << d->code_name()
	<<"] *= " << d->branch()->code_name() <<"->_loss0;\n"; // maybe let src decide?
    }else{ untested();
      o__ "// bogus probe " << b.state() << " : " << d->code_name() << "\n";
    }
    if(d->branch() == &b){
    }else if(d->is_flow_probe()){
      // todo?
    }
  }
  if (b.deps().ddeps().size()) {
    o__ b.state(cn) << "[0] = double(" << b.state(cn) << "[0] - sp);\n";
  }else{
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_set_one_branch_contribution(std::ostream& o, const Branch& br)
{
  Branch const* b = &br;
  assert(!br.is_short());
  o__ "assert(_value" << b->code_name() << " == _value" << b->code_name() << ");\n";

  o__ b->state() << "[0] = _value" << b->code_name() << ";\n";

  if(b->deps().is_linear()){
    // TODO incomplete();
    // o__ b->state() << "[0] = 0.;\n";
  }else{
  }

  for(auto const& d : b->deps().ddeps()){
    if(d->branch() == b){
      o__ "// same1 " << d->code_name() << "\n";
      if(b->has_pot_source() && b->has_flow_probe()){
	if(br.num_states()<=2){
	}else{ untested();
	  incomplete(); // the other ones??
	}
      }else{
      }

      make_set_self_contribution(o, d);
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
  o__ "// sources...\n";
  {
    indent ii;
    make_cc_set_state(o, *b, "");
  }

}
/*--------------------------------------------------------------------------*/
// some filters are not reached in do_tr. set output is zero.
static void make_cc_zero_filter_readout(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::zero_filter_readout()\n{\n";
  for(auto x : m.circuit()->branches()){
    Branch const* b = x;
    assert(b);
    if(!b->is_filter()){
    }else{
      if(b->is_short()){
	o__ "// filter " << b->code_name() << " short branch\n";
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
static void make_cc_set_branch_contributions(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::set_branch_contributions()\n{\n";
  for(auto i : m.circuit()->branches()){
    Branch const* b = i;

    if(b->is_short()) {
    }else if(b->has_flow_source() || b->has_pot_source()) {
      indent ii;
      o__ "{ // 1\n";
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
      o__ "if(!" << b->code_name() << "){\n";
      o__ "}else if(_pot" << b->code_name() << "){\n";
      o____ b->code_name() << "->_loss0 = 1./OPT::shortckt;\n";
      o__ "}else{\n";
      o____ b->code_name() << "->_loss0 = 0.; // AA\n";
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
static void make_one_variable_proxy(std::ostream& o, Token_VAR_REF const& V)
{
  o__ "class _V_" << V.name() << " : public ddouble {\n";
  o____ "MOD__ * const _m;\n";
  o__ "public:\n";
  o____ "typedef ddouble base;\n";
  o____ "typedef va::ddouble_tag base_tag;\n";
  o____ "_V_" << V.name() << "(ddouble const& p) : ddouble(p), _m(NULL) { itested(); }\n";
  o____ "_V_" << V.name() << "(double const& p) : ddouble(p), _m(NULL) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(PARAMETER<double> const& p) : ddouble(p), _m(NULL) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(_V_" << V.name() << " const& p) : ddouble(p), _m(NULL) {}\n";
  o____ "explicit _V_" << V.name() << "() : ddouble(), _m(NULL) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(MOD__* m) : "
    << "ddouble(m->" << V.long_code_name() << "), _m(m) {}\n";
  o____ "~_V_" << V.name() << "() {\n";
  o______ "if(_m){\n";
  o________ "_m->" << V.long_code_name() << " = value();\n";
  o______ "}else{\n";
  o______ "}\n";
  o____ "}\n";
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
void OUT_ANALOG::make_one_local_var(std::ostream& , const Token_VAR_REF& ) const
{ untested();
  incomplete();
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_one_variable_load(std::ostream& o,
                                        const Token_VAR_REF& V) const
{
  if(!is_dynamic() || is_tr_accept() ) {
    if(V.type().is_int()) {
      o__ "int";
    }else if(V.type().is_real()) {
      if(is_precalc()) {
	o__ "ddouble"; // precalc hacks derivatives a bit.
      }else{
	o__ "double";
      }
    }else{ untested();
      unreachable();
    }

    if(is_precalc() || is_tr_accept()) {
      o << " " << V.code_name() << "(m->" << V.long_code_name() << "); // precalc 1068\n";
    }else{
      o << "& " << V.code_name() << "(m->" << V.long_code_name() << "); // (1068)\n";
    }
    o__ "(void) " << V.code_name() << ";\n";
  }else if(V.type().is_int()) {
    o__ "int& " << V.code_name() << "(d->" << V.long_code_name() << ");\n";
  }else if(V.type().is_real()) {
    if(V.deps().ddeps().size() == 0){
      if(dynamic_cast<Module const*>(V.scope())) {
	o__ "double& " << V.code_name() << "(d->" << V.long_code_name() << "); // (823)\n";
      }else{
	o__ "// tmp block proxy (823b)\n";
	make_one_variable_proxy(o, V);
	o << V.code_name() << "(d);\n";
      }
    }else if(options().optimize_deriv()) {
      make_one_variable_proxy(o, V);
      o << V.code_name() << "(d);\n";
    }else{itested();
      o__ "ddouble " << V.code_name() << "(d->" << V.code_name() << "); // (828)\n";
    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
// no longer needed.
void OUT_ANALOG::make_one_variable_store(std::ostream& o, Token_VAR_REF const& V) const
{ untested();
  if(!V.type().is_real()) { untested();
  }else if(is_precalc()) { untested();
    o__ "// d->" << V.code_name() << " = " << V.code_name() << ";\n";
  }else if(is_tr_review()) { untested();
  }else if(V.deps().ddeps().size() == 0){ untested();
    // it's a reference.
  }else if(options().optimize_deriv()) { untested();
    // use destructor
  }else{untested();
    o__ "d->" << V.code_name() << " = " << V.code_name() << ".value();\n";
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_load_block_variables(std::ostream& o, const
    Variable_List_Collection& P) const
{
  if(is_dynamic()) {
    // not sure if these are actually unused. GCC bug?
    o << "#pragma GCC diagnostic push\n";
    o << "#pragma GCC diagnostic ignored \"-Wunused-local-typedefs\"\n";
  }else{
  }

  for (auto q = P.begin(); q != P.end(); ++q) {
    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      Variable_Decl const* V = *p;
      assert(V);

      if(V->is_state_var()){
	make_one_variable_load(o, V->token());
      }else{ untested();
	make_one_local_var(o, V->token());
	incomplete();
      }
    }
  }

  if(is_dynamic()) {
    o << "#pragma GCC diagnostic pop\n";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_load_variables(std::ostream& o, const Module& m) const
{
  make_load_block_variables(o, m.variables());
}
/*--------------------------------------------------------------------------*/
#if 0
static void make_cc_ac_begin(std::ostream& o, const Module& m)
{ untested();
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() << 
    "::ac_begin(MOD_" << m.identifier() << "* d) const\n{\n";
  o << "incomplete();\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
#endif
/*--------------------------------------------------------------------------*/
void OUT_ANALOG::make_analog_list(std::ostream& o, const Module& m) const
{
  if(is_precalc()){
    o__ "MOD_" << m.identifier() << " const* p = m;\n";
    o__ "MOD_" << m.identifier() << "* d = m;\n";
  }else if(is_dynamic()){
    o__ "MOD_" << m.identifier() << " const* p = d;\n";
  }else{
    o__ "MOD_" << m.identifier() << " const* p = m;\n";
    o__ "MOD_" << m.identifier() << "* d = m;\n";
  }
  o__ "(void)p;\n";
  o__ "assert(p);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";
  o__ "(void)d;\n";

  for(auto const& bb : analog_list(m)){
    assert(bb);
    if(dynamic_cast<AnalogStmt const*>(bb)){ untested();
    }else{
    }
    if(_src){
    }else{
    }
    trace1("analoglist", bb->is_used_in(NULL));
    if(_src && !bb->is_used_in(_src)){ untested();
      o__ "// omit2 " << typeid(*bb).name() << "\n";
    }else if(auto ab = dynamic_cast<AnalogConstruct const*>(bb)){
      o__ "{ //\n";
      {
	indent a;
	make_construct(o, *ab);
      }
      o__ "}\n";
    }else{ untested();
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr_advance(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_advance_analog(MOD_" << m.identifier() << "* m) const\n{\n";
  // o << "eval_t mode = m_TR_ADVANCE;\n";
  // o << "(void)mode;\n";

  OUT_ANALOG oo(OUT_ANALOG::modeTR_ADVANCE, &tr_advance_tag);
  oo.make_load_variables(o, m);
  oo.make_analog_list(o, m);
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr_regress(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_regress_analog(MOD_" << m.identifier() << "* m) const\n{\n";

  OUT_ANALOG oo(OUT_ANALOG::modeTR_REGRESS, &tr_advance_tag);
  oo.make_load_variables(o, m);
  oo.make_analog_list(o, m);
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr(std::ostream& o, const Module& m, OUT_ANALOG::mode mode, Base const* dep)
{
  OUT_ANALOG oo(mode, dep);
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::" << oo.ctx() << "_analog(MOD_" << m.identifier() << "* m) const\n{\n";
 // o__ "trace1(\"" << m.identifier() <<"::tr_begin_analog\", d);\n";
 o__ "trace1(\"" << m.identifier() <<"::tr_"<<oo.ctx()<<"_analog\", m->long_label());\n";

  oo.make_load_variables(o, m);
  oo.make_analog_list(o, m);
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr_eval(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_eval_analog(MOD_" << m.identifier() << "* d) const\n{\n";
  o__ "trace1(\"" << m.identifier() <<"::tr_eval_analog\", d);\n";
  o__ "trace1(\"" << m.identifier() <<"::tr_eval_analog\", d->long_label());\n";

  OUT_ANALOG oo(OUT_ANALOG::modeDYNAMIC);

  oo.make_load_variables(o, m);
  oo.make_analog_list(o, m);
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr_review(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_review_analog(MOD_" << m.identifier() << "* m) const\n{\n";
  o__ "trace1(\"review analog1\", m->_time_by._event);\n";
//  o << "eval_t mode = m_TR_REVIEW;\n";

  OUT_ANALOG oo(OUT_ANALOG::modeTR_REVIEW, &tr_review_tag);
  oo.make_load_variables(o, m);
  oo.make_analog_list(o, m);

  o__ "trace1(\"review analog2\", m->_time_by._event);\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr_accept(std::ostream& o, const Module& m)
{
  // o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_accept_analog(MOD_" << m.identifier() << "* m) const\n{\n";

  OUT_ANALOG oo(OUT_ANALOG::modeTR_ACCEPT, &tr_accept_tag);
  oo.make_load_variables(o, m);
  oo.make_analog_list(o, m);
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_precalc(std::ostream& o, const Module& m)
{
  o << "inline void COMMON_" << m.identifier() << 
    "::precalc_analog(MOD_" << m.identifier() << "* m) const\n{\n";
  o << "//OUT_ANALOG precalc\n";

  OUT_ANALOG oo(OUT_ANALOG::modePRECALC);
  oo.make_load_variables(o, m);
  oo.make_analog_list(o, m);
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_clear_branch_contributions(std::ostream& o, const Module& m)
{
  o << "inline void MOD_" << m.identifier() << "::clear_branch_contributions()\n{\n";
  for(auto x : m.circuit()->branches()){
    assert(x);
    if(x->has_element()){
      if(x->has_pot_source()){
	o____ "_pot" << x->code_name() << " = false;\n";
      }else{
      }
      o____ "_value" << x->code_name() << " = 0.;\n";
      o____ "std::fill_n(_st" << x->code_name() << "+1, " << x->num_states()-1 << ", 0.);\n";
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
  o << "typedef MOD_" << m.identifier() << " MOD__;\n"; // here?
  make_cc_zero_filter_readout(o, m);
  make_cc_set_branch_contributions(o, m);
  make_clear_branch_contributions(o, m);
//  make_cc_ac_begin(o, m);
  make_cc_common_tr_eval(o, m);
  make_cc_common_precalc(o, m);

  // assert(m.has_analog_block());
  // assert(m.has_analog_stuff()); // in always blocks..
  if(m.has_tr_begin_analog()){
    make_cc_common_tr(o, m, OUT_ANALOG::modeTR_BEGIN, &tr_begin_tag);
  }else{
  }
  if(m.has_tr_restore_analog()){
    make_cc_common_tr(o, m, OUT_ANALOG::modeTR_RESTORE, &tr_restore_tag);
  }else{
  }
  if(m.has_tr_review()){
    make_cc_common_tr_review(o, m);
  }else{
  }
  if(m.has_tr_accept()){
    make_cc_common_tr_accept(o, m);
  }else{
  }
  if(m.has_tr_advance()){
    make_cc_common_tr_advance(o, m);
    make_cc_common_tr_regress(o, m);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void make_node_ref(std::ostream& o, const Node& n, bool used=true);
void make_cc_branch_ctrl(std::ostream& o, Branch const* br)
{
  for(auto i : br->ddeps()){
    Branch const* bb = i->branch();
    if(bb->is_short()){
      // here: skip filter dependency.
    }else if(bb == br){
    }else if(i->is_pot_probe()){
      assert(i->branch());
      o << ", ";
      make_node_ref(o, *i->branch()->p());
      o << ", ";
      make_node_ref(o, *i->branch()->n());
    }else if(i->is_flow_probe()){
    }else{ untested();
      o << "/* nothing " << i->code_name() << " */";
    }
  }
}
/*--------------------------------------------------------------------------*/
void make_cc_current_ports(std::ostream& o, Branch const* br, Element_2 const& e)
{
  // set_current ports.
  int kk = 1;
  for(auto i : br->ddeps()){
    if(!i->is_flow_probe()){
    }else if(i->branch() == br){
      // self control is current
      o______ e.code_name() << "->set_current_port_by_index(0,\"\");\n";
    }else if(i->branch()){
      if(i->branch()->is_short()){
      }else{
	o______ e.code_name() << "->set_current_port_by_index( "<< kk << ", \"" << i->branch()->code_name() << "\");\n";
	++kk;
      }
    }else{ untested();
      untested();
    }
  }
}
/*--------------------------------------------------------------------------*/
std::string Probe::code_name() const
{
  if ( _type == t_flow ){
    return "_flow" + _br->code_name(); // BUG. named_branch.
  }else if (_type == t_pot){
    return "_potential" + _br->code_name();
  }else{
    return("unreachable_probe"); // trace.
  }
}
/*--------------------------------------------------------------------------*/
std::string Branch_Ref::code_name_() const
{
  if(has_name()){
    return "_br_" + name();
  }else if(is_reversed()){
    return "_b_" + _br->n()->name() + "_" + _br->p()->name();
  }else{
    return "_b_" + _br->p()->name() + "_" + _br->n()->name();
  }
}
/*--------------------------------------------------------------------------*/
std::string Probe::code_name_() const
{
  std::string cn = _br.code_name_();

  std::string xsname;
  if (is_flow_probe()) {
    xsname = "_flow";
  }else if (is_pot_probe()) {
    xsname = "_potential";
  }else{ untested();
    unreachable();
    xsname = "unreachable_probe";
  }

  return xsname + cn;
}
/*--------------------------------------------------------------------------*/
void Probe::make_cc_common(std::ostream&) const{ untested();}
void Probe::make_cc_dev(std::ostream& o) const
{
  o__ "ddouble xs" << code_name_() << "() const { // " << label() << "\n";
  o____ "ddouble t;\n";
  std::string bcn = _br->code_name();

  if(_br->is_short()){
    // why?
    o______ "t = 0.;\n";
  }else if(_br->is_source() && is_flow_probe()) {
    o____ "if(" << _br->code_name() << ") {\n";
    if(_br->has_pot_source()) {
      o______ "t = " << bcn << "->tr_amps();\n";
      // o__ "if (!_pot" << bcn << "){\n";
      // o______ "t = " << "_value" << bcn << ";\n";
      // o__ "}else{ untested();\n";
      // o__ "}\n";
    }else{
      o______ "// no pot src\n";
      o______ "t = " << "_value" << bcn << ";\n";
    }
    o____ "}else{\n";
    o______ "t = 0.;\n";
    o____ "}\n";
    o____ "t[d" << code_name() << "] = 1;\n";
  }else if(is_pot_probe()) {
    if(_br->is_short()){ untested();
      o______ "t = 0.;\n";
    }else if (auto nn = dynamic_cast<Named_Branch const*>(&*_br)) {
      // assert(!_br.is_reversed());
      std::string pn = nn->base()->code_name();
      o____ "t = _potential" << pn << "; // named\n";
      o____ "t[d_potential" << pn << "] = 1; // named\n"; // sign?
      // o____ "t = -t\n;";
      if(nn->is_reversed()) {
      }else{
      }
    }else{
      o____ "t = " << code_name() << "; // unnamed\n";
      o____ "t[d" << code_name() << "] = 1; // unnamed\n"; // sign?
    }
  }else if(_br->is_short()) { untested();
      o______ "// short\n";
      o______ "t = 0.;\n";
  }else if(is_flow_probe()) {
      o______ "// flow probe\n";
      o______ "t = " << code_name() << ";\n";
      o____ "t[d" << code_name() << "] = 1;\n";  // BUG?
  }else{ untested();
      incomplete(); //?
      o______ "// other\n";
  }

  if(_br.is_reversed()) {
    o____ "return -t;\n";
  }else{
    o____ "return t;\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
// TODO. obsolete.
void make_cc_analog_list(std::ostream& o, const Module& m, Branch const*
    src=NULL)
{
  OUT_ANALOG oo(OUT_ANALOG::modeDYNAMIC, src);
  oo.make_analog_list(o, m);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
