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
#include <gnucap/io_.h>
#include "mg_.h"
/*--------------------------------------------------------------------------*/
static C_Comment   dummy_c_comment;
static Cxx_Comment dummy_cxx_comment;
static Skip_Block  skip_block;
/*--------------------------------------------------------------------------*/
void C_Comment::parse(CS& file)
{
  size_t here = file.cursor();
  for (;;) {
    file.skipto1('*');
    if (file >> "*/") {
      break;  // done with comment
    }else if (file.stuck(&here)) {untested();
      file.warn(0, "unterminated C comment");
      break;
    }else{
      file.skip();
    }
  }
}
/*--------------------------------------------------------------------------*/
void Cxx_Comment::parse(CS& file)
{
  size_t here = file.cursor();
  file.skipto1('\n');
  if (file.stuck(&here)) {untested();
    file.warn(0, "unterminated C++ comment");
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Skip_Block::parse(CS& file)
{
  int nest = 0;
  size_t here = file.cursor();
  for (;;) {
    file.skipto1('`');
    if (file >> "`endif") {
      if (nest == 0) {
	break;  // done with skip_block
      }else{untested();
	--nest;
      }
    }else if (file >> "`else") {
      if (nest == 0) {
	break;  // done with skip_block
      }else{untested();
      }
    }else if (file >> "`ifndef") {untested();
      ++nest;
    }else if (file >> "`ifdef") {untested();
      ++nest;
    }else if (file.stuck(&here)) {untested();
      file.warn(0, "unterminated ifdef block");
      break;
    }else{
      file.skip();
    }
  }
}
/*--------------------------------------------------------------------------*/
void Port_1::parse(CS& file)
{
  file >> _name;
  size_t here = file.cursor();
  for (;;) {
    ONE_OF
      || ((file >> "short_to =") && (file >> _short_to))
      || ((file >> "short_if =") && (file >> _short_if))
      ;
    if (file.skip1b(";")) {
      break;
    }else if (!file.more()) {untested();
      file.warn(0, "premature EOF (Port_1)");
      break;
    }else if (file.stuck(&here)) {
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void Port_1::dump(std::ostream& out)const
{
  if (short_to() != "" || short_if() != "") {
    out << name() << " short_to=\"" << short_to() 
	<< "\" short_if=\"" << short_if() << "\";\n";
  }else{
    out << name() << "; ";
  }
}
/*--------------------------------------------------------------------------*/
void Head::parse(CS& file)
{
  size_t here = file.cursor();
  size_t begin = 0;
  size_t end = here;
  for (;;) {
    file.skipto1('*');
    if (file >> "*/") {
      end = file.cursor();
      break;  // done with head
    }else if (file.stuck(&here)) {untested();
      file.warn(0, "unterminated head");
      break;
    }else{
      file.skip();
    }
  }
  _s = file.substr(begin, end-begin);
}
/*--------------------------------------------------------------------------*/
/*
The following compiler directives are supported:
- `begin_keywords[1364-2005]
- `celldefine[1364-2005]
- `default_discipline[10.2]
- `default_nettype[1364-2005]
- `default_transition[10.3]
+ `define[10.4]
+ `else[1364-2005]
+ `elsif[1364-2005]
- `end_keywords[1364-2005]
- `endcelldefine[1364-2005]
+ `endif[1364-2005]
+ `ifdef[1364-2005]
- `ifndef[1364-2005]
+ `include[1364-2005]
- `line[1364-2005]
- `nounconnected_drive [1364-2005]
- `pragma[1364-2005]
- `resetall[1364-2005]
- `timescale[1364-2005]
- `unconnected_drive[1364-2005]
- `undef[10.4]
*/
/*--------------------------------------------------------------------------*/
void Define::parse(CS& f)
{
  f >> _name;
  _value = f.get_to("\n");
  // TODO: resolve ` in value
  // TODO: macros with args?
}
/*--------------------------------------------------------------------------*/
void Define::dump(std::ostream& f)const
{ untested();
  f << "`define " << name() << " " << value() << "\n";
}
/*--------------------------------------------------------------------------*/
std::string File::preprocess(const std::string& file_name)
{ untested();
  CS file(CS::_WHOLE_FILE, file_name);

  std::string stripped_file;
  size_t here = file.cursor();
  int if_block = 0;
  int else_block = 0;
  for (;;) { untested();
    stripped_file += file.get_to("\"/`");
    if (file.match1('\"')) { //---------------- quoted string
      stripped_file += '"' + file.ctos("", "\"", "\"", "") + '"';
    }else if (file >> "/*") { //---------------- C comment
      file >> dummy_c_comment; //BUG// line count may be wrong
    }else if (file >> "//") { //---------------- C++ comment
      file >> dummy_cxx_comment;
    }else if (file >> "`define") { untested();
      file >> _define_list;
    }else if (file >> "`include") { //---------------- include
      std::string include_file_name;
      file >> include_file_name;
      stripped_file += include(include_file_name);
    }else if (file >> "`ifdef") { untested();
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	++if_block;
      }else{
	file >> skip_block;
	++else_block;
      }
    }else if (file >> "`ifndef") { untested();
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) { untested();
	file >> skip_block;
	++else_block;
      }else{ untested();
	String_Arg s;
	file >> s; // discard.
	++if_block;
      }
    }else if (file >> "`else") { //---------------- else
      if (if_block > 0) {
	file >> skip_block;
	--if_block;
      }else{untested();
	// error
      }
    }else if (file >> "`endif") { //---------------- endif
      if (else_block > 0) {
	--else_block;
      }else if (if_block > 0) {
	--if_block;
      }else{untested();
	// error
      }
    }else if (file >> "`") { //---------------- macro substitution
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	assert(*x);
	stripped_file += (**x).value().to_string() + " ";
      }else{untested();
	// error: not defined
      }
    }else{ //---------------- plain code, just copy
      // move on, just copy
    }
    if (!file.ns_more()) {
      // proper end of file
      break;
    }else if (file.stuck(&here)) {
      // comment ran to end of file
      break;
    }else{
    }    
  }
  //std::cout << stripped_file;
  return stripped_file;
}
/*--------------------------------------------------------------------------*/
File::File() : _file(CS::_STRING, "")
{
}
/*--------------------------------------------------------------------------*/
void File::add_include_path(std::string const& what)
{
  trace1("add include", what);
  static std::string colon = "";
  _include_path = _include_path + colon + what;
  colon = ":";
}
/*--------------------------------------------------------------------------*/
void File::define(std::string const& what)
{
  trace1("define", what);
  CS cmd(CS::_STRING, what);
  cmd >> _define_list;
}
/*--------------------------------------------------------------------------*/
std::string File::include(std::string const& file_name)
{ untested();
  std::string full_file_name = findfile(file_name, _cwd+":"+_include_path, R_OK);

  if(full_file_name == ""){
    throw Exception_File_Open(file_name + ": No such file");
  }else{
  }

  std::string::size_type sepplace;
  sepplace = full_file_name.find_last_of("/");

  std::string dirname = full_file_name.substr(0, sepplace);

  std::string old_include_path = _include_path;
  std::string old_cwd = _cwd;

  std::string ret = preprocess(full_file_name);

  _include_path = old_include_path;
  _cwd = old_cwd;

  return ret;
}
/*--------------------------------------------------------------------------*/
/* A.1.2
+ source_text ::=
+	  { description }
+ description ::=
+	  module_declaration
-	| udp_declaration
-	| config_declaration
-	| paramset_declaration
+	| nature_declaration
+	| discipline_declaration
-	| connectrules_declaration
*/
void File::read(std::string const& file_name)
{
  _name = file_name;
  std::string::size_type sepplace;
  sepplace = file_name.find_last_of("/");
  if(sepplace == std::string::npos){
    _cwd = ".";
  }else{
    _cwd = file_name.substr(0, sepplace);
  }
  _file = preprocess(file_name);
//  trace1("=============", _file.fullstring());
//  trace0("=============");


  size_t here = _file.cursor();
  for (;;) {
    ONE_OF	// description
      || _file.umatch(";")
      || ((_file >> "module ")	     && (_file >> _module_list))
      || ((_file >> "macromodule ")  && (_file >> _macromodule_list))
      || ((_file >> "connectmodule ")&& (_file >> _connectmodule_list))
      || ((_file >> "nature ")	     && (_file >> _nature_list))
      || ((_file >> "discipline ")   && (_file >> _discipline_list))
#if 0
      || ((_file >> "h_headers ")    && (_file >> _h_headers))
      || ((_file >> "cc_headers ")   && (_file >> _cc_headers))
      || ((_file >> "device ")	     && (_file >> _device_list))
      || ((_file >> "model ") 	     && (_file >> _model_list))
      || ((_file >> "h_direct ")     && (_file >> _h_direct))
      || ((_file >> "cc_direct ")    && (_file >> _cc_direct))
#endif
      ;
    if (!_file.more()) { untested();
      break;
    }else if (_file.stuck(&here)) {untested();
      _file.warn(0, "syntax error, need nature, discipline, or module");
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
