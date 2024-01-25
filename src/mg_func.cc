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
#include "m_tokens.h"
#include "mg_out.h"
#include "mg_analog.h" // Probe
#include "mg_discipline.h" // Probe
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void FUNCTION_::stack_op(Expression const& arg, Expression* E) const
{
  Expression const* ee = &arg;
  bool all_float = false;
  double argv[5];
  argv[0] = NOT_VALID;
  argv[1] = NOT_VALID;
  argv[2] = NOT_VALID;
  argv[3] = NOT_VALID;
  argv[4] = NOT_VALID;
  double* seek = argv;
  for (Expression::const_iterator i = ee->begin(); i != ee->end(); ++i) {
    trace1("float?", (**i).name());
    auto F = dynamic_cast<Float const*>((**i).data());
    all_float = F;
    if(!all_float){
      break;
    }else{
      *seek = F->value();
      ++seek;
      assert(seek<argv+5);
    }
  }

  if(!all_float){
    throw Exception("invalid");
    // restore argument.
  }else if(arg.size()==1){
    double value = evalf(argv);
    const Float* v = new Float(value);
    E->push_back(new Token_CONSTANT(to_string(value), v, ""));
  }else if(arg.size()==2){
    double value = evalf(argv);
    const Float* v = new Float(value);
    E->push_back(new Token_CONSTANT(to_string(value), v, ""));
  }else{
    incomplete();
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
#if 0
Token* MGVAMS_FUNCTION::new_token(Module& m, size_t na) const
{
  m.install(this);
  if(code_name() != ""){
    return new Token_CALL(label(), this);
  }else if(label() != ""){
    return new Token_CALL(label(), this);
  }else{
    incomplete();
    return NULL;
  }
}
#endif
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
