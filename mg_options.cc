/*                             -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023 Felix Salfelder
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
#include "mg_options.h"
/*--------------------------------------------------------------------------*/
// global?
Base& modelgen_opts()
{
  static Options opt;
  return opt;
}
/*--------------------------------------------------------------------------*/
void Options::parse(CS& f)
{ untested();
  bool changed = false;
  size_t here = f.cursor();
  do{ untested();
    ONE_OF
      || Get(f, "optimize-binop",  &_optimize_binop)
      || Get(f, "optimize-swap",   &_optimize_swap)
      || Get(f, "gen-module",      &_gen_module)
      || Get(f, "gen-paramset",    &_gen_paramset)
      || Get(f, "dump-module",     &_dump_module)
      || Get(f, "dump-paramset",   &_dump_paramset)
      || Get(f, "dump-unreachable",&_dump_unreachable)
      ;

    if (!f.stuck(&here)) { untested();
      changed = true;
    }
  }while (f.more() && changed);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
