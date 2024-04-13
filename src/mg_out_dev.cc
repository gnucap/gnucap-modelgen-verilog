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
#include "mg_token.h"
#include "mg_.h" // TODO
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#if 0
static std::string fix_expression(const std::string& in)
{ untested();
  std::string out;
  out[0] = '\0';
  
  CS x(CS::_STRING, in);
  for (;;) { untested();
    if (x.peek() == '@') { untested();
      x.skip1('@');
      std::string object(x.ctos("[,"));
      x.skip1('[');
      std::string attrib(x.ctos("]"));
      x.skip1(']');
      if (object[0] == 'n') { untested();
	out += " _n[" + object + "]";
	if (attrib != "") { untested();
	  out += ".v0()";
	}else{ untested();
	}
      }else{ untested();
	out += " CARD::probe(_" + object + ",\"" + attrib + "\")";
      }
    }else if (x.more()) { untested();
      out += ' ' + x.ctos("@");
    }else{ untested();
      break;
    }
  }
  return out;
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static void make_probe_parameter(std::ostream& o, const Parameter_1& p)
{ untested();
  make_tag(o);
  o << "if (Umatch(x, \"";
  if (!(p.alt_name().empty())) {untested();
    o << to_lower(p.alt_name()) << " \" || \"";
  }
  o << to_lower(p.user_name()) << " \")) {\n"
    "    return " << p.code_name() << ";\n"
    "  }else ";
}
/*--------------------------------------------------------------------------*/
void make_probe_parameter_list(std::ostream& o,const Parameter_1_List& pl)
{ untested();
  make_tag(o);
  for (Parameter_1_List::const_iterator
       p = pl.begin();
       p != pl.end();
       ++p) { untested();
    make_probe_parameter(o, **p);
  }
}
/*--------------------------------------------------------------------------*/
#if 0
/*--------------------------------------------------------------------------*/
static void make_dev_aux(std::ostream& o, const Device& d)
{ untested();
  make_tag(o);
  for (Function_List::const_iterator
       p = d.function_list().begin();
       p != d.function_list().end();
       ++p) { untested();
    o << "void DEV_" << d.name() << "::" << (**p).name() << "\n"
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
