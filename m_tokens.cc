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
 * arithmetic tokens
 */
#include "m_tokens.h"
/*--------------------------------------------------------------------------*/
void Token_SFCALL::stack_op(Expression* E)const
{
  assert(E);
  // replace single token with its value
  if (!E->is_empty() && dynamic_cast<const Token_PARLIST*>(E->back())) {
    // has parameters (table or function)
    {
      const Token* T1 = E->back(); // Token_PARLIST
      assert(T1->data()); // expression
      Base const* d = T1->data();
      auto ee = prechecked_cast<Expression const*>(d);
      assert(ee);

      {
	// restore argument.
	E->pop_back();
	E->push_back(new Token_STOP("stopname"));
	for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
	  trace1("stackop restore arg", (**i).name());
	  (**i).stack_op(E);
	}
	E->push_back(new Token_PARLIST(".."));
	E->push_back(clone());
      }
      delete T1;
    }
  }else{
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
