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
#include <stack>
#include <u_opt.h>
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
void Attribute_Instance::parse(CS& file)
{
  if(file>>"(*"){
    file.skipto1("*)");
    file.skip(2);
  }else{
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
  trace1("File::parse", file.fullstring());
  _module_list.set_file(this); // needed?
			       //
  _module_list.set_owner(this);
  _macromodule_list.set_owner(this);
  _connectmodule_list.set_owner(this);
  _nature_list.set_owner(this);
  _discipline_list.set_owner(this);

  size_t here = _file.cursor();
  for (;;) {
    ONE_OF	// description
      || (file >> _attribute_dummy)
      || file.umatch(";")
      || ((file >> "module ")	     && (file >> _module_list))
      || ((file >> "macromodule ")   && (file >> _macromodule_list))
      || ((file >> "connectmodule ") && (file >> _connectmodule_list))
      || ((file >> "nature ")	     && (file >> _nature_list))
      || ((file >> "discipline ")    && (file >> _discipline_list))
      ;
    if (!file.more()) {
      break;
    }else if (file.stuck(&here)) { untested();
      file.warn(0, "syntax error, need nature, discipline, or module");
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void Preprocessor::read(std::string const& file_name)
{
  if(OPT::case_insensitive == 0){
  }else{ untested();
  }
  // _name = file_name;
  std::string::size_type sepplace;
  sepplace = file_name.find_last_of("/");
  if(sepplace == std::string::npos){
    _cwd = ".";
  }else{
    _cwd = file_name.substr(0, sepplace);
  }

  CS file(CS::_INC_FILE, file_name);
  trace1("whole file", file.fullstring());

  parse(file);
}
/*--------------------------------------------------------------------------*/
void String_Arg::parse(CS& f)
{
  f >> _s;
}
/*--------------------------------------------------------------------------*/
void Raw_String_Arg::parse(CS& f)
{
  assert(_s=="");
  bool quote = false;
  while(f.ns_more()) {
    char p = f.peek();
    if(p == '\"') {
      quote = !quote;
      _s += p;
      f.skip();
    }else if(quote) {
      _s += p;
      f.skip();
    }else if(p == ',') {
      f.skip();
      break;
    }else if(p == ')') {
      break;
    }else {
      _s += p;
      f.skip();
    }
  }
  trace2("RSA", _s, f.tail().substr(0,10));
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
