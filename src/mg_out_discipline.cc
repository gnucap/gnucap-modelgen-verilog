/*                          -*- C++ -*-
 * Copyright (C) 2022-26 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
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
#include "mg_discipline.h"
#include "mg_out.h"
/*--------------------------------------------------------------------------*/
void make_continuous_discipline(std::ostream& o, Discipline const& d)
{
  auto i = &d;
  // obsolete.
  o << "class DISCIPLINE_" << i->identifier() << " : public DISCIPLINE {\n";
  o << "public:\n";
  if(i->flow()){
    o__ "NATURE const* flow()const override{\n";
    o____ "return &_N_" << i->flow()->identifier() << ";\n" ;
    o__ "}\n";
  }else{itested();
  }
  if(i->potential()){
    o__ "NATURE const* potential()const override{\n";
    o____ "return &_N_" << i->potential()->identifier() << ";\n" ;
    o__ "}\n";
  }else{itested();
  }
  o << "}_D_"<<i->identifier()<<";\n";
  o << "class _COMMON_VASRC_" << i->identifier() << " : public COMMON_VASRC {\n";
  o << "public:\n";
  o__ "_COMMON_VASRC_" << i->identifier() << "(int i) : COMMON_VASRC(i){}\n";
  o << "private:\n";
  o__ "_COMMON_VASRC_" << i->identifier() << "(_COMMON_VASRC_" << i->identifier() << " const&p)";
  o____ " : COMMON_VASRC(p){}\n";
  o__ "COMMON_COMPONENT* clone()const override{\n";
  o____ "return new _COMMON_VASRC_" << i->identifier() << "(*this);\n";
  o__ "}\n";
  o__ "std::string name()const override{untested(); return \""<<i->identifier()<<"\";}\n";
  o__ "DISCIPLINE const* discipline()const override {return &_D_"<<i->identifier()<<";}\n";

  o << "public:\n";
  o << "};\n";
  o << "static _COMMON_VASRC_" << i->identifier() << " _C_V_"<<i->identifier()<<"(CC_STATIC);\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  static int num;
  std::string id = d.identifier().to_string();
  std::string cn = "CDISC_" + id;
  o << "class " << cn << " : public NODE_TYPE {\n";
  o__ "explicit " << cn << "(" << cn << " const& p ) : NODE_TYPE(p) {}\n";
  o << "public:\n";
  o__ "explicit " << cn << "() : NODE_TYPE(\"" << id << "\") {}\n";
  o__ "NODE_TYPE* clone()const override {untested();\n";
  o____ "return new " << cn << "(*this);\n";
  o__ "}\n";
  o__ "NODE* deflate()override {untested();\n";
  o____ "return new LOGIC_NODE();\n"; // TODO
  o__ "}\n";
  o << "}cdisc_" << ++num << ";\n";
  o << "/*--------------------------------------"
       "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
void make_discrete_discipline(std::ostream& o, Discipline const& d)
{
  static int num;
  std::string id = d.identifier().to_string();
  std::string cn = "DDISC_" + id;
  o << "class " << cn << " : public NODE_TYPE {\n";
  o__ "explicit " << cn << "(" << cn << " const& p ) : NODE_TYPE(p) {}\n";
  o << "public:\n";
  o__ "explicit " << cn << "() : NODE_TYPE(\"" << id << "\") {}\n";
  o__ "NODE_TYPE* clone()const override{untested();\n";
  o____ "return new " << cn << "(*this);\n";
  o__ "}\n";
  o << "}ddisc_"<< ++num << ";\n";
  o << "/*--------------------------------------"
       "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
