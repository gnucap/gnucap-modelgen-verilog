/*                        -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
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
 */
/*--------------------------------------------------------------------------*/
#include "mg_func.h"
#include "mg_out.cc"
#include "mg_token.h"
#include "mg_module.h"
#include <globals.h>
#include <u_parameter.h>
/*--------------------------------------------------------------------------*/
// BUG
void Data_Type::parse(CS&) {unreachable();}
void Data_Type::dump(std::ostream&)const {unreachable();}
Data_Type const* MGVAMS_FUNCTION::return_type() const
{ untested();
  static Data_Type_Real r;
  return &r;
}
/*--------------------------------------------------------------------------*/
bool Statement::propagate_rdep(Base const* )
{ untested();
  unreachable();
  incomplete();
  return false;
}
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
class STUB : public MGVAMS_FUNCTION {
public:
  explicit STUB(std::string const l) : MGVAMS_FUNCTION() {itested();
    set_label(l);
  }
  ~STUB() { }
private:
  std::string code_name()const override{itested();
    if(label()!=""){itested();
      return "";
    }else{ untested();
      return "va::" + label();
    }
  }
  void make_cc_common(std::ostream& o)const override {itested();
    o << "// dummy " << label() << "\n";
  }
  void stack_op(Expression*)const override { untested();
    throw Exception("invalid");
  }
};
/*--------------------------------------------------------------------------*/
STUB white_noise("white_noise");
DISPATCHER<FUNCTION>::INSTALL d_white_noise(&function_dispatcher, "white_noise", &white_noise);
STUB flicker_noise("flicker_noise");
DISPATCHER<FUNCTION>::INSTALL d_flicker_noise(&function_dispatcher, "flicker_noise", &flicker_noise);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
