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
#include "mg_pp.h"
#include <stack>
#include <u_opt.h>
/*--------------------------------------------------------------------------*/
class C_Comment : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ incomplete();}
} dummy_c_comment;
/*--------------------------------------------------------------------------*/
class Cxx_Comment : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ incomplete();}
} dummy_cxx_comment;
/*--------------------------------------------------------------------------*/
class Skip_Block : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ incomplete();}
} skip_block;
/*--------------------------------------------------------------------------*/
class Pragma : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ incomplete();}
} pragma;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// static Define_List& define_list()
// {
//   static Define_List _define_list;
//   return _define_list;
// }
/*--------------------------------------------------------------------------*/
// copied in from a_construct.
static std::string getlines(FILE *fileptr);
CS& CS::get_line(const std::string& prompt)
{
  ++_line_number;
  if (is_file()) {
    _cmd = getlines(_file);
    _cnt = 0;
    _length = _cmd.length();
    _ok = true;
  }else{itested();
    assert(_file == stdin);
    char cmdbuf[BUFLEN];
    getcmd(prompt.c_str(), cmdbuf, BUFLEN);
    _cmd = cmdbuf;
    _cnt = 0;
    _length = _cmd.length();
    _ok = true;
  }

  if (OPT::listing) { untested();
    IO::mstdout << "\"" << fullstring() << "\"\n";
  }else{
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
static std::string getlines(FILE *fileptr)
{
  const bool spice_style_line_continuation = false; // arg? flag?
  assert(fileptr);
  const int buffer_size = BIGBUFLEN;
  std::string s;

  bool need_to_get_more = true;  // get another line (extend)
  while (need_to_get_more) {
    char buffer[buffer_size+1];
    char* got_something = fgets(buffer, buffer_size, fileptr);
    if (!got_something) { // probably end of file
      need_to_get_more = false;
      if (s == "") {
	throw Exception_End_Of_Input("");
      }else{
      }
    }else if(spice_style_line_continuation) { untested();
      trim(buffer);
      size_t count = strlen(buffer);
      if (count && buffer[count-1] == '\\') { untested();
	buffer[count-1] = '\0';
      }else{ untested();
	// look ahead at next line
	//int c = fgetc(fileptr);
	int c;
	while (isspace(c = fgetc(fileptr))) { untested();
	  // skip
	}
	if (c == '+') { untested();
	  need_to_get_more = true;
	}else if (c == '\n') {unreachable();
	  need_to_get_more = true;
	  ungetc(c,fileptr);
	}else{ untested();
	  need_to_get_more = false;
	  ungetc(c,fileptr);
	}
      }
    }else{
      size_t count = strlen(buffer);
      if (buffer[count-2] == '\\') {
	assert(buffer[count-1] == '\n');
	assert(buffer[count] == '\0');
      }else{
	need_to_get_more = false;
      }
      s += buffer;
    }
  }
  trace1("getlines", s.substr(0, 30));
  return s;
}
/*--------------------------------------------------------------------------*/
static void append_to(CS& f, std::string& to, std::string until)
{
  trace2("append_to", f.tail().substr(0,10), f.ns_more());
  if(!f.ns_more()) {
    try{
//      to+="\n";
      f.get_line("");
      trace2("got line", f.tail().substr(0,10), f.ns_more());
    }catch (Exception_End_Of_Input const&) {
      assert(!f.ns_more());
    }
  }else{
  }

  // while (f.peek() && (!isgraph(f.peek()))) { untested();
  //   to += f.ctoc();
  // }

  while (f.ns_more()) {
    std::string chunk = f.get_to(until);
    to += chunk;
    if(f.match1(until)){
      // match
      return;
    }else{
      trace3("no match", until, chunk, f.tail().substr(0,30));
//      to += "\n"; // BUG? BUG1
      try{
	f.get_line("");
	trace2("got line2 ", f.tail().substr(0,10), f.ns_more());
      }catch( Exception_End_Of_Input const&){
	trace2("EOI", to.substr(0,20), chunk);
      }
    }
  }

}
/*--------------------------------------------------------------------------*/
void C_Comment::parse(CS& file)
{
//  size_t here = file.cursor();
  for (;;) {
    if(!file.skipto1('*')){ untested(); untested();
      file.get_line("");
    }else if (file >> "*/") {
      break;  // done with comment
    }else if (!file.ns_more()){
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
    // comment ran to EOF
    while(file.ns_more()){
      file.skip();
    }
  }
}
/*--------------------------------------------------------------------------*/
void Skip_Block::parse(CS& file)
{
  int nest = 0;
  for (;;) {
    size_t here = file.cursor();
    file.skipto1('`');
    if (file >> "`endif") {
      if (nest == 0) {
	break;  // done with skip_block
      }else{itested();
	--nest;
      }
    }else if (file >> "`else") {
      trace1("else in SB", nest);
      if (nest == 0) {
	file.reset(here);
	break;  // done with skip_block
      }else{
      }
    }else if (file >> "`elsif") {
      if(!nest) {
	file.reset(here);
	break;
      }else{
      }
    }else if (file >> "`ifndef") { untested();
      ++nest;
    }else if (file >> "`ifdef") {itested();
      ++nest;
    }else if (!file.more()) {
      file.get_line("");
    //}else if (file.stuck(&here)) { untested();
    //  file.warn(0, "unterminated ifdef block");
    //  break;
    }else{
      file.skip();
    }
  }
}
/*--------------------------------------------------------------------------*/
void Pragma::parse(CS& f)
{ untested();
  size_t here = f.cursor();
  if (f >> "reset{all}") { untested();
    // reset pragmas by name
    if (f >> "modelgen") { untested();
      //  modelgen_opts().reset(f); or so.
      incomplete();
    }else{
      incomplete();
    }
  }else if (f >> "resetall") {
    // reset all pragmas recognised by implementation
    //  modelgen_opts().reset(f); or so.
    incomplete();
  }else if (f >> "warn") { untested();
    f.warn(bWARNING, f.tail());
    f >> dummy_cxx_comment;
  }else if (f >> "error") { untested();
    f.warn(bDANGER, f.tail());
    f >> dummy_cxx_comment;
    throw Exception("pragma error");
  }else if (f >> "modelgen") { untested();
    f >> modelgen_opts();
  }else{ untested();
    f.reset(here);
    f.warn(bWARNING, "ignoring unrecognised pragma");
    f >> dummy_cxx_comment;
  }
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
+ `ifndef[1364-2005]
+ `include[1364-2005]
- `line[1364-2005]
- `nounconnected_drive [1364-2005]
+ `pragma[1364-2005]
- `resetall[1364-2005]
- `timescale[1364-2005]
- `unconnected_drive[1364-2005]
+ `undef[10.4]
*/
/*--------------------------------------------------------------------------*/
template<class T>
size_t listpos(T const& l, typename T::const_iterator i)
{
  size_t ret = 0;
  typename T::const_iterator j = l.begin();
  while(i!=j){
    ++ret;
    ++j;
  }
  return ret;
}
/*--------------------------------------------------------------------------*/
// PP_identifier::parse?
static std::string get_identifier(CS& f)
{
  std::string ret;
  if(f.is_alpha() || f.match1('_')){
    ret += f.ctoc();
  }else{
    throw(Exception_CS("syntax error", f));
  }
  while(f.is_alnum() || f.match1("_$")) {
    ret += f.ctoc();
  }

  return ret;
}
/*--------------------------------------------------------------------------*/
void Define::stash(std::string const& s, String_Arg_List const& args)
{
  CS f(CS::_STRING, s);
  while(f.ns_more()) {
    if (f.is_alpha() || f.peek() == '_'){
      std::string token = get_identifier(f);
      trace1("stash?", token);
      String_Arg_List::const_iterator x = args.find(String_Arg(token));

      if(x == args.end()){
	_value_tpl += token;
      }else{
	size_t pos = listpos(args, x);
	trace2("mark", _value_tpl.size(), pos);
	_pos.push_back(std::make_pair(_value_tpl.size(), pos));
      }
    }else{
      _value_tpl += f.ctoc();
    }
  }
  size_t ts = _value_tpl.size();
  if(!ts){
  }else if(_value_tpl[ts-1] == '\n') {
    _value_tpl.resize(ts-1);
  }else{
  }
  trace1("stashed", _value_tpl);
}
/*--------------------------------------------------------------------------*/
void Define::parse(CS& f)
{
  _name = get_identifier(f);
  trace2("define::parse", _name, f.tail().substr(0,10));
  String_Arg_List args;
  if(f.peek() == '('){
    f >> args;
  }else{
  }

  stash(f.get_to("/\\"), args);
  while (f.match1('/') || f.match1('\\') || f.match1('\n')) {
    if (f >> "//") {
      f.get_to("\n"); //  dummy_cxx_comment;
    }else if (f >> "/*") /* C comment */ {
      f >> dummy_c_comment; //BUG// line count may be wrong
      stash(f.get_to("\\/\n"), args);
    }else if(f.match1('\\')) {
      f.skip();
      if(f.match1('\n')) {
	std::string more = f.get_to("\\/\n"); // BUG
	trace1("more?", more);
	stash("\n" + more, args);
      }else{itested();
	std::string more = f.get_to("\\/\n"); // BUG
	trace1("more?", more);
	stash(more, args);
      }
    }else if(f.match1('/')){
      stash("/", args);
      f.skip();
      std::string more = f.get_to("\\/\n"); // BUG
      trace1("more?", more);
      stash(more, args);
    }else if(f.match1('\n')){
      stash("\n", args);
      f.skip();
      stash(f.get_to("\\/\n"), args);
    }
  }

  _num_args = args.size();
}
/*--------------------------------------------------------------------------*/
static String_Arg_List eval_args(CS& f, size_t howmany, Define_List const&);
/*--------------------------------------------------------------------------*/
static std::string expand_macro(CS& file, Define_List const& d)
{
  trace1("exmacro", file.tail().substr(0,10));
  std::string stripped_file;
  if (file >> "else"
      ||file >> "endif"
      ||file >> "ifndef"
      ||file >> "ifdef"
      ||file >> "include"
      ||file >> "undef") {
    throw Exception_CS("not allowed here", file);
  }else{
    // macro substitution
    std::string token = get_identifier(file);
    trace1("find", token);
    Define_List::const_iterator x = d.find(String_Arg(token));
    if (x != d.end()) {
      assert(*x);
      auto values = eval_args(file, (*x)->num_args(), d);
      std::string subst = (*x)->substitute(values, d);
      stripped_file += subst; //  + "\n";
    }else{ untested();
      throw(Exception_CS("undefined macro", file));
    }
  }
  return stripped_file;
}
/*--------------------------------------------------------------------------*/
std::string expand_macros(CS& file, Define_List const& d)
{
  std::string stripped_file;
  size_t here = file.cursor();
  for (;;) {
    stripped_file += file.get_to("\"/`");
    if (file >> '"') {
      // quoted string
      stripped_file += '"' + file.get_to("\"");
      if (file >> '"') {
	stripped_file += '"';
      }else{ untested();
	throw Exception_CS("need '\"'", file);
      }
    }else if (file >> "`") {
      stripped_file += expand_macro(file, d);
    }else if (file.skip1('/')) {
      stripped_file += "/";
    }else{
      // move on, just copy
    }
    if (!file.ns_more()) {
      // proper end of file
      break;
    }else if (file.stuck(&here)) { untested();
      // comment ran to end of file
      break;
    }else{
    }
  }
  return stripped_file;
}
/*--------------------------------------------------------------------------*/
void Raw_String_Arg::parse(CS& f)
{
  int par = 0;
  assert(_s=="");
  bool quote = false;
  while(f.ns_more()) {
    char p = f.peek();
    if(p == '\"') {
      quote = !quote;
      _s += f.ctoc();
    }else if(quote) {
      _s += f.ctoc();
    }else if(p == '(') {
      ++par;
      _s += f.ctoc();
    }else if(p == ')') {
      if(!par){
	break;
      }else{
	--par;
	_s += f.ctoc();
      }
    }else if(par) {
      _s += f.ctoc();
    }else if(p == '\\') {
      f.skip();
      p = f.peek();
      if(p=='\n'){
//	f.skip();
      }else{ untested();
	_s += '\\';
      }
    }else if(p == ',') {
      f.skip();
      break;
    }else {
      _s += f.ctoc();
    }
  }
  trace1("got rsa", _s);
}
/*--------------------------------------------------------------------------*/
// Define::?
static String_Arg_List eval_args(CS& f, size_t howmany, Define_List const& d)
{
  trace2("eval_rgs", f.tail().substr(0,20), howmany);
  Raw_String_Arg_List values;
  if(!howmany) {
  }else if(f.match1('(')){
    values.parse_n(f, howmany);
    for(auto i : values){
      trace1("parse_n dbg", i->to_string());
    }
  }else{
  }

  if(values.size() == howmany){
  }else{
    throw Exception_CS("Need more values", f);
  }

  String_Arg_List ret;

  for(auto i : values){
    CS file(CS::_STRING, i->to_string());
    std::string value = expand_macros(file, d);
    ret.push_back(new String_Arg(value));
  }

  return ret;
}
/*--------------------------------------------------------------------------*/
void Define::dump(std::ostream&)const
{ untested();
//  f << "`define " << name();
//  if(_args.size()){ untested();
//   f << _args;
//  }else{ untested();
//  }
//  f << " " << value() << "\n";
}
/*--------------------------------------------------------------------------*/
std::string Define::substitute(String_Arg_List const& values, Define_List const& d) const
{
//  trace2("substitute", file.fullstring(), values.size());
  std::string stripped_file = "";
  std::string sep = "";

  size_t seek = 0;
  for(auto const& p : _pos){
    auto k = values.begin();
//    if(p.second < values.size()){ untested();
//    }else{ untested();
//      throw Exception_CS("need more values", file);
//    }
    std::advance(k, p.second);
    trace2("place",seek, p.first);
    stripped_file += _value_tpl.substr(seek, p.first-seek);
    seek = p.first;
    stripped_file += (*k)->to_string();
  }
  stripped_file += _value_tpl.substr(seek, std::string::npos);

  CS ff(CS::_STRING, stripped_file);
  return expand_macros(ff, d);
}
/*--------------------------------------------------------------------------*/
Preprocessor::Preprocessor() : CS(CS::_STRING, "")
{
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
  assert(file.fullstring()=="");

  parse(file);
}
/*--------------------------------------------------------------------------*/
class PP_Quoted_String : public String {
public:
  void parse(CS& f) override {
    _data = "";
    while(true){
      while (f.peek() && (!isgraph(f.peek()))) {
	_data += f.ctoc();
      }
      trace2("qs a1", _data, f.peek());

      append_to(f, _data, "\\\"\n");
      trace2("qs a2", _data, f.peek());
      if (f.peek() == '"') {
	f.skip();
	break;
      }else if (f.peek() == '\n') { untested();
	_data += '\\';
	_data += f.ctoc();
      }else if (f.peek() == '\\') {
	_data += f.ctoc();
	if(f.peek() == '\\'){
	  _data += f.ctoc();
	}else if(f.peek() == '\"'){
	  _data += f.ctoc();
	}else if(f.peek() == '\n'){
	  _data += f.ctoc();
	}else{itested();
	}
      }else{ untested();
	trace2("qs", _data, f.peek());
	throw Exception_CS("need '\"'", f);
      }
    }
    trace1("PP_Quoted_String", _data);
  }
  void dump(std::ostream& o)const override;
} quoted_string;
/*--------------------------------------------------------------------------*/
void PP_Quoted_String::dump(std::ostream& o)const{ untested();
  for(char c : _data){ untested();
    if(c=='\n'){ untested();
      o << '\\';
    }else{ untested();
    }
    o << c;
  }
}
/*--------------------------------------------------------------------------*/
void Preprocessor::parse(CS& file)
{
  size_t here = file.cursor();
  int if_block = 0;
  int else_block = 0;
  for (;;) {
    append_to(file, _stripped_file, "\"/`");
    if (!file.ns_more()){
    }else if (file >> '"') {
      file >> quoted_string;
      _stripped_file += '"' + quoted_string.val_string() + '"';
    }else if (file >> "/*") /* C comment */ {
      file >> dummy_c_comment; //BUG// line count may be wrong
    }else if (file >> "//") /* C++ comment */ {
      file >> dummy_cxx_comment;
    }else if (file >> "`define") {
      size_t l = file.cursor();
      auto defi = define_list().find(file);
      if(defi != define_list().end()){
	file.reset(l);
	std::string def = file.tail();
	file.warn(0, "already defined: " + def);
	define_list().erase(defi);
      }else{
      }
      if(file >> define_list()) {
      }else{ untested();
	throw Exception_CS("expecting macro name", file);
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
	++if_block;
      }
    }else if (file >> "`else") {
      if (if_block > 0) {
	trace3("else skip", file.tail(), if_block, else_block);
	file >> skip_block;
	--if_block;
	--else_block;
      }else{
	// error
      }
    }else if (file >> "`endif") {
      trace3("endif", file.tail(), if_block, else_block);
      if (else_block > 0) {
	--else_block;
      }else{
      }
      if (if_block > 0) {
	--if_block;
      }else{
	// error
      }
    }else if (file >> "`elsif") {
      Define_List::const_iterator x = define_list().find(file);
      trace3("elsif", if_block, else_block, file.tail());
      if (if_block == else_block) {
	file >> skip_block;
      }else if (else_block == 0) {
	file >> skip_block;
	++else_block;
      }else if (x != define_list().end()) {
	++if_block;
	--else_block;
      }else{
	file >> skip_block;
	// ++else_block;
      }
    }else if (file >> "`undef") {
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	define_list().erase(x);
      }else{ untested(); untested();
	std::string err;
	file >> err;
	file.warn(0, "not defined: " + err);
      }
    }else if (file >> "`pragma") {
      file >> pragma;
    }else if (file >> "`") {
      String_Arg id(get_identifier(file));
      Define_List::const_iterator x = define_list().find(id);
      if (x != define_list().end()) {
	assert(*x);
	auto values = eval_args(file, (*x)->num_args(), define_list());
	std::string subst = (*x)->substitute(values, define_list());
	trace1("match macro", subst);
	_stripped_file += subst + " ";
      }else{
	throw Exception_CS("undefined macro", file);
      }
    }else if (file.skip1('/')) {
      _stripped_file += "/";
    }else{ untested();
      unreachable();
//      _stripped_file += "\n";
      // move on, just copy
    }

    if (!file.ns_more()) {
      try {
//	_stripped_file += "\n"; // BUG?
	file.get_line("");
      }catch (Exception_End_Of_Input const&) {
	break;
      }
      trace1("got more", file.fullstring().substr(0,10));
    }else if (!file.ns_more()) { untested();
      // proper end of file
      break;
    }else if (file.stuck(&here)) {
      // comment ran to end of file
      // break;
    }else{
    }
  }
  CS::operator=(_stripped_file);
}
/*--------------------------------------------------------------------------*/
void Preprocessor::dump(std::ostream& out)const
{
  out << fullstring();
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
  cmd >> define_list();
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
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
