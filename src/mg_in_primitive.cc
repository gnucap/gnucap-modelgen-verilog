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
#include "mg_base.h"
#include "mg_primitive.h"
#include "mg_in.h"
#include "mg_.h" // BUG?
#include "mg_circuit.h" // Node_Map;
/*--------------------------------------------------------------------------*/
/*
- udp_declaration ::= (From A.5.1)
-    { attribute_instance } primitive udp_identifier ( udp_port_list ) ;
-    udp_port_declaration { untested(); udp_port_declaration }
+    udp_body
-    endprimitive
-    | { attribute_instance } primitive udp_identifier ( udp_declaration_port_list ) ;
-    udp_body
+    endprimitive
- udp_port_list ::= (From A.5.2)
-    output_port_identifier , input_port_identifier { untested(); , input_port_identifier }
- udp_declaration_port_list ::=
-    udp_output_declaration , udp_input_declaration { untested(); , udp_input_declaration }
*/
void Primitive::parse(CS& f)
{
  File* o = prechecked_cast<File*>(owner());
  assert(o);
  attr.move_attributes(tag_t(&f), tag_t(this));

  _ports.set_owner(this);

  // f >> "primitive"; // done
  f >> _identifier;
  f >> _ports;
  f >> ';';
  parse_body(f);
}
/*--------------------------------------------------------------------------*/
void Primitive::dump(std::ostream& o)const
{
  print_attributes(o, this);
  o << "primitive " << identifier() << _ports << ";\n";
 // if(options().dump_annotate()){ untested();
 //   dump_annotate(*this, o);
 // }else{ untested();
 // }
  if(input().size()){
    o << "input " << input() << "\n";
  }else{ untested();
  }
  if(output().size()){
    o << "output " << output() << "\n";
  }else{ untested();
  }
  if(inout().size()){ untested();
    o << "inout " << inout() << "\n";
  }else{
  }
  o << table();
  o << "endprimitive\n";
}
/*
- udp_port_declaration ::=
- udp_output_declaration ;
- | udp_input_declaration ;
- | udp_reg_declaration ;
- udp_output_declaration ::=
- { attribute_instance } output port_identifier
- | { attribute_instance } output reg port_identifier [ = constant_expression ]
- udp_input_declaration ::=
- { attribute_instance } input list_of_port_identifiers
- udp_reg_declaration ::=
- { attribute_instance } reg variable_identifier
- udp_body ::= (From A.5.3)
- combinational_body | sequential_body
- combinational_body ::=
- table combinational_entry { combinational_entry } endtable
- combinational_entry ::=
- level_input_list : output_symbol ;
- sequential_body ::=
- [ udp_initial_statement ] table sequential_entry { sequential_entry } endtable
- udp_initial_statement ::=
- initial output_port_identifier = init_val ;
- init_val ::= 1'b0 | 1'b1 | 1'bx | 1'bX | 1'B0 | 1'B1 | 1'Bx | 1'BX | 1 | 0
- sequential_entry ::=
- seq_input_list : current_state : next_state ;
- seq_input_list ::=
- level_input_list | edge_input_list
- level_input_list ::=
- level_symbol { untested(); level_symbol }
- edge_input_list ::=
- { untested(); level_symbol } edge_indicator { level_symbol }
- edge_indicator ::=
- ( level_symbol level_symbol ) | edge_symbol
- current_state ::= level_symbol
- next_state ::=output_symbol | -
- output_symbol ::= 0 | 1 | x | X
- level_symbol ::= 0 | 1 | x | X | ? | b | B
- edge_symbol ::= r | R | f | F | p | P | n | N | *
*/
/*--------------------------------------------------------------------------*/
void Primitive::parse_body(CS& f)
{
  //_variables.set_owner(this);
  _input.set_owner(this);
  _output.set_owner(this);
  _inout.set_owner(this);

  size_t here = f.cursor();
  for (;;) {
    parse_attributes(f, &f);
    ONE_OF	// module_item
      || f.umatch(";")
      || ((f >> "input ") && (f >> input()))
      || ((f >> "output ") && (f >> output()))
      || ((f >> "inout ") && (f >> inout()))
      ;

    if (f.stuck(&here)) {
      break;
    }else{
    }
  }

  table().set_ports(&_ports);

  if(f >> "table "){
    f >> table();
  }else{ untested();
    f.warn(bWARNING, "need table\n");
  }

  if(f >> "endprimitive"){
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Primitive::Primitive() : Block()
{
  _nodes = new Node_Map();
 // _nodes->set_owner(this);
}
/*--------------------------------------------------------------------------*/
Primitive::~Primitive()
{
  delete _nodes;
}
/*--------------------------------------------------------------------------*/
void UDP_Table::parse_line(CS& f)
{
  std::vector<int> line;
  bool end = false;

  int i;
  for(i=0; i<_width + 2; ++i){
    bool sep = false;
    size_t here = f.cursor();
    ONE_OF
      || ((f >> '1') && (line.push_back(1), true))
      || ((f >> '0') && (line.push_back(0), true))
      || ((f >> 'x') && (line.push_back(2), true))
      || ((f >> ':') && (sep=true))
      || ((f >> ';') && (end=true))
      ;

    if (sep && int(line.size())+1!=_width){ untested();
      throw Exception_CS_("bad table", f);
    }else if (end){
      break;
    }else if (f.stuck(&here)) { untested();
      throw Exception_CS_("bad table", f);
    }else{
    }
  }
  if(i!=_width+1){ untested();
    throw Exception_CS_("bad table", f);
  }else{
  }
  _lines.push_back(line);
}
/*--------------------------------------------------------------------------*/
void UDP_Table::parse(CS& f)
{
  for (;;) {
    if(f >> "endtable"){
      break;
    }else{
      parse_line(f);
    }
  }
}
/*--------------------------------------------------------------------------*/
void UDP_Table::dump(std::ostream& o) const
{
  o << "table\n";
  if(_ports && _ports->size()){
    o << "//";
    auto p = _ports->begin();
    std::string out = (*p)->name();
    while(++p != _ports->end()){
      o << " " << (*p)->name();
    }
    o << " : " << out << "\n";
  }
  for(auto i : _lines){
    o << "  ";
    int k = 1;
    for(auto j : i){
      o << " " << name(j);
      ++k;
      if(k == _width){
	o << " :";
      }else{
      }
    }
    o << ";\n";
  }
  o << "endtable\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
