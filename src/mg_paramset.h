/*                       -*- C++ -*-
 * Copyright (C) 2023-26 Felix Salfelder
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
#ifndef MG_PARAMSET_H
#define MG_PARAMSET_H
/*--------------------------------------------------------------------------*/
class Paramset_Stmt : public Owned_Base {
  Parameter_Base const* _what{nullptr};
  Expression_ _rhs;
  bool _overridden{false};
public:
  explicit Paramset_Stmt() : Owned_Base() {}
  void set_parameter(Parameter_Base const* b) {_what = b;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  void set_overridden() {_overridden = true;}
  bool is_overridden()const {return _overridden;}
  Data_Type const& type()const;
public:
  std::string name()const;
  Expression_ const& value()const;
};
/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
