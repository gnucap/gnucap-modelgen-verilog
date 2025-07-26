/*                        -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 * Author: Felix Salfelder
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
 * modelgen function plugins base
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_token.h"
#include "mg_out.h"
#include "mg_analog.h" // Probe
#include "mg_discipline.h" // Probe
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void stack_op(FUNCTION_ const* f, Expression const& arg, Expression* E)
{
  assert(E);
  size_t s = E->size();
  E->push_back(new Token_STOP(")"));
  bool literal = true;
  for(auto i : arg) {
    if(dynamic_cast<Token_CONSTANT const*>(i)) {
      if(i->data()) {
	E->push_back(new Token_CONSTANT(i->data()->clone()));
      }else{
	// parameter.
	literal = false;
      }
    }else{
      literal = false;
    }
  }
  E->push_back(new Token_PARLIST("("));

  if(literal){
    try{
      f->stack_op(E);
    }catch(Exception const& x){
      while (E->size()>s){
	delete E->back();
	E->pop_back();
      }
      throw x;
    }
  }else{
    while (E->size()>s) {
      delete E->back();
      E->pop_back();
    }
    throw Exception("failed.");
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Node_Ref MGVAMS_FILTER::p() const
{ untested();
  return Node_Ref(); // (&Node_Map::mg_ground_node);
}
/*--------------------------------------------------------------------------*/
Node_Ref MGVAMS_FILTER::n() const
{ untested();
  return Node_Ref(); // (&Node_Map::mg_ground_node);
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FILTER::set_n_to_gnd(Module* m) const
{
  assert(m);
  assert(branch());
  m->set_to_ground(branch()->n());
  assert(n()->is_ground());
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FILTER::set_p_to_gnd(Module* m) const
{
  assert(m);
  assert(branch());
  m->set_to_ground(branch()->p());
  assert(p()->is_ground());
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FILTER::set_branch(Branch* br)
{
  assert(br);
  assert(!_br);
  _br = br;
  Branch_Ref prb(br);
  assert(_m);
  _prb = _m->new_probe("potential", prb);
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
