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
#include <u_opt.h>
#include "mg_options.h"
/*--------------------------------------------------------------------------*/
// global?
Base& modelgen_opts()
{
  static Options opt;
  return opt;
}
/*--------------------------------------------------------------------------*/
Options::Options() :
  _optimize_binop(true),  // fold binary operators
  _optimize_common(true), // put aside model constants
  _optimize_swap(true),   // swap operands.
  _optimize_deriv(true),  // suppress zero derivative propagation
  _optimize_deps(true),   // consider dependency types
  _optimize_unused(true), // dont emit unused sources
  _optimize_nodes(true),  // prune unused nodes
  _gen_module(true),
  _gen_paramset(true),
  _dump_module(true),
  _dump_paramset(true),
  _store_unreachable(true), // ignore unreachable structures (TODO)
  _dump_unreachable(true),
  _dump_discipline(true),
  _dump_nature(true),
  _dump_annotate(false),
  _expand_paramset(true)
{
}
/*--------------------------------------------------------------------------*/
void Options::parse(CS& f)
{
  bool changed = false;
  size_t here = f.cursor();
  do{
    ONE_OF
      || Get(f, "optimize-binop",  &_optimize_binop)
      || Get(f, "optimize-common", &_optimize_common)
      || Get(f, "optimize-swap",   &_optimize_swap)
      || Get(f, "optimize-deriv",  &_optimize_deriv)
      || Get(f, "optimize-deps",   &_optimize_deps)
      || Get(f, "optimize-unused", &_optimize_unused)
      || Get(f, "gen-module",      &_gen_module)
      || Get(f, "gen-paramset",    &_gen_paramset)
      || Get(f, "dump-module",     &_dump_module)
      || Get(f, "dump-paramset",   &_dump_paramset)
      || Get(f, "dump-unreachable",&_dump_unreachable)
      || Get(f, "dump-discipline", &_dump_discipline)
      || Get(f, "dump-nature",     &_dump_nature)
      || Get(f, "dump-annotate",   &_dump_annotate)
      || Get(f, "expand-paramset", &_expand_paramset)
      || Set(f, "nag",		   &OPT::picky,	bNOERROR)
      || Set(f, "nonag",	   &OPT::picky,	bTRACE)
      || Set(f, "trace",	   &OPT::picky,	bTRACE)
      || Set(f, "notrace",	   &OPT::picky,	bLOG)
      || Set(f, "log",		   &OPT::picky,	bLOG)
      || Set(f, "nolog",	   &OPT::picky,	bDEBUG)
      || Set(f, "debug",	   &OPT::picky,	bDEBUG)
      || Set(f, "nodebug",	   &OPT::picky,	bPICKY)
      || Set(f, "picky",	   &OPT::picky,	bPICKY)
      || Set(f, "nopicky",	   &OPT::picky,	bWARNING)
      || Set(f, "warn{ing}",	   &OPT::picky,	bWARNING)
      || Set(f, "nowarn",	   &OPT::picky,	bDANGER)
      || (f.check(bWARNING, "what's this?"), f.skiparg());
      ;

    if (!f.stuck(&here)) {
      changed = true;
    }else{ untested();
    }
  }while (f.more() && changed);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
