/*$Id: mg_out_dev.cc 2014/11/23 al$ -*- C++ -*-
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
#include "mg_out.h"
#include "m_tokens.h"
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static std::string fix_expression(const std::string& in)
{
  std::string out;
  out[0] = '\0';
  
  CS x(CS::_STRING, in);
  for (;;) {
    if (x.peek() == '@') {
      x.skip1('@');
      std::string object(x.ctos("[,"));
      x.skip1('[');
      std::string attrib(x.ctos("]"));
      x.skip1(']');
      if (object[0] == 'n') {
	out += " _n[" + object + "]";
	if (attrib != "") {
	  out += ".v0()";
	}else{
	}
      }else{
	out += " CARD::probe(_" + object + ",\"" + attrib + "\")";
      }
    }else if (x.more()) {
      out += ' ' + x.ctos("@");
    }else{
      break;
    }
  }
  return out;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_probe_parameter(std::ostream& out, const Parameter_1& p)
{
  make_tag();
  out << "if (Umatch(x, \"";
  if (!(p.alt_name().empty())) {untested();
    out << to_lower(p.alt_name()) << " \" || \"";
  }
  out << to_lower(p.user_name()) << " \")) {\n"
    "    return " << p.code_name() << ";\n"
    "  }else ";
}
/*--------------------------------------------------------------------------*/
void make_probe_parameter_list(std::ostream& out,const Parameter_1_List& pl)
{
  make_tag();
  for (Parameter_1_List::const_iterator
       p = pl.begin();
       p != pl.end();
       ++p) {
    make_probe_parameter(out, **p);
  }
}
/*--------------------------------------------------------------------------*/
#if 0
/*--------------------------------------------------------------------------*/
static void make_dev_aux(std::ostream& out, const Device& d)
{
  make_tag();
  for (Function_List::const_iterator
       p = d.function_list().begin();
       p != d.function_list().end();
       ++p) {
    out << "void DEV_" << d.name() << "::" << (**p).name() << "\n"
      "{\n"
	<< (**p).code() <<
      "}\n"
      "/*--------------------------------------"
      "------------------------------------*/\n";
  }
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
