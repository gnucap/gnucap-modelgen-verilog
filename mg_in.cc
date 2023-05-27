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
static C_Comment   dummy_c_comment;
static Cxx_Comment dummy_cxx_comment;
static Skip_Block  skip_block;
/*--------------------------------------------------------------------------*/
void C_Comment::parse(CS& file)
{
//  size_t here = file.cursor();
  for (;;) {
    trace2("cp", file.tail(), file.fullstring());
    if(!file.skipto1('*')){ untested(); untested();
      file.get_line("");
    }else if (file >> "*/") {
      break;  // done with comment
    }else if (!file.more()){
      file.get_line("");
    }else{
      file.skip();
    }
  }
}
/*--------------------------------------------------------------------------*/
void Cxx_Comment::parse(CS& file)
{
  try{
    file.get_line("");
  }catch (Exception_End_Of_Input const&) {
    assert(!file.more());
  }
}
/*--------------------------------------------------------------------------*/
void Skip_Block::parse(CS& file)
{
  int nest = 0;
  // size_t here = file.cursor();
  for (;;) {
    trace1("Skip_Block", file.fullstring());
    file.skipto1('`');
    if (file >> "`endif") {
      if (nest == 0) {
	break;  // done with skip_block
      }else{ untested();untested();
	--nest;
      }
    }else if (file >> "`else") {
      if (nest == 0) {
	break;  // done with skip_block
      }else{ untested();untested();
      }
    }else if (file >> "`ifndef") { untested();untested();
      ++nest;
    }else if (file >> "`ifdef") { untested();untested();
      ++nest;
    }else if (!file.more()) {
      file.get_line("");
    //}else if (file.stuck(&here)) { untested();untested();
    //  file.warn(0, "unterminated ifdef block");
    //  break;
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

  _value = f.get_to("/");
  while (f.match1('/')) {
    if (f >> "//") {
      f.get_to("\n"); //  dummy_cxx_comment;
    }else if (f >> "/*") /* C comment */ {
      f >> dummy_c_comment; //BUG// line count may be wrong
      _value += f.get_to("/\n");
    }else{ untested();
      _value += f.get_to("/\n");
    }
  }
  if(_value.is_empty()){
  }else{
  }
  trace3("defparse", _name, _value, f.tail());
}
/*--------------------------------------------------------------------------*/
void Define::preprocess(Define_List const& d)
{
  CS file(CS::_STRING, _value.to_string());
  trace1("Define::preprocess", _value.to_string());
  std::string stripped_file;
  size_t here = file.cursor();
  // int if_block = 0;
  // int else_block = 0;
  for (;;) {
    stripped_file += file.get_to("\"/`");
    trace1("def prep got to", file.tail());
    if (file.match1('\"')) { untested(); untested();
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
	trace1("def prep sub", file.tail());
	stripped_file += (*x)->substitute(file) + "\n";
      }else{ untested();untested();
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
    }else if (file.stuck(&here)) { untested(); untested();
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
  if(_args.size()){ untested();
   f << _args;
  }else{
  }
  f << " " << value() << "\n";
}
/*--------------------------------------------------------------------------*/
std::string Define::substitute(CS& f) const
{
  trace2("subs", f.tail(), _args.size());
  Raw_String_Arg_List values;
  if(_args.size() == 0) {
  }else if(f.match1('(')){
    values.parse_n(f, int(_args.size()));
    trace1("parsed n", values.size());
  }else{
  }

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
    trace1("loop", file.tail());
    size_t here = file.cursor();
    std::string more;

    if(!file.peek()){
    }else if(file.is_term()){
      more = file.peek(); // BUG?
      file.skip(1); // WHAT?
    }else{
      trace1("arg?", file.tail());
      auto it = _args.find(file);
      if(it != _args.end()){ untested();
	auto k = values.begin();
	for(; it != _args.begin(); --it){ untested();
	  ++k;
	}
	more = (*k)->to_string();
      }else{
	file >> more;
      }
    }

//    trace2("loop grow", stripped_file, more);
    stripped_file = stripped_file + " " + more;

    if (!file.ns_more()) {
      break;
    }else if (file.stuck(&here)) { untested();
      break;
    }else{
    }
  }

  return stripped_file;
}
/*--------------------------------------------------------------------------*/
static void append_to(CS& file, std::string& to, std::string until)
{
//  trace2("append_to", file.tail(), file.more());
  if(!file.more()) {
    try{
      file.get_line("");
      trace1("got line", file.tail());
    }catch (Exception_End_Of_Input const&) {
      assert(!file.more());
    }
  }else{
  }

  while (file.more()) {
    to += file.get_to(until);
    trace2("got_to", until, file.tail());
    if(file.match1(until)){
      // match
      return;
    }else{
      to += "\n";
      try{
	file.get_line("");
	trace1("got line", file.tail());
      }catch( Exception_End_Of_Input const&){
	trace0("EOI");
      }
    }
  }

}
/*--------------------------------------------------------------------------*/
Preprocessor::Preprocessor() : CS(CS::_STRING, "")
{
}
/*--------------------------------------------------------------------------*/
void Preprocessor::parse(CS& file)
{
  size_t here = file.cursor();
  int if_block = 0;
  int else_block = 0;
  for (;;) {
    append_to(file, _stripped_file, "\"/`");
    if (!file.more()){
    }else if (file.match1('\"')) {
      // quoted string
      _stripped_file += '"' + file.ctos("", "\"", "\"", "") + '"';
    }else if (file >> "/*") /* C comment */ {
      file >> dummy_c_comment; //BUG// line count may be wrong
    }else if (file >> "//") /* C++ comment */ {
      file >> dummy_cxx_comment;
    }else if (file >> "`define") {
      size_t l = file.cursor();
      auto defi = _define_list.find(file);
      if(defi != _define_list.end()){
	file.reset(l);
	std::string def = file.tail();
	file.warn(0, "already defined: " + def);
	_define_list.erase(defi);
      }else{
      }
      if(file >> _define_list) {
	auto e = _define_list.back();
	e->preprocess(define_list());
      }else{ untested(); untested();
	unreachable();
      }
    }else if (file >> "`include") {
      std::string include_file_name;
      file >> include_file_name;
//      _stripped_file += 
	include(include_file_name);
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
	trace1("discard", s);
	++if_block;
      }
    }else if (file >> "`else") {
      if (if_block > 0) {
	file >> skip_block;
	--if_block;
      }else{ untested();untested();
	// error
      }
    }else if (file >> "`endif") {
      if (else_block > 0) {
	--else_block;
      }else if (if_block > 0) { untested();
	--if_block;
      }else{ untested();untested();
	// error
      }
    }else if (file >> "`undef") {
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	_define_list.erase(x);
      }else{ untested(); untested();
	std::string err;
	file >> err;
	file.warn(0, "not defined: " + err);
      }
    }else if (file >> "`") {
      trace2("match macro?", file.tail(), define_list().size());
      // macro substitution
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	trace1("match macro", file.tail());
	assert(*x);
	_stripped_file += (*x)->substitute(file) + " ";
      }else{ untested();
	throw Exception_CS("undefined macro ", file);
      }
    }else if (file.skip1('/')) {
      _stripped_file += "/";
    }else{
      trace1("moveon", file.tail());
      _stripped_file += "\n";
      // move on, just copy
    }

    trace2("more?", file.fullstring(), file.tail());
    if (!file.more()) {
      try {
	file.get_line("");
      }catch (Exception_End_Of_Input const&) {
	break;
      }
      trace1("got more", file.fullstring());
    }else if (!file.ns_more()) { untested();
      // proper end of file
      break;
    }else if (file.stuck(&here)) {
      // comment ran to end of file
      // break;
    }else{
    } untested();
  }
  CS::operator=(_stripped_file);
}
/*--------------------------------------------------------------------------*/
void Preprocessor::dump(std::ostream& out)const
{
  out << fullstring();
}
/*--------------------------------------------------------------------------*/
File::File() : _file(CS::_STRING, "")
{
}
/*--------------------------------------------------------------------------*/
void Preprocessor::add_include_path(std::string const& what)
{
  std::string colon = "";
  if(_include_path.size()){ untested();
    colon = ":";
  }else{
  }
  _include_path = _include_path + colon + what;
}
/*--------------------------------------------------------------------------*/
void Preprocessor::define(std::string const& what)
{
  CS cmd(CS::_STRING, what);
  cmd >> _define_list;
}
/*--------------------------------------------------------------------------*/
void Preprocessor::include(std::string const& file_name)
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

  read(full_file_name);

  _include_path = old_include_path;
  _cwd = old_cwd;
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
  _s = f.get_to(",)");
  f.skip1(",");
  trace1("RSA", _s);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
