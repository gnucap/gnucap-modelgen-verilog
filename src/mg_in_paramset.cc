/*                       -*- C++ -*-
 * Copyright (C) 2023, 2024 Felix Salfelder
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
#include "mg_in.h"
#include "mg_options.h"
#include "mg_error.h"
#include "l_stlextra.h"
#include "mg_analog.h"
/*--------------------------------------------------------------------------*/
Base* parse_proc_assignment(CS& f, Block* o); // in_analog.cc
CS& Paramset::parse_stmt(CS& f)
{
  f.skipbl();
  size_t here = f.cursor();
  if (f >> ".") {
    assert(_proto);
    Base const* b = _proto->lookup(f);

    Paramset_Stmt* s = new Paramset_Stmt();

    if (auto pp = dynamic_cast<Parameter_Base const*>(b)) {
      s->set_parameter(pp);
    }else{
      f.reset_fail(here);
      throw Exception_CS_("parameter does not exist", f);
    }

    trace1("got", s->name());
    Base* dup = NULL;
    if((dup = lookup("." + s->name()))){
      if(auto dd = dynamic_cast<Paramset_Stmt*>(dup)){
	dd->set_overridden();
      }else{ untested();
	unreachable();
      }
      f.warn(bWARNING, "duplicate parameter assignment");
    }else{
    }

    try{
      new_var_ref(s);
    }catch (Exception const& eee){ untested();
      f.reset_fail(here);
      throw Exception_CS_(eee.message(), f);
    }

    s->set_owner(this);
    f >> *s;
    if(s){
      push_back(s);
    }else{ untested();
      delete s;
      throw Exception_CS_("something wrong", f);
    }

    // delete dup;

  }else if(Base* ret=parse_proc_assignment(f, this)){
    move_attributes(&f, ret);
    push_back(ret);
    // ...
  }else{
    // incomplete(); // analog_function_statement...
  }

  return f;
}
/*--------------------------------------------------------------------------*/
/*
- A.1.9 Paramset Declaration
+ paramset_declaration ::=
+ { attribute_instance } paramset paramset_identifier module_or_paramset_identifier ;
+ paramset_item_declaration { paramset_item_declaration }
- paramset_statement { paramset_statement }
+ endparamset
+ paramset_item_declaration ::=
+   { attribute_instance } parameter_declaration ;
- | { attribute_instance } local_parameter_declaration ;
- | aliasparam_declaration
- | { attribute_instance } integer_declaration
- | { attribute_instance } real_declaration
- paramset_statement ::=
+   .module_parameter_identifier = paramset_constant_expression ;
- | .module_output_variable_identifier = paramset_constant_expression;
- | .system_parameter_identifier = paramset_constant_expression;
- | analog_function_statement
- paramset_constant_expression ::=
-   constant_primary
- | hierarchical_parameter_identifier
- | unary_operator { attribute_instance } constant_primary
- | paramset_constant_expression binary_operator { attribute_instance } paramset_constant_expression
- | paramset_constant_expression ? { attribute_instance } paramset_constant_expression :untested();
- paramset_constant_expression
*/
/*--------------------------------------------------------------------------*/
void Paramset::parse(CS& f)
{
  File* o = prechecked_cast<File*>(owner());
  assert(o);

  move_attributes(&f, this);
  _parameters.set_owner(this);
  _variables.set_owner(this);

  f >> _identifier;
  size_t here = f.cursor();
  String_Arg proto_name;
  f >> proto_name >> ";";

  // BUG: nested paramsets?
  auto it = o->modules().find(proto_name);
  if(it == o->modules().end()){
    f.reset_fail(here);
    throw Exception_CS_("prototype does not exist", f);
  }else{
  }
  _proto = *it;
  if(o->modules().find_again(++it, proto_name) != o->modules().end()){
    f.reset_fail(here);
    throw Exception_CS_("too many", f);
  }else{
  }


  here = f.cursor();
  bool end = false;


  trace0("paramset body ========");
  for (;;) {
    parse_attributes(f, &f);
    ONE_OF
      || f.umatch(";")
      || ((f >> "parameter ") && (f >> _parameters))
      || ((f >> "localparam ") && (f >> _parameters))
      || ((f >> "real ") && (f >> _variables))
      || ((f >> "integer ") && (f >> _variables))
      || parse_stmt(f)
      || ((f >> "endparamset ") && (end = true))
      ;

    if (has_attributes(&f)){
      f.warn(bWARNING, "dangling attributes "
	   + attributes(&f).string(NULL));
    }else{
    }
    if (end){
      if(options().expand_paramset()){
	expand();
      }else{
      }
      break;
    }else if (!f.more()) { untested();
      // throw?
      f.warn(0, "premature EOF (paramset)");
      break;
    }else if (f.stuck(&here)) { untested();
      // throw?
      f.warn(0, "bad module");
      break;
    }else{
    }
  }

  trace0("paramset body done ========");
}
/*--------------------------------------------------------------------------*/
static void copy_ps_params(Module* sub, Paramset const* thiS)
{
  std::stringstream s;
  thiS->parameters().dump(s);
  s << "endmodule;";
  CS cmd(CS::_STRING, s.str());
  sub->parse_body(cmd);
}
/*--------------------------------------------------------------------------*/
static void import_dot_params(Module* sub, Module const*, Block* thiS)
{
  auto dots = new Parameter_2_List;
  dots->set_owner(sub);

  std::stringstream s;
  bool have_dots = false;
  std::string comma;
  s << "l real "; // TODO: type.
  for(auto i : *thiS) {
    auto dot=dynamic_cast<Paramset_Stmt*>(i);
    if(!dot){
    }else if(dot->is_overridden()){ untested();
    }else{
//       dot->value().dump(std::cerr);
//       std::cerr << "\n";

  /*
     auto l = new Parameter_2;
     l->set_owner(_sub);
     l->set_name("." + dot->name());
     l->set_given();
     l->set_local();
     l->parse_rhs(dot->value());
     */

      // TODO: add '_' as needed.
      s << comma << PS_MANGLE_PREFIX << dot->name() << " = " <<
	 dot->value();
      comma = ", ";
      have_dots = true;

    }
  }
  s << ";";

  if(have_dots){
    CS cmd(CS::_STRING, s.str());
    cmd >> "l" >> *dots;
    for (auto i : *dots){
      i->set_given();
    }
    sub->parameters().push_back(dots);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
static void import_proto_params(Module* sub, Module const* proto)
{
  auto& P = proto->parameters();

  for(auto const& x : P) {
    std::stringstream o;

    o << "localparam";
    o << " " << x->type() << " ";

    std::string comma = "";
    for (auto y : *x) {
      auto p = sub->lookup(y->name(), false);
      auto q = dynamic_cast<Parameter_2 const*>(p);
      assert(q||!p);

      if(q && q->is_given()){
	// omit those that have been set with paramset_statement.
	trace1("already there", y->name());
      } else {
	o << comma << PS_MANGLE_PREFIX << y->name() << " = " << y->default_val();
	comma = ", ";
      }
    }

    o << ";";
    o << "endmodule;";

    if(comma.size()){
      CS cmd(CS::_STRING, o.str());
      sub->parse_body(cmd);
    }else{
    }
  }

}
/*--------------------------------------------------------------------------*/
// temporary kludge, until we have deepcopy/rebase
template<class T>
void streamcp(Module* sub, T const& t)
{
  for(auto const& x : t) {
    std::stringstream o;
    x->dump(o);
    o << "endmodule;";

    CS cmd(CS::_STRING, o.str());
//    trace1("streamcp", cmd.fullstring());
    sub->parse_body(cmd);
  }
}
/*--------------------------------------------------------------------------*/
void streamcp(Module* sub, Owned_Base const& t)
{
  std::stringstream o;
  t.dump(o);
  o << "endmodule;";

  CS cmd(CS::_STRING, o.str());
  trace1("streamcp", cmd.fullstring());
  sub->parse_body(cmd);
}
/*--------------------------------------------------------------------------*/
static void import_proto_vars(Module* sub, Module const* proto)
{
  auto& pv = proto->variables();

  for(Variable_List const* x : pv) {
    if(::attributes(x)[std::string("desc")]!="0"
     ||::attributes(x)[std::string("units")]!="0"){
    }else{
    }

    auto copy = x->deep_copy_(sub, PS_MANGLE_PREFIX);
//    assert(copy->owner() == sub);
    sub->push_back(copy);
  }

  return;

  for(auto const& x : pv) { untested();
    std::stringstream o;

    o << x->type() << " ";

    std::string comma = "";
    for (auto y : *x) { untested();
      trace1("import_var", y->name());
      auto p = sub->lookup(y->name(), false);
      assert(!dynamic_cast<Variable_Decl const*>(p));
      auto P = dynamic_cast<Parameter_2 const*>(p);
      assert(!p||P);

      trace2("import_var2", x->type(), y->name());
      o << comma << PS_MANGLE_PREFIX << y->name();
      comma = ", ";
    }

    o << ";";
    o << "endmodule;";

    if(comma.size()){ untested();
      trace1("command", o.str());
      CS cmd(CS::_STRING, o.str());
      sub->parse_body(cmd);
    }else{ untested();
    }
  }

} // import_proto_vars
/*--------------------------------------------------------------------------*/
static void import_proto_impl(Module* sub, Module const* proto)
{
  assert(proto);
  assert(proto->circuit());
  assert(sub);

//  CS c(CS::_STRING, proto->identifier().to_string());
//  sub->parse(c);

  std::stringstream s;
  proto->circuit()->ports().dump(s);
//  s << "endmodule;";
  CS cmd(CS::_STRING, s.str());
  sub->parse_ports(cmd);

  streamcp(sub, proto->circuit()->local_nodes());
  streamcp(sub, proto->circuit()->net_decl());
  streamcp(sub, proto->circuit()->branch_decl());

  streamcp(sub, proto->analog());
}
/*--------------------------------------------------------------------------*/
static void import_ps_vars(Module* sub, Module const* ps)
{
  auto& pv = ps->variables();

  for(auto const& x : pv) {
    auto copy = x->deep_copy_(sub);
//    assert(copy->owner() == sub);
    sub->push_back(copy);
  }
}
/*--------------------------------------------------------------------------*/
static void import_assignments(Module* sub, Module const* ps)
{
  auto ac = new AnalogConstruct;
  ac->set_owner(sub);
  ac->new_block();
//  sub->new_analog();
  for(auto x : *ps){
    if(dynamic_cast<Paramset_Stmt const*>(x)){
      // todo?
    }else if(auto a = dynamic_cast<Statement const*>(x)){
      trace1("import assignment", *a);
      ac->push_back(a->deep_copy(ac->block()));
    }else{ untested();
      assert(false);
    }
  }
  if(ac->block()->size()){
    sub->push_back(ac);
  }else{
    delete(ac);
  }
}
/*--------------------------------------------------------------------------*/
void Paramset::expand()
{
  option_nodump_unreachable while_expanding_ps_and;
  option_nodump_annotate while_expanding_ps;
  assert(_proto);
  _sub = new Paramset(); // new Module();?
  _sub->set_owner(this);
  assert(_sub->file() == file());
//  _sub->set_file(file());
  _sub->_identifier = _identifier;

  trace1("1. copyparams ========", _identifier);
  copy_ps_params(_sub, this);
  trace0("2. dotparams ========");
  import_dot_params(_sub, _proto, this);
  trace0("3. protoparams ========");
  import_proto_params(_sub, _proto);

  trace0("3b. proto vars ========");
  import_proto_vars(_sub, _proto);
  trace0("5. variables ========");
  import_ps_vars(_sub, this);

  trace0("4. implementation ========");
  import_proto_impl(_sub, _proto);

  trace0("6. assignments ========");
  import_assignments(_sub, this);

}
/*--------------------------------------------------------------------------*/
Expression_ const& Paramset_Stmt::value() const
{
  return _rhs;
}
/*--------------------------------------------------------------------------*/
std::string Paramset_Stmt::name() const
{
  assert(_what);
  return _what->name();
}
/*--------------------------------------------------------------------------*/
Module* Paramset::deflate()
{
  if(_sub){
    Paramset* sub = _sub;
    _sub->set_owner((Base*)NULL);
    _sub->set_owner(scope());
    assert(dynamic_cast<Paramset*>(_sub));
    _sub = NULL;
    sub->setup_functions();
    sub->setup_nodes();
    return sub;
  }else{
    return this;
  }
}
/*--------------------------------------------------------------------------*/
void Paramset_Stmt::parse(CS& f)
{
  assert(owner());
  _rhs.set_owner(owner());
  Expression_ ex;
  f >> "=" >> ex >> ";";
  if(f){
    _rhs.resolve_symbols(ex);
  }else{ untested();
    throw Exception_CS_("bad expression", f);
  }
}
/*--------------------------------------------------------------------------*/
void Paramset_Stmt::dump(std::ostream& o) const
{
  assert(_what);
  // if what == param
  o__ "." << _what->name() << " = " << _rhs << "\n";
}
/*--------------------------------------------------------------------------*/
void Paramset::dump(std::ostream& o) const
{
  print_attributes(o, this);
  if(!_proto){
    return Module::dump(o);
  }else{
    o << "paramset " << _identifier << " " << _proto->identifier() << ";\n";
    {
      indent a;
      dump_parameters(o);
      dump_variables(o);

      // dump_body..?
      for(auto i : *this) {
	assert(i);
	o << *i;
      }
    }
    o << "endparamset;\n";
  }
}
/*--------------------------------------------------------------------------*/
bool Paramset::new_var_ref(Base* what)
{
  assert(!dynamic_cast<Variable_Decl const*>(what));
  auto P = dynamic_cast<Parameter_2 const*>(what);
  auto T = dynamic_cast<Token const*>(what);
  auto R = dynamic_cast<Token_VAR_REF const*>(what);
 // auto T = dynamic_cast<Token const*>(what);
  std::string p;
  if(auto A = dynamic_cast<Aliasparam const*>(what)){ untested();
    p = A->name();
  }else if(auto nn = dynamic_cast<Node const*>(what)){
    p = nn->name();
  }else if(T){
    p = T->name();
  }else if(R){ untested();
    p = R->name();
  }else if(P){
    p = P->name();
  }else if(T){ untested();
    incomplete();
  }
  trace1("PS::nvr", p);

  if(p!=""){
    auto const& alias = aliasparam();
    // alias.find(p)?
    if(alias.end() == notstd::find_ptr(alias.begin(), alias.end(), p)){
    }else{ untested();
      throw(Exception("alias already there: '" + p + "'"));
    }
  }else{
  }


  if(auto ps = dynamic_cast<Paramset_Stmt const*>(what)){
    p = "."+ps->name();
    _var_refs[p] = what;
  }else if(p.substr(0,2)==PS_MANGLE_PREFIX) {
    if(P) {
      p = p.substr(2);
    }else if(T){
      _var_refs[p] = what;
      p = p.substr(2);
    }else if(R){ untested();
    }else{ untested();
    }
    trace1("PS::nvr mangle", p);
    _var_refs[p] = what;
    return false;
  }else{
    return Module::new_var_ref(what);
  }
  return false;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
