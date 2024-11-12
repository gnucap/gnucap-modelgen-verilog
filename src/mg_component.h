/*                        -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023, 2024 Felix Salfelder
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
 * module circuit data
 */
/*--------------------------------------------------------------------------*/
#ifndef MG_COMPONENT_H
#define MG_COMPONENT_H
#include "mg_base.h"
/*--------------------------------------------------------------------------*/
class Discipline;
/*--------------------------------------------------------------------------*/
class Port_3 : public Owned_Base {
  std::string _name;
  std::string _value; // needed?
  Node_Ref _node;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Port_3() {}
  const std::string& name()const  {return _name;}
  const std::string& value()const  {
    if(has_identifier()){
      return _value;
    }else{
      return _name; // _node->name?
    }
  }
  bool has_identifier()const;
  String_Arg key()const { return String_Arg(value()); }
  Node_Ref const& node()const {return _node;}
  void set_node(Node*n){_node = n;}
  void set_discipline(Discipline const* d, Module* owner);
};
// TODO: Port_Base?
class New_Port : public Port_3 {
//  Block* _owner{nullptr};
public:
//  void set_owner(Block* c) { untested(); _owner = c; }
  void parse(CS& f) override;
  New_Port() : Port_3() {}
};
// list ::= "(" port {"," port} ")"
typedef LiSt<New_Port, '(', ',', ')'> New_Port_List;
/*--------------------------------------------------------------------------*/
#endif
