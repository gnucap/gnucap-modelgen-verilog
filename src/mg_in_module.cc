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
#include "mg_analog.h" // BUG: Analog_Function_Arg, push_back
#include "mg_token.h"
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
    for(auto const& p : _list_of_port_connections) {
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

  if(f >> '='){
  }else{
    throw Exception_CS_("expecting '='", f);
  }

  try{
    f >> _default_val;
  }catch(Exception_CS_ const& ec){
    throw ec;
  }catch(Exception const& e){ untested();
    incomplete();
    throw Exception_CS_("sth wrong: " + e.message(), f);
  }
  {
    f >> _value_range_list;
  }

  assert(owner());
}
/*--------------------------------------------------------------------------*/
inline Parameter_2::~Parameter_2()
{
}
/*--------------------------------------------------------------------------*/
void Parameter_2::resolve()
{
 // _default_val.resolve();

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
class HS_Parameter : public Parameter_Base {
public:
  explicit HS_Parameter(std::string const& p) : Parameter_Base(p) {}
private:
  void parse(CS&)override { untested();unreachable();}
  void dump(std::ostream&)const override { untested();unreachable();}
};
HS_Parameter hp_mfactor("$mfactor");
HS_Parameter hp_xpos("$xposition");
HS_Parameter hp_ypos("$yposition");
HS_Parameter hp_zpos("$zposition");
HS_Parameter hp_angle("$angle");
HS_Parameter hp_vflip("$bflip");
HS_Parameter hp_hflip("$hflip");
HS_Parameter hp_bflip("$bflip");
/*--------------------------------------------------------------------------*/
bool Aliasparam::is_hs() const
{
  return dynamic_cast<HS_Parameter const*>(_param);
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
  Parameter_2* pp = nullptr;
  for(auto pl : p){
    auto it = notstd::find_ptr(pl->begin(), pl->end(), paramname);
    if(pp){
      assert(it == pl->end());
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
  }else if(paramname == "$mfactor"){
    _param = &hp_mfactor;
  }else if(paramname == "$xposition"){ untested();
    _param = &hp_xpos;
  }else if(paramname == "$yposition"){
    _param = &hp_ypos;
  }else if(paramname == "$zposition"){ untested();
    _param = &hp_zpos;
  }else if(paramname == "$angle"){ untested();
    _param = &hp_angle;
  }else if(paramname == "$hflip"){ untested();
    _param = &hp_hflip;
  }else if(paramname == "$vflip"){ untested();
    _param = &hp_vflip;
  }else if(paramname == "$bflip"){ untested();
    // sflip?
    _param = &hp_bflip;
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
  }else if(file.umatch("integer")){
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
  (void)m;
  attr.move_attributes(tag_t(&file), tag_t(this));

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

  try{
    LiSt<Parameter_2, '\0', ',', ';'>::parse(file);
  }catch(Exception_CS_ const& e){
    throw e;
  }
  for(auto& i : *this){
    i->set_type(type);
    i->set_local(_is_local);
    i->resolve();
  }
}
/*--------------------------------------------------------------------------*/
void Parameter_2_List::dump(std::ostream& o)const
{
  print_attributes(o, this);

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
void Variable_Stmt::dump(std::ostream& o)const
{
  print_attributes(o, this);

  o__ _type << " ";
  _l.dump(o);
  o << "\n";
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
static std::string get_identifier(CS& cmd, std::string const& term)
{
  trace1("get_identifier", cmd.tail().substr(0,2));
  cmd.skipbl();
  std::string id;

  if(cmd.is_digit()) {
    cmd.warn(bWARNING, "invalid identifier");
  }else{
  }

  if(cmd >> "\\") {
    id = cmd.get_to(" \t\f");
    trace1("got to", cmd.peek());
    cmd.skip();

    {
      bool plain = true;
      for(size_t i = 0; plain && i<id.size() ; ++i) {
	if (isalnum(id[i])) {
	}else if (id[i] == '$') { untested();
	  plain = false;
	}else{ untested();
	  plain = false;
	}
      }

      if(plain) {
	// don't touch, for now.
      }else{ untested();
	// store escaped string.
	id = "\\" + id;
      }
    }
  }else{
    id = cmd.ctos(term, "", "");
  }

  trace1("identifier", id);
  return id;
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
void Port_3::parse(CS& f)
{
  size_t here = f.cursor();
  if (f >> '.') {
    _name = get_identifier(f, "(");
    int paren = f.skip1b('(');
    _value = get_identifier(f, ";,)");

    if (!paren){untested();
      //?
    }else if( f.skip1b(')')) {
    }else{untested();
      f.warn(bDANGER, here, "need ')'");
    }

  }else{
    _name = get_identifier(f, ";,)");
  }
  f >> ",";
  trace3("Port_3::name", _name, value(), f.tail().substr(0,2));
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
  print_attributes(o, this);

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
static std::string mangle(std::string const& name)
{
  if(isdigit(name[0])) {
    return '\\' + name + " ";
  }else if(name[0] == '\\') { untested();
    return name + " ";
  }else{
    // ok, for now.
    // probably need '\\' ... ' ' whenever special characters are used.
    return name;
  }
}
/*--------------------------------------------------------------------------*/
void Port_3::dump(std::ostream& out)const
{
  if(has_identifier()){
    out << "." << mangle(name()) << "(" << mangle(value()) << ")";
  }else{
    out << mangle(name());
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
  assert(o);
  (void)o;
  attr.move_attributes(tag_t(&f), tag_t(this));

  // f >> "module |macromodule |connectmodule "; from caller
  f >> _identifier;
  f >> *_circuit; // HACK
  _circuit->parse_ports(f);
  f >> ';';
  parse_body(f);
  setup_functions();
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
  assert(_always);
  _always->set_owner(this);
  // _tr_eval.set_owner(this);
  // _validate.set_owner(this);


  bool end = false;
  bool reserved = false;
  for (;;) {
    trace1("module body parse", f.tail().substr(0,20));
    parse_attributes(f, &f);
    size_t here = f.cursor();
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
      || ((f >> "always ") && f >> *_always)
      || ((f >> "endmodule ") && (end = true))
      // subdevice instances. can't use reserved keywords.
      || ((f >> "paramset ") && (reserved = true))
      || ((f >> "module ") && (reserved = true))
      || (f >> _circuit->element_list())	// module_instantiation
      ;
    if (attr.has_attributes(tag_t(&f))) { untested();
      f.warn(bWARNING, "dangling attributes "
	   + attr.attributes(tag_t(&f))->string(tag_t(nullptr)));
    }else{
    }
    trace2("endloop", end, f.last_match());
    if (reserved){
      f.reset(here);
      throw Exception_CS_("not allowed here: " + f.last_match(), f);
    }else if (end){
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
static void dump_annotate(Module const& m, std::ostream& o)
{
  return;
  for(auto x: m.var_refs()){ untested();
    o__ "// var_ref: " << x.first << "\n";
  }
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

  print_attributes(o, this);
  indent x;
  o << "module " << identifier() << _circuit->ports() << ";\n";
  if(options().dump_annotate()){
    dump_annotate(*this, o);
  }else{
  }
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
  dump_parameters(o);
  dump_variables(o);
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
  assert(_always);
  _always->dump(o);

  o << "endmodule\n";
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
  try{
    file >> _lb;
  }catch(Exception const&){ untested();
    throw Exception_CS_("Syntax error\n", file);
  }
  if(file >> ':'){
  }else if(file >> ','){
    file.warn(bWARNING, file.cursor()-1, "maybe ':'?");
  }else{
    throw Exception_CS_("Syntax error\n", file);
  }
  trace1("ValueRangeInterval::parse0", file.tail().substr(0,19));
  try{
    file >> _ub;
  }catch(Exception const&){ untested();
    throw Exception_CS_("Syntax error\n", file);
  }
  trace1("ValueRangeInterval::parse1", file.tail().substr(0,19));

  if(file >> ']') {
    _ub_is_closed = true;
  }else if(file >> ')') {
    _ub_is_closed = false;
  }else{
    trace1("ValueRangeInterval::parse2", file.tail().substr(0,19));
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
bool Module::new_var_ref(Base* what)
{
  assert(!dynamic_cast<Variable_Decl const*>(what));
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

  if(p != ""){
    auto const& alias = aliasparam();
    // alias.find(p)?
    if(alias.end() == notstd::find_ptr(alias.begin(), alias.end(), p)){
    }else{
      throw(Exception("alias already there: '" + p + "'"));
    }
  }else{
  }

  return Block::new_var_ref(what);
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
void Module::push_back(FUNCTION_* f)
{
  _func.push_back(f);
  install(f);
}
/*--------------------------------------------------------------------------*/
// TODO always push into body?
void Module::push_back(Base* x)
{
  if(auto vl = dynamic_cast<Variable_Stmt*>(x)){
    _variables.push_back(vl);
  }else if(auto a = dynamic_cast<AnalogConstruct*>(x)){
    auto A = prechecked_cast<Analog*>(_analog); // needed? use LiSt?
    assert(A);
    A->push_back(a);
  }else{
    Block::push_back(x);
  }
}
/*--------------------------------------------------------------------------*/
void Module::push_back(Token*)
{ untested();
  assert(0); // remove.
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
void Module::detach_out_vars()
{
#ifndef NDEBUG
  assert(circuit());
  for(auto br : circuit()->branches()){
    if(br->is_used_in(&variables())){ untested();
      br->unset_used_in(&variables());
    }else{
    }
  }
#endif
}
/*--------------------------------------------------------------------------*/
Module::~Module()
{
  delete_analog();
  detach_out_vars(); // delete variables?
  delete_circuit();
}
/*--------------------------------------------------------------------------*/
bool Node::is_used() const
{
  if(1 && is_ground()){
    trace1("Node::is_used, ground", name());
  }else{
    trace2("Node::is_used", name(), _fanout.size());
    for(Element_2 const* e : _fanout){
      if(e->is_used()){
	trace2("Node::is_used", name(), e->code_name());
	return true;
      }else{
      }
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
void Module::delete_circuit()
{
  delete _circuit;
  _circuit = nullptr;
}
/*--------------------------------------------------------------------------*/
void Module::dump_parameters(std::ostream& o) const
{
  if(parameters().size()){
    o << parameters() << "\n";
  }else{
  }
  if(aliasparam().size()){
    o << aliasparam() << "\n";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Module::dump_variables(std::ostream& o) const
{
  if(variables().size()){
    o << variables() << "\n";
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Module::import_flags(FUNCTION_ const* f)
{
  if(f->has_analysis()){
    set_analysis();
  }else{
  }
  if(f->has_tr_begin()){
    set_tr_begin();
  }else{
  }
  if(f->has_tr_restore()){
    set_tr_restore();
  }else{
  }
  if(f->has_tr_review()){
    set_tr_review();
  }else{
  }
  if(f->has_tr_advance()){
    set_tr_advance();
  }else{
  }
  if(f->has_tr_accept()){
    set_tr_accept();
  }else{
  }
  if(f->has_set_event()){
    set_set_event();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
// in_analog.cc?
static Token* new_filter_token(int na, FUNCTION_ const* func, Module* m)
{
  auto f = prechecked_cast<MGVAMS_FILTER const*>(func);
  assert(f);

  assert(na != -1);

  static int n_filters;
  std::string filter_code_name = f->label() + "_" + std::to_string(n_filters++);

  FUNCTION* fc = f->clone();
  auto fcl = prechecked_cast<MGVAMS_FILTER*>(fc);

#if 1
  {
    fcl->set_label(filter_code_name); // label()); // "_b_" + filter_code_name);
    if(int(na) <= f->max_args()) {
    }else{ untested();
      incomplete();
      throw Exception_Too_Many(na, f->max_args(), 0);
      error(bDANGER, "too many arguments, have " + to_string(na) + "\n");
    }
    fcl->set_num_args(na);
    fcl->set_owner(m);
    m->push_back(fcl);
  }

  Node* np = m->new_node(filter_code_name + "_p");
  Node* nn = m->new_node(filter_code_name + "_n"); // &mg_ground_node
  np->set_to(&Node_Map::mg_ground_node, "_short_b_"+filter_code_name+"()");

  {
    Branch* br = m->new_branch(np, nn);
    //      br->set_source();
    assert(br);
    assert(const_cast<Branch const*>(br)->owner());
    Branch_Ref prb(br);
    fcl->set_branch(br);

    br->set_filter(fcl); // needed?
    assert(m->circuit());
    m->new_filter();

    if(f->eval_name()!=""){
      br->set_eval(fcl->eval_name());
    }else{
    }
  }
#endif

  return new Token_FILTER(f->label(), fcl); // BUG. in_module
}
/*--------------------------------------------------------------------------*/
Token* Module::new_token(FUNCTION const* f_, size_t num_args_)
{
  int num_args = int(num_args_);
  auto f = prechecked_cast<FUNCTION_ const*>(f_);
  assert(f);
  Token* t = nullptr;


  if(f->has_tr_review()){
    auto c = prechecked_cast<Token_CALL*>(t);
    (void)c;
    // incomplete();
   // m.set_tr_review(); // wrong? because token may be deleted later on.
   //                    // inc/dec?
   //                    // do it in token?
   //
   // c->set_used_in(tr_review_tag);
  }else{
  }

  trace2("Module::new_token", f->label(), f->static_code());
  if(f->static_code()) {
    install(f);
    t = f->new_token(*this, num_args);
    if(!t){
      t = new Token_CALL(f->label(), f);
      import_flags(f);
    }else{
    }
  }else if( (t = f->new_token(*this, num_args)) ){
    import_flags(f);
  }else if(f && f->is_analog_filter()) { // cast to MGVAMS_FILTER?
    import_flags(f);
    t = new_filter_token(num_args, f, this);
  }else if(f){
    FUNCTION_* cl = f->clone();
    assert(cl);
    std::string label = f->label();
    static int cnt;
    cl->set_label("t_" + f->label() + std::to_string(cnt++));
    cl->set_num_args(num_args);
    push_back(cl);

    import_flags(f);
    t = new Token_CALL(label, cl);
  }else{
    unreachable();
  }

  return t;
}
/*--------------------------------------------------------------------------*/
void filter_setup(MGVAMS_FILTER*, Module*);
void Module::setup_functions()
{
  for(FUNCTION_* f : _func){
    if(auto ff=dynamic_cast<MGVAMS_FILTER*>(f)){
      filter_setup(ff, this);
    }else{
      f->setup(this);
    }
  }
}
/*--------------------------------------------------------------------------*/
void Module::install(FUNCTION_ const* f)
{
  _funcs.insert(f);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
