/*                -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023 Felix Salfelder
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
#include <io_.h>
#include "mg_error.h"
#include "mg_options.h"
#include "mg_out.h"
#include "mg_in.h"
#include <stack>
#include <u_opt.h>
#include <e_cardlist.h> // BUG?
#include <l_stlextra.h>
#include "mg_.h" // TODO
/*--------------------------------------------------------------------------*/
void Head::parse(CS& file)
{ untested();
  size_t here = file.cursor();
  size_t begin = 0;
  size_t end = here;
  for (;;) { untested();
    file.skipto1('*');
    if (file >> "*/") { untested();
      end = file.cursor();
      break;  // done with head
    }else if (file.stuck(&here)) { untested();untested();
      file.warn(0, "unterminated head");
      break;
    }else{ untested();
      file.skip();
    }
  }
  _s = file.substr(begin, end-begin);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Attribute_Spec::parse(CS& f)
{
  assert(owner());
  _key = f.ctos("=", "", "");
  trace2("Attribute_Spec", _key, f.tail().substr(0,18));
  if( f >> '=' ){
    // _expr = new ConstExpression(f, owner());
    _expr = new value_type;
//    f >> *_expr;
    if(f.match1('\"')) {
      f >> *_expr;
      *_expr = "\"" + *_expr + "\"";
    }else{
      *_expr = f.ctos(",=;)*"); // , "\"'{(", "\"'})");
    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void Attribute_Spec::dump(std::ostream& o)const
{ untested();
  o << _key;
  if(has_expression()){ untested();
    o << "=" << expression();
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
#if 0
void Attribute_Instance::parse(CS& f)
{ untested();
  size_t here = f.cursor();
  if(f>>"(*"){ untested();
    while(f.more() && !f.stuck(&here)){ untested();
//      std::string value("1");
      if(f >> "*)") { untested();
	trace1("got all attributes", f.tail().substr(0,17));
	break;
      }else{ untested();
      }
      assert(owner());
      auto s = new Attribute_Spec(f, owner());
      push_back(s);
      f.skip1(',');
    }
  }else{
  }
}
#endif
/*--------------------------------------------------------------------------*/
void parse_attributes(CS& cmd, void const* x)
{
  assert(x);
  while (cmd >> "(*") {
    std::string attrib_string;
    while(cmd.ns_more() && !(cmd >> "*)")) {
      attrib_string += cmd.ctoc();
    }
    attributes(x).add_to(attrib_string, x);
  }
}
/*--------------------------------------------------------------------------*/
void print_attributes(std::ostream& o, const void* x)
{
  assert(x);
  if (has_attributes(x)) {
    o__ "";
    o << "(* " << attributes(x).string(NULL) << " *)\n";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool has_attributes(void const* x)
{
  assert(CKT_BASE::_attribs);
  return CKT_BASE::_attribs->at(x);
}
/*--------------------------------------------------------------------------*/
void move_attributes(void* from, void const* to)
{
  assert(!has_attributes(to)); //for now.
  if(has_attributes(from)){
    (*CKT_BASE::_attribs)[to] = (*CKT_BASE::_attribs)[from].chown(from, to);
    CKT_BASE::_attribs->erase(from, reinterpret_cast<bool*>(from)+1);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
template class List_Base<Base>;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
