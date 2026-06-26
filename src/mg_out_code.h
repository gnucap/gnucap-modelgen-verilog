/*                                      -*- C++ -*-
 * Copyright (C) 2026 Felix Salfelder
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
#ifndef MG_OUT_CODE_H
#define MG_OUT_CODE_H
/*--------------------------------------------------------------------------*/
#include "mg_out.h"
#include "mg_out_code.h"
/*--------------------------------------------------------------------------*/
class Variable_Decl;
class Token_VAR_REF;
/*--------------------------------------------------------------------------*/
class OUT_CODE {
  virtual bool is_dynamic()const {return false;}
  virtual bool is_tr_accept()const = 0;
  virtual bool is_precalc()const = 0;
  virtual bool is_tr_restore()const = 0;
  virtual bool is_tr_eval()const {return is_dynamic();}
protected:
  void make_one_local_var(std::ostream& o, Variable_Decl const& V)const;
  void make_one_variable_load(std::ostream& o, Token_VAR_REF const& V)const;
}; // OUT_CODE
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
