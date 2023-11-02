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
#include "mg_options.h"
#include "l_stlextra.h"
/*--------------------------------------------------------------------------*/
#if 0
void Name_String::parse(CS& File)
{ untested();
  File.skipbl();
  _data = "";
  if (File.is_pfloat()) { untested();
    while (File.is_pfloat()) { untested();
      _data += File.ctoc();
    }
    if (File.match1("eE")) { untested();
      _data += File.ctoc();
      if (File.match1("+-")) { untested();
        _data += File.ctoc();
      }else{ untested();
      }
      while (File.is_digit()) { untested();
        _data += File.ctoc();
      }
    }else{ untested();
    }
    while (File.is_alpha()) { untested();
      _data += File.ctoc();
    }
  }else{ untested();
    while (File.is_alpha() || File.is_pfloat() || File.match1("_$")) { untested();
      _data += File.ctoc();
    }
  }
  File.skipbl();
}
#endif
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

  f >> ','; // ??
//  size_t here = f.cursor();
  f >> _name;

  assert(dynamic_cast<Module const*>(owner()));
  if(owner()->lookup(_name, false)){
    throw Exception_CS_("already declared", f);
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

  _type = file.last_match();
  LiSt<Variable_Decl, '\0', ',', ';'>::parse(file);
  if(_type.to_string()[0] == 'i'){
    for (auto x : *this){
      x->set_type(Data_Type_Int());
    }
  }else{
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
  if(root){
  }else{
    //incomplete();
    //f.reset_fail(f.cursor());
    //return;

    root = prechecked_cast<File const*>(root_scope->owner());
  }
  auto ii = root->discipline_list().find(f);
  Net_Decl_List* d = NULL;

  if(ii!=root->discipline_list().end()){
//    size_t here = f.cursor();
    auto m = new Net_Decl_List_Discipline();
    m->set_discipline(*ii);

    m->set_owner(owner());
    f >> *m;
    for(auto i : *m){
      i->set_discipline(*ii);
    }

    d = m;
  }else if(f.umatch("ground ")){
    auto m = new Net_Decl_List_Ground();
    m->set_owner(owner());
    f >> *m;
    d = m;
  }else{
    assert(!f);
  }

  Module* mod = prechecked_cast<Module*>(owner());
  assert(mod);
  if(d){
    push_back(d);

    if(mod->attribute_stash().is_empty()){
    }else if(size()){
      assert(!_attributes);
      d->set_attributes(mod->attribute_stash().detach());
    }else{ untested();
    }
  }else{
  }
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
  Net_Identifier::parse(f);
  if(owner()->node(name())){
  }else{ untested();
    throw Exception_CS_("ground: need previously declared net", f);
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
  parse_ports(f);
  f >> ';';
  parse_body(f);
}
/*--------------------------------------------------------------------------*/
void Module::parse_ports(CS& f)
{
  _ports.set_owner(this);
  f >> _ports;
}
/*--------------------------------------------------------------------------*/
void Module::parse_body(CS& f)
{
  // do we need a second pass? or just connect the dots while reading in?
  _input.set_owner(this);
  _output.set_owner(this);
  _inout.set_owner(this);
  _ground.set_owner(this);
  _net_decl.set_owner(this);
  _branch_decl.set_owner(this);
  _branches.set_owner(this);
  _variables.set_owner(this);
  _parameters.set_owner(this);
  _aliasparam.set_owner(this);
  //_local_params.set_owner(this);
  _element_list.set_owner(this);
  _local_nodes.set_owner(this);
  _analog_functions.set_owner(this);
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
      || ((f >> "input ") && (f >> _input))
      || ((f >> "output ") && (f >> _output))
      || ((f >> "inout ") && (f >> _inout))
      // mi, npmi, mogi, mogid
      // net_declaration
      || (f >> _net_decl)
      || ((f >> "ground ") && (f >> _net_decl)) // really?
      // mi, non_port_module_item
      // mi, npmi, mogi, module_or_generate_item_declaration
      || ((f >> "branch ") && (f >> _branch_decl))
//      || ((f >> "analog function ") && (f >> _analog_functions))
      // mi, npmi, module_or_generate_item
//      || ((f >> "localparam ") && (f >> _local_params))
      || ((f >> "real ") && (f >> _variables))
      || ((f >> "integer ") && (f >> _variables))
      || ((f >> "parameter ") && (f >> _parameters))
      || ((f >> "localparam ") && (f >> _parameters))
      || ((f >> "aliasparam ") && (f >> _aliasparam))
      || ((f >> "analog ") && parse_analog(f)) // TODO:: f >> analog
      || ((f >> "endmodule ") && (end = true))
      || (f >> _element_list)	// module_instantiation
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
  o << "module " << identifier() << ports() << ";\n";
  if(input().size()){
    o__ "input "	    << input()			<< "\n";
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
  if(ground().size()){ untested();
    o << "  ground "	    << ground()			<< "\n";
  }else{
  }
  o << net_declarations();
  o << branch_declarations();
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
  if(circuit().size()){
    o << ind << "// circuit\n";
    o << circuit() << "\n";
  }else{
  }

  for(auto i: analog_functions()){
    o << *i << "\n";
  }
  for(auto i: analog_list()){
    o << *i << "\n";
  }
//  for(auto i: *this){ untested();
//    o << *i << "\n";
//  }

  o << "endmodule\n";
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::parse(CS& file)
{
  file >> ','; // ??
  file >> _name;
  trace1("variable decl", _name);
  new_var_ref();
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::dump(std::ostream& o)const
{
  o__ name();
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
/// set_vr_ref?
void Block::new_var_ref(Base* what)
{
  assert(what);
  std::string p;
  if(auto V = dynamic_cast<Variable const*>(what)){
    p = V->name();
  }else if(auto P = dynamic_cast<Parameter_2 const*>(what)){
    p = P->name();
  }else if(auto A = dynamic_cast<Aliasparam const*>(what)){
    p = A->name();
  }else if(auto A = dynamic_cast<Paramset_Stmt const*>(what)){
    p = "."+A->name();
  }else{ untested();
    incomplete();
    assert(false);
  }

  auto m = dynamic_cast<Module const*>(this);
  if(!m){
  }else{
    auto const& alias = m->aliasparam();
    // alias.find(p)?
    if(alias.end() == notstd::find_ptr(alias.begin(), alias.end(), p)){
    }else{
      throw(Exception("alias already there: '" + p + "'"));
    }
  }

  trace3("new_var_ref, stashing", p, this, dynamic_cast<Module const*>(this));
  Base* cc = _var_refs[p];

  // yikes.
  if(dynamic_cast<Analog_Function_Arg const*>(what)
   &&!dynamic_cast<Analog_Function_Arg const*>(cc)){
    _var_refs[p] = what;
  }else if(dynamic_cast<Analog_Function_Arg const*>(cc)
   &&!dynamic_cast<Analog_Function_Arg const*>(what)){
    _var_refs[p] = cc;
  }else if(p.substr(0,2)==PS_MANGLE_PREFIX) {
    p = p.substr(2);
    _var_refs[p] = what;
  }else if(dynamic_cast<Paramset_Stmt const*>(what)){
    _var_refs[p] = what;
  }else if(cc) { untested();
    _var_refs[p] = what;
    throw(Exception("already there: '" + p + "'"));
  }else{
    _var_refs[p] = what;
  }
}
/*--------------------------------------------------------------------------*/
void Module::push_back(Filter /*const?*/ * f)
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
void Module::install(FUNCTION_ const* f)
{
  _funcs.insert(f);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
