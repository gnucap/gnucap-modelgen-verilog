/*                       -*- C++ -*-
 * Copyright (C) 2023-2026 Felix Salfelder
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
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#include "mg_.h"
#include "mg_token.h"
/*--------------------------------------------------------------------------*/
Node Node_Map::mg_ground_node("", 0);
/*--------------------------------------------------------------------------*/
void Node::new_token(std::string const& name)
{
  assert(!_token);
  _token = new Token_NODE(name, this);
}
/*--------------------------------------------------------------------------*/
Node* Node_Map::new_node(std::string const& p, Block* owner)
{
  assert(dynamic_cast<Module*>(owner)
       ||dynamic_cast<Primitive*>(owner));
  Node*& cc = _map[p];
  if(cc) {
  }else{
    // new_ref here?
    cc = new Node(p, int(_nodes.size()));
    owner->new_var_ref(cc->token());
    _nodes.push_back(cc);
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
