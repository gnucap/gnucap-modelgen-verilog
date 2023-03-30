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
 */

#ifndef GNUCAP_E_VA_H
#define GNUCAP_E_VA_H

#include <gnucap/e_compon.h>
class COMMON_VASRC : public COMMON_COMPONENT {
  double _abstol{0.};
public:
  explicit COMMON_VASRC(int i) : COMMON_COMPONENT(i) {}
  ~COMMON_VASRC() {}
public:
  double abstol(){return _abstol;}
  void set_abstol(double a){_abstol = a;}
};
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
