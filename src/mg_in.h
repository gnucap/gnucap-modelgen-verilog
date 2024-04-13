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
 * input
 */
/*--------------------------------------------------------------------------*/
#ifndef MG_IN_H
#define MG_IN_H
#include "mg_base.h"
#include "mg_attrib.h" // <u_attrib>?
/*--------------------------------------------------------------------------*/
class Head : public Base {
  std::string _s;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override {f << _s;}
  Head() {}
};
/*--------------------------------------------------------------------------*/
class Discipline;
class Module;
class Paramset;
typedef Collection<Nature> Nature_List;
typedef Collection<Paramset> Paramset_List;
typedef Collection<Module> Module_List;
typedef Collection<Discipline> Discipline_List;
class File : public Block {
  std::string	_name;
  std::string   _cwd;
  CS		_file;
  Head		_head;
  Nature_List	_nature_list;
  Discipline_List _discipline_list;
  Module_List	_module_list;
  Module_List	_macromodule_list;
  Module_List	_connectmodule_list;
  Paramset_List   _paramset_list;
public: // build
  File();
  ~File(){ }

  void read(std::string const&);
  void parse(CS& f) override;
  void dump(std::ostream&)const override;

public: // readout
  const std::string& name()const	{ untested();return _name;}
  const std::string  fullstring()const	{ untested();return _file.fullstring();}
  const Head&	     head()const	{return _head;}
//  const Code_Block&  h_headers()const	{ untested();return _h_headers;}
//  const Code_Block&  cc_headers()const	{ untested();return _cc_headers;}
  const Module_List& modules()const	{return _module_list;}
//  const Code_Block&  h_direct()const	{ untested();return _h_direct;}
//  const Code_Block&  cc_direct()const	{ untested();return _cc_direct;}

  const Nature_List&	 nature_list()const	{ return _nature_list;}
  const Discipline_List& discipline_list()const	{return _discipline_list;}
  const Module_List&	 module_list()const	{return _module_list;}
  const Module_List&	 macromodule_list()const	{return _macromodule_list;}
  const Module_List&	 connectmodule_list()const	{return _connectmodule_list;}
  const Paramset_List&	 paramset_list()const	{return _paramset_list;}

};
/*--------------------------------------------------------------------------*/
void parse_attributes(CS& cmd, void const* x);
inline void parse_attributes(CS& cmd, intptr_t x)
{ untested();
  parse_attributes(cmd, (void const*) x);
}
/*--------------------------------------------------------------------------*/
bool has_attributes(void const* x);
inline bool has_attributes(intptr_t x)
{ untested();
  return has_attributes((void const*)x);
}
void print_attributes(std::ostream& o, const void* x);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
