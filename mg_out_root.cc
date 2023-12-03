/*$Id: mg_out_root.cc,v 26.81 2008/05/27 05:33:43 al Exp $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
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

#include <md.h>
#include "mg_out.h"
#include "mg_options.h"
/*--------------------------------------------------------------------------*/
static void make_header(std::ostream& o, const File& in,
			const std::string& /*dump_name*/)
{
  o << in.head() << 
    "/* This file is automatically generated. DO NOT EDIT */\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o <<
    "#include <globals.h>\n"
    "#include <e_compon.h>\n"
    "#include <e_subckt.h>\n"
    "#include <e_node.h>\n"
    "#include <e_elemnt.h>\n"
    "// #include <e_paramlist.h>\n"
    "#include <u_nodemap.h>\n"; // if submodules are used anywhere
#if 0
    "#include <m_va.h>\n"
    "#include <e_va.h>\n"
#else
  o <<
#include "m_va.raw"
     ;
  o <<
#include "e_va.raw"
     ;
#endif
  o <<
    "#include <u_limit.h>\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o <<
    "const double NA(NOT_INPUT);\n"
    "const double INF(BIGBIG);\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "namespace {\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tail(std::ostream& o, const File&)
{
  o << // in.cc_direct() <<
    "/*--------------------------------------"
    "------------------------------------*/\n"
    "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_common_nature(std::ostream& o, const File& f)
{
  for(auto i : f.nature_list()) {
    o << "class NATURE_" << i->identifier() << " : public NATURE {\n";
    o__ "double abstol()const override {return "<<i->abstol()<<";}\n";
    o << "}_N_"<<i->identifier()<<";\n";
  }
  o << "/*--------------------------------------"
       "------------------------------------*/\n";
  o << "/*--------------------------------------"
       "------------------------------------*/\n";
  for(auto i : f.discipline_list()) {
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
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void make_cc(std::ostream& out, const File& in)
{
  indent x; // HACK
  int num = 0;
  make_header(out, in, "dumpname");
  make_common_nature(out, in);
  if(options().gen_module()){
    for (Module_List::const_iterator
	 m = in.module_list().begin();
	 m != in.module_list().end();
	 ++m) {
      out << "namespace n" << std::to_string(num) << "{\n";
      make_cc_module(out, **m);
      out << "}\n" <<
    "/*--------------------------------------"
    "------------------------------------*/\n";
      ++num;
    }
  }else if(!options().expand_paramset()){ untested();
  }else if(options().gen_paramset()){
    for (Paramset_List::const_iterator
	 m = in.paramset_list().begin();
	 m != in.paramset_list().end();
	 ++m) {
      out << "namespace n" << std::to_string(num) << "{\n";
      make_cc_module(out, **m);
      out << "}\n";
      ++num;
    }

  }
  make_tail(out, in);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
