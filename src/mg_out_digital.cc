/*                                      -*- C++ -*-
 * Copyright (C) 2026 Felix Salfelder
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
#include "mg_out_code.h"
#include "mg_digital.h"
#include "mg_options.h"
#include "mg_assign.h"
#include <typeinfo>
/*--------------------------------------------------------------------------*/
class OUT_DIGITAL : OUT_CODE {
public:
  enum mode{
    modePRECALC = 0,
    modeTR_EVAL = 1,
    modeTR_INITIAL = 2,
    modeTR_BEGIN = 3,
    modeTR_RESTORE = 4,
    modeTR_ADVANCE = 5,
    modeTR_REGRESS = 6,
    modeTR_REVIEW = 7,
    modeTR_ACCEPT = 8,
    modeFINAL = 9,
    modeNUM = 10
  }_mode;
  Base const* _src{nullptr};
  std::string ctx()const override {
    char const* names[modeNUM] = { //
      "precalc", "tr_eval", "tr_initial", "tr_begin", "tr_restore",
      "tr_advance", "tr_regress", "tr_review", "tr_accept", "finish"
    };
    return names[_mode];
  }

public:
  explicit OUT_DIGITAL(mode m, Base const* src=nullptr)
    : _mode(m),
      _src(src){}

public:
  bool is_precalc()const override { return _mode==modePRECALC; }
  bool is_tr_eval()const override { untested(); return _mode==modeTR_EVAL; }
  bool is_tr_restore()const override { return _mode==modeTR_RESTORE; }
  bool is_tr_accept()const override { return _mode==modeTR_ACCEPT; }
  bool is_tr_regress()const override { return _mode==modeTR_REGRESS; }
  bool is_tr_initial()const override { return _mode==modeTR_INITIAL; }
  bool is_tr_begin()const  { return _mode==modeTR_BEGIN; }
  bool is_tr_review()const  { untested(); return _mode==modeTR_REVIEW; }
  bool is_tr_advance()const  { return _mode==modeTR_ADVANCE; }
  bool is_final()const  { untested(); return _mode==modeFINAL; }
public:
  void make_list(std::ostream& o, const Module& m)const;
  void make_construct  (std::ostream& o, AlwaysConstruct const& ab)const;
  void make_load_variables(std::ostream& o, const Module& m)const;
 // void make_store_variables(std::ostream& o, const Variable_List_Collection& P)const;
private:
  void make_load_block_variables(std::ostream& o, const Variable_List_Collection& P)const;
  void make_stmt       (std::ostream& o, Statement const& a)const;
  void make_block      (std::ostream& o, Block const& s)const;
private:
//  void make_initial    (std::ostream& o, InitialStmt const& s)const;
//  void make_cond       (std::ostream& o, ConditionalStmt const& s)const;
//  void make_switch     (std::ostream& o, SwitchStmt const& s)const;
//  void make_for        (std::ostream& o, ForStmt const& s)const;
//  void make_while      (std::ostream& o, WhileStmt const& s)const;
//  void make_seq        (std::ostream& o, SeqStmt const& s)const;
  //void make_ctrl       (std::ostream& o, SeqBlock const& s)const override;
  void make_assignment (std::ostream& o, Assignment const& a)const override;
  void make_evt        (std::ostream& o, DigitalEvtCtlStmt const& s)const;
  void make_loop       (std::ostream& o, WhileStmt const& s) const;
  void make_task       (std::ostream& o, System_Task const& s)const;
  void make_variable   (std::ostream& o, Token_VAR_REF const& v)const;
  void make_variable   (std::ostream& o, Variable_Decl const& v)const;
private:
  void make_node_refs(std::ostream& o, const Module& m)const;
  void make_block_variables(std::ostream& o, Variable_Stmt const&)const;
  void make_real_variable  (std::ostream& o, Token_VAR_DECL const&)const;
  void make_seq_block      (std::ostream& o, SeqBlock const&)const override;
private:
  std::string make_cc_expression(std::ostream& o, Expression const& e, bool=false)const {
    return ::make_cc_expression(o, e, _mode!=modePRECALC, ctx());
  }
}; // OUT_DIGITAL
/*--------------------------------------------------------------------------*/
static void make_one_node_load(std::ostream& o, Node const& V)
{
  if(V.is_used()) {
    assert(V.is_discrete());
    o__ "const va::LNR _v_" << V.code_name() <<
      "(m->n_(MOD__::" << V.code_name() << ")); // load\n";
  }else{
    o__ "if(m->n_(MOD__::" << V.code_name() << ").n_() == &ground_node){\n";
    o____ "incomplete();\n";
    o__ "}\n";
    o__ "const va::LNR _v_" << V.code_name() <<
      "(m->n_(MOD__::" << V.code_name() << ")); // load. unused?\n";
  }
}
/*--------------------------------------------------------------------------*/
void make_tr_advance_digital(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_advance_digital(MOD_" << m.identifier() << "* m) const\n{\n";

  for(auto n : m.circuit()->nodes()){
    if(n->is_reg()){
      assert(n->is_discrete());
      o__ "{\n";
      o____ "node_l& nl = reinterpret_cast<node_l&>(m->n_(MOD::n_" << n->name() << "));\n";
      o____ "if (nl->in_transit()) {\n";
      o______ "m->q_eval();\n";
      o______ "if (CKT_BASE::_sim->_time0 >= nl->final_time()) {\n";
      o________ "nl->propagate();\n";
      o______ "}else{\n";
      o______ "}\n";
      o____ "}\n";
      o__ "}\n\n";
    }else{
    }
#if 0
    if(n->is_discrete()) {
      done in make_load_variables
      make_one_node_load(o, *n);
    }else{
      o__ "// not discrete " << n->name() << "\n";
      assert(n->is_ground() || n->discipline() || n->short_to());
    }
#endif
  }

  OUT_DIGITAL oo(OUT_DIGITAL::modeTR_ADVANCE, &tr_advance_tag);
  o__ "// load variables\n";
  oo.make_load_variables(o, m);
  o__ "// /load variables\n";
  oo.make_list(o, m);
  o << "} // tr_advance_digital\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_node_loads(std::ostream& o, const Module& m)
{
  for(Node const* n : m.circuit()->nodes()){
    assert(n);
    if(n->is_reg()){
      assert(!n->is_ground());
      make_one_node_load(o, *n);
    }else if(n->is_discrete()) {
      assert(!n->is_ground());
      make_one_node_load(o, *n);
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
static void make_one_node_proxy(std::ostream& o, Node const& V)
{
  o__ "va::LNR _v_" << V.code_name() <<
      "(m->n_(MOD__::" << V.code_name() << ")); // proxy\n";
}
/*--------------------------------------------------------------------------*/
void make_tr_accept_digital(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_accept_digital(MOD_" << m.identifier() << "* m) const\n{\n";
  // o << "eval_t mode = m_TR_ADVANCE;\n";
  // o << "(void)mode;\n";

  OUT_DIGITAL oo(OUT_DIGITAL::modeTR_ACCEPT, &tr_accept_tag);
  oo.make_load_variables(o, m);
  oo.make_list(o, m);

  for(auto const& n : m.circuit()->nodes()){
    if(n->is_reg()){
      o__ "_v_n_" << n->name() << ".accept();\n";
    }else{
    }
  }

  o << "} // tr_accept_digital\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_tr_regress_digital(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_regress_digital(MOD_" << m.identifier() << "* m) const\n{\n";

#if 1 // revisit sweep ..
  o__ "if(_sim->_phase == p_RESTORE) {untested();\n";
  o____ "return;\n";
  o__ "}else{\n";
  o__ "}\n";
#endif

  for(auto n : m.circuit()->nodes()){
    if(n->is_reg()){
      o__ "{\n";
      o____ "m->q_eval();\n";
      o____ "node_l& nl = reinterpret_cast<node_l&>(m->n_(MOD::n_" << n->name() << "));\n";
      o____ "if (nl->last_change_time() > m->_sim->_time0) {\n";
      o______ "nl->unpropagate();\n";
      o______ "assert(_sim->_time0 < nl->final_time());\n";
      o____ "}else if (m->_sim->_time0 >= nl->final_time()) { untested();\n";
      o______ "nl->propagate();\n";
      o____ "}else{\n";
      o____ "}\n";
      o__ "}\n\n";
    }else{
    }
  }

  OUT_DIGITAL oo(OUT_DIGITAL::modeTR_REGRESS, &tr_advance_tag);
  oo.make_load_variables(o, m);
//   oo.make_list(o, m);
  o << "} // tr_regress_digital\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr(std::ostream& o, const Module& m, OUT_DIGITAL::mode mode, Base const* dep)
{
  OUT_DIGITAL oo(mode, dep);
//  o << "typedef double ddouble;\n"; // wreal?
  o << "inline void COMMON_" << m.identifier() <<
    "::" << oo.ctx() << "_digital(MOD_" << m.identifier() << "* m)";
  if(oo.is_tr_initial()){
  }else{
    o << "const";
  }
  o<< "\n{\n";
 // o__ "trace1(\"" << m.identifier() <<"::tr_begin_analog\", d);\n";
 o__ "trace1(\"" << m.identifier() <<"::"<<oo.ctx()<<"_digital\", m->long_label());\n";

  oo.make_load_variables(o, m);
  oo.make_list(o, m);

  // yikes. state is shared.
//  if(oo.is_tr_initial()){
//    o__ "m->_v_1 = m->_v_;\n";
//  }else if(oo.is_tr_begin()) {
//    o__ "// m->_v_1 = m->_v_;\n";
//  }else{
//  }
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr_eval(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_eval_digital(MOD_" << m.identifier() << "* m) const\n{\n";
  o__ "trace1(\"" << m.identifier() <<"::tr_eval_digital\", m->long_label());\n";

  OUT_DIGITAL oo(OUT_DIGITAL::modeTR_EVAL);

  oo.make_load_variables(o, m);
//  oo.make_list(o, m);
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_common_tr_review(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << "::ddouble ddouble;\n";
  o << "inline void COMMON_" << m.identifier() <<
    "::tr_review_digital(MOD_" << m.identifier() << "* m) const\n{\n";

  OUT_DIGITAL oo(OUT_DIGITAL::modeTR_REVIEW, &tr_review_tag);
  oo.make_load_variables(o, m);
  oo.make_list(o, m);

  o__ "trace1(\"review digital\", m->_time_by.event());\n";
  o << "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_digital(std::ostream& o, const Module& m)
{
  o << "typedef MOD_" << m.identifier() << " MOD__;\n"; // here?
//  make_cc_ac_begin(o, m);
  make_cc_common_tr_eval(o, m);
  make_cc_common_tr_review(o, m);
  // make_cc_common_precalc(o, m);
  if(m.has_tr_begin_digital()){
    o__ "// tr_initial ...\n";
    make_cc_common_tr(o, m, OUT_DIGITAL::modeTR_INITIAL, &tr_begin_tag);
    o__ "// tr_begin ...\n";
    make_cc_common_tr(o, m, OUT_DIGITAL::modeTR_BEGIN, &tr_begin_tag);
    // o__ "// assign..\n";
    // make_common_init_assign(o, m);
  }else{
    o__ "// !has_tr_begin_digital\n";
  }
}
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_block(std::ostream& o, Block const& ab) const
{
  if(auto s = dynamic_cast<DigitalSeqBlock const*>(&ab)){
    return make_seq_block(o, *s);
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_seq_block(std::ostream& o, SeqBlock const& s) const
{
  if(s.has_identifier()) { untested();
    o__ "{ // : " << s.identifier() << "\n";
  }else{
    o__ "{ // anonymous block\n";
  }
//  make_load_block_variables(o, s.variables_());

  auto S = prechecked_cast<DigitalSeqBlock const*>(&s);
  assert(S);
  for(auto i : S->block()) {
    indent x;
    auto st = prechecked_cast<Statement const*>(i);
    assert(st);
    make_stmt(o, *st);
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_construct(std::ostream& o, AlwaysConstruct const& ab) const
{
  if(is_tr_initial()){ untested();
  }else{
  }

  if(_src && !ab.is_used_in(_src)){ untested();
    o << "// omit construct...\n";
  }else if(ab.block_or_null()){
    o << "// construct block...\n";
    make_block(o, *ab.block_or_null());
  }else{ untested();
    unreachable();
    o << ";\n";
  }
}
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_list(std::ostream& o, const Module& m) const
{
  if(is_tr_initial() || is_tr_begin() ){
	 o__ "//initial digital list\n";
    o__ "MOD_" << m.identifier() << " const* p = m;\n";
    o__ "MOD_" << m.identifier() << "* d = m;\n";
  }else if(is_precalc()){ untested();
    o__ "MOD_" << m.identifier() << " const* p = m;\n";
    o__ "MOD_" << m.identifier() << "* d = m;\n";
  }else{
    o__ "MOD_" << m.identifier() << " const* p = m;\n";
    o__ "MOD_" << m.identifier() << "* d = m;\n";
  }
  o__ "(void)p;\n";
  o__ "assert(p);\n";
  o__ "COMMON_" << m.identifier() << " const* pc = this;\n";
  o__ "(void)pc;\n";
  o__ "(void)d;\n";

  for(auto const& bb : always_list(m)){
    assert(bb);
    if(_src && !bb->is_used_in(_src)){
      o__ "// omit2 " << typeid(*bb).name() << "\n";
    }else if(auto in = dynamic_cast<InitialStmt const*>(bb)){
      o__ "// InitialStmt\n";
      if(is_tr_initial()){
	make_stmt(o, *in);
      }else{
	o__ "// omit InitialStmt\n";
      }
    }else if(is_tr_initial()){
      o__ "// is_tr_initial\n";
    }else if(auto ab = dynamic_cast<AlwaysConstruct const*>(bb)){
      o__ "{ // AlwaysConstruct\n";
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
void OUT_DIGITAL::make_load_block_variables(std::ostream& o, const
    Variable_List_Collection& P) const
{
  for (auto q = P.begin(); q != P.end(); ++q) {
    for (auto p = (*q)->begin(); p != (*q)->end(); ++p) {
      Variable_Decl const* V = *p;
      assert(V);

      if(options().optimize_common() && V->is_common()){ untested();
	o__ "auto& _v_" << V->name() << " = _v_" << V->token().long_code_name() << ";\n";
      }else if(options().optimize_common() && V->is_temporary()){
	make_one_local_var(o, *V);
      }else{
	make_one_variable_load(o, V->token());
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_node_refs(std::ostream& o, const Module& m) const
{
  if(is_tr_accept()) {
    // special case..
    for(auto const& n : m.circuit()->nodes()){
      assert(n);
      if(n->is_reg()){
	make_one_node_proxy(o, *n);
      }else if(n->is_discrete()){
	make_one_node_load(o, *n);
      }else{
	o__ "// not discrete " << n->name() << "\n";
      }
    }
  }else{
    return make_node_loads(o, m);
  }
}
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_load_variables(std::ostream& o, const Module& m) const
{
  make_load_block_variables(o, m.variables());

    make_node_refs(o, m);
}
/*--------------------------------------------------------------------------*/
// void OUT_DIGITAL::make_ctrl(std::ostream& o, SeqBlock const& s) const
// {
//   make_seq_block(o, s);
// }
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_evt(std::ostream& o, DigitalEvtCtlStmt const& s) const
{
  o__ "{ // DigitalEventCtrlStmt\n";
  {
    indent x;
    std::string name = make_cc_expression(o, s.cond());
    o__ "if ("<<name<<") {\n";
    {
      indent y;
      make_seq_block(o, s.code());
    }
    o__ "}else{\n";
    o__ "}\n";

    o << "\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_DIGITAL::make_stmt(std::ostream& o, Statement const& ab) const
{
  if(_src && !ab.is_used_in(_src)){
    o << "// omit Statement " << typeid(ab).name() << " " << _src->val_string() << "\n";
    return;
    o << "#if 0 // omit Statement " << typeid(ab).name() << "\n";
  }else{
  }

  if(auto a=dynamic_cast<DigitalProceduralAssignment const*>(&ab)) {
    make_assignment(o, a->expression());
  }else if(auto assign=dynamic_cast<Assignment const*>(&ab)) { untested();
    // incomplete.
    make_assignment(o, *assign);
  }else if(auto cs=dynamic_cast<ConditionalStmt const*>(&ab)) {
    make_cond(o, *cs);
#if 0 // later
  }else if(auto ss=dynamic_cast<SwitchStmt const*>(&ab)) { untested();
    make_switch(o, *ss);
#endif
  }else if(auto ww=dynamic_cast<ForStmt const*>(&ab)) {
    make_for(o, *ww);
  }else if(auto aws=dynamic_cast<WhileStmt const*>(&ab)) { untested();
    make_while(o, *aws);
  }else if(auto ev=dynamic_cast<DigitalEvtCtlStmt const*>(&ab)) {
    make_evt(o, *ev);
    //throw Exception("analogevtctl unsupported");
  }else if(auto is=dynamic_cast<InitialStmt const*>(&ab)) {
    if(is_tr_initial()){
      make_initial(o, *is);
    }else{ untested();
      o__ "// omit initial\n";
    }
  }else if(auto ct = dynamic_cast<DigitalCtrlStmt const*>(&ab)){ untested();
    make_seq_block(o, ct->body());
  }else if(auto t=dynamic_cast<System_Task const*>(&ab)) {
    make_system_task(o, *t);
  }else if(dynamic_cast<DigitalSeqStmt const*>(&ab)) { untested();
    incomplete();
//    make_seq(o, *ass);
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
static bool within_af(Base const*) { untested();return false;}
static bool is_static() { untested();return true;}
/*--------------------------------------------------------------------------*/
bool is_cc_ref(Expression const* e);
void OUT_DIGITAL::make_assignment(std::ostream& o, Assignment const& a) const
{
  Expression_ const& e = a.rhs();

  std::string lhsname = a.lhs().code_name();
  std::string name = a.lhs().name();
  o__ "{ // Digital Assignment " << a.type() << " '" << name << "'.\n";

  if(dynamic_cast<Token_NODE const*>(&a.lhs())){
    // BUG. wrong code name?
    lhsname = "MOD::n_" + name;
  }else{
  }
  // wrong place?
//   if(!a.is_used()){ untested();
//     o__ "// not used\n";
//   }else
  if(a.lhs().is_common() && options().optimize_common()
      && _mode!=modePRECALC
      && _mode!=modeTR_INITIAL) { untested();
    o__ "// " << lhsname << " is common\n";
  }else if(dynamic_cast<Token_NODE const*>(&a.lhs())){
    if(is_tr_accept() || is_tr_initial()) {
       if(is_cc_ref(&e)){
       }else{
       }
       o__ "bool rhs;\n";
       o__ "{ // rhs eval\n";
       {
	 indent rhsindent;
	 auto rhsname = make_cc_expression(o, e);
	 o__ "rhs = " << rhsname << ";\n";
	 if(is_tr_accept()) {
	   o__ "_v_n_" + a.lhs().name() << " = " << rhsname << "; // (accept)\n";
	 }else{
	 }
       }
       o__ "} // rhs eval\n";
       o__ "_LOGICVAL lv = rhs?lvSTABLE1:lvSTABLE0;\n"; // VALOGIC..
      if(is_tr_accept()) {
       o__ "// va::accept_node_value(d->n_(" << lhsname << "), lv, 0.);\n";
      }else if(is_tr_initial()) {
       o__ "va::initial_node_value(d->n_(" << lhsname << "), lv);\n";
      }
    }else if(is_tr_regress()) {
      o__ "incomplete();\n";
    }else{
    }
  }else if(is_tr_accept() || is_tr_advance() || is_tr_regress()) { //  || is_tr_regress()) {
    indent x;
    auto rhsname = make_cc_expression(o, e);
//    if(e.is_ref()){ untested();
//      o__ lhsname << " = " << rhsname << ";\n";
//    }else
    if(is_tr_accept()){
      o__ lhsname << " = " << rhsname << "; // (accept)\n";
    }else if(is_tr_advance()){
      o__ lhsname << " = " << rhsname << "; // (advance)\n";
    }else if(is_tr_regress()){
      o__ lhsname << " = " << rhsname << "; // (regress)\n";
    }else if(a.is_int()){ untested();
      o__ lhsname << " = int(" << rhsname << "); // (int*)\n";
    }else if(within_af(&a)){ untested();
      o__ lhsname << " = " << rhsname << "; // (1a)\n";
    }else if(_mode==modePRECALC){ untested();
      o__ lhsname << " = " << rhsname << "; // (prec)\n";
    }else if(is_static()){ untested();
      o__ lhsname << " = " << rhsname << "; // (s)\n";
    }else if(!options().optimize_deriv()) { untested();
      unreachable();
    }else{ untested();
      unreachable();
    }
  }else{
    o__ "// ! accept.. " << lhsname << "\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
