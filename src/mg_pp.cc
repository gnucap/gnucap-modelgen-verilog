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
#include "mg_error.h"
#include <stack>
#include <u_opt.h>
/*--------------------------------------------------------------------------*/
class C_Comment : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ untested(); incomplete();}
} dummy_c_comment;
/*--------------------------------------------------------------------------*/
class Cxx_Comment : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ untested(); incomplete();}
} dummy_cxx_comment;
/*--------------------------------------------------------------------------*/
class Skip_Block : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ untested(); incomplete();}
} skip_block;
/*--------------------------------------------------------------------------*/
class Pragma : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ untested(); incomplete();}
} pragma;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// static Define_List& define_list()
// { untested();
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
  }else{untested();
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
//  trace2("append_to", f.tail().substr(0,10), f.ns_more());
  if(!f.ns_more()) {
    try{
//      to+="\n";
      f.get_line("");
//      trace2("got line", f.tail().substr(0,10), f.ns_more());
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
	file.reset(here);
	--nest;
	break;  // done with skip_block
      }else{
	--nest;
      }
    }else if (file >> "`else") {
      trace1("else in SB", nest);
      if (!nest) {
	file.reset(here);
	--nest;
	break;  // done with skip_block
      }else{
      }
    }else if (file >> "`elsif") {
      if(!nest) {
	file.reset(here);
	--nest;
	break;
      }else{
      }
    }else if (file >> "`ifndef") { untested();
      ++nest;
    }else if (file >> "`ifdef") {
      ++nest;
    }else if (!file.more()) {
      try{
	file.get_line("");
      }catch (Exception_End_Of_Input const& e) {
	file.warn(bDANGER, "unterminated conditional block");
	throw e;
      }
    }else {
      file.skip();
    }
  }
}
/*--------------------------------------------------------------------------*/
void Pragma::parse(CS& f)
{
  size_t here = f.cursor();
  if (f >> "reset{all}") { untested();
    // reset pragmas by name
    if (f >> "modelgen") { untested();
      //  modelgen_opts().reset(f); or so.
      incomplete();
    }else{ untested();
      incomplete();
    }
  }else if (f >> "resetall") { untested();
    // reset all pragmas recognised by implementation
    //  modelgen_opts().reset(f); or so.
    incomplete();
  }else if (f >> "warn") {
    f.warn(bWARNING, f.tail());
    f >> dummy_cxx_comment;
  }else if (f >> "error") {
    f.warn(bDANGER, f.tail());
    f >> dummy_cxx_comment;
    throw Exception("pragma error");
  }else if (f >> "modelgen") {
    f >> modelgen_opts();
  }else{ untested();
    f.reset(here);
    f.warn(bWARNING, "ignoring unrecognised pragma");
    f >> dummy_cxx_comment;
  }
}
/*--------------------------------------------------------------------------*/
/*
The following compiler directives are supported:untested();
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
    throw(Exception_CS_("syntax error", f));
  }
  while(f.is_alnum() || f.match1("_$")) {
    ret += f.ctoc();
  }

  return ret;
}
/*--------------------------------------------------------------------------*/
class PP_Quoted_String : public String {
public:
  void parse(CS& f) override {
    if(f >> "\"") {
      parse_(f);
    }else{
    }
  }
  std::string val_string()const override	{
    if(_data){
      // BUG: missing escape?
      return '"' + std::string(_data) + '"';
    }else{ untested();
      unreachable();
      return "nul"; // uh. make sure to query is_NA;
    }
  }

private:
  void parse_(CS& f) {
    std::string data; // = "\"";
    while(true) {
      while (f.peek() && (!isgraph(f.peek()))) {
	data += f.ctoc();
      }
      trace2("qs a1", data, f.peek());

      append_to(f, data, "\\\"\n");
      trace2("qs a2", data, f.peek());
      if (f.peek() == '"') {
	f.skip();
	break;
      }else if (f.peek() == '\n') { untested();
	data += '\\';
	data += f.ctoc();
      }else if (f.peek() == '\\') {
	data += f.ctoc();
	if(f.peek() == '\\'){
	  data += f.ctoc();
	}else if(f.peek() == '\"'){
	  data += f.ctoc();
	}else if(f.peek() == '\n'){
	  data += f.ctoc();
	}else{
	}
      }else{ untested();
	trace2("qs", data, f.peek());
	throw Exception_CS("need '\"'", f);
      }
    }
    // data += "\n";
    trace1("PP_Quoted_String", data);
    String::operator=(data);
  }
  void dump(std::ostream& o)const override;
} quoted_string;
/*--------------------------------------------------------------------------*/
void Define::stash(std::string const& s, String_Arg_List const& args)
{
  trace1("Def::stash", s);
  CS f(CS::_STRING, s);
  while(f.ns_more()) {
    trace2("stash more?", f.peek(), f.tail().substr(0,20));
    if (f.is_alpha() || f.peek() == '_'){
      std::string token = get_identifier(f);
      trace1("stash?", token);
      String_Arg_List::const_iterator x = args.find(String_Arg(token));

      if(x == args.end()){
	_value_tpl += token;
      }else{
	size_t pos = listpos(args, x);
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
}
/*--------------------------------------------------------------------------*/
void Define::parse(CS& f)
{
  _name = get_identifier(f);
  String_Arg_List args;
  if(f.peek() == '('){
    f >> args;
  }else{
  }

  stash(f.get_to("/\\\""), args);
  while (f.match1("/\\\n\"")) {
    if (f >> quoted_string) {
      _value_tpl += quoted_string.val_string();
      stash(f.get_to("\\/\n\""), args); // here?
    }else if (f >> "//") {
      f.get_to("\n"); //  dummy_cxx_comment;
    }else if (f >> "/*") /* C comment */ {
      f >> dummy_c_comment; //BUG// line count may be wrong
      stash(f.get_to("\\/\n"), args);
    }else if(f.match1('\\')) {
      f.skip();
      if(f.match1('\n')) {
	std::string more = f.get_to("\\/\n"); // BUG
	stash("\n" + more, args);
      }else{ untested();
	std::string more = f.get_to("\\/\n"); // BUG
	stash(more, args);
      }
    }else if(f.match1('/')){
      stash("/", args);
      f.skip();
      std::string more = f.get_to("\\/\n"); // BUG
      stash(more, args);
    }else if(f.match1('\n')){
      stash("\n", args);
      f.skip();
      stash(f.get_to("\\/\n\""), args); // here?
    }
  }

  _num_args = args.size();
} // Define::parse
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
void PP_Quoted_String::dump(std::ostream& o)const
{ untested();
  assert(_data);
 // o << '"';
  for(char* c=_data; c; ++c) { untested();
    if(*c=='\n'){ untested();
      o << '\\';
    }else{ untested();
    }
    o << *c;
  }
  //o << '"';
}
/*--------------------------------------------------------------------------*/
void Preprocessor::parse(CS& file)
{
  std::stack<int> _cond;
  size_t here = file.cursor();
  for (;;) {
    append_to(file, _stripped_file, "\"/`");
    if (!file.ns_more()){
    }else if (file >> quoted_string) {
      _stripped_file += quoted_string.val_string();
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
	throw Exception_CS_("expecting macro name", file);
      }
    }else if (file >> "`include") {
      std::string include_file_name;
      file >> include_file_name;
//      _stripped_file += 
	include(include_file_name);
    }else if (file >> "`ifdef") {
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	_cond.push(1);
      }else{
	_cond.push(0);
	file >> skip_block;
      }
    }else if (file >> "`ifndef") {
      Define_List::const_iterator x = define_list().find(file);
      if (x != define_list().end()) {
	file >> skip_block;
	_cond.push(0);
      }else{
	String_Arg s;
	file >> s; // discard.
	_cond.push(1);
      }
    }else if (file >> "`else") {
      if (_cond.empty()){ untested();
	throw Exception_CS_("not allowed here", file);
      }else if (_cond.top() != 1) {
      }else if( file >> skip_block ){
      }else{ untested();
      }
    }else if (file >> "`endif") {
      if (_cond.empty()){ untested();
	throw Exception_CS_("not allowed here", file);
      }else{
	_cond.pop();
      }
    }else if (file >> "`elsif") {
      if (_cond.empty()){ untested();
	throw Exception_CS_("not allowed here", file);
      }else{
      }
      Define_List::const_iterator x = define_list().find(file);
      if (_cond.top() == 1) {
	file >> skip_block;
      }else if (x != define_list().end()) {
	_cond.top() = 1;
      }else{
	file >> skip_block;
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
  if (_cond.empty()){
  }else{
    file.warn(bDANGER, "unmatched `ifdef\n");
  }
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
