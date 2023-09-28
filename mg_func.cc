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
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Token* MGVAMS_FUNCTION::new_token(Module& m, size_t /*na*/, Deps&) const
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
/*--------------------------------------------------------------------------*/
void MGVAMS_FUNCTION::make_cc_dev(std::ostream& o) const
{
  o__ "// " << label() << "\n";
}
/*--------------------------------------------------------------------------*/
void MGVAMS_FUNCTION::make_cc_common(std::ostream& o) const
{
  o__ "// " << label() << "\n";
}
/*--------------------------------------------------------------------------*/
FUNCTION_::~FUNCTION_()
{
  if(has_refs()){
    incomplete();
    std::cerr << "stale ref " << _label << "\n";
  }else{
  }
//  assert(has_refs());
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
