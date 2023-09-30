/*                       -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
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
#include "mg_.h"
#include "mg_out.h"
/*--------------------------------------------------------------------------*/
void Paramset::parse(CS& f)
{
  File* o = prechecked_cast<File*>(owner());
  assert(o);
  if(o->attribute_stash().is_empty()){
  }else{
    assert(!_attributes);
    set_attributes(o->attribute_stash().detach());
  }
  _attribute_stash.set_owner(this);

  f >> _name >> _proto_name >> ";";

  size_t here = f.cursor();
  bool end = false;

  for (;;) {
    while (f >> _attribute_stash) { }
    ONE_OF
      || f.umatch(";")
//      || ((f >> "parameter ") && (f >> _parameters))
      || ((f >> "endparamset ") && (end = true))
      ;

    if (_attribute_stash.is_empty()){
    }else{
      f.warn(0, "dangling attributes");
    }
    if (end){
      break;
    }else if (!f.more()) {
      f.warn(0, "premature EOF (module)");
      break;
    }else if (f.stuck(&here)) {
      f.warn(0, "bad module");
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void Paramset::dump(std::ostream& o) const
{
  if(has_attributes()){
    o << attributes();
  }else{
  }
  o << "paramset " << _name << " " << _proto_name << ";\n";
  o << "endparamset;\n";
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
