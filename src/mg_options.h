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
//#include "mg_base.h"
#include <m_base.h>
/*--------------------------------------------------------------------------*/
class Options : public Base {
  bool _optimize_binop{true};  // fold binary operators
  bool _optimize_swap{true};   // swap operands.
  bool _optimize_deriv{true};  // suppress zero derivative propagation
  bool _optimize_deps{true};   // consider dependency types
  bool _optimize_unused{true}; // dont emit unused sources
  bool _optimize_nodes{true};  // prune unused nodes
  bool _gen_module{true};
  bool _gen_paramset{true};
  bool _dump_module{true};
  bool _dump_paramset{true};
  bool _store_unreachable{true}; // ignore unreachable structures (TODO)
  bool _dump_unreachable{true};
  bool _dump_discipline{true};
  bool _dump_nature{true};
  bool _dump_annotate{false};
  bool _expand_paramset{true};
public:
  explicit Options(){ }
  void parse(CS& f) override;
  void dump(std::ostream&)const override{ untested();unreachable();}
public:
  bool optimize_binop()   const{ return _optimize_binop; }
  bool optimize_swap()    const{ return _optimize_swap; }
  bool optimize_deriv()   const{ return _optimize_deriv; }
  bool optimize_deps()    const{ untested(); return _optimize_deps; }
  bool optimize_unused()  const{ return _optimize_unused; }
  bool optimize_nodes()   const{ return _optimize_nodes; }
  bool gen_module()       const{ return _gen_module; }
  bool gen_paramset()     const{ return _gen_paramset; }
  bool dump_module()      const{ return _dump_module; }
  bool dump_paramset()    const{ return _dump_paramset; }
  bool store_unreachable()const{ untested(); untested(); return _store_unreachable; }
  bool dump_unreachable() const{ return _dump_unreachable; }
  bool dump_discipline()  const{ return _dump_discipline; }
  bool dump_nature()      const{ return _dump_nature; }
  bool dump_annotate()    const{ return _dump_annotate; }
  bool expand_paramset()  const{ return _expand_paramset; }
public:
  friend class option_nodump_annotate;
  friend class option_nodump_unreachable;
};
/*--------------------------------------------------------------------------*/
Base& modelgen_opts(); //?
inline Options& options()
{
  return prechecked_cast<Options&>(modelgen_opts());
}
/*--------------------------------------------------------------------------*/
class option_nodump_annotate{
  bool _prev;
public:
  explicit option_nodump_annotate(){
    _prev = options().dump_annotate();
    options()._dump_annotate = false;
  }
  ~option_nodump_annotate(){
    options()._dump_annotate = _prev;
  }
};
/*--------------------------------------------------------------------------*/
class option_nodump_unreachable{
  bool _prev;
public:
  explicit option_nodump_unreachable(){
    _prev = options().dump_unreachable();
    options()._dump_unreachable = false;
  }
  ~option_nodump_unreachable(){
    options()._dump_unreachable = _prev;
  }
};
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
