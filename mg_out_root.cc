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

#include <gnucap/md.h>
#include "mg_out.h"
/*--------------------------------------------------------------------------*/
static void make_header(std::ostream& o, const File& in,
			const std::string& dump_name)
{
  o << in.head() << 
    "/* This file is automatically generated. DO NOT EDIT */\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
  o <<
    "#include <gnucap/globals.h>\n"
    "#include <gnucap/e_compon.h>\n"
    "#include <gnucap/e_node.h>\n"
    "#include \"../va.h\"\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    "const double NA(NOT_INPUT);\n"
    "const double INF(BIGBIG);\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
    o << "namespace {\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
static void make_tail(std::ostream& o, const File& in)
{
  o << // in.cc_direct() <<
    "/*--------------------------------------"
    "------------------------------------*/\n"
    "}\n"
    "/*--------------------------------------"
    "------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void make_cc(std::ostream& out, const File& in)
{
  make_header(out, in, "dumpname");
  for (Module_List::const_iterator
       m = in.module_list().begin();
       m != in.module_list().end();
       ++m) {
    // make_cc_common(out, **m);
    make_cc_module(out, **m);
    // make_cc_dev(out, **m);
  }
  make_tail(out, in);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
