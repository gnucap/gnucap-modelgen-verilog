/*                -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023 Felix Salfelder
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
 */
#include <io_.h>
#include "mg_.h"
#include "mg_out.h"
#include <stack>
#include <u_opt.h>
#include <e_cardlist.h> // BUG?
#include <l_stlextra.h>
/*--------------------------------------------------------------------------*/
void Port_1::parse(CS& file)
{
  trace1("Port_1::parse", file.last_match());
  file >> _name;
  size_t here = file.cursor();
  for (;;) {
    ONE_OF
      || ((file >> "short_to =") && (file >> _short_to))
      || ((file >> "short_if =") && (file >> _short_if))
      ;
    if (file.skip1b(";")) {
      break;
    }else if (!file.more()) { untested();untested();
      file.warn(0, "premature EOF (Port_1)");
      break;
    }else if (file.stuck(&here)) { untested();
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void Port_1::dump(std::ostream& out)const
{ untested();
  if (short_to() != "" || short_if() != "") { untested();
    out << name() << " short_to=\"" << short_to() 
	<< "\" short_if=\"" << short_if() << "\";\n";
  }else{ untested();
    out << name() << "; ";
  }
}
/*--------------------------------------------------------------------------*/
void Head::parse(CS& file)
{ untested();
  size_t here = file.cursor();
  size_t begin = 0;
  size_t end = here;
  for (;;) { untested();
    file.skipto1('*');
    if (file >> "*/") { untested();
      end = file.cursor();
      break;  // done with head
    }else if (file.stuck(&here)) { untested();untested();
      file.warn(0, "unterminated head");
      break;
    }else{ untested();
      file.skip();
    }
  }
  _s = file.substr(begin, end-begin);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
File::File() : _file(CS::_STRING, "")
{
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Attribute_Spec::parse(CS& f)
{
  assert(owner());
  _key = f.ctos("=", "", "");
  trace2("Attribute_Spec", _key, f.tail().substr(0,18));
  if( f >> '=' ){
    // _expr = new ConstExpression(f, owner());
    _expr = new value_type;
//    f >> *_expr;
    if(f.match1('\"')) {
      f >> *_expr;
      *_expr = "\"" + *_expr + "\"";
    }else{
      *_expr = f.ctos(",=;)*"); // , "\"'{(", "\"'})");
    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void Attribute_Spec::dump(std::ostream& o)const
{
  o << _key;
  if(has_expression()){ untested();
    o << "=" << expression();
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void Attribute_Stash::parse(CS& f)
{
  assert(owner());
  if(_a){
    assert(owner() == _a->owner());
    _a->parse(f);
  }else{
    _a = new Attribute_Instance(f, owner());
  }
  if(_a->is_empty()){
    delete _a;
    _a = NULL;
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Attribute_Instance::parse(CS& f)
{
  size_t here = f.cursor();
  if(f>>"(*"){
    while(f.more() && !f.stuck(&here)){
//      std::string value("1");
      if(f >> "*)") {
	trace1("got all attributes", f.tail().substr(0,17));
	break;
      }else{
      }
      assert(owner());
      auto s = new Attribute_Spec(f, owner());
      push_back(s);
      f.skip1(',');
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
#if 0
Attribute_Instance::value_type const* Attribute_Instance::find(String_Arg const& key) const
{
  auto i = notstd::find_ptr(begin(), end(), key);
  if(i == end()){ untested();
    return NULL;
  }else{ untested();
    return (*i)->expression_or_null();
  }
}
#endif
/*--------------------------------------------------------------------------*/
void Attribute_Instance::clear()
{
  Collection<Attribute_Spec>::clear();
}
/*--------------------------------------------------------------------------*/
void Attribute_Instance::dump(std::ostream& o)const
{
  if(is_empty()){
  }else{
    o__ "(*";
    std::string comma;
    for(auto const& i : *this){
      o << comma << " " << i->key();
      if(i->has_expression()){
	o << "=" << i->expression();
      }else{
      }
      comma = ",";
    }
    o << " *)\n";
  }
}
/*--------------------------------------------------------------------------*/
/* A.1.2
+ source_text ::=
+	  { untested(); description }
+ description ::=
+	  module_declaration
-	| udp_declaration
-	| config_declaration
-	| paramset_declaration
+	| nature_declaration
+	| discipline_declaration
-	| connectrules_declaration
*/
void File::parse(CS& file)
{
  _module_list.set_file(this); // needed?
			       //
  _attribute_stash.set_owner(this);

  _module_list.set_owner(this);
  _macromodule_list.set_owner(this);
  _connectmodule_list.set_owner(this);
  _nature_list.set_owner(this);
  _discipline_list.set_owner(this);
  _paramset_list.set_owner(this);

  size_t here = _file.cursor();
  for (;;) {
    while(file >> _attribute_stash){ }
    ONE_OF	// description
      || file.umatch(";")
      || ((file >> "module ")	     && (file >> _module_list))
      || ((file >> "macromodule ")   && (file >> _macromodule_list))
      || ((file >> "connectmodule ") && (file >> _connectmodule_list))
      || ((file >> "nature ")	     && (file >> _nature_list))
      || ((file >> "discipline ")    && (file >> _discipline_list))
      || ((file >> "paramset ")      && (file >> _paramset_list))
      ;
    if (_attribute_stash.is_empty()){
    }else{
      file.warn(0, "dangling attributes");
    }
    if (!file.more()) {
      break;
    }else if (file.stuck(&here)) {
      file.warn(0, "syntax error, need nature, discipline, module or paramset");
      break;
    }else{
    }
  }

  // HACK
  for(auto i: _module_list){
    i->set_owner(this);
  }
}
/*--------------------------------------------------------------------------*/
void String_Arg::parse(CS& f)
{
  f >> _s;
}
/*--------------------------------------------------------------------------*/
void ConstExpression::parse(CS& file)
{
  trace1("ConstExpression::parse", file.tail().substr(0,19));
  assert(owner());
  Expression ce(file);
  Expression tmp;
  assert(owner());
  resolve_symbols(ce, tmp, owner());
  _expression = new Expression(tmp, &CARD_LIST::card_list);
}
/*--------------------------------------------------------------------------*/
void ConstExpression::dump(std::ostream& o) const
{
  o << "(";
  if(_expression) {
    o << *_expression;
  }else{ untested();
  }
  o << ")";
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
