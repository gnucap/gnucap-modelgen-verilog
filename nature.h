/*                                 -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
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
#include <gnucap/e_compon.h>
/*--------------------------------------------------------------------------*/
// TODO: need COMMON_NODE : public COMMON (or so.)
class NATURE : public COMMON_COMPONENT {
	double _abstol{0.};
public:
	explicit NATURE() : COMMON_COMPONENT(){}
	~NATURE(){}
public:
	void set_abstol(double t) { _abstol = t; }
	double abstol()const { return _abstol; }
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
