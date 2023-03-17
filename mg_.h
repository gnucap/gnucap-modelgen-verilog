/*                             -*- C++ -*-
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
//testing=script 2006.10.31
#ifndef GNUCAP_MG_H
#define GNUCAP_MG_H
#include <gnucap/ap.h>
#include <gnucap/m_base.h>
#include <gnucap/m_expression.h>
#include <set>
/*--------------------------------------------------------------------------*/
#ifdef PASS_TRACE_TAGS
#define make_tag() (out << "//" << __func__ << ":" << __LINE__ << "\n")
#else
#define make_tag()
#endif
/*--------------------------------------------------------------------------*/
extern std::string ind;
/*--------------------------------------------------------------------------*/
inline std::string to_lower(std::string s)
{ untested();
  for (std::string::iterator i = s.begin(); i != s.end(); ++i) { untested();
    *i = static_cast<char>(tolower(*i));
  }
  return s;
}
/*--------------------------------------------------------------------------*/
inline std::string to_upper(std::string s)
{ untested();
  for (std::string::iterator i = s.begin(); i != s.end(); ++i) { untested();
    *i = static_cast<char>(toupper(*i));
  }
  return s;
}
/*--------------------------------------------------------------------------*/
inline void error(const std::string& message)
{untested();
  std::cerr << message << '\n';
  exit(1);
}
/*--------------------------------------------------------------------------*/
inline void os_error(const std::string& name)
{untested();
  error(name + ':' + strerror(errno));
}
/*--------------------------------------------------------------------------*/
#if 0 // m_base.h
class Base
{ untested();
public:
  virtual void parse(CS&) = 0;
  virtual void dump(std::ostream& f)const {unreachable(); f << "Base::dump";}
  virtual ~Base() {}
};
inline CS&	     operator>>(CS& f, Base& b)
				{untested();b.parse(f); return f;}
inline std::ostream& operator<<(std::ostream& f, const Base& d)
				{d.dump(f); return f;}
#endif
/*--------------------------------------------------------------------------*/
class Block;
class String_Arg :public Base {
  std::string	_s;
public:
  String_Arg() {}
  String_Arg(const char* s) : _s(s) {}
  explicit String_Arg(std::string const& s) : _s(s) {}
  //String_Arg(const std::string& s) : _s(s) {}
  const String_Arg&  key()const	  {return *this;}
  void parse(CS& f)			 {f >> _s;}
  void dump(std::ostream& f)const	 {f << _s;}
  void operator=(const std::string& s)	 {_s = s;}
  void operator+=(const std::string& s)	 {_s += s;}
  bool operator!=(const std::string& s)const {return _s != s;}
  bool operator==(const String_Arg& s)const {return _s == s._s;}
  bool			is_empty()const	 {return _s.empty();}
  std::string		lower()const	 {return to_lower(_s);}
  const std::string&	to_string()const {return _s;}
  void set_ctx(Block*){
    // incomplete();
  }
};
/*--------------------------------------------------------------------------*/
#if 0 // eek, also in m_base.h
template <class T>
class List_Base
  :public Base
{ untested();
  //protected:
public:
  typedef typename std::list<T*> _Std_List_T;
  _Std_List_T _list;
  virtual ~List_Base() { untested();
    for (typename std::list<T*>::iterator
	 i = _list.begin();
	 i != _list.end();
	 ++i) { untested();
      delete *i;
    }
  }
public:
  virtual void parse(CS& f) = 0;
  typedef typename std::list<T*>::const_iterator const_iterator;
  const_iterator begin()const	 {return _list.begin();}
  const_iterator end()const	 {return _list.end();}
  bool		 is_empty()const {return _list.empty();}
  size_t	 size()const	 {return _list.size();}
  const_iterator find(const String_Arg& s) const { untested();
    for (const_iterator ii = begin(); ii != end(); ++ii) { untested();
      assert(ii != end());
      assert(*ii);
      if (s == (**ii).key()) { untested();
	return ii;
      }else{ untested();
      }
    }
    return end();
  }
  const_iterator find(CS& file) const { untested();
    size_t here = file.cursor();
    String_Arg s;
    file >> s;
    const_iterator x = find(s);
    if (x == end()) { untested();
      file.reset(here);
    }else{ untested();
    }
    return x;
  }
  String_Arg _dummy;
  const String_Arg& operator[](const String_Arg& s) const { untested();
    const_iterator x = find(s);
    if (x != end()) { untested();
      assert(*x);
      return (**x).value();
    }else{ untested();
      return _dummy;
    }
  }
  
public:
  const T* front()const {untested();assert(!is_empty()); return _list.front();}
  const T* back()const {assert(!is_empty()); return _list.back();}
  T*   back() {assert(!is_empty()); return _list.back();}
  void push_back(T* x) {assert(x);       _list.push_back(x);}
  void pop_back() {assert(!is_empty()); _list.pop_back();}
};
#endif
/*--------------------------------------------------------------------------*/
class C_Comment :public Base {
public:
  void parse(CS& f);
  void dump(std::ostream&)const override{ incomplete();}
};
/*--------------------------------------------------------------------------*/
class Cxx_Comment :public Base {
public:
  void parse(CS& f);
  void dump(std::ostream&)const override{ incomplete();}
};
/*--------------------------------------------------------------------------*/
class Skip_Block :public Base {
public:
  void parse(CS& f);
  void dump(std::ostream&)const override{ incomplete();}
};
/*--------------------------------------------------------------------------*/
/* A "Collection" differs from a "LiSt" in how it is parsed.
 * Each parse of a "Collection" created one more object and stores
 * it in the Collection.  The size of the Collection therefore grows by 1.
 * A "Collection" is often parsed many times.
 * Each parse of a "LiSt" creates a bunch of objects, and storing them.
 * A list has opening and closing delimeters, usually {}.
 * A "LiSt" is usually parsed once.
 */
template <class T, char BEGIN, char SEP, char END>
class LiSt :public List_Base<T> {
  Block* _ctx{NULL};
//  using List_Base<T>::_list;
public:
  using List_Base<T>::size;
  using List_Base<T>::push_back;
  using List_Base<T>::begin;
  using List_Base<T>::end;
  typedef typename List_Base<T>::const_iterator const_iterator;
  //BUG//  why not inherited?
//  const_iterator begin()const	 {return _list.begin();}
//  const_iterator end()const	 {return _list.end();}

  void set_ctx(Block* b){ _ctx = b; }
  Block const* ctx() const{return _ctx;}
  void parse(CS& file) override{
    parse_n(file);
  }
  void parse_n(CS& file, unsigned max=-1u) {
    int paren = !BEGIN || file.skip1b(BEGIN);
    size_t here = file.cursor();
    for (;;) {
      if (file.stuck(&here)) {
	paren -= file.skip1b(END);
	if (paren == 0) {
	  //file.warn(0, "list exit");
	  break;
	}else{
	  //file.warn(0, "list");
	}
	T* p = new T;
	p->set_ctx(_ctx);
	file >> *p;
	if (file.stuck(&here)) { untested();
	  delete p;
	  file.warn(0, "not valid here");
	  break;
	}else if (max==size()){
	  throw Exception_Too_Many(size()+1, max, 0);
	}else{
	  push_back(p);
	}
      }else{ untested();
      }
    }
  }
public:
  void dump(std::ostream& f)const override {
    if (BEGIN) {
      f << BEGIN;
    }else{
    }
    for (const_iterator i = begin(); i != end(); ) {
      f << (**i);
      ++i;
      if (i == end()) {
	break;
      }else{
	if (SEP) {
	  f << SEP << ' ';
	}else{
	}
      }      
    }
    if (END) {
      f << END;
    }else{ untested();
    }
  }

  // List_Base? (see Collection)
  const_iterator find(const String_Arg& s) const {
    for (const_iterator ii = begin(); ii != end(); ++ii) {
      assert(ii != end());
      assert(*ii);
      if (s == (**ii).key()) {
	return ii;
      }else{
      }
    }
    return end();
  }
  const_iterator find(CS& file) const {
    size_t here = file.cursor();
    String_Arg s;
    file >> s;
    const_iterator x = find(s);
    if (x == end()) {
      file.reset(here);
    }else{
    }
    return x;
  }
};
typedef LiSt<String_Arg, '(', ',', ')'> String_Arg_List;
/*--------------------------------------------------------------------------*/
/* A "Collection" differs from a "LiSt" in how it is parsed.
 * Each parse of a "Collection" created one more object and stores
 * it in the Collection.  The size of the Collection therefore grows by 1.
 * A "Collection" is often parsed many times.
 * Each parse of a "LiSt" creates a bunch of objects, and storing them.
 * A list has opening and closing delimeters, usually {}.
 * A "LiSt" is usually parsed once.
 */
template <class T>
class Collection :public List_Base<T> {
  Block* _ctx{NULL};
public:
  using List_Base<T>::push_back;
  using List_Base<T>::begin;
  using List_Base<T>::end;
  typedef typename List_Base<T>::const_iterator const_iterator;

  void set_ctx(Block* c) { _ctx = c; }
  void parse(CS& file) {
    size_t here = file.cursor();
    T* m = new T(file);
    if (!file.stuck(&here)) {
      push_back(m);
      file.skip(0); // set _ok;
    }else{
      delete m;
      file.warn(0, "what's this??");
    }
  }
  void dump(std::ostream& f)const {
    for (const_iterator i = begin(); i != end(); ++i) {
      f << (**i);
    }
  }
  // List_Base?
  const_iterator find(const String_Arg& s) const {
    for (const_iterator ii = begin(); ii != end(); ++ii) {
      assert(ii != end());
      assert(*ii);
      if (s == (**ii).key()) {
	return ii;
      }else{
      }
    }
    return end();
  }
  const_iterator find(CS& file) const {
    size_t here = file.cursor();
    String_Arg s;
    file >> s;
    const_iterator x = find(s);
    if (x == end()) {
      file.reset(here);
    }else{
    }
    return x;
  }
  String_Arg _dummy;
  const String_Arg& operator[](const String_Arg& s) const { untested();
    const_iterator x = find(s);
    if (x != end()) { untested();
      assert(*x);
      return (**x).value();
    }else{ untested();
      return _dummy;
    }
  }
};
/*--------------------------------------------------------------------------*/
class Key :public Base {
  std::string _name;
  std::string _var;
  std::string _value;
public:
  void parse(CS& f) {f >> _name >> _var >> '=' >> _value >> ';';}
  void dump(std::ostream& f)const
  {f << name() << " " << var() << "=" << value() << "; ";}
  Key() : Base() {}
  const std::string& name()const	{return _name;}
  const std::string& var()const 	{return _var;}
  const std::string& value()const	{return _value;}
  void set_ctx(Block*){
    incomplete();
  }
};
typedef LiSt<Key, '{', '#', '}'> Key_List;
/*--------------------------------------------------------------------------*/
class Bool_Arg :public Base {
  bool _s;
public:
  void parse(CS& f)	{_s = true; f.skip1b(";");}
  void dump(std::ostream& f)const override {untested();f << _s;}
  Bool_Arg() :_s(false) {}
  operator bool()const {return _s;}
};
/*--------------------------------------------------------------------------*/
class Parameter_Base :public Base {
  Block* _ctx{NULL};
protected:
  std::string _name;
  std::string _type;
  std::string _code_name;
  std::string _user_name;
  std::string _alt_name;
  std::string _default_val;
  std::string _comment;
  std::string _print_test;
  std::string _calc_print_test;
  std::string _scale;
  std::string _offset;
  std::string _calculate;
  std::string _quiet_min;
  std::string _quiet_max;
  std::string _final_default;
  bool	      _positive;
  bool	      _octal;
public:
  // void parse(CS& f);
  // void print(std::ostream& f)const;
  Parameter_Base() :_positive(false), _octal(false) {}
  const std::string& type()const		{return _type;}
  const std::string code_name()const		{return "_p_" + _name;}
  const std::string& user_name()const		{return _user_name;}
  const std::string& alt_name()const		{return _alt_name;}
  const std::string& comment()const		{return _comment;}
  const std::string& default_val()const 	{return _default_val;}
  const std::string& print_test()const		{return _print_test;}
  const std::string& calc_print_test()const	{return _calc_print_test;}
  const std::string& scale()const		{return _scale;}
  const std::string& offset()const		{return _offset;}
  const std::string& calculate()const		{return _calculate;}
  const std::string& quiet_min()const		{return _quiet_min;}
  const std::string& quiet_max()const		{return _quiet_max;}
  const std::string& final_default()const	{return _final_default;}
  bool		positive()const			{return _positive;}
  bool		octal()const			{return _octal;}

  void fill_in_default_name() { untested();
    if (_user_name.empty()) { untested();
      _user_name = to_upper(_code_name);
    }else{ untested();
    }
  }
  void set_ctx(Block* c) { _ctx = c; }
  std::string const& name() const{ return _name; }
protected:
  Block* ctx(){ return _ctx; }
};
// typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
/*--------------------------------------------------------------------------*/
// 
class Parameter_1 :public Parameter_Base {
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Parameter_1() :Parameter_Base() {}
};
typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
/*--------------------------------------------------------------------------*/
// parameter type name = value ;
class Parameter_2 :public Parameter_Base {
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Parameter_2() :Parameter_Base() {}
};
typedef LiSt<Parameter_2, '\0', '\0', ';'> Parameter_2_List;
/*--------------------------------------------------------------------------*/
// .name(value)
// TODO? param_by_index?
class Parameter_3 :public Parameter_Base {
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Parameter_3() :Parameter_Base() {}
};
typedef LiSt<Parameter_3, '(', ',', ')'> Parameter_3_List;
/*--------------------------------------------------------------------------*/
class Code_Block :public Base {
  std::string _s;
  Block* _ctx{NULL};
public:
  void set_ctx(Block* c) { _ctx = c; }
  void parse(CS& f);
  void dump(std::ostream& f)const override{f << _s;}
  Code_Block() {}
  bool is_empty()const {return _s.length() < 2;}
};
/*--------------------------------------------------------------------------*/
class Parameter_Block :public Base {
  String_Arg	 _unnamed_value;
  Parameter_1_List _override;
  Parameter_1_List _raw;
  Parameter_1_List _calculated;
  Code_Block	 _code_pre;
  Code_Block	 _code_mid;
  Code_Block     _code_post;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  const String_Arg&	unnamed_value()const	{return _unnamed_value;}
  const Parameter_1_List& override()const 	{return _override;}
  const Parameter_1_List& raw()const		{return _raw;}
  const Parameter_1_List& calculated()const	{return _calculated;}
  const Code_Block&	code_pre()const		{return _code_pre;}
  const Code_Block&	code_mid()const		{return _code_mid;}
  const Code_Block&	code_post()const	{return _code_post;}
  bool is_empty()const {return (calculated().is_empty() 
				&& code_post().is_empty()
				&& code_mid().is_empty()
				&& override().is_empty()
				&& raw().is_empty()
				&& code_pre().is_empty());}
  void fill_in_default_values();
};
/*--------------------------------------------------------------------------*/
class Eval :public Base {
protected:
  String_Arg _name;
  Code_Block _code;
  Eval() :_name(), _code() {}
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Eval(CS& f) :_name(), _code() {parse(f);}
  const String_Arg&	name()const	{return _name;}
  const Code_Block&	code()const	{return _code;}
};
typedef Collection<Eval> Eval_List;
/*--------------------------------------------------------------------------*/
class Function :public Eval {
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Function(CS& f) :Eval() {parse(f);}
};
typedef Collection<Function> Function_List;
/*--------------------------------------------------------------------------*/
class Port_1 :public Base {
  std::string _name;
  std::string _short_to;
  std::string _short_if;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Port_1() {untested();}
  const std::string& name()const	{return _name;}
  const std::string& short_to()const 	{return _short_to;}
  const std::string& short_if()const 	{return _short_if;}
  void set_ctx(Block*){
    incomplete();
  }
};
typedef LiSt<Port_1, '{', '#', '}'> Port_1_List;
/*--------------------------------------------------------------------------*/
// TODO: Port_Base?
class Port_3 :public Base {
  std::string _name;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Port_3() {}
  const std::string&  name()const  {return _name;}

  void set_ctx(Block*) { }
};
// list ::= "(" port {"," port} ")"
typedef LiSt<Port_3, '(', ',', ')'> Port_3_List_2;
// list ::= port {"," port} ";"
typedef LiSt<Port_3, '\0', ',', ';'> Port_3_List_3;
/*--------------------------------------------------------------------------*/
// TODO: Port_Base?
class New_Port :public Port_3 {
  Block* _ctx{NULL};
public:
  void set_ctx(Block* c) { _ctx = c; }
  void parse(CS& f);
  New_Port() : Port_3() {}
};
// list ::= "(" port {"," port} ")"
typedef LiSt<New_Port, '(', ',', ')'> New_Port_List;
/*--------------------------------------------------------------------------*/
/// Some kind of sequential block with scope for parameters, variables..
class Probe;
class Branch;
class Node;
class Block : public List_Base<Base> {
  typedef std::map<std::string, Base const*> map; // set?
  typedef map::const_iterator const_iterator;
protected:
  map _var_refs;
  Block* _ctx{NULL};
public:
  template<class T>
  void new_var_ref(T const* what);
  virtual Probe const* new_probe(std::string const& xs, std::string const& p, std::string const& n) = 0;
  virtual Branch const* new_branch(std::string const& p, std::string const& n) = 0;
  virtual Node const* new_node(std::string const& p){ untested();
    assert(_ctx);
    return _ctx->new_node(p);
  }
  virtual Node const* node(std::string const& p) const{ untested();
    assert(_ctx);
    return _ctx->new_node(p);
  }

  void set_ctx(Block* b){
    _ctx = b;
  }
  Base const* resolve(std::string const& k) const{
    trace2("resolve", _ctx, k);
    for(auto x : _var_refs){
      trace1("var_ref", x.first);
    }
    const_iterator f = _var_refs.find(k);
    if(f != _var_refs.end()) {
      return f->second;
    }else if(_ctx) {
      return _ctx->resolve(k);
    }else{
      return NULL;
    }
  }
//  Base const* item(std::string const&name) { untested();
//    const_iterator f = _items.find(name);
//    if(f != _items.end()) { untested();
//      return f->second;
//    }else{ untested();
//      return NULL;
//    }
//  }
};
/*--------------------------------------------------------------------------*/
class Element_1 :public Base {
  std::string _dev_type;
  std::string _name;
  Port_1_List _port_list;
  std::string _eval;
  std::string _value;
  std::string _args;
  std::string _omit;
  std::string _reverse;
  std::string _state;
public:
  void parse(CS&);
  void dump(std::ostream& f)const;
  Element_1() {untested();}
  Element_1(CS& f) {parse(f);}
  const std::string& dev_type()const	{return _dev_type;}
  const Port_1_List& ports()const 	{return _port_list;}
  const std::string& name()const 	{return _name;}
  const std::string& eval()const 	{return _eval;}
  const std::string& value()const 	{return _value;}
  const std::string& args()const 	{return _args;}
  const std::string& omit()const 	{return _omit;}
  const std::string& reverse()const 	{return _reverse;}
  const std::string& state()const	{return _state;}
	size_t	     num_nodes()const	{return ports().size();}
};
typedef Collection<Element_1> Element_1_List;
/*--------------------------------------------------------------------------*/
class Element_2 :public Base {
  std::string _module_or_paramset_identifier;
  std::string _name_of_module_instance;
  Port_3_List_2 _list_of_port_connections;
  Parameter_3_List _list_of_parameter_assignments;
  std::string _eval;
  std::string _value;
  std::string _args;
  std::string _omit;
  std::string _reverse;
  std::string _state;
  Block* _ctx{NULL};
public:
  void parse(CS&) override;
  void dump(std::ostream& f)const override;
  Element_2() {}
  Element_2(CS& f) {
    parse(f);
  }
  void set_ctx(Block* b){ untested();
    _ctx = b;
  }
  const std::string& module_or_paramset_identifier()const {return _module_or_paramset_identifier;}
  const Parameter_3_List& 
		     list_of_parameter_assignments()const {return _list_of_parameter_assignments;}
  const Port_3_List_2& list_of_port_connections()const	  {return _list_of_port_connections;}
  const std::string& name_of_module_instance()const 	  {return _name_of_module_instance;}
  const std::string& eval()const 	{return _eval;}
  const std::string& value()const 	{return _value;}
  const std::string& args()const 	{return _args;}
  const std::string& omit()const 	{return _omit;}
  const std::string& reverse()const 	{return _reverse;}
  const std::string& state()const	{return _state;}
	size_t	     num_nodes()const	{return list_of_port_connections().size();}
};
typedef Collection<Element_2> Element_2_List;
/*--------------------------------------------------------------------------*/
class Arg :public Base {
  std::string _arg;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override {f << "      " << arg() << ";\n";}
  Arg() {}
  const std::string& arg()const {return _arg;}
  void set_ctx(Block*){
    incomplete();
  }
};
typedef LiSt<Arg, '{', '#', '}'> Arg_List;
/*--------------------------------------------------------------------------*/
class Args :public Base {
  String_Arg _name;
  String_Arg _type;
  Arg_List   _arg_list;
public:
  void parse(CS& f) {f >> _name >> _type >> _arg_list;}
  void dump(std::ostream& f)const
  {f << "    args " << name() << " " << type() << "\n"
     << arg_list() << "\n";}
  Args(CS& f) {parse(f);}
  const String_Arg& name()const {return _name;}
  const String_Arg& type()const {return _type;}
  const Arg_List&   arg_list()const {return _arg_list;}
  typedef Arg_List::const_iterator const_iterator;
  const_iterator begin()const	{return _arg_list.begin();}
  const_iterator end()const	{return _arg_list.end();}
};
typedef Collection<Args> Args_List;
/*--------------------------------------------------------------------------*/
class Circuit :public Base {
  Port_1_List	_required_nodes;
  Port_1_List	_optional_nodes;
  Port_1_List	_local_nodes;
  Element_1_List _element_list;
  Args_List	_args_list;
  bool		_sync;
public:
  void parse(CS&);
  void dump(std::ostream& f)const override;
  Circuit() : _sync(false) {}
  bool		      sync()const	 {return _sync;}
  const Port_1_List&    req_nodes()const   {return _required_nodes;}
  const Port_1_List&    opt_nodes()const   {return _optional_nodes;}
  const Port_1_List&    local_nodes()const {return _local_nodes;}
  const Element_1_List& elements()const	 {return _element_list;}
  const Args_List&    args_list()const	 {return _args_list;}
	size_t	      min_nodes()const	 {return req_nodes().size();}
        size_t	      max_nodes()const {return opt_nodes().size()+min_nodes();}
	size_t	      net_nodes()const	 {untested();return max_nodes();}
};
/*--------------------------------------------------------------------------*/
#if 0
class Probe :public Base {
  std::string _name;
  std::string _expression;
public:
  void parse(CS& f) {f >> _name >> '=' >> _expression >> ';';}
  void dump(std::ostream& f)const
	{f << "    " << name() << " = \"" << expression() << "\";\n";}
  Probe() {untested();}
  Probe(CS& f) {parse(f);}
  const std::string& name()const	{return _name;}
  const std::string& expression()const	{return _expression;}
};
typedef LiSt<Probe, '{', '#', '}'> Probe_List;
#endif
// Name clash
class Probe : public Base {
  std::string _name;
//  Branch const* _p;
  std::string _pp;
  std::string _pn;
public:
  explicit Probe(std::string const& Name, std::string const&, std::string const& p, std::string const& n)
    : _name(Name), _pp(p), _pn(n) {}
  std::string const& name()const {return _name;}
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override {incomplete();}

  std::string const& pname() const{ return _pp; }
  std::string const& nname() const{ return _pn; }
  std::string code_name() const{
    return "_"+_name;
  }

};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Model :public Base {
  String_Arg		_name;
  String_Arg		_level;
  String_Arg		_dev_type;
  String_Arg		_inherit;
  Key_List		_public_key_list;
  Key_List		_private_key_list;
  Parameter_Block	_independent;
  Parameter_Block	_size_dependent;
  Parameter_Block	_temperature;
  Code_Block		_tr_eval;
  Code_Block		_validate;
  Bool_Arg		_hide_base;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Model(CS& f) {parse(f);}
  bool			 hide_base()const	{return _hide_base;}
  const String_Arg&	 name()const		{return _name;}
  const String_Arg&	 level()const		{return _level;}
  const String_Arg&	 dev_type()const	{return _dev_type;}
  const String_Arg&	 inherit()const		{return _inherit;}
  const Key_List&	 public_key_list()const	{return _public_key_list;}
  const Key_List&	 private_key_list()const{return _private_key_list;}
  const Parameter_Block& independent()const	{return _independent;}
  const Parameter_Block& size_dependent()const	{return _size_dependent;}
  const Parameter_Block& temperature()const	{return _temperature;}
  const Code_Block&	 tr_eval()const		{return _tr_eval;}
  const Code_Block&	 validate()const	{return _validate;}
};
typedef Collection<Model> Model_List;
/*--------------------------------------------------------------------------*/
class Device :public Base {
  String_Arg		_name;
  String_Arg		_parse_name;
  String_Arg		_id_letter;
  String_Arg		_model_type;
  Circuit		_circuit;
  // Probe_List		_probes;
  Parameter_Block	_device;
  Parameter_Block	_common;
  Code_Block		_tr_eval;
  Eval_List		_eval_list;
  Function_List		_function_list;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Device(CS& f) {parse(f);}
  const String_Arg&	 name()const		{return _name;}
  const String_Arg&	 parse_name()const	{return _parse_name;}
  const String_Arg&	 id_letter()const	{return _id_letter;}
  const String_Arg&	 model_type()const	{return _model_type;}
  const Circuit&	 circuit()const		{return _circuit;}
  // const Probe_List&	 probes()const		{return _probes;}
  const Parameter_Block& device()const		{return _device;}
  const Parameter_Block& common()const		{return _common;}
  const Code_Block&	 tr_eval()const		{return _tr_eval;}
  const Eval_List&	 eval_list()const	{return _eval_list;}
  const Function_List&	 function_list()const	{return _function_list;}
    	size_t		 min_nodes()const	{return circuit().min_nodes();}
    	size_t		 max_nodes()const	{return circuit().max_nodes();}
    	size_t	    net_nodes()const {untested();return circuit().net_nodes();}
};
typedef Collection<Device> Device_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Head :public Base {
  std::string _s;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override {f << _s;}
  Head() {}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Attribute :public Base {
  String_Arg _name;
  String_Arg _value;
public:
  void parse(CS& f) {f >> _name >> '=' >> _value >> ';';}
  void dump(std::ostream& f)const
	  {f << "  " << name() << " = \"" << value() << "\";\n";}
  Attribute(CS& f) {parse(f);}
  const String_Arg&  key()const	  {return _name;}
  const String_Arg&  name()const  {return _name;}
  const String_Arg&  value()const {return _value;}
};
typedef Collection<Attribute> Attribute_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Define :public Base {
  String_Arg _name;
  String_Arg_List _args;
  String_Arg _value;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Define(CS& f) {parse(f);}
  const String_Arg&  key()const   {return _name;}
  const String_Arg&  name()const  {return _name;}
  const String_Arg&  value()const {return _value;}

  std::string substitute(CS& f) const;
  void preprocess(Collection<Define> const&);
};
typedef Collection<Define> Define_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Nature :public Base {
  String_Arg		_identifier;
  String_Arg		_parent_nature;
  Attribute_List	_attributes;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Nature(CS& f) {parse(f);}
  const String_Arg&	identifier()const	{return _identifier;}
  const String_Arg&	parent_nature()const	{return _parent_nature;}
  const Attribute_List&	attributes()const	{return _attributes;}
  const String_Arg&	operator[](const String_Arg& k)const {return _attributes[k];}
  const String_Arg&	access()const		{return _attributes["access"];}
};
typedef Collection<Nature> Nature_List;
/*--------------------------------------------------------------------------*/
class Discipline :public Base {
  String_Arg	_identifier;
  String_Arg	_potential_ident;
  String_Arg	_flow_ident;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Discipline(CS& f) {parse(f);}
  const String_Arg&  identifier()const	    {return _identifier;}
  const String_Arg&  potential_ident()const {return _potential_ident;}
  const String_Arg&  flow_ident()const	    {return _flow_ident;}
};
typedef Collection<Discipline> Discipline_List;
/*--------------------------------------------------------------------------*/
class Branch : public Base {
  std::string _pname; // TODO: object...
  std::string _nname; // TODO: object...
public:
  explicit Branch(std::string const& p, std::string const& n)
    : _pname(p), _nname(n) {}
  std::string name()const {return _pname+_nname;}
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class Node;
// TODO: decide if it is a Block, or has-a Block aka Scope or whetever.
class Module :public Block {
  typedef std::map<std::string, Probe*> Probe_Map;
  typedef std::map<std::string, Branch*> Branch_Map;
  typedef std::map<std::string, Node*> Node_Map;
private: // verilog input data
  String_Arg	_identifier;
  New_Port_List	_ports;
  Port_3_List_3	_input;
  Port_3_List_3	_output;
  Port_3_List_3	_inout;
  Port_3_List_3	_ground;
  Port_3_List_3	_electrical;
  Parameter_2_List _parameters;
  Parameter_2_List _local_params;
  Element_2_List _element_list;
  Port_1_List	_local_nodes;
  Code_Block		_tr_eval;
  Code_Block		_validate;
private: // elaboration data
  Probe_Map _probes;
  Branch_Map _branches;
  Node_Map _nodes;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Module(CS& f) {
    // do we need a second pass? or just connect the dots while reading in?
    _ports.set_ctx(this);
    _input.set_ctx(this);
    _output.set_ctx(this);
    _inout.set_ctx(this);
    _ground.set_ctx(this);
    _electrical.set_ctx(this);
    _parameters.set_ctx(this);
    _local_params.set_ctx(this);
    _element_list.set_ctx(this);
    _local_nodes.set_ctx(this);
    _tr_eval.set_ctx(this);
    _validate.set_ctx(this);
    parse(f);
  }
  const String_Arg&	  identifier()const	{return _identifier;}
  const New_Port_List&	  ports()const		{return _ports;}
  const Port_3_List_3&	  input()const		{return _input;}
  const Port_3_List_3&	  output()const		{return _output;}
  const Port_3_List_3&	  inout()const		{return _inout;}
  const Port_3_List_3&	  ground()const		{return _ground;}
  const Port_3_List_3&	  electrical()const	{return _electrical;}
  const Parameter_2_List& parameters()const	{return _parameters;}
  const Parameter_2_List& local_params()const	{return _local_params;}
  const Element_2_List&	  element_list()const	{return _element_list;}
  const Element_2_List&	  circuit()const	{return _element_list;}
  const Port_1_List&	  local_nodes()const	{return _local_nodes;}
  const Code_Block&	 tr_eval()const		{return _tr_eval;}
  const Code_Block&	 validate()const	{return _validate;}
    	size_t		  min_nodes()const	{return ports().size();}
    	size_t		  max_nodes()const	{return ports().size();}
    	size_t		  net_nodes()const	{return ports().size();}
public:
  const Probe_Map&	 probes()const		{return _probes;}
  const Node_Map&	 nodes()const		{return _nodes;}
  const Branch_Map&	 branches()const	{return _branches;}
private: // misc
  CS& parse_analog(CS& cmd);

private:
  Probe const* new_probe(std::string const&, std::string const&, std::string const&) override;
  Branch const* new_branch(std::string const&, std::string const&) override;
  Node const* new_node(std::string const& p) override;
  Node const* node(std::string const& p) const override;
}; // Module
typedef Collection<Module> Module_List;
/*--------------------------------------------------------------------------*/
class Expression;
class Variable : public Base {
private:
  Block* _ctx{NULL};
  std::string _name;
protected:
  std::set<Probe const*> _deps;
  Block const* ctx() const{ return _ctx; }
public:
  std::set<Probe const*> const& deps()const { return _deps; }
  Variable(std::string const& name)
   :Base()
   ,_name(name)
  {}
  ~Variable() {
  }
public:
  std::string const& name()const {return _name;}

  void set_ctx(Block* ctx) {_ctx = ctx;}
  virtual void parse(CS&) { untested();
    incomplete();
  }
  void dump(std::ostream& o)const override { o << "Variable: incomplete\n"; }
  Branch const* new_branch(std::string const& p, std::string const& n) {
    assert(_ctx);
    return(_ctx->new_branch(p, n));
  }

private:
  bool is_node(std::string const& n) const{
    assert(_ctx);
    return _ctx->node(n);
  }

protected:
  void resolve_symbols(Expression const& e, Expression& E);
};
/*--------------------------------------------------------------------------*/
// analog_procedural_assignment
class Assignment : public Variable {
protected:
  std::string _lhsname;
  Expression* _rhs;
public:
  Assignment(std::string const& lhsname)
   :Variable(lhsname)
   ,_lhsname(lhsname)
   ,_rhs(NULL) {}
  ~Assignment();
public:
  std::string const& lhsname()const {return _lhsname;}
  Expression const* rhs()const {return _rhs;}
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override { incomplete(); }
};
/*--------------------------------------------------------------------------*/
class Contribution : public Assignment {
protected: // BUG
  std::string _name;
protected:
  Branch const* _branch;
public:
  Contribution(std::string const& lhsname)
    : Assignment(lhsname), _branch(NULL) {}

  std::string const& name() const{return _name;}
  Branch const* branch() const{return _branch;}
};
/*--------------------------------------------------------------------------*/
class PotContribution : public Contribution {
public:
  PotContribution(std::string const& lhsname) : Contribution(lhsname) {}
public:
  void parse(CS&)override;
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class FlowContribution : public Contribution {
public:
  FlowContribution(std::string const& lhsname) : Contribution(lhsname) {}
public:
  void parse(CS&)override;
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Node :public Base {
  std::string _name;
  std::string _short_to;
  std::string _short_if;
public:
  void parse(CS&)override{};
  void dump(std::ostream&)const{};
  Node() {untested();}
  Node(CS& f) {parse(f);}
  Node(std::string const& f) : _name(f) {}
  const std::string& name()const	{return _name;}
//  const std::string& short_to()const 	{return _short_to;}
//  const std::string& short_if()const 	{return _short_if;}
};
/*--------------------------------------------------------------------------*/
class Node_List : public List<Node> {
public:
	explicit Node_List() : List<Node>() {}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// analog_procedural_block
class AnalogBlock : public Block {
public:

  // this is a stub
  CS& parse_seq(CS& cmd);
  CS& parse_real(CS& cmd);

//  void set_ctx(Block* ctx) {_ctx = ctx;}
  void parse(CS& cmd) override;
  void dump(std::ostream& o)const override;
  Probe const* new_probe(std::string const& xs, std::string const& p, std::string const& n)override {
    assert(_ctx);
    return _ctx->new_probe(xs, p, n);
  }
  Branch const* new_branch(std::string const& p, std::string const& n)override {
    assert(_ctx);
    return _ctx->new_branch(p, n);
  }
  Node const* node(std::string const& n)const override {
    assert(_ctx);
    return _ctx->node(n);
  }
/*--------------------------------------------------------------------------*/
};
/*--------------------------------------------------------------------------*/
class File {
  std::string	_name;
  std::string   _cwd;
  std::string   _include_path;
  CS		_file;
  Head		_head;
  Code_Block	_h_headers;
  Code_Block	_cc_headers;
  Model_List	_model_list;
  Device_List	_device_list;
  Code_Block	_h_direct;
  Code_Block	_cc_direct;
  Define_List	_define_list;
  Nature_List	_nature_list;
  Discipline_List _discipline_list;
  Module_List	_module_list;
  Module_List	_macromodule_list;
  Module_List	_connectmodule_list;
private:
  std::string preprocess(const std::string& file_name);
  std::string include(const std::string& file_name);
public: // build
  File();
  void read(std::string const&);
  void define(std::string const&);
  void add_include_path(std::string const&);

public: // readout
  const std::string& name()const	{return _name;}
  const std::string fullstring()const	{return _file.fullstring();}
  const Head&	     head()const	{return _head;}
  const Code_Block&  h_headers()const	{return _h_headers;}
  const Code_Block&  cc_headers()const	{return _cc_headers;}
  const Model_List&  models()const	{return _model_list;}
  const Module_List&  modules()const	{return _module_list;}
  const Device_List& devices()const	{return _device_list;}
  const Code_Block&  h_direct()const	{return _h_direct;}
  const Code_Block&  cc_direct()const	{return _cc_direct;}

  const Define_List&	 define_list()const	{return _define_list;}
  const Nature_List&	 nature_list()const	{return _nature_list;}
  const Discipline_List& discipline_list()const {return _discipline_list;}
  const Module_List&	 module_list()const	{return _module_list;}
  const Module_List&	 macromodule_list()const	{return _macromodule_list;}
  const Module_List&	 connectmodule_list()const	{return _connectmodule_list;}

};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// TODO: clean this up. name in intermediate base class or so.
template<class T>
void Block::new_var_ref(T const* what)
{
  assert(what);
  std::string p = what->name();
  trace1("new_var_ref", p);
  Base const* cc = _var_refs[p];
  if(cc) { untested();
    incomplete(); // already there, error?
  }else{
    _var_refs[p] = what;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
