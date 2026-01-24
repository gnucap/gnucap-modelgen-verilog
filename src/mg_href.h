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
#ifndef MG_HIER_REFS_H
#define MG_HIER_REFS_H
#include "mg_token.h"
/*--------------------------------------------------------------------------*/
class Hierarchical_Refs{
  typedef Keyed_List<Token_HIER_REF /*const*/> container;
  typedef container::const_iterator const_iterator;
  container _refs;
public:
  Base* get(std::string const&);

  const_iterator begin()const {return _refs.begin();}
  const_iterator end()const {return _refs.end();}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
