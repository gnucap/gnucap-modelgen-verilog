/*                       -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
#include "mg_.h"
#include "mg_out.h"
/*--------------------------------------------------------------------------*/
/* A.4.1	6.2.2
+ module_instantiation ::=
+	  module_or_paramset_identifier [ parameter_value_assignment ] 
+	  module_instance
-	  { "," module_instance }
+	  ";"
+ parameter_value_assignment ::=
+	  "# (" list_of_parameter_assignments ")"
+ list_of_parameter_assignments ::=
-	  ordered_parameter_assignment { "," ordered_parameter_assignment }
+	| named_parameter_assigmnent { "," named_parameter_assignment }

+ module_instance ::=
+	   name_of_module_instance "(" [ list_of_port_connections ] ")"
+ name_of_module_instance ::=
+	   module_instance_identifier
-	  [ range ]
+ list_of_port_connections ::=
+	  ordered_port_connection { "," ordered_port_connection }
-	| named_port_connection { "," named_port_connection }
+ ordered_port_connection ::=
+	  {attribute_instance} [ expression ]
- named_port_connection ::=
-	  {attribute_instance} "." port_identifier "(" [ expression ] ")"
// A.9.3
+ module_or_paramset_identifier ::=
+	  module_identifier
+	| paramset_identifier
+ module_identifier ::=
+	  identifier
+ paramset_identifier ::=
+	  identifier
*/
void Element_2::parse(CS& file)
{
  file >> _module_or_paramset_identifier
       >> '#' >> _list_of_parameter_assignments
       >> _name_of_module_instance >> _list_of_port_connections >> ';';
}
/*--------------------------------------------------------------------------*/
void Element_2::dump(std::ostream& out)const
{
  out << "  " << dev_type()
      << " #" << list_of_parameter_assignments() << " "
      << name_of_module_instance()
      << list_of_port_connections() << ";\n";
}
/*--------------------------------------------------------------------------*/
/* A.4.1	6.2.2
- ordered_parameter_assignment ::= expression
+ named_parameter_assignment ::=
+	  "." parameter_identifier "(" [ mintypmax_expression ] ")"
-	| "." system_parameter_identifier "(" [ constant_expression ] ")"
// A.8.3
+ mintypmax_expression ::=
+	  expression
-	| expression ":" expression ":" expression
*/
void Parameter_3::parse(CS& file)
{
  file >> '.' >> _name >> '(' >> _default_val >> ')' >> ',';
}
/*--------------------------------------------------------------------------*/
void Parameter_3::dump(std::ostream& out)const
{
  out << "." << name() << "(" << default_val() << ")";
}
/*--------------------------------------------------------------------------*/
/* A.2.1.1
+ local_parameter_declaration ::=
-	  "localparam" ["signed"] [range] list_of_param_assignments
+	| "localparam" parameter_type list_of_param_assignments
+ parameter_declaration ::=
-	  "parameter" ["signed"] [range] list_of_param_assignments
+	| "parameter" parameter_type list_of_param_assignments
- specparam_declaration ::=
-	  "specparam" [range] list_of_specparam_assignments
- parameter_type ::=
-	  "integer"
-	| "real"
-	| "realtime"
-	| "time"
-	| "string"
- aliasparam_declaration ::=
-	  "aliasparam" parameter_identifier "=" parameter_identifier ";"
// A.2.3
+ list_of_param_assignments ::=
+	  param_assignment { "," param_assignment }
// A.2.4
+ param_assignment ::=
+	  parameter_identifier "=" constant_mintypmax_expression { value_range }
-	| parameter_identifier range "=" constant_arrayinit { value_range }
*/
void Parameter_2::parse(CS& file)
{
  // file >> "parameter "; from caller
  file >> _type >> _name >> '=' >> _default_val;
  assert(ctx());
  trace1("Parameter2", name());
  ctx()->new_var_ref(this);
  assert(ctx()->resolve(name()));
}
/*--------------------------------------------------------------------------*/
void Parameter_2::dump(std::ostream& out)const
{
  out <<
    "  parameter " << type() << " " << name() << " = " << default_val() << ";\n";
}
/*--------------------------------------------------------------------------*/
/* A.1.3	6.2, 6.5
+ port ::=
+       [ port_expression ]
-     | "." port_identifier "(" [ port_expression ] ")"
+ port_expression ::=
+       port_reference
-     | "{" port_reference { "," port_reference } "}"
+ port_reference ::=
+       port_identifier
-       [ "[" constant_range_expression "]" ]
*/
void Port_3::parse(CS& file)
{
  file >> _name;
}
/*--------------------------------------------------------------------------*/
void New_Port::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
  assert(_owner);
  _owner->new_node(name());
}
/*--------------------------------------------------------------------------*/
void Port_Discipline_List_Collection::parse(CS& f)
{
  assert(ctx()); // Module
  Block const* root_scope = ctx()->ctx();
  assert(root_scope);
  File const* root = prechecked_cast<File const*>(root_scope);
  assert(root);
  auto ii = root->discipline_list().find(f);

  if(ii!=root->discipline_list().end()){
    size_t here = f.cursor();
    Port_Discipline_List* m = new Port_Discipline_List();
    m->set_discipline(*ii);

    m->set_owner(ctx());
    f >> *m;
    for(auto i : *m){
      i->set_discipline(*ii);
    }

    push_back(m);

  }else{
    f.umatch("HACKHACK_HACK_UNLIKELY_STRING"); // unset _ok.
    assert(!f);
  }
}
/*--------------------------------------------------------------------------*/
void Port_Discipline_List_Collection::dump(std::ostream& out)const
{
  Collection<Port_Discipline_List>::dump(out);
}
/*--------------------------------------------------------------------------*/
void Port_Discipline_List::parse(CS& f)
{
  trace1("Port_Disc_List::parse", f.last_match());
  return LiSt<Port_Discipline, '\0', ',', ';'>::parse(f);
}
/*--------------------------------------------------------------------------*/
void Port_Discipline_List::dump(std::ostream& o)const
{
  assert(_disc);
  o__ _disc->identifier() << " ";
  LiSt<Port_Discipline, '\0', ',', ';'>::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Port_Discipline::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
  assert(_owner);
  _node = _owner->new_node(name());
}
/*--------------------------------------------------------------------------*/
void Port_Discipline::set_discipline(Discipline const* d)
{
  _node->set_discipline(d);
}
/*--------------------------------------------------------------------------*/
void Port_3::dump(std::ostream& out)const
{
  out << name();
}
/*--------------------------------------------------------------------------*/
/* A.1.2        2.9.1, 6.2
+ module_declaration ::=
+	  {attribute_instance}  module_keyword  module_identifier
-		[ module_parameter_port_list ]
+		list_of_ports ";" { module_item }
+	  "endmodule"
-	| {attribute_instance}  module_keyword  module_identifier
-		[ module_parameter_port_list ]
-		[ list_of_port_declarations ] ";" { non_port_module_item }
-	  "endmodule"
+ module_keyword ::=
+	  "module"
+	| "macromodule"
+	| "connectmodule"
// A.1.3
- module_parameter_port_list ::=		// not really a "port" list
-	  "# (" parameter_declaration  { "," parameter_declaration }  ")"
+ list_of_ports ::=
+	  "("  port  { "," port }  ")"
- list_of_port_declarations ::=
-	  "("  port_declaration  { "," port_declaration }  ")"
-	| "("  ")"
+ port_declaration ::=
+	  {attribute_instance}  inout_declaration
+	| {attribute_instance}  input_declaration
+	| {attribute_instance}  output_declaration
// A.1.4
+ module_item ::=
+	  port_declaration  ";"
+	| non_port_module_item
+ module_or_generate_item ::=
+	  {attribute_instance}  module_or_generate_item_declaration
+	| {attribute_instance}  local_parameter_declaration  ";"
-	| {attribute_instance}  parameter_override
-	| {attribute_instance}  continuous_assign
-	| {attribute_instance}  gate_instantiation
-	| {attribute_instance}  udp_instantiation
+	| {attribute_instance}  module_instantiation
-	| {attribute_instance}  initial_construct
-	| {attribute_instance}  always_construct
-	| {attribute_instance}  loop_generate_construct
-	| {attribute_instance}  conditional_generate_construct
-	| {attribute_instance}  analog_construct
+ module_or_generate_item_declaration ::=
?	  net_declaration
-	| reg_declaration
-	| integer_declaration
-	| real_declaration
-	| time_declaration
-	| realtime_declaration
-	| event_declaration
-	| genvar_declaration
-	| task_declaration
-	| function_declaration
-	| branch_declaration
-	| analog_function_declaration
+ non_port_module_item ::=
+	  module_or_generate_item
-	| generate_region
-	| specify_block
+	| {attribute_instance}  parameter_declaration  ";"
-	| {attribute_instance}  specparam_declaration
-	| aliasparam_declaration
- parameter_override ::=
-	  "defparam"  list_of_param_assignments  ";"
// A.2.1.2
+ inout_declaration ::=
+	  "inout"
-		[discipline_identifier] [net_type | "wreal"] ["signed"] [range]
+		list_of_port_identifiers
+ input_declaration ::=
+	  "input"
-		[discipline_identifier] [net_type | "wreal"] ["signed"] [range]
+		list_of_port_identifiers
+ output_declaration ::=
+	  "output"
-		[discipline_identifier] [net_type | "wreal"] ["signed"] [range]
+		list_of_port_identifiers
+	| "output"
-		[discipline_identifier]  "reg"  ["signed"] [range]
+		list_of_port_identifiers
+	| "output"
-		output_variable_type
+		list_of_port_identifiers


net_declaration ::=
- net_type [ discipline_identifier ] [ signed ]
- [ delay3 ] list_of_net_identifiers ;
- | net_type [ discipline_identifier ] [ drive_strength ] [ signed ]
- [ delay3 ] list_of_net_decl_assignments ;
- | net_type [ discipline_identifier ] [ vectored | scalared ] [ signed ]
- range [ delay3 ] list_of_net_identifiers ;
- | net_type [ discipline_identifier ] [ drive_strength ] [ vectored | scalared ] [ signed ]
- range [ delay3 ] list_of_net_decl_assignments ;
- | trireg [ discipline_identifier ] [ charge_strength ] [ signed ]
- [ delay3 ] list_of_net_identifiers ;
- | trireg [ discipline_identifier ] [ drive_strength ] [ signed ]
- [ delay3 ] list_of_net_decl_assignments ;
- | trireg [ discipline_identifier ] [ charge_strength ] [ vectored | scalared ] [ signed ]
- range [ delay3 ] list_of_net_identifiers ;
- | trireg [ discipline_identifier ] [ drive_strength ] [ vectored | scalared ] [ signed ]
- range [ delay3 ] list_of_net_decl_assignments ;
~ | discipline_identifier [ range ] list_of_net_identifiers ;
- | discipline_identifier [ range ] list_of_net_decl_assignments ;
- | wreal [ discipline_identifier ] [ range] list_of_net_identifiers ;
- | wreal [ discipline_identifier ] [ range] list_of_net_decl_assignments ;
- | ground [ discipline_identifier ] [ range ] list_of_net_identifiers ;
*/
void Module::parse(CS& file)
{
  // do we need a second pass? or just connect the dots while reading in?
  _ports.set_owner(this);
  _input.set_owner(this);
  _output.set_owner(this);
  _inout.set_owner(this);
  _ground.set_owner(this);
  _disc_assign.set_owner(this);
  _parameters.set_owner(this);
  _local_params.set_owner(this);
  _element_list.set_owner(this);
  _local_nodes.set_owner(this);
  // _tr_eval.set_owner(this);
  _validate.set_owner(this);

  // file >> "module |macromodule |connectmodule "; from caller
  file >> _identifier >> _ports >> ';';
  assert(_parameters.ctx() == this);


  Block* root_scope = ctx();
  File const* root = prechecked_cast<File const*>(ctx());
  assert(root);

  size_t here = file.cursor();
  bool end = false;
  for (;;) {
    ONE_OF	// module_item
      || (file >> _attribute_dummy)
      || file.umatch(";")
      // mi, port_declaration
      || ((file >> "input ") && (file >> _input))
      || ((file >> "output ") && (file >> _output))
      || ((file >> "inout ") && (file >> _inout))
      // mi, npmi, mogi, mogid
      || ((file >> "ground ") && (file >> _ground))
      || ((file >> "branch ") && (file >> _branches))
      // net_declaration
      || (file >> _disc_assign)
      // mi, npmi, mogi, module_or_generate_item_declaration
      // mi, npmi, module_or_generate_item
      || ((file >> "localparam ") && (file >> _local_params))
      || ((file >> "analog ") && parse_analog(file)) // TODO:: file >> analog
      // mi, non_port_module_item
      || ((file >> "parameter ") && (file >> _parameters))
      || ((file >> "endmodule ") && (end = true))
      || (file >> _element_list)	// module_instantiation
      ;
    if (end){
      break;
    }else if (!file.more()) {
      file.warn(0, "premature EOF (module)");
      break;
    }else if (file.stuck(&here)) {
      file.warn(0, "bad module");
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void Module::dump(std::ostream& o)const
{
  o << "module " << identifier() << ports() << ";\n";
  if(input().size()){
    o << "  input "	    << input()			<< "\n";
  }else{
  }
  if(output().size()){
    o << "  output "	    << output()			<< "\n";
  }else{
  }
  if(inout().size()){
    o << "  inout "	    << inout()			<< "\n";
  }else{
  }
  if(ground().size()){
    o << "  ground "	    << ground()			<< "\n";
  }else{
  }
  o << disc_assign();
  if(parameters().size()){
    o << ind << "// params\n";
    o << parameters() << "\n";
  }else{
  }
  if(local_params().size()){
    o << ind << "// local params\n";
    o << local_params() << "\n";
  }else{
  }
  if(circuit().size()){
    o << ind << "// circuit\n";
    o << circuit() << "\n";
  }else{
  }

  for(auto i: analog_list()){
    o << *i << "\n";
  }
//  for(auto i: *this){
//    o << *i << "\n";
//  }

  o << "endmodule\n";
}
/*--------------------------------------------------------------------------*/
CS& Module::parse_analog(CS& cmd)
{
  AnalogBlock* ab = new AnalogBlock();
  ab->set_owner(this);
  ab->parse(cmd);
  _analog_list.push_back(ab);

  return cmd;
}
/*--------------------------------------------------------------------------*/
void AnalogBlock::dump(std::ostream& o)const
{
  o << ind << "analog begin\n";
  for(auto i: *this){
    // indent x();
    o << ind << *i << "\n";
  }
  o << ind << "end\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
