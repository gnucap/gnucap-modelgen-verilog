/*$Id: mg_out_dump.cc,v 26.81 2008/05/27 05:33:43 al Exp $ -*- C++ -*-
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
/*--------------------------------------------------------------------------*/
static void make_header(std::ostream& out, const File& in)
{
  out << in.head();
}
/*--------------------------------------------------------------------------*/
static void make_tail(std::ostream& out)
{
  out <<
    "/*--------------------------------------------------------------------------*/\n"
    "/*--------------------------------------------------------------------------*/\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void make_dump_file(const File& in)
{
  std::string dump_name = in.name();
  { // chop prefix path
    std::string::size_type loc = dump_name.find_last_of(ENDDIR);
    if (loc != std::string::npos) {
      dump_name.erase(0, loc+1);
    }else{
    }
  }
  
  // open file
  dump_name += ".dump";
  std::ofstream out((dump_name).c_str());
  if (!out) {
    untested();
    os_error(dump_name);
  }
  
  make_header(out, in);
  
  // in.dump(out) ?
  out << in.nature_list() << '\n'
      << in.discipline_list() << '\n'
      << in.module_list() << '\n'
      << in.macromodule_list() << '\n'
      << in.connectmodule_list() << '\n';
  
  make_tail(out);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
