/*                                      -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
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
#include "mg_out.h"
#include "mg_primitive.h"
#include "mg_.h" // TODO
/*--------------------------------------------------------------------------*/
// make_eval?
static void make_common_primitive(std::ostream& o, const Primitive& p)
{
  std::string class_name = "COMMON_" + p.identifier().to_string();
  std::string base_class_name;
  base_class_name = "COMMON_LOGIC";
  o << "class " << class_name << " :public " << base_class_name << "{\n";
  o__ "explicit " << class_name << "(const " << class_name << "& p) : "
                  << base_class_name << "(p) { }\n";
  o__ "COMMON_COMPONENT* clone()const override {return new "<<class_name<<"(*this);}\n";
  o << "public:\n";
  o__ "explicit " << class_name << "(int c=0) : " << base_class_name << "(c) {}\n";
  o__ "         ~" << class_name << "() {}\n";
  o << "private:\n";
  o__ "bool    operator==(const COMMON_COMPONENT& x)const override {\n";
  o____ class_name << " const* p = dynamic_cast<const " << class_name << "*>(&x);\n";
  o____ "bool rv = p && " << base_class_name << "::operator==(x);\n";
  o____ "return rv;\n";
  o__ "}\n";
  o__ "virtual LOGICVAL logic_eval(node_l const*, int)const override;\n";
  o__ "std::string name()const override {itested();return \"" << p.identifier() << "\";}\n";
  o__ "std::string port_name(int i)const override {\n";
  o____ "assert(i >= 0);\n";
  o____ "static std::string names[] = {";
  std::string comma = "";
  for (int nn = 0; nn < p.net_nodes(); ++nn){
    o << comma << '"' << p.port_name(nn) << '"';
    comma = ", ";
  }
  o____ "};\n";
  o____ "if(i < " << p.net_nodes() << "){\n";
  o______ "return names[i];\n";
  o____ "}else{ untested();\n";
  o______ "return \"\";\n";
  o____ "}\n";
  o__ "}\n";

  o << "}; //" << class_name << "\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
} // make_common
/*--------------------------------------------------------------------------*/
static std::string mkval(std::vector<int> const& line)
{
  int out = line.back();
  if (out == 0){
    return "lvSTABLE0";
  }else if (out == 1){
    return "lvSTABLE1";
  }else{ untested();
    unreachable();
    return "lv...";
  }
}
/*--------------------------------------------------------------------------*/
static std::string mkmask(std::vector<int> const& line, int what)
{
  assert(line.size());
  std::string s = "std::bitset<" + to_string(int(line.size())-1) + ">(0b";
  int i = int(line.size());
  while(i--){
    if(line[i] == UDP_Table::udp_q){
      s+= "1";
    }else if(line[i] == what){
      s+= "1";
    }else{
      s+= "0";
    }
  }
  s += ")";
  return s;
}
/*--------------------------------------------------------------------------*/
// table lookup inspired by iverilog/vvp
static void make_cc_eval(std::ostream& o, const Primitive& p)
{
  std::string class_name = "COMMON_" + p.identifier().to_string();
  o << "LOGICVAL " << class_name << "::logic_eval(node_l const* p, int incount) const\n{\n";
  o__ "assert(incount < " << p.net_nodes() << ");\n";
  o__ "std::bitset<" << p.net_nodes() - 1 << "> in0, in1, inx;\n";
  o__ "for(int i=0; i<incount; ++i){\n";
  o____ "if(p[i]->lv() == lvSTABLE0) {\n";
  o______ "in0[i] = 1;\n";
  o____ "}else if(p[i]->lv() == lvSTABLE1) {\n";
  o______ "in1[i] = 1;\n";
  o____ "}else if(p[i]->lv() == lvUNKNOWN) {\n";
  o______ "inx[i] = 1;\n";
  o____ "}else{\n";
  o______ "incomplete();\n";
  o____ "}\n";
  o__ "}\n";

  o__ "LOGICVAL out = lvUNKNOWN;\n";
  o__ "if(0){\n";
  for(auto& line : p.table().lines()) {
    if(line.back() == UDP_Table::udp_0 || line.back() == UDP_Table::udp_1){
      o__ "}else if( in0 == (in0 & " << mkmask(line, UDP_Table::udp_0) << ")\n";
      o__ "       && in1 == (in1 & " << mkmask(line, UDP_Table::udp_1) << ")\n";
      o__ "       && inx == (inx & " << mkmask(line, UDP_Table::udp_x) << ")) {\n";
      o____ "out = " << mkval(line) << ";\n";
    }else{ untested();
    }
  }
  o__ "}else{\n";
  o__ "}\n";

  o__ "return out;\n";

  o << "}\n";
}
/*--------------------------------------------------------------------------*/
static void make_cc_install(std::ostream& o, const Primitive& p)
{
  o << "class udp_installer : public CARD {\n";
  o__ "CARD* clone()const override {\n";
  o____ "CARD* c = device_dispatcher.clone(\"__udp\");\n";
  o____ "auto cc = prechecked_cast<COMPONENT*>(c);\n";
  o____ "assert(cc);\n";
  o____ "auto nc = new COMMON_" << p.identifier() << "();\n";
 // o____ "nc->set_modelname(\"dummy_tmp\");\n";
 // o____ "nc->set_param_by_name(\"model\", \"dummy_tmp\");\n";
  o____ "cc->attach_common(nc);\n";
  o____ "return cc;\n";
  o__ "}\n";
  o__ "std::string value_name()const override { unreachable(); return \"\"; }\n";
  o << "}li;\n";

  o << "DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, \""
    << p.identifier() << "\", &li);\n";
}
/*--------------------------------------------------------------------------*/
void make_cc_primitive(std::ostream& o, const Primitive& p)
{
  make_tag(o);
  make_common_primitive(o, p);
  make_cc_eval(o, p);
  make_cc_install(o, p);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
