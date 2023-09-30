/*                             -*- C++ -*-
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
 */
/*--------------------------------------------------------------------------*/
#include <m_base.h>
#include "mg_.h"
/*--------------------------------------------------------------------------*/
class Options : public Base {
  bool _optimize_binop{true};
  bool _optimize_swap{true};
  bool _gen_module{true};
  bool _gen_paramset{true};
  bool _dump_module{true};
  bool _dump_paramset{true};
  bool _dump_unreachable{true};
public:
  explicit Options(){ }
  void parse(CS& f) override;
  void dump(std::ostream&)const override{unreachable();}
public:
  bool optimize_binop()   const{ return _optimize_binop; }
  bool optimize_swap()    const{ return _optimize_swap; }
  bool gen_module()       const{ untested(); return _gen_module; }
  bool gen_paramset()     const{ untested(); return _gen_paramset; }
  bool dump_module()      const{ return _dump_module; }
  bool dump_paramset()    const{ return _dump_paramset; }
  bool dump_unreachable() const{ untested(); return _dump_unreachable; }
};
/*--------------------------------------------------------------------------*/
inline Options& options()
{
  return prechecked_cast<Options&>(modelgen_opts());
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
