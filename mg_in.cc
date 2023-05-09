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
#include <stack>
#include <gnucap/u_opt.h>
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
+ `undef[10.4]
*/
/*--------------------------------------------------------------------------*/
void Define::parse(CS& f)
{
  f >> _name;
  f >> _args;
  // BUG: need nonempty macro_text
  _value = f.get_to("\n");
  if(_value.is_empty()){
    _value = std::string("1");
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void Define::preprocess(Define_List const& d)
{
  CS file(CS::_STRING, _value.to_string());
  trace1("Define::preprocess", _value.to_string());
  std::string stripped_file;
  size_t here = file.cursor();
  for (;;) {
    stripped_file += file.get_to("\"/`");
    if (file.match1('\"')) { untested();
      // quoted string
      stripped_file += '"' + file.ctos("", "\"", "\"", "") + '"';
    }else if (file >> "`else"
	    ||file >> "`endif"
	    ||file >> "`ifndef"
	    ||file >> "`ifdef"
	    ||file >> "`include"
	    ||file >> "`undef") {
      throw Exception_CS("not allowed here", file);
    }else if (file >> "`") {
      // macro substitution
      Define_List::const_iterator x = d.find(file);
      if (x != d.end()) {
	assert(*x);
	stripped_file += (*x)->substitute(file) + " ";
      }else{untested();
	// error: not defined
      }
    }else if (file.skip1('/')) { untested();
      stripped_file += "/";
    }else{
      // move on, just copy
    }
    if (!file.ns_more()) {
      trace1("prep", file.tail());
      // proper end of file
      break;
    }else if (file.stuck(&here)) { untested();
      // comment ran to end of file
      break;
    }else{
    }
  }
  _value = stripped_file;
}
/*--------------------------------------------------------------------------*/
void Define::dump(std::ostream& f)const
{
  f << "`define " << name();
  if(_args.size()){
   f << _args;
  }else{
  }
  f << " " << value() << "\n";
}
/*--------------------------------------------------------------------------*/
std::string Define::substitute(CS& f) const
{
  String_Arg_List values;
  values.parse_n(f, _args.size());

  std::map<std::string, String_Arg*> subs;
  auto j = values.begin();
  for(auto i : _args){
    if(j == values.end()){
      // BUG: positioning seems wrong
      throw Exception_CS("Need more values", f);
    }else{
      subs[i->to_string()] = *j;
      ++j;
    }
  }
  CS file(CS::_STRING, _value.to_string());
  std::string stripped_file = "";

  for (;;) {
    size_t here = file.cursor();
    std::string more;

    if(file.is_term()){
      more = file.peek();
      file.skip(1);
    }else{
      auto it = _args.find(file);
      if(it != _args.end()){
	auto k = values.begin();
	for(; it != _args.begin(); --it){
	  ++k;
	}
	more = (*k)->to_string();
      }else{
	file >> more;
      }
    }

    stripped_file = stripped_file + " " + more;

    if (!file.ns_more()) {
      break;
    }else if (file.stuck(&here)) {
      break;
    }else{
    }
  }

  return stripped_file;
}
/*--------------------------------------------------------------------------*/
std::string File::preprocess(const std::string& file_name)
{
  CS file(CS::_WHOLE_FILE, file_name);
  trace1("whole file", file.fullstring());

  std::string stripped_file;
  size_t here = file.cursor();
  int if_block = 0;
  int else_block = 0;
  for (;;) {
    stripped_file += file.get_to("\"/`");
    if (file.match1('\"')) {
      // quoted string
      stripped_file += '"' + file.ctos("", "\"", "\"", "") + '"';
    }else if (file >> "/*") { //---------------- C comment
      file >> dummy_c_comment; //BUG// line count may be wrong
    }else if (file >> "//") { //---------------- C++ comment
      file >> dummy_cxx_comment;
    }else if (file >> "`define") {
      size_t l = file.cursor();
      if(_define_list.find(file) != _define_list.end()){
	file.reset(l);
	std::string def = file.get_to("\n");
	_file.warn(0, "already defined: " + def);
      }else if(file >> _define_list){
	auto e = _define_list.back();
	e->preprocess(define_list());
      }else{ untested();
	unreachable();
      }
    }else if (file >> "`include") { //---------------- include
      std::string include_file_name;
      file >> include_file_name;
      stripped_file += include(include_file_name);
    }else if (file >> "`ifdef") {
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	++if_block;
      }else{
	file >> skip_block;
	++else_block;
      }
    }else if (file >> "`ifndef") {
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	file >> skip_block;
	++else_block;
      }else{
	String_Arg s;
	file >> s; // discard.
	++if_block;
      }
    }else if (file >> "`else") {
      if (if_block > 0) {
	file >> skip_block;
	--if_block;
      }else{untested();
	// error
      }
    }else if (file >> "`endif") {
      if (else_block > 0) {
	--else_block;
      }else if (if_block > 0) {
	--if_block;
      }else{untested();
	// error
      }
    }else if (file >> "`undef") {
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	_define_list.erase(x);
      }else{ untested();
	std::string err;
	file >> err;
	_file.warn(0, "not defined: " + err);
      }
    }else if (file >> "`") {
      // macro substitution
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	assert(*x);
	stripped_file += (*x)->substitute(file) + " ";
      }else{untested();
	// error: not defined
      }
    }else if (file.skip1('/')) {
      stripped_file += "/";
    }else{
      trace1("moveon", file.tail());
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
  std::string colon = "";
  if(_include_path.size()){
    colon = ":";
  }else{
  }
  _include_path = _include_path + colon + what;
}
/*--------------------------------------------------------------------------*/
void File::define(std::string const& what)
{
  CS cmd(CS::_STRING, what);
  cmd >> _define_list;
}
/*--------------------------------------------------------------------------*/
std::string File::include(std::string const& file_name)
{
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
    }else if (file.stuck(&here)) {untested();
      file.warn(0, "syntax error, need nature, discipline, or module");
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void File::read(std::string const& file_name)
{
  if(OPT::case_insensitive == 0){
  }else{ untested();
  }
  _name = file_name;
  std::string::size_type sepplace;
  sepplace = file_name.find_last_of("/");
  if(sepplace == std::string::npos){
    _cwd = ".";
  }else{
    _cwd = file_name.substr(0, sepplace);
  }
  _file = preprocess(file_name);
  parse(_file);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
