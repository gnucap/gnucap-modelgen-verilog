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
 * Verilog-AMS event "functions"
 */
#ifndef MGVAMS_EVT_H
#define MGVAMS_EVT_H
#include "mg_func.h"
/*--------------------------------------------------------------------------*/
class MGVAMS_EVENT : public FUNCTION_ {
private:
  void stack_op(Expression*)const override {
    throw Exception("invalid");
  }
public:
  MGVAMS_EVENT() : FUNCTION_() { }
  MGVAMS_EVENT(MGVAMS_EVENT const& p) : FUNCTION_(p) { }
  ~MGVAMS_EVENT() {} //  {delete _rdeps;}
  bool needs_context()const override{ return true; }
  Data_Type const* return_type()const override {
    static Data_Type_Int r; // bool?
    return &r;
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
