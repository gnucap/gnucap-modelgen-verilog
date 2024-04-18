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
/*--------------------------------------------------------------------------*/
#include "mg_code.h"
#include "mg_module.h"
#include "mg_error.h"
#include "mg_token.h"
#include "mg_attrib.h"
#include "mg_options.h"
/*--------------------------------------------------------------------------*/
void Variable_Decl::parse(CS& f)
{
  f >> ','; // ?? BUG.
  assert(owner());
  assert(!_data);
  assert(!_token);
  std::string name;

  name = f.ctos(",=(){};[]");
  trace1("Variable_Decl::parse", name);
  if(f.peek() == '['){
    f >> _dimensions;
  }else{
  }
  if(f >> "="){
    Expression init(f);
    resolve_symbols(init);
  }else{
  }

  // new_data();
  _data = new TData();
  _token = new Token_VAR_DECL(name, this, _data);
  trace1("variable decl", name);

  auto l = prechecked_cast<Variable_List*>(owner());
  assert(l);
  assert(l->type());
  set_type(l->type());

  if(l->scope()->new_var_ref(_token)){
  }else{
    throw Exception_CS_("already declared", f);
  }

  ::attributes(_token) = ::attributes(&f);
}
/*--------------------------------------------------------------------------*/
void Variable_List::parse(CS& f)
{
  assert(owner());
  Module* mod = prechecked_cast<Module*>(owner());
  if(mod){
  }else{
  }

  char t = f.last_match()[0];
  if(t=='r') {
    _type = Data_Type_Real();
  }else if(t=='i') {
    _type = Data_Type_Int();
  }else{ untested();
    throw Exception_CS_("What type? " + t, f);
  }

//  if( f >> "[" ){
//    incomplete();
//  }else{
//  }

  _l.set_owner(this);
  _l.parse(f);

  move_attributes(&f, this);
//  update();
} // Variable_List::parse
/*--------------------------------------------------------------------------*/
void Variable_Decl::update()
{
  assert(_token);
  _data->clear();
  new_var_ref_(); // already declared
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::dump(std::ostream& o)const
{
  o << name() << _dimensions;
  if(Expression_::size()){
    o << " = ";
    Expression_::dump(o);
  }else{
  }
  if(options().dump_annotate()){
    std::string nl;
    for(Dep const& d : deps().ddeps()){
      o << "// dep " << d.code_name();
      nl = "\n";
    }
    o << nl;
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool Variable_Decl::propagate_deps(Token_VAR_REF const& v)
{
  if(!v.data()){ untested();
    incomplete();
    return false;
  }else{
  }
  TData const& incoming = v.deps();
  assert(&deps() != &incoming);
  data().update(incoming);
  assert(deps().ddeps().size() >= incoming.ddeps().size());
  return false;
}
/*--------------------------------------------------------------------------*/
bool Variable_List::update()
{
  trace0("AnalogDeclareVars::update");
  for(Variable_Decl* d : *this){
    assert(d);
    d->update();
  }
  return false;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
