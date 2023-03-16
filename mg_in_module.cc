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
  out << "  " << module_or_paramset_identifier()
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
void New_Port::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
  assert(_ctx);
  _ctx->new_node(name());
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
*/
void Module::parse(CS& file)
{ untested();
  // file >> "module |macromodule |connectmodule "; from caller
  file >> _identifier >> _ports >> ';';
  assert(_parameters.ctx() == this);

  trace1("Module::parse", _identifier);

  size_t here = file.cursor();
  for (;;) {
    ONE_OF	// module_item
      || file.umatch(";")
      // mi, port_declaration
      || ((file >> "input ") && (file >> _input))
      || ((file >> "output ") && (file >> _output))
      || ((file >> "inout ") && (file >> _inout))
      // mi, npmi, mogi, mogid, net_declaration
      || ((file >> "ground ") && (file >> _ground))
      || ((file >> "electrical ") && (file >> _electrical))
      || ((file >> "parameter ") && (file >> _parameters))
      || ((file >> "localparam ") && (file >> _local_params))
      ;
    if (file.stuck(&here)) { untested();
      break;
    }else{ untested();
    }
  }
  bool end=false;
  for (;;) { untested();
    ONE_OF	// non-port module_item
      || file.umatch(";")
      // mi, npmi, mogi, module_or_generate_item_declaration
      // mi, npmi, module_or_generate_item
      || ((file >> "localparam ") && (file >> _local_params))
      || ((file >> "analog ") && parse_analog(file))
      // mi, non_port_module_item
      || ((file >> "parameter ") && (file >> _parameters))
      || ((file >> "endmodule ") && (end = true))
      || (file >> _element_list)	// module_instantiation
      ;
    if (end){ untested();
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
  if(electrical().size()){
    o << "  electrical "  << electrical()		<< "\n";
  }else{
  }
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

  for(auto i: *this){ untested();
    o << *i << "\n";
  }

  o << "endmodule\n";
}
/*--------------------------------------------------------------------------*/
CS& Module::parse_analog(CS& cmd)
{ untested();
  AnalogBlock* ab = new AnalogBlock();
  ab->set_ctx(this);
  ab->parse(cmd);
  push_back(ab);
  untested();

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
Branch const* Module::new_branch(std::string const& p, std::string const& n)
{
  std::string k = p + " " + n;
  Branch*& cc = _branches[k];
  if(cc) { untested();
  }else{
    size_t s = _branches.size() - 1;
    // TODO: resolve k
    cc = new Branch(p, n);
    //      cc->deps().insert(cc)??
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Probe const* Module::new_probe(std::string const& xs, std::string const& p, std::string const& n)
{
  std::string k = xs + "_" + p + "_" + n;
  Node const* pp = node(p);
  Node const* pn = node(n);
  Probe*& prb = _probes[k];
  if(prb) {
  }else{
    size_t s = _probes.size() - 1;
    // TODO: resolve k
    prb = new Probe("prb_" + std::to_string(s), xs, p, n);
    //      prb->deps().insert(prb)??
  }
  return prb;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
