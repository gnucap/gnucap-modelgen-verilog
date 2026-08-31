/*                       -*- C++ -*-
 * Copyright (C) 2025 Felix Salfelder
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
#include "mg_code.h"
#include "mg_module.h"
#include "mg_options.h"
#include "mg_out_code.h"
/*--------------------------------------------------------------------------*/
char const* code_name(Data_Type const*x)
{
  if(!x) { untested();
    unreachable();
    return "ddouble"; //?
  }else if(x->is_real()) {
    return "ddouble";
  }else if(x->is_int()) {
    return "int";
  }else if(x->is_string()) { untested();
    return "string";
  }else{ untested();
    unreachable();
    return "??";
  }
}
/*--------------------------------------------------------------------------*/
std::string Variable_Decl::code_name() const
{
  return token().code_name();
}
/*--------------------------------------------------------------------------*/
static void make_one_variable_proxy(std::ostream& o, Token_VAR_REF const& V)
{
  o__ "class _V_" << V.name() << " : public ddouble {\n";
  o____ "MOD__ * const _m{nullptr};\n";
  o__ "public:\n";
  o____ "typedef ddouble base;\n";
  o____ "typedef va::ddouble_tag base_tag;\n";
  o____ "_V_" << V.name() << "(ddouble const& p) : ddouble(p) { itested(); }\n";
  o____ "_V_" << V.name() << "(double const& p) : ddouble(p) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(PARAMETER<double> const& p) : ddouble(p) {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(_V_" << V.name() << " const& p) : ddouble(p) {}\n";
  o____ "explicit _V_" << V.name() << "() : ddouble() {set_all_deps();}\n";
  o____ "_V_" << V.name() << "(MOD__* m) : "
    << "ddouble(m->_v_" << V.long_code_name() << "), _m(m) {}\n";
  o____ "~_V_" << V.name() << "() {\n";
  o______ "if(_m){\n";
  o________ "_m->_v_" << V.long_code_name() << " = value();\n";
  o______ "}else{\n";
  o______ "}\n";
  o____ "}\n";
  o____ "ddouble& operator=(double const& t){\n";
  o______ "ddouble::operator=(t);\n";
  o______ "return *this;\n";
  o____ "}\n";
  o____ "ddouble& operator=(PARAMETER<double> const& t){\n";
  o______ "ddouble::operator=(t);\n";
  o______ "return *this;\n";
  o____ "}\n";
  o____ "ddouble& operator=(ddouble const& t){\n";
  o______ "ddouble::operator=(t);\n";
  o______ "return *this;\n";
  o____ "}\n";
  o____ "ddouble& operator=(_V_" << V.name() <<" const & t){\n";
  o______ "ddouble::operator=(t);\n";
  o______ "return *this;\n";
  o____ "}\n";
  o__ "}";

  auto d = dynamic_cast<Variable_Decl const*>(V.item());
  if(!d){
  }else if(d->rhs().is_empty()){
  }else if(d->is_state_var()){
    incomplete();
    error(bDANGER, d->name() + ": block state variable initialiser unsupported.\n");
  }else{
  }

}
/*--------------------------------------------------------------------------*/
void OUT_CODE::make_one_variable_load(std::ostream& o,
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

    if(is_tr_accept()) {
      o << "& " << V.code_name() << "(m->_v_" << V.long_code_name() << "); // accept 1113\n";
      // TODO? assert post-accept values against _v_
    }else if(is_precalc() || is_tr_restore()) {
      o << " " << V.code_name() << "(m->_v_" << V.long_code_name() << "); // precalc 1068\n";
    }else{
      o << "& " << V.code_name() << "(m->_v_" << V.long_code_name() << "); // (1068)\n";
    }
    o__ "(void) " << V.code_name() << ";\n";
  }else if(V.type().is_int()) {
    o__ "int& " << V.code_name() << "(d->_v_" << V.long_code_name() << ");\n";
  }else if(V.type().is_real()) {
    if(V.deps().ddeps().size() == 0){
      if(dynamic_cast<Module const*>(V.scope())) {
	o__ "double& " << V.code_name() << "(d->_v_" << V.long_code_name() << "); // (823)\n";
      }else{
	o__ "// tmp block proxy (823b)\n";
	make_one_variable_proxy(o, V);
	o << V.code_name() << "(d);\n";
      }
    }else if(options().optimize_deriv()) {
      incomplete();
      make_one_variable_proxy(o, V);
      o << V.code_name() << "(d);\n";
    }else{ untested();
      make_one_variable_proxy(o, V);
      o << V.code_name() << "(d);\n";
    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void OUT_CODE::make_one_local_var(std::ostream& o, Variable_Decl const& V) const
{
  o__ "";
  if(!V.type().is_real()){
    o << code_name(&V.type());
  }else if(V.data().ddeps().size()){
    o << code_name(&V.type());
    o << "/*" << V.data().ddeps().size() << "*/";
  }else{
    o << "double /*?*/";
  }
  o << " ";
  assert(V.code_name().size());
  o << V.code_name();
  o << "; // local_var\n";

  if(V.rhs().is_empty()){
  }else if(V.is_state_var()){ untested();
    incomplete();
    error(bDANGER, V.name() + ": block state variable initialiser unsupported.\n");
  }else{
    o__ "{\n";
    std::string name = make_cc_expression(o, V.rhs());
    o____ V.code_name() << " = " << name << ";\n";
    o__ "}\n";
  }
}
/*--------------------------------------------------------------------------*/
void OUT_CODE::make_initial(std::ostream& o, InitialStmt const& s) const
{
  if(is_tr_initial()) {
    o__ "{ // initial statement\n";
    make_seq_block(o, s.block());
    o__ "}\n";
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void OUT_CODE::make_cond(std::ostream& o, ConditionalStmt const& s) const
{
  o__ "{\n";
  if(s.conditional().is_true()) {
    if(s.true_part()) {
      indent y;
      make_seq_block(o, s.true_part());
    }else{ untested();
    }
  }else if(s.conditional().is_false()){
    if(s.false_part()) {
      indent y;
      make_seq_block(o, s.false_part());
    }else{
    }
  }else{
    indent x;
    std::string name = make_cc_expression(o, s.conditional());
    o__ "if ("<<name<<") {\n";
    if(s.true_part()) {
      indent y;
      make_seq_block(o, s.true_part());
    }else{
    }
    o__ "}";
    if(s.false_part()) {
      o << "else {\n";
      {
	indent y;
	make_seq_block(o, s.false_part());
      }
      o__ "}\n";
    }else{
    }
    o << "\n";
  }
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
void OUT_CODE::make_while(std::ostream& o, WhileStmt const& s) const
{
  o__ "while(true) {\n";
  {
    indent x;
    std::string name = make_cc_expression(o, s.conditional());
    o__ "if ("<<name<<") {\n";
    if(s.has_body()) {
      indent y;
      if(auto bb = dynamic_cast<SeqBlock const*>(&s.body())){
	make_seq_block(o, *bb);
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
void OUT_CODE::make_for(std::ostream& o, ForStmt const& s) const
{
  if(s.has_init()){
    make_assignment(o, s.init());
  }else{ untested();
  }
  make_while(o, s);
}
/*--------------------------------------------------------------------------*/
void OUT_CODE::make_system_task(std::ostream& o, System_Task const& s) const
{
  o__ "{\n";
  make_cc_expression(o, s.expression());
  o << "\n";
  o__ "}\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
