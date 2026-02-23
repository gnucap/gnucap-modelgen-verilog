/*                        -*- C++ -*-
 * Copyright (C) 2026 Felix Salfelder
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
 *------------------------------------------------------------------
 * Verilog-AMS, hierarchical references
 */
/*--------------------------------------------------------------------------*/
#include "mg_href.h"
/*--------------------------------------------------------------------------*/
void new_hier_refs(Hierarchical_Refs** h)
{
  assert(!*h);
  *h = new Hierarchical_Refs();
};
/*--------------------------------------------------------------------------*/
void delete_hier_refs(Hierarchical_Refs** h)
{
  delete *h;
  *h = nullptr;
};
/*--------------------------------------------------------------------------*/
Base /*const*/ * Hierarchical_Refs::get(std::string const& n)
{
  auto f = _refs.find(n);

  if(f == _refs.end()){
    auto h = new Token_HIER_REF(n);
    _refs.push_back(h);
    return h;
  }else{ untested();
    return *f;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
