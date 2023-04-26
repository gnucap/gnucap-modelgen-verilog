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
  assert(owner());
  _list_of_parameter_assignments.set_owner(owner());
  _list_of_port_connections.set_owner(owner());

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
      << ports() << ";\n";
}
/*--------------------------------------------------------------------------*/
/* A.4.1	6.2.2
- ordered_parameter_assignment ::= expression
+ named_parameter_assignment ::=
+	  "." parameter_identifier "(" [ mintypmax_expression ] ")"
-	| "." system_parameter_identifier "(" [ constant_expression ] ")"
*/
void Parameter_3::parse(CS& file)
{
  _default_val.set_owner(owner());
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
  _default_val.set_owner(owner());
  file >> ','; // ??
  file >> _name;
  trace2("Parameter2", _name, file.tail());
  file >> '=' >> _default_val;
  assert(owner());
  owner()->new_var_ref(this);
  assert(owner()->resolve(name()));
}
/*--------------------------------------------------------------------------*/
void Parameter_2::dump(std::ostream& o)const
{
  o << name() << " = " << default_val(); // range?
}
/*--------------------------------------------------------------------------*/
/*
+ parameter_declaration ::=
- parameter [ signed ] [ range ] list_of_param_assignments
+ | parameter parameter_type list_of_param_assignments
*/
void Parameter_2_List::parse(CS& file)
{
  _is_local = file.last_match()[0]=='l';
  if(file.umatch("real")){
    _type = std::string("real"); // TODO: enum
  }else if(file.umatch("integer")){
    _type = std::string("integer"); // TODO: enum
  }else{
    throw Exception_CS("parameter: need \"real\", \"integer\"\n", file);
  }
  std::string type = _type.to_string();
  trace2("Parameter_2_List", _type, _is_local);
  LiSt<Parameter_2, '\0', ',', ';'>::parse(file);
  for(auto& i : *this){
    i->set_type(type);
  }
}
/*--------------------------------------------------------------------------*/
void Variable_List::parse(CS& file)
{
  _type = file.last_match();
  LiSt<Variable_2, '\0', ',', ';'>::parse(file);
}
/*--------------------------------------------------------------------------*/
void Parameter_2_List::dump(std::ostream& o)const
{
  if(is_local()){
    o__ "localparam";
  }else{
    o__ "parameter";
  }
  // "specparam" ...
  o << " " << _type << " ";
  LiSt<Parameter_2, '\0', ',', ';'>::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Variable_List::dump(std::ostream& o)const
{
  o__ _type << " ";
  LiSt<Variable_2, '\0', ',', ';'>::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Variable_List_Collection::dump(std::ostream& o)const
{
  for(auto const& i : *this){ untested();
    i->dump(o);
  }
  // Collection<Parameter_2_List>::dump(o);
}
/*--------------------------------------------------------------------------*/
void Parameter_List_Collection::dump(std::ostream& o)const
{
  for(auto const& i : *this){
    i->dump(o);
  }
  // Collection<Parameter_2_List>::dump(o);
}
/*--------------------------------------------------------------------------*/
// void Localparam_List_Collection::dump(std::ostream& o)const
// {
//   for(auto const& i : *this){
//     o__ "localparam ";
//     i->dump(o);
//   }
//   // Collection<Localparam_List>::dump(o);
// }
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
  if (file >> '.') {
    _name  = file.ctos("(", "", "");
    _value = file.ctos(",)", "(", ")");
  }else{
    file >> _name; // value?
  }
  if(owner()){
    owner()->new_node(value());
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Port_Connection_List::parse(CS& f)
{
  assert(owner());
  LiSt<Port_3, '(', ',', ')'>::parse(f);

  auto i = begin();
  if(i!=end()){
    _has_names = (*i)->has_identifier();
    ++i;
  }else{
  }

  for(; i!=end(); ++i ){
    if(_has_names == (*i)->has_identifier()){
    }else{
      throw Exception("Cannot mix port connections");
    }
  }
}
/*--------------------------------------------------------------------------*/
bool Port_3::has_identifier() const
{
  return _value != "";
}
/*--------------------------------------------------------------------------*/
void New_Port::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
  assert(_owner);
  _owner->new_node(name());
}
/*--------------------------------------------------------------------------*/
void Net_Declarations::parse(CS& f)
{
  assert(owner()); // Module
  Block const* root_scope = owner()->owner();
  assert(root_scope);
  File const* root = prechecked_cast<File const*>(root_scope);
  assert(root);
  auto ii = root->discipline_list().find(f);

  if(ii!=root->discipline_list().end()){
//    size_t here = f.cursor();
    auto m = new Net_Decl_List_Discipline();
    m->set_discipline(*ii);

    m->set_owner(owner());
    f >> *m;
    for(auto i : *m){
      i->set_discipline(*ii);
    }

    push_back(m);

  }else if(f.umatch("ground ")){
    auto m = new Net_Decl_List_Ground();
    m->set_owner(owner());
    f >> *m;
    push_back(m);
  }else{
    assert(!f);
  }
}
/*--------------------------------------------------------------------------*/
void Net_Declarations::dump(std::ostream& o) const
{
  Collection<Net_Decl_List>::dump(o);
}
/*--------------------------------------------------------------------------*/
// void Port_Discipline_List_Collection::dump(std::ostream& out)const
// {
//   Collection<Port_Discipline_List>::dump(out);
// }
/*--------------------------------------------------------------------------*/
// 3.6.4 Ground declaration
// Each ground declaration is associated with an already declared net of continuous discipline. The node asso-
// ciated with the net will be the global reference node in the circuit. The net must be assigned a continuous
// discipline to be declared ground.
void Net_Decl_List_Ground::parse(CS& f)
{
  return Net_Decl_List::parse_n_<Net_Identifier_Ground>(f);
}
/*--------------------------------------------------------------------------*/
void Net_Identifier_Discipline::parse(CS& f)
{
  Net_Identifier::parse(f);

  assert(owner());
  set_node( owner()->new_node(name()));
}
/*--------------------------------------------------------------------------*/
void Net_Identifier_Ground::parse(CS& f)
{
  Net_Identifier::parse(f);
  if(owner()->node(name())){
  }else{ untested();
    throw Exception_CS("ground: need previously declared net", f);
  }
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Ground::dump(std::ostream& o)const
{
  o__ "ground ";
  Net_Decl_List::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Discipline::parse(CS& f)
{
  trace1("Port_Disc_List::parse", f.last_match());
  return Net_Decl_List::parse_n_<Net_Identifier_Discipline>(f);
  // return Net_Identifier, '\0', ',', ';'>::parse(f);
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Discipline::dump(std::ostream& o)const
{
  assert(_disc);
  o__ _disc->identifier() << " ";
  Net_Decl_List::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Net_Identifier::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
}
/*--------------------------------------------------------------------------*/
void Net_Identifier::set_discipline(Discipline const* d)
{
  _node->set_discipline(d);
}
/*--------------------------------------------------------------------------*/
void Port_3::dump(std::ostream& out)const
{
  if(has_identifier()){
    out << "." << name() << "(" << value() << ")";
  }else{
    out << name();
  }
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
+	  net_declaration
-	| reg_declaration
-	| integer_declaration
+	| real_declaration
-	| time_declaration
-	| realtime_declaration
-	| event_declaration
-	| genvar_declaration
-	| task_declaration
-	| function_declaration
+	| branch_declaration
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
  _net_decl.set_owner(this);
  _branch_decl.set_owner(this);
  _variables.set_owner(this);
  _parameters.set_owner(this);
  //_local_params.set_owner(this);
  _element_list.set_owner(this);
  _local_nodes.set_owner(this);
  // _tr_eval.set_owner(this);
  _validate.set_owner(this);

  // file >> "module |macromodule |connectmodule "; from caller
  file >> _identifier >> _ports >> ';';
  assert(_parameters.owner() == this);


//  Block* root_scope = owner();
  File const* root = prechecked_cast<File const*>(owner());
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
      // net_declaration
//      || (( root->disciplines().match(file) ) && (file >> _disc_assign))
//      || (file >> _node_assignments)
      || (file >> _net_decl)
      || ((file >> "ground ") && (file >> _net_decl))
      || ((file >> "branch ") && (file >> _branch_decl))
      // mi, non_port_module_item
      // mi, npmi, mogi, module_or_generate_item_declaration
      // mi, npmi, module_or_generate_item
//      || ((file >> "localparam ") && (file >> _local_params))
      || ((file >> "real ") && (file >> _variables))
      // || ((file >> "integer ") && (file >> _variables))
      || ((file >> "parameter ") && (file >> _parameters))
      || ((file >> "localparam ") && (file >> _parameters))
      || ((file >> "analog ") && parse_analog(file)) // TODO:: file >> analog
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
/*
// A.2.1.3
+ branch_declaration ::=
+ branch ( branch_terminal [ , branch_terminal ] ) list_of_branch_identifiers ;
- | port_branch_declaration
- 
- // A.2.3
- port_branch_declaration ::=
- branch ( < port_identifier > ) list_of_branch_identifiers ;
- | branch ( < hierarchical_port_identifier > ) list_of_branch_identifiers ;
- branch_terminal ::=
- net_identifier
- | net_identifier [ constant_expression ]
- | net_identifier [ constant_range_expression ]
- | hierarchical_net_identifier
- | hierarchical_net_identifier [ constant_expression ]
- | hierarchical_net_identifier [ constant_range_expression ]
- list_of_branch_identifiers ::=
- branch_identifier [ range ] { , branch_identifier [ range ] }
*/
/*--------------------------------------------------------------------------*/
void Branch_Declaration::dump(std::ostream& o) const
{
  o__ "branch ";
  Branch_Ref::dump(o);
  o << " " << _list << "\n";
}
/*--------------------------------------------------------------------------*/
void Branch_Declaration::parse(CS& f)
{
  assert(owner());
  _list.set_owner(owner());
  Branch_Ref::parse(f);
  assert(owner());
  f >> _list;
  for(auto const& i : _list) {
    owner()->new_branch_name(i->to_string(), *this);
  }
}
/*--------------------------------------------------------------------------*/
void Branch_Declarations::parse(CS& f)
{
  Collection<Branch_Declaration>::parse(f);
}
/*--------------------------------------------------------------------------*/
void List_Of_Branch_Identifiers::dump(std::ostream& o)const
{
  LiSt<Branch_Identifier, '\0', ',', ';'>::dump(o);
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::parse(CS& f)
{
  f >> "(";
  std::string pp = f.ctos(",)");
  std::string pn = f.ctos(",)");
  f >> ")";

  assert(owner());
  trace3("Branch_Ref::parse", pp, pn, _br);
  assert(!_br);
  Branch_Ref b;
  assert(!b._br);
  b = owner()->new_branch(pp, pn);

//  assert(b->has(&b));
  assert(b._br);

  *this = b;
  assert(owner());
}
/*--------------------------------------------------------------------------*/
void Branch_Ref::dump(std::ostream& o)const
{
  incomplete();
  o << "(" << pname() << ", " << nname() << ")";
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
  o << net_declarations();
  o << branch_declarations();
  if(parameters().size()){
    o << parameters() << "\n";
  }else{
  }
  if(variables().size()){
    o << variables() << "\n";
  }else{
  }
//  if(local_params().size()){
//    o << local_params() << "\n";
//  }else{
//  }
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
void Variable_2::parse(CS& file)
{
  file >> ','; // ??
  file >> _name;
  new_var_ref();
}
/*--------------------------------------------------------------------------*/
void Variable_2::dump(std::ostream& o)const
{
  o__ name();
}
/*--------------------------------------------------------------------------*/
void Variable::dump(std::ostream& o)const
{
  unreachable(); // ?
//  o__ _type << " " << name() << ";\n";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
