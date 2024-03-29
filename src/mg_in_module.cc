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
#include "mg_error.h"
#include "mg_out.h"
#include "mg_in.h"
#include "mg_options.h"
#include "mg_analog.h" // BUG: Analog_Function_Arg
#include "l_stlextra.h"
/*--------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------*/
void Element_2::parse(CS& file)
{
  assert(owner());
  _list_of_parameter_assignments.set_owner(owner());
  _list_of_port_connections.set_owner(owner());

  file >> _module_or_paramset_identifier
       >> '#' >> _list_of_parameter_assignments
       >> _name_of_module_instance >> _list_of_port_connections >> ';';

  Module* mod = dynamic_cast<Module*>(owner());
  if(mod /* && owner()->is_always() */){
    for(auto const& p : _list_of_port_connections){
      assert(mod->node(p->node()));
      mod->node(p->node())->connect(this);
    }
  }else{ untested();
    assert(0);
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
void Element_2::dump(std::ostream& out)const
{
  out << "  " << dev_type()
      << " #" << list_of_parameter_assignments() << " "
      << short_label()
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
  _default_val.resolve();
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
+ parameter_type ::=
+	  "integer"
+	| "real"
-	| "realtime"
-	| "time"
-	| "string"
+ aliasparam_declaration ::=
+	  "aliasparam" parameter_identifier "=" parameter_identifier ";"
// A.2.3
+ list_of_param_assignments ::=
+	  param_assignment { "," param_assignment }
// A.2.4
+ param_assignment ::=
+	  parameter_identifier "=" constant_mintypmax_expression { value_range }
-	| parameter_identifier range "=" constant_arrayinit { value_range }
*/
void Parameter_2::parse(CS& f)
{
  assert(owner());
  //assert(dynamic_cast<Parameter_2_List const*>(owner()));
  // file >> "parameter "; from caller
  _default_val.set_owner(owner());
  _value_range_list.set_owner(owner());

  size_t here = f.cursor();
  f >> ','; // ??
  f >> _name;

  assert(dynamic_cast<Module const*>(owner()));
  if(owner()->lookup(_name, false)){
    throw Exception_CS_("already declared", f, here);
  }else{
    trace3("not there", _name, name(), dynamic_cast<Module const*>(owner()) );
  }

  f >> '=' >> _default_val;
  f >> _value_range_list;

  assert(owner());
}
/*--------------------------------------------------------------------------*/
void Parameter_2::resolve()
{
  _default_val.resolve();

#if 1
  try{
     owner()->new_var_ref(this);
  }catch(Exception const&){
    //unreachable();
    //  assert(owner()->lookup(name()));
     throw Exception("resolve: already declared " + name()); // , file); // BUG: cs?
  }
#endif

  assert(owner()->lookup(name())
       ||owner()->lookup(name().substr(2)));
}
/*--------------------------------------------------------------------------*/
void Parameter_2::dump(std::ostream& o)const
{
  o << name() << " = " << default_val();
  if(!_value_range_list.is_empty()){
    o << _value_range_list;
  }else{
  }
  //}else{ untested();
  //}
}
/*--------------------------------------------------------------------------*/
void Parameter_2::add_alias(Aliasparam const* a)
{
  _aliases.push_back(a);
}
/*--------------------------------------------------------------------------*/
void Aliasparam::parse(CS& f)
{
  std::string paramname;
  size_t here0 = f.cursor();
  f >> _name >> "=";
  size_t here = f.cursor();
  f >> paramname >> ";";
  assert(owner());
  Module* m = prechecked_cast<Module*>(owner());
  assert(m);
  auto& p = m->parameters();
  Parameter_2* pp = NULL;
  for(auto pl : p){
    auto it = notstd::find_ptr(pl->begin(), pl->end(), paramname);
    if(pp){
    }else if(it != pl->end()){
      pp = *it;
    }else{
    }
    it = notstd::find_ptr(pl->begin(), pl->end(), _name);
    if(it != pl->end()){
      f.reset(here0);
      throw Exception_CS_("already exists", f);
    }else{
    }
  }

  if(pp){
    pp->add_alias(this);
    _param = pp;
    // owner()->new_var_ref(this); // nope. does not create symbol acc to LRM
  }else{
    f.reset(here);
    throw Exception_CS_("no such parameter", f);
  }
}
/*--------------------------------------------------------------------------*/
void Aliasparam::dump(std::ostream& o)const
{
  o__ "aliasparam " << _name << " = " << param_name() << ";\n";
}
/*--------------------------------------------------------------------------*/
void Data_Type::parse(CS& file)
{
  if(file.umatch("real")){
    _type = t_real;
  }else if(file.umatch("integer")){ untested();
    _type = t_int;
  }else{ untested();
    throw Exception_CS_("need \"real\", \"integer\"\n", file);
  }
}
/*--------------------------------------------------------------------------*/
void Data_Type::dump(std::ostream& o)const
{
  static std::string names[] = {"unknown", "real", "integer"};
  o << names[_type];
}
/*--------------------------------------------------------------------------*/
/*
+ parameter_declaration ::=
- parameter [ signed ] [ range ] list_of_param_assignments
+ | parameter parameter_type list_of_param_assignments
*/
void Parameter_2_List::parse(CS& file)
{
  Module* m = prechecked_cast<Module*>(owner());
  assert(m);
  if(m->attribute_stash().is_empty()){
  }else{
    assert(!_attributes);
    set_attributes(m->attribute_stash().detach());
  }

  assert(file.last_match().size());

  _is_local = file.last_match()[0]=='l';
//  file >> _type;
  if(file.umatch("real")){
    _type = std::string("real"); // TODO: enum
  }else if(file.umatch("integer")){
    _type = std::string("integer"); // TODO: enum
  }else{
    throw Exception_CS_("parameter: need \"real\", \"integer\"\n", file);
  }
  std::string type = _type.to_string();
  trace2("Parameter_2_List", _type, _is_local);

  LiSt<Parameter_2, '\0', ',', ';'>::parse(file);
  for(auto& i : *this){
    i->set_type(type);
    i->set_local(_is_local);
    i->resolve();
  }
}
/*--------------------------------------------------------------------------*/
void Variable_List::parse(CS& file)
{
  assert(owner());
  Module* mod = prechecked_cast<Module*>(owner());
  if(mod){
    if(mod->attribute_stash().is_empty()){
    }else{
      set_attributes(mod->attribute_stash().detach());
    }
  }else{ untested();
  }

  char t = file.last_match()[0];
  if(t=='r') {
    _type = Data_Type_Real();
  }else if(t=='i') {
    _type = Data_Type_Int();
  }else{ untested();
    throw Exception_CS_("What type? " + t, file);
  }

  LiSt<Variable_Decl, '\0', ',', ';'>::parse(file);
  if(_type.is_real()) {
    for (auto x : *this){
      x->set_type(Data_Type_Real());
    }
  }else if(_type.is_int()) {
    for (auto x : *this){
      x->set_type(Data_Type_Int());
    }
  }else{ untested();
    unreachable();
  }
  if(has_attributes()){
    for (auto x : *this){
      x->set_attributes(&attributes());
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Parameter_2_List::dump(std::ostream& o)const
{
  if(has_attributes()){
    o << attributes();
  }else{
  }

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
  if(has_attributes()){
    o << attributes();
  }else{
  }

  o__ _type << " ";
  LiSt<Variable_Decl, '\0', ',', ';'>::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Variable_List_Collection::parse(CS& f)
{
#if 0
  assert(owner());
  Module* mod = prechecked_cast<Module*>(owner());
  if(mod){ untested();
    if(mod->attribute_stash().is_empty()){ untested();
    }else if(size()){ untested();
      set_attributes(mod->attribute_stash().detach());
    }else{ untested();
    }
  }else{ untested();
  }
#endif
//  char t=f.last_match()[0];
  Collection<Variable_List>::parse(f);

//  if(t=='i'){ untested();
//    for(auto x:*this){ untested();
//      x->set_int();
//    }
//  }else{ untested();
//  }
}
/*--------------------------------------------------------------------------*/
void Variable_List_Collection::dump(std::ostream& o)const
{
  for(auto const& i : *this){
    i->dump(o);
  }
  // Collection<Parameter_2_List>::dump(o);
}
/*--------------------------------------------------------------------------*/
size_t Parameter_List_Collection::count_nonlocal() const
{
  size_t sum = 0;
  for(auto const& i : *this){
    if(i->is_local()){
    }else{
      sum += i->size();
    }
  }
  return sum;
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
// { untested();
//   for(auto const& i : *this){ untested();
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
    _node = owner()->new_node(value());
  }else{
  }
}
/*--------------------------------------------------------------------------*/
Port_3* Module::find_port(std::string const& s)
{
  assert(_circuit);
  return _circuit->find_port(s);
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
void Net_Declarations::dump(std::ostream& o) const
{
  Collection<Net_Decl_List>::dump(o);
}
/*--------------------------------------------------------------------------*/
// void Port_Discipline_List_Collection::dump(std::ostream& out)const
// { untested();
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
  assert(owner());
  Net_Identifier::parse(f);
  Module* mod = prechecked_cast<Module*>(owner());
  assert(mod);
  Node_Ref const& nn = owner()->node(name());
  if(nn) {
    set_node(mod->node(nn));
  }else{ untested();
    throw Exception_CS_("ground: need previously declared net", f);
  }

  Module* m = prechecked_cast<Module*>(owner());
  assert(m);
  assert(node());
  m->set_to_ground(node());
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
  // o__ "";
  if(has_attributes()){
    o << attributes();
  }else{
  }
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
void Port_3::set_discipline(Discipline const* d, Module* owner)
{
  owner->node(_node)->set_discipline(d);
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
+	| analog_function_declaration
+ non_port_module_item ::=
+	  module_or_generate_item
-	| generate_region
-	| specify_block
+	| {attribute_instance}  parameter_declaration  ";"
-	| {attribute_instance}  specparam_declaration
+	| aliasparam_declaration
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
void Module::parse(CS& f)
{
  assert(_circuit);
  _circuit->set_owner(this);
  File* o = prechecked_cast<File*>(owner());
  if(!o){ untested();
    incomplete();
  }else if(o->attribute_stash().is_empty()){
  }else{
    assert(!_attributes);
    set_attributes(o->attribute_stash().detach());
  }
  // f >> "module |macromodule |connectmodule "; from caller
  f >> _identifier;
  f >> *_circuit; // HACK
  _circuit->parse_ports(f);
  f >> ';';
  parse_body(f);
  setup_nodes();
}
/*--------------------------------------------------------------------------*/
void Module::parse_body(CS& f)
{
  assert(_circuit);
  _circuit->set_owner(this);
  f >> *_circuit; // todo;
		 //
  _variables.set_owner(this);
  _parameters.set_owner(this);
  _aliasparam.set_owner(this);
  //_local_params.set_owner(this);
  //_local_nodes.set_owner(this);
  assert(_analog);
  _analog->set_owner(this);
  // _tr_eval.set_owner(this);
  // _validate.set_owner(this);


  size_t here = f.cursor();
  bool end = false;
  _attribute_stash.set_owner(this);
  for (;;) {
    trace1("module body parse", f.tail().substr(0,20));
    while (f >> _attribute_stash) { }
    ONE_OF	// module_item
      || f.umatch(";")
      // mi, port_declaration
      // || (f >> _circuit)
      || ((f >> "input ") && (f >> _circuit->input()))
      || ((f >> "output ") && (f >> _circuit->output()))
      || ((f >> "inout ") && (f >> _circuit->inout()))
      // mi, npmi, mogi, mogid
      // net_declaration
      || (f >> _circuit->net_decl())
      || ((f >> "ground ") && (f >> _circuit->net_decl())) // really?
      // mi, non_port_module_item
      // mi, npmi, mogi, module_or_generate_item_declaration
      || ((f >> "branch ") && (f >> _circuit->branch_decl()))
//      || ((f >> "analog function ") && (f >> _analog_functions))
      // mi, npmi, module_or_generate_item
//      || ((f >> "localparam ") && (f >> _local_params))
      || ((f >> "real ") && (f >> _variables))
      || ((f >> "integer ") && (f >> _variables))
      || ((f >> "parameter ") && (f >> _parameters))
      || ((f >> "localparam ") && (f >> _parameters))
      || ((f >> "aliasparam ") && (f >> _aliasparam))
      || ((f >> "analog ") && f >> *_analog)
      || ((f >> "endmodule ") && (end = true))
      || (f >> _circuit->element_list())	// module_instantiation
      ;
    if (_attribute_stash.is_empty()){
    }else{ untested();
      f.warn(0, "dangling attributes");
    }
    if (end){
      break;
    }else if (!f.more()) { untested();
      f.warn(0, "premature EOF (module)");
      break;
    }else if (f.stuck(&here)) {
      throw Exception_CS_("bad module", f);
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
  o__ "branch " << _br; // dump(o);
  o << " " << _list << "\n";
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch_name(std::string const& n, Branch_Ref const& b)
{
  return _circuit->branches().new_branch(b, n);
}
/*--------------------------------------------------------------------------*/
Branch_Ref parse_branch(Block* owner, CS& f); // mg_in_analog.
void Branch_Declaration::parse(CS& f)
{
  assert(owner());
  _list.set_owner(owner());
//   Branch_Ref::parse(f);
  _br = parse_branch(owner(), f);
  f >> _list;
  for(auto const& i : _list) {
    owner()->new_branch_name(i->to_string(), _br);
  }
  trace2("BD::parse", f.tail().substr(0,20), (bool)f);
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
void Module::dump(std::ostream& o)const
{
  if(dynamic_cast<Paramset const*>(this)){
    if (!options().dump_paramset()) { untested();
      o << "// omit ps\n";
      return;
    }else{
    }
  }else if (!options().dump_module()) { untested();
    o << "// omit module\n";
    return;
  }else{
  }

  if(has_attributes()){
    o << attributes();
  }else{
  }
  indent x;
  o << "module " << identifier() << _circuit->ports() << ";\n";
  if(_circuit->input().size()){
    o__ "input " << circuit()->input() << "\n";
  }else{
  }
  if(_circuit->output().size()){
    o__ "output " << circuit()->output() << "\n";
  }else{
  }
  if(_circuit->inout().size()){
    o__ "inout " << circuit()->inout() << "\n";
  }else{
  }
  if(_circuit->ground().size()){ untested();
    o__ "ground " << circuit()->ground() << "\n";
  }else{
  }
  o << _circuit->net_decl();
  o << _circuit->branch_decl();
  if(parameters().size()){
    o << parameters() << "\n";
  }else{
  }
  if(aliasparam().size()){
    o << aliasparam() << "\n";
  }else{
  }
  if(variables().size()){
    o << variables() << "\n";
  }else{
  }
//  if(local_params().size()){ untested();
//    o << local_params() << "\n";
//  }else{ untested();
//  }
  if(circuit()->element_list().size()){
//    o__ "// circuit\n";
    o << circuit()->element_list() << "\n";
  }else{
  }

  assert(_analog);
  _analog->dump(o);

  o << "endmodule\n";
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::parse(CS& file)
{
  file >> ','; // ?? BUG.
  assert(owner());
  assert(!_data);
  assert(!_token);
  std::string name;
  file >> name;

  _data = new TData();
  _token = new Token_VAR_REF(name, this, _data);
  trace1("variable decl", name);
  owner()->new_var_ref(_token);
}
/*--------------------------------------------------------------------------*/
void ValueRangeInterval::parse(CS& file)
{
  if(file.last_match() == "["){
    _lb_is_closed = true;
  }else if(file.last_match() == "("){
    _lb_is_closed = false;
  }else{ untested();
    unreachable();
  }
  assert(owner());
  _lb.set_owner(owner());
  _ub.set_owner(owner());
  file >> _lb; //_lb = file.ctos(":");
  file.skip1(":");
  file >> _ub; // _ub = file.ctos("])");
  trace1("ValueRangeInterval::parse", file.tail().substr(0,19));

  if(file.match1(']')) {
    file.skip1(']');
    _ub_is_closed = true;
  }else if(file.match1(')')) {
    file.skip1(')');
    _ub_is_closed = false;
  }else{ untested();
    throw Exception_CS_("need ')' or ']'", file);
  }
}
/*--------------------------------------------------------------------------*/
void ValueRangeInterval::dump(std::ostream& o)const
{
  if(_lb_is_closed){
    o << "[";
  }else{
    o << "(";
  }
  o << _lb << ":" << _ub;
  if(_ub_is_closed){
    o << "]";
  }else{
    o << ")";
  }
}
/*--------------------------------------------------------------------------*/
void ValueRange::parse(CS& file)
{
  assert(owner());
  if (file >> "from"){
    _type = vr_FROM;
    if(file >> "[" || file >> "("){
      _what = new ValueRangeInterval;
      _what->set_owner(owner());
      file >> *_what;
    }else{ untested();
      incomplete();
    }
  }else if(file >> "exclude"){
    _type = vr_EXCLUDE;
    if(file >> "[" || file >> "("){ untested();
      _what = new ValueRangeInterval;
      _what->set_owner(owner());
    }else if(file >> "'{"){ untested();
      incomplete();
//      _what = new ValueRangeStrings;
    }else{
      _what = new ValueRangeConstant;
      _what->set_owner(owner());
    }
    file >> *_what;
    trace1("ValueRange::parse b", file.tail().substr(0,10));
  }else{ untested();
  }

}
/*--------------------------------------------------------------------------*/
void ValueRange::dump(std::ostream& o)const
{
  assert(_what);
  static std::string names[] = {"from", "exclude"};
  o << " " << names[_type] << " " << *_what;
}
/*--------------------------------------------------------------------------*/
void ValueRangeConstant::parse(CS& file)
{
  file >> _cexpr;
}
/*--------------------------------------------------------------------------*/
void ValueRangeConstant::dump(std::ostream& o)const
{
  o << _cexpr;
}
/*--------------------------------------------------------------------------*/
void Module::new_var_ref(Base* what)
{
  auto P = dynamic_cast<Parameter_2 const*>(what);
  auto T = dynamic_cast<Token const*>(what);

  std::string p;
  if(auto A = dynamic_cast<Aliasparam const*>(what)){ untested();
    p = A->name();
  }else if(auto nn = dynamic_cast<Node const*>(what)){
    p = nn->name();
  }else if(P){
    p = P->name();
  }else if(T){
    p = T->name();
  }

  if(p!=""){
    auto const& alias = aliasparam();
    // alias.find(p)?
    if(alias.end() == notstd::find_ptr(alias.begin(), alias.end(), p)){
    }else{
      throw(Exception("alias already there: '" + p + "'"));
    }
  }else{ untested();
  }

  return Block::new_var_ref(what);
}
/*--------------------------------------------------------------------------*/
void Block::clear_vars()
{
      // for(Dep d : deps().ddeps()) { untested();
      //   (*d)->unset_used_in(this);
      // }
  // detach?
  _var_refs.clear();
}
/*--------------------------------------------------------------------------*/
/// set_vr_ref?
void Block::new_var_ref(Base* what)
{
  assert(what);
  std::string p;
  auto V = dynamic_cast<Variable const*>(what);
  auto P = dynamic_cast<Parameter_2 const*>(what);
  auto T = dynamic_cast<Token const*>(what);

  if(T){
    p = T->name();
    // assert(T->data());
  }else if(V){ untested();
    p = V->name();
  }else if(P){
    p = P->name();
  }else if(auto A = dynamic_cast<Aliasparam const*>(what)){ untested();
    p = A->name();
  }else if(auto ps = dynamic_cast<Paramset_Stmt const*>(what)){ untested();
    unreachable();
    p = "."+ps->name();
  }else if(auto nn = dynamic_cast<Node const*>(what)){
    p = nn->name();
  }else{ untested();
    incomplete();
    assert(false);
  }

  trace3("new_var_ref", V, p, dynamic_cast<Paramset const*>(this));

  trace3("new_var_ref, stashing", p, this, dynamic_cast<Module const*>(this));
  Base* cc = _var_refs[p];

  // yikes.
  if(dynamic_cast<Analog_Function_Arg const*>(what)
   &&!dynamic_cast<Analog_Function_Arg const*>(cc)){ untested();
    _var_refs[p] = what;
  }else if(dynamic_cast<Analog_Function_Arg const*>(cc)
   &&!dynamic_cast<Analog_Function_Arg const*>(what)){ untested();
    _var_refs[p] = cc;
  }else if(dynamic_cast<Paramset_Stmt const*>(what)){ untested();
    unreachable();
    _var_refs[p] = what;
  }else if(cc) {
    _var_refs[p] = what;
    if(V){ untested();
      // updating variable...
    }else if(T){
      // updating variable...
      if(auto VT = dynamic_cast<Token_VAR_REF const*>(T)){
	trace2("new_var_ref, update token", p, VT->deps().size());
      }else{ untested();
	trace1("new_var_ref, update token", p);
      }
    }else{ untested();
      throw(Exception("already there: '" + p + "'"));
    }
  }else if(T){
    if(auto VT = dynamic_cast<Token_VAR_REF const*>(T)){
      trace2("new_var_ref, reg new token", p, VT->deps().size());
    }else{ untested();
      trace1("new_var_ref, reg new token", p);
    }
    _var_refs[p] = what;
  }else{
    trace1("new_var_ref, reg new", p);
    _var_refs[p] = what;
  }
}
/*--------------------------------------------------------------------------*/
void Module::push_back(Filter /*const?*/ * f)
{
  assert(_circuit);
  _circuit->push_back(f);
}
/*--------------------------------------------------------------------------*/
void Circuit::push_back(Filter /*const?*/ * f)
{
  _filters.push_back(f);
}
/*--------------------------------------------------------------------------*/
void Module::push_back(FUNCTION_ const* f)
{
  _func.push_back(f);
  install(f);
}
/*--------------------------------------------------------------------------*/
Branch::Branch(Branch_Ref b, Module* m)
    : Element_2(), _p(b->p()), _n(b->n())
{
  set_owner(m);
  new_deps();

  if(b.is_reversed()) {
//      std::swap(_p, _n); // ??
  } else {
  }
  m->node(_n)->connect(this);
  m->node(_p)->connect(this);
}
/*--------------------------------------------------------------------------*/
Branch::Branch(Node_Ref p, Node_Ref n, Module* m)
    : Element_2(), _p(p), _n(n)
{
  set_owner(m);
  assert(p);
  assert(n);
  new_deps();
  //_code_name = "_b_" + p->name() + "_" + n->name();
  //
//  incomplete();
  m->node(n)->connect(this);
  m->node(p)->connect(this);

  // p->inc_use();
  // n->inc_use();
}
/*--------------------------------------------------------------------------*/
Module::~Module()
{
  delete_analog();
  delete _probes; // .clear();

  delete_circuit();
}
/*--------------------------------------------------------------------------*/
bool Node::is_used() const
{
  for(auto e : _fanout){
    if(e->is_used()){
      return true;
    }else{
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
void Node::connect(Element_2 const* e)
{
  _fanout.push_back(e);
}
/*--------------------------------------------------------------------------*/
Node::~Node()
{
  trace1("~Node", code_name());
  // assert(!is_used()); incomplete.
}
/*--------------------------------------------------------------------------*/
void SeqBlock::merge_sens(Sensitivities const& s)
{
  if(_sens){ untested();
  }else{
    _sens = new Sensitivities;
  }
  _sens->merge(s);
}
/*--------------------------------------------------------------------------*/
void SeqBlock::set_sens(Base const* s)
{
  if(_sens){ untested();
  }else{
    _sens = new Sensitivities;
  }
  _sens->add(s);
}
/*--------------------------------------------------------------------------*/
SeqBlock::~SeqBlock()
{
  delete _sens;
  _sens = NULL;
}
/*--------------------------------------------------------------------------*/
void Module::delete_circuit()
{
  delete _circuit;
  _circuit = NULL;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
