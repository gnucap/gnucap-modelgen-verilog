/*                        -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
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
 * Verilog-AMS primitive
 */
/*--------------------------------------------------------------------------*/
#ifndef MG_PRIMITIVE_H
#define MG_PRIMITIVE_H
#include "mg_base.h"
#include "mg_code.h"
#include "mg_component.h"
#include "mg_circuit.h" // for now.
/*--------------------------------------------------------------------------*/
class Node_Map;
/*--------------------------------------------------------------------------*/
class UDP_Table : public Base {
  int _width{0};
  std::string _blob;
  std::vector<std::vector<int>> _lines;
  New_Port_List const* _ports{nullptr};
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;

  void set_ports(New_Port_List const* p) {
    assert(p);
    _ports = p;
    set_width(int(_ports->size()));
  }
  void set_width(int i) { _width = i; }
  void parse_line(CS& f);
};
/*--------------------------------------------------------------------------*/
class Port_3;
class Node;
class Primitive : public Block /*Component?*/ {
  File const* _file{nullptr};
  New_Port_List	_ports;
  Node_Map* _nodes{nullptr};
  Port_3_List_3	_input;
  Port_3_List_3	_output;
  Port_3_List_3	_inout;
  UDP_Table _table;
protected:
  String_Arg	_identifier;
  // Table
public:
  Primitive();
  ~Primitive();
public:
  String_Arg const& key()const	  {return _identifier;}
  File const* file() const{ return _file; }; // owner?
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  const String_Arg& identifier()const	{return _identifier;}
private:
  void parse_body(CS& f);

public: // share "circuit" with Module?
	// stash here, for now.
  const Node_Map& nodes()const		{assert(_nodes); return *_nodes;}
  Block* scope() /* override */ {
    return this; // need _circuit?
  }
  Node* new_node(std::string const& p)override;
  Port_3_List_3 const& input()const  {return _input;}
  Port_3_List_3 const& output()const {return _output;}
  Port_3_List_3 const& inout()const  {return _inout;}
  UDP_Table const& table()const {return _table;}
private:
  Node_Map& nodes() {assert(_nodes); return *_nodes;}
  Port_3_List_3& input()  {return _input;}
  Port_3_List_3& output() {return _output;}
  Port_3_List_3& inout()  {return _inout;}
  UDP_Table& table() {return _table;}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
