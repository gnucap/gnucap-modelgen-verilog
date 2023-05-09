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
class Base {
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
class Block;
class Owned_Base : public Base {
  Block* _owner{NULL};
public:
  explicit Owned_Base() : Base() { }
  explicit Owned_Base(Owned_Base const& b) : Base(), _owner(b._owner) { }
public:
  void set_owner(Block* b){  _owner = b; }
  Block const* owner() const{ untested(); return _owner;}
protected:
  Block* owner(){ return _owner;}
};
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
  void set_owner(Block*){
    // incomplete();
  }
};
/*--------------------------------------------------------------------------*/
// a stub
class Attribute_Instance :public Base {
public:
  void parse(CS& f);
  void dump(std::ostream& o)const override{}
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
template <class T, char BEGIN, char SEP, char END, char END2='\0', char END3='\0'>
class LiSt :public List_Base<T> {
  Block* _owner{NULL};
public:
  using List_Base<T>::size;
  using List_Base<T>::push_back;
  using List_Base<T>::begin;
  using List_Base<T>::end;
  typedef typename List_Base<T>::const_iterator const_iterator;

  void set_owner(Block* b){ _owner = b; }
  Block const* owner() const{return _owner;}
  Block* owner(){return _owner;}
  void parse(CS& file) override{
    parse_n(file);
  }
// protected:??
  void parse_n(CS& file, unsigned max=-1u) {
    parse_n_<T>(file, max);
  }
protected:
  template<class TT>
  void parse_n_(CS& file, unsigned max=-1u) {
    int paren = !BEGIN || file.skip1b(BEGIN);
    size_t here = file.cursor();
    for (;;) {
      if (file.stuck(&here)) {
	paren -= file.skip1b(END);
	if (paren == 0) {
	  //file.warn(0, "list exit");
	  break;
	}else if (END2 && file.peek() == END2) {
	  break;
	}else if (END3 && file.peek() == END3) {
	  break;
	}else{
	  //file.warn(0, "list");
	}
	TT* p = new TT;
	if(owner()){
	}else{
	}
	p->set_owner(owner());
	file >> *p;
	if (file.stuck(&here)) {
	  delete p;
	  file.warn(0, "not valid here");
	  std::cout.flush();
	  assert(false);
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
    }else{
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
class File;
template <class T>
class Collection :public List_Base<T> {
  Block* _owner{NULL};
  File const* _file{NULL};
public:
  using List_Base<T>::push_back;
  using List_Base<T>::begin;
  using List_Base<T>::end;
  using List_Base<T>::back;
  using List_Base<T>::is_empty;
  using List_Base<T>::pop_back;
  typedef typename List_Base<T>::const_iterator const_iterator;

  void set_owner(Block* c) { _owner = c; }
  Block const* owner() const{return _owner;}
  Block* owner(){return _owner;}
  void set_file(File const* f){ _file = f; }
  void parse(CS& file) {
    size_t here = file.cursor();
    T* m = new T;
    m->set_owner(_owner);
    file >> *m;
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
  const String_Arg& operator[](const String_Arg& s) const {
    const_iterator x = find(s);
    if (x != end()) {
      assert(*x);
      return (**x).value();
    }else{ untested();
      return _dummy;
    }
  }
  void clear(){
    while(!is_empty()){
      T* b = back();
      pop_back();
      delete b;
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
  void set_owner(Block*){
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
// class AnalogExpression?
class ConstantMinTypMaxExpression :public Base {
  Expression* _e{NULL};
  Block* _owner{NULL};
public:
  explicit ConstantMinTypMaxExpression() : Base(){}
  ~ConstantMinTypMaxExpression();
  void set_owner(Block* b){ _owner = b; }
public:
  Expression const& expression()const{ assert(_e); return *_e; }
  bool empty()const;
  void parse(CS& file)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Parameter_Base :public Base {
  Block* _owner{NULL};
protected:
  std::string _name;
  std::string _type;
  std::string _code_name;
  std::string _user_name;
  std::string _alt_name;
  ConstantMinTypMaxExpression _default_val;
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
  const ConstantMinTypMaxExpression& default_val()const 	{return _default_val;}
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
  void set_owner(Block* c) { _owner = c; }
  std::string const& name() const{ return _name; }
protected:
  Block* owner(){ return _owner; }
};
// typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
/*--------------------------------------------------------------------------*/
// 
class Parameter_1 :public Parameter_Base {
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
  Parameter_1() :Parameter_Base() {}
};
typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
/*--------------------------------------------------------------------------*/
class ValueRangeSpec :public Base {
  // incomplete();
};
/*--------------------------------------------------------------------------*/
class ValueRangeConstant :public ValueRangeSpec {
  std::string _cexpr;
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class ValueRangeStrings :public ValueRangeSpec {
};
/*--------------------------------------------------------------------------*/
class ValueRangeInterval :public ValueRangeSpec {
  std::string _lb;
  std::string _ub;
  bool _ub_is_closed;
  bool _lb_is_closed;
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class ValueRange :public Owned_Base {
  enum{
    vr_FROM,
    vr_EXCLUDE
  } _type;
  ValueRangeSpec* _what{NULL};
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
};
typedef LiSt<ValueRange, '\0', '\0', '\0', ',', ';'> ValueRangeList;
/*--------------------------------------------------------------------------*/
// parameter type name = value ;
class Parameter_2 :public Parameter_Base {
  ValueRangeList _value_range_list;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Parameter_2() :Parameter_Base() {}
  void set_type(std::string const& a){_type=a;}
};
/*--------------------------------------------------------------------------*/
class Parameter_2_List : public LiSt<Parameter_2, '\0', ',', ';'> {
  String_Arg _type;
  bool _is_local{false};
public:
  bool is_local()const {return _is_local;}
  String_Arg const& type()const {return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Parameter_List_Collection : public Collection<Parameter_2_List>{
public:
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Probe;
class Deps : public std::set<Probe const*>{
  typedef std::set<Probe const*> S;
  typedef S::const_iterator const_iterator;
public:
  ~Deps();
  std::pair<const_iterator, bool> insert(Probe const* x){
    return std::set<Probe const*>::insert(x);
  }
  void update(Deps const& other){
    for(auto i : other){
      insert(i);
    }
  }
};
/*--------------------------------------------------------------------------*/
class Branch;
class Branch_Ref : public Owned_Base {
  Branch* _br{NULL};
  bool _r;
protected:
  std::string _alias;
public:
  Branch_Ref(Branch_Ref&& b);
  Branch_Ref(Branch_Ref const& b);
  explicit Branch_Ref() : Owned_Base() {}
  explicit Branch_Ref(Branch* b, bool reversed=false);
  ~Branch_Ref();
  operator bool() const{return _br;}
  Branch_Ref& operator=(Branch_Ref const& o);
  Branch_Ref& operator=(Branch_Ref&& o);
public:
  void parse(CS&) override;
  void dump(std::ostream& o)const override;

  operator Branch*() const{ return _br; }
  Branch* operator->() const{ return _br; }
  bool is_reversed() const{return _r;}

  std::string const& pname() const;
  std::string const& nname() const;
};
/*--------------------------------------------------------------------------*/
class Token_PROBE; //bug?
class Node;
class Expression;
class Variable : public Base {
  Block* _owner{NULL};
protected:
  std::string _name;
  Deps _deps;
  Block const* owner() const{ return _owner; }
public:
  explicit Variable() : Base() {}
  Variable(std::string const& name)
   :Base()
   ,_name(name)
  {}
  ~Variable() {
  }
public:
  Deps const& deps()const { return _deps; }
  std::string const& name()const {return _name;}
  virtual bool is_module_variable() const;

  void set_owner(Block* owner) {_owner = owner;}
  void parse(CS&) override { untested();
    incomplete();
  }
  void dump(std::ostream& o)const override;
  virtual Branch_Ref new_branch(std::string const& p, std::string const& n);
  virtual Branch_Ref new_branch(Node const* p, Node const* n);

private:
  bool is_node(std::string const& n)const;

protected:
  Block* owner(){ return _owner; }
  void new_var_ref();
  void resolve_symbols(Expression const& e, Expression& E);
private:
  Token* resolve_filter_function(Expression& E, std::string const& name, Deps const&);
  Token_PROBE* resolve_xs_function(Expression& E, std::string const& name, Deps const&);
};
/*--------------------------------------------------------------------------*/
class Variable_2 :public Variable {
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Variable_2() : Variable() {}
//   void set_type(std::string const& a){_type=a;}
};
/*--------------------------------------------------------------------------*/
class Variable_List : public LiSt<Variable_2, '\0', ',', ';'> {
  String_Arg _type; // ENUM?
public:
  String_Arg const& type()const {return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Variable_List_Collection : public Collection<Variable_List>{
public:
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
// class Localparam :public Parameter_2 {
// public:
//   void dump(std::ostream& f)const override;
//   Localparam() :Parameter_2() {}
// };
/*--------------------------------------------------------------------------*/
typedef Parameter_2_List Localparam_List;
// class Localparam_List : public LiSt<Parameter_2, '\0', '\0', ';'> {
// };
/*--------------------------------------------------------------------------*/
class Localparam_List_Collection : public Collection<Localparam_List>{
public:
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
// .name(value)
// TODO? param_by_index?
class Parameter_3 :public Parameter_Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const;
  Parameter_3() :Parameter_Base() {}
};
typedef LiSt<Parameter_3, '(', ',', ')'> Parameter_3_List;
/*--------------------------------------------------------------------------*/
class Code_Block :public Base {
  std::string _s;
  Block* _owner{NULL};
public:
  void set_owner(Block* c) { _owner = c; }
  void parse(CS& f)override;
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
  void parse(CS& f)override;
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
/*--------------------------------------------------------------------------*/
/// Some kind of sequential block with scope for parameters, variables..
class Deps;
class Filter;
class Node;
class Nature;
class Block : public List_Base<Base> {
  typedef std::map<std::string, Base const*> map; // set?
  typedef map::const_iterator const_iterator;
protected:
  map _var_refs;
private:
  Block* _owner{NULL};
public:
  Block const* owner() const{ return _owner;}
  Block* owner(){ return _owner;}
public:
  template<class T>
  void new_var_ref(T const* what);
  virtual Probe const* new_probe(std::string const& xs, std::string const& p, std::string const& n)
  {unreachable(); return NULL;}
  virtual Probe const* new_probe(std::string const& xs, Branch_Ref const&)
  {unreachable(); return NULL;}

  virtual Node* new_node(std::string const& p){ untested();
    assert(_owner);
    return _owner->new_node(p);
  }
  virtual Node const* node(std::string const& p) const{ untested();
    assert(_owner);
    return _owner->new_node(p); // new??
  }
  virtual Filter const* new_filter(std::string const& xs, Deps const& d) {
    assert(_owner);
    return _owner->new_filter(xs, d);
  }

  virtual Branch_Ref new_branch(std::string const& p, std::string const& n) {
    unreachable();
    return Branch_Ref(NULL);
  }
  virtual Branch_Ref new_branch(Node const*, Node const*) {
    unreachable();
    return Branch_Ref(NULL);
  }
  virtual Branch_Ref branch(std::string const& n)const {
    assert(_owner);
    return _owner->branch(n);
  }
  virtual Branch_Ref const& new_branch_name(std::string const& p, Branch_Ref const& r) { untested();
    assert(_owner);
    return _owner->new_branch_name(p, r);
  }

  void set_owner(Block* b){
    _owner = b;
  }
  Base const* resolve(std::string const& k) const;

//  Base const* item(std::string const&name) { untested();
//    const_iterator f = _items.find(name);
//    if(f != _items.end()) { untested();
//      return f->second;
//    }else{ untested();
//      return NULL;
//    }
//  }
  void push_back(Base* x);
};
/*--------------------------------------------------------------------------*/
// analog_procedural_block analog_statement?
class AnalogStmt : public Base {
public:
//  void parse(CS& cmd)override;
//  void dump(std::ostream& o)const override;
};
/*--------------------------------------------------------------------------*/
class SeqBlock : public Block {
public:
  void parse(CS& cmd)override{incomplete();}
  void dump(std::ostream& o)const override;

//  Block?
  Probe const* new_probe(std::string const& xs, std::string const& p, std::string const& n)override {
    assert(owner());
    return owner()->new_probe(xs, p, n);
  }
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br) override {
    assert(owner());
    return owner()->new_probe(xs, br);
  }
  Branch_Ref new_branch(std::string const& p, std::string const& n)override {
    assert(owner());
    return owner()->new_branch(p, n);
  }
  Node const* node(std::string const& n)const override {
    assert(owner());
    return owner()->node(n);
  }
  Branch_Ref branch(std::string const& n)const override {
    assert(owner());
    return owner()->branch(n);
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// mg_analog.h?
class AnalogSeqBlock : public AnalogStmt {
  SeqBlock _block;
public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* owner) {_block.set_owner(owner);}

  SeqBlock const& block() const{ return _block; }
};
/*--------------------------------------------------------------------------*/
class AnalogConditionalExpression : public Owned_Base /*expression?*/ {
  Expression* _exp{NULL};
public:
  ~AnalogConditionalExpression();
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  Block* owner() {return Owned_Base::owner();}
  Expression const& expression() const{ assert(_exp); return *_exp;}
};
/*--------------------------------------------------------------------------*/
class AnalogConditionalStmt : public AnalogStmt {
  AnalogConditionalExpression _cond;
  Base* _true_part{NULL};
  Base* _false_part{NULL};
public:
  ~AnalogConditionalStmt(){
    delete _true_part;
    _true_part = NULL;
    delete _false_part;
    _false_part = NULL;
  }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* o) {_cond.set_owner(o);}
  Block* owner() {return _cond.owner();}
  AnalogConditionalExpression const& conditional() const{return _cond;}
  const Base* true_part_or_null() const{ return _true_part; }
  const Base* false_part_or_null() const{ return _false_part; }
  const Base& true_part() const{assert(_true_part); return *_true_part; }
  const Base& false_part() const{assert(_false_part); return *_false_part; }
};
/*--------------------------------------------------------------------------*/
class AnalogConstruct : public Owned_Base {
  Base* _stmt{NULL};
public:
  ~AnalogConstruct(){
    delete _stmt;
    _stmt = NULL;
  }
private: // this is a stub
  CS& parse_seq(CS& cmd); // TODO
  CS& parse_flow_contrib(CS& cmd, std::string const&);
  CS& parse_pot_contrib(CS& cmd, std::string const&);
  CS& parse_real(CS& cmd);
  bool parse_assignment(CS& cmd);

public: // this is a stub
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;

  Base const* statement_or_null() const{ return _stmt; }
};
typedef Collection<AnalogConstruct> AnalogList;
#if 0
class Eval :public Base {
protected:
  String_Arg _name;
  Code_Block _code;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Eval(CS& f) :_name(), _code() {parse(f);}
  Eval() :_name(), _code() {}
  const String_Arg&	name()const	{return _name;}
  const Code_Block&	code()const	{return _code;}
};
typedef Collection<Eval> Eval_List;
/*--------------------------------------------------------------------------*/
class Function :public Eval {
public:
  void set_owner(Block const*) { }
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Function() :Eval() {}
};
typedef Collection<Function> Function_List;
#endif
/*--------------------------------------------------------------------------*/
class Port_1 :public Base {
  std::string _name;
  std::string _short_to;
  std::string _short_if;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Port_1() {}
  const std::string& name()const	{return _name;}
  const std::string& short_to()const 	{return _short_to;}
  const std::string& short_if()const 	{return _short_if;}
  void set_owner(Block*){
    incomplete();
  }
};
typedef LiSt<Port_1, '{', '#', '}'> Port_1_List;
/*--------------------------------------------------------------------------*/
// TODO: Port_Base?
class Port_3 :public Owned_Base {
  std::string _name;
  std::string _value;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Port_3() {}
  const std::string& name()const  {return _name;}
  const std::string& value()const  {
    if(has_identifier()){
      return _value;
    }else{
      return _name;
    }
  }
  bool has_identifier()const;
};
// list ::= "(" port {"," port} ")"
typedef LiSt<Port_3, '(', ',', ')'> Port_3_List_2;
// list ::= port {"," port} ";"
typedef LiSt<Port_3, '\0', ',', ';'> Port_3_List_3;
/*--------------------------------------------------------------------------*/
class Port_Connection_List : public LiSt<Port_3, '(', ',', ')'> {
  bool _has_names{false};
  // Block* _owner{NULL};
public:
  void parse(CS& f)override;
  bool has_names() const {return _has_names;}
//  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
// TODO: Port_Base?
class New_Port :public Port_3 {
  Block* _owner{NULL};
public:
  void set_owner(Block* c) { _owner = c; }
  void parse(CS& f);
  New_Port() : Port_3() {}
};
// list ::= "(" port {"," port} ")"
typedef LiSt<New_Port, '(', ',', ')'> New_Port_List;
/*--------------------------------------------------------------------------*/
class Discipline;
// TODO: Port_Base?
class Node;
class Net_Identifier :public Port_3 {
  Block* _owner{NULL};
  Node* _node{NULL};
public:
  Net_Identifier() : Port_3() {}
protected:
  Block* owner(){return _owner;}
  void set_node(Node*n){_node = n;}
public:
  void set_owner(Block* c) { _owner = c; }
  void parse(CS& f);
  void set_discipline(Discipline const* d);
};
/*--------------------------------------------------------------------------*/
typedef LiSt<Net_Identifier, '\0', ',', ';'> Net_Decl_List;
/*--------------------------------------------------------------------------*/
class Net_Identifier_Discipline : public Net_Identifier {
public:
   void parse(CS& f)override;
};
/*--------------------------------------------------------------------------*/
class Net_Identifier_Ground : public Net_Identifier {
public:
   void parse(CS& f)override;
};
/*--------------------------------------------------------------------------*/
class Net_Decl_List_Discipline : public Net_Decl_List {
  Discipline const* _disc{NULL};
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  void set_discipline(Discipline const* d){_disc = d;}
};
/*--------------------------------------------------------------------------*/
class Net_Decl_List_Ground : public Net_Decl_List {
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Net_Declarations : public Collection<Net_Decl_List>{
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
typedef String_Arg Branch_Identifier;
class List_Of_Branch_Identifiers : public LiSt<Branch_Identifier, '\0', ',', ';'>{
public:
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Branch_Declaration : public Branch_Ref{
  List_Of_Branch_Identifiers _list;
public:
  explicit Branch_Declaration() : Branch_Ref() {}
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Branch_Declarations : public Collection<Branch_Declaration>{
  Branch_Ref _branch_ref;
public:
  void parse(CS& f)override;
  // void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
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
  void set_owner(Block const*) { }
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
  std::string state()const	{return _state;}
	size_t	     num_nodes()const	{return ports().size();}
};
typedef Collection<Element_1> Element_1_List;
/*--------------------------------------------------------------------------*/
// Component?
class Element_2 :public Owned_Base {
  std::string _module_or_paramset_identifier;
  std::string _name_of_module_instance;
  Port_Connection_List _list_of_port_connections;
  Port_1_List _current_port_list;
  Parameter_3_List _list_of_parameter_assignments;
  std::string _eval;
  std::string _value;
  std::string _args;
  std::string _omit;
  std::string _reverse;
  std::string _state;
//  Block* _owner{NULL};
public:
  void parse(CS&) override;
  void dump(std::ostream& f)const override;
  explicit Element_2() {}
  virtual ~Element_2() {}
  Element_2(CS& f) {
    parse(f);
  }
//  void set_owner(Block* b) { _owner = b; }
//  const std::string& module_or_paramset_identifier()const {return _module_or_paramset_identifier;}
  virtual const std::string& dev_type()const {return _module_or_paramset_identifier;}
  virtual Nature const* nature()const {return NULL;}
  virtual Discipline const* discipline()const {return NULL;}
  const Parameter_3_List& 
		     list_of_parameter_assignments()const {return _list_of_parameter_assignments;}
  const Port_Connection_List& ports()const	  {return _list_of_port_connections;}
  const Port_1_List& current_ports() const{return _current_port_list;}
  virtual std::string instance_name()const  {return _name_of_module_instance;}
  virtual std::string short_label()const 	  {return _name_of_module_instance;}
  virtual std::string code_name()const  {return "_e_" + _name_of_module_instance;}
  const std::string& eval()const 	{return _eval;}
  const std::string& value()const 	{return _value;}
  const std::string& args()const 	{return _args;}
  virtual const std::string& omit()const 	{return _omit;}
  const std::string& reverse()const 	{return _reverse;}
  virtual std::string state()const	{return _state;}
  virtual size_t	     num_nodes()const	{return ports().size();}
  virtual size_t	     num_states()const	{unreachable(); return 0;}
//  bool is_reversed() const{return _reverse;}
};
typedef Collection<Element_2> Element_2_List;
typedef Collection<Filter> Filter_List;
/*--------------------------------------------------------------------------*/
class Arg :public Base {
  std::string _arg;
public:
  void parse(CS& f);
  void dump(std::ostream& f)const override {f << "      " << arg() << ";\n";}
  Arg() {}
  const std::string& arg()const {return _arg;}
  void set_owner(Block*){
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
  void set_owner(Block const*){}
  void parse(CS& f) {f >> _name >> _type >> _arg_list;}
  void dump(std::ostream& f)const
  {f << "    args " << name() << " " << type() << "\n"
     << arg_list() << "\n";}
  Args(){}
  const String_Arg& name()const {return _name;}
  const String_Arg& type()const {return _type;}
  const Arg_List&   arg_list()const {return _arg_list;}
  typedef Arg_List::const_iterator const_iterator;
  const_iterator begin()const	{return _arg_list.begin();}
  const_iterator end()const	{return _arg_list.end();}
};
typedef Collection<Args> Args_List;
/*--------------------------------------------------------------------------*/
#if 0
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
#endif
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
  std::string _xs;
  Branch_Ref _br;
  bool _is_flow_probe{false};
  bool _is_pot_probe{false}; // type?
public:
  explicit Probe(std::string const& xs, Branch_Ref b);
//  std::string const& name()const {return _name;}
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override {incomplete();}

  std::string const& pname() const{ return _br.pname(); }
  std::string const& nname() const{ return _br.nname(); }

  bool is_flow_probe() const{ return _is_flow_probe;}
  bool is_pot_probe() const{ return _is_pot_probe;}
  bool is_filter_probe() const;

  std::string code_name() const;
  Branch const* branch() const{
    return _br;
  }
  bool is_reversed() const;
  Nature const* nature() const;
}; // Probe
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
  void set_owner(Block const*){}
  void parse(CS& f) {f >> _name >> '=' >> _value >> ';';}
  void dump(std::ostream& f)const
	  {f << "  " << name() << " = \"" << value() << "\";\n";}
  Attribute() {}
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
  void set_owner(Block const*){}
  void parse(CS& f);
  void dump(std::ostream& f)const;
  Define(){}
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
  void set_owner(Block const*){}
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Nature() {}
  const String_Arg&	identifier()const	{return _identifier;}
  const String_Arg&  key()const	  {return _identifier;}
  const String_Arg&	parent_nature()const	{return _parent_nature;}
  const Attribute_List&	attributes()const	{return _attributes;}
  const String_Arg&	operator[](const String_Arg& k)const {return _attributes[k];}
  const String_Arg&	access()const		{return _attributes["access"];}
  const String_Arg&	abstol()const		{return _attributes["abstol"];}
};
typedef Collection<Nature> Nature_List;
/*--------------------------------------------------------------------------*/
class Discipline :public Base {
  String_Arg	_identifier;
  String_Arg	_potential_ident;
  String_Arg	_flow_ident;
  Nature const* _flow{NULL};
  Nature const* _potential{NULL};
  Block const* _owner;
public:
  void set_owner(Block const* c) {_owner=c;}
  Block const* owner() {return _owner;}
  const String_Arg&  key()const	  {return _identifier;}
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Discipline() {}
  const String_Arg&  identifier()const	    {return _identifier;}
  const String_Arg&  potential_ident()const {return _potential_ident;}
  const String_Arg&  flow_ident()const	    {return _flow_ident;}

  Nature const* flow() const{return _flow;}
  Nature const* potential()const{return _potential;}
};
typedef Collection<Discipline> Discipline_List;
/*--------------------------------------------------------------------------*/
class Branch : public Element_2 {
  Node const* _p{NULL};
  Node const* _n{NULL};
  Deps _deps; // delete?
  bool _has_flow_probe{false};
  bool _has_pot_probe{false};
  bool _has_flow_src{false};
  bool _has_pot_src{false};
  bool _is_filter{false};
  std::vector<Branch_Ref*> _refs;
public:
  explicit Branch(Node const* p, Node const* n)
    : Element_2(), _p(p), _n(n) {
    assert(p);
    assert(n);
    //_code_name = "_b_" + p->name() + "_" + n->name();
  }
  explicit Branch(){}
  Branch( Branch const&) = delete;
  ~Branch();
  std::string name()const;
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override;
  Node const* p() const{ return _p; }
  Node const* n() const{ return _n; }
  virtual std::string code_name() const;
  std::string short_label()const  { return code_name();}
//  std::string name_of_module_instance()const  {return code_name();}
  std::string const& omit()const;
  const std::string& dev_type()const;
  Deps const& deps()const { return _deps; } // delete?
  void add_probe(Probe const*);
  size_t num_nodes()const;
  std::string state()const;
  virtual bool has_element() const;
  void set_flow_probe(){ _has_flow_probe=true; }
  void set_pot_probe(){ _has_pot_probe=true; }
  void set_flow_source(){ _has_flow_src=true; }
  void set_pot_source(){ _has_pot_src=true; }
  void set_filter(){ _is_filter=true; }
  bool has_flow_probe() const;
  bool has_pot_probe() const;
  bool has_flow_source() const { return _has_flow_src; }
  bool is_filter() const { return _is_filter; }
  bool has_pot_source() const;
  size_t num_states() const;
  Discipline const* discipline() const;
  Nature const* nature() const;
public:
//  bool has(Branch_Ref*) const;
  void attach(Branch_Ref*);
  void detach(Branch_Ref*);
}; // Branch
/*--------------------------------------------------------------------------*/
class Branch_Names {
  typedef std::string key;
  typedef std::map<key, Branch_Ref> map; // set?
  typedef map::const_iterator const_iterator;
private:
  map _m;
public:
  explicit Branch_Names() {}
  Branch_Ref const& new_name(std::string const&, Branch_Ref const&);
  Branch_Ref const& lookup(std::string const&) const;

  void clear();
};
/*--------------------------------------------------------------------------*/
// the branches used in the model, in probes and sources, deduplicated.
class Branch_Map {
  typedef std::pair<Node const*, Node const*> key;
  typedef std::map<key, Branch*> map; // set?
  typedef map::const_iterator const_iterator;
private:
  map _m;
public:
  explicit Branch_Map(){}
  ~Branch_Map(){
    for(auto x : _m){
      delete x.second;
      x.second = NULL;
    }
  }
  //BranchRef new_branch(Node const* a, Node const* b, Block* owner);
  const_iterator begin() const{ return _m.begin(); }
  const_iterator end() const{ return _m.end(); }

  Branch_Ref new_branch(Node const* a, Node const* b);
  void parse(CS& f);
  void dump(std::ostream& f)const;
};
/*--------------------------------------------------------------------------*/
// similar to u_nodemap.h, somewhat more fancy.
class Node_Map{
  typedef std::map<std::string, Node*> map; // use set??
  typedef std::vector<Node*> nodes;
  typedef nodes::const_iterator const_iterator;
  nodes _nodes;
  map _map;
public:
  explicit Node_Map();
  ~Node_Map();
public:
  const_iterator begin() const{ return _nodes.begin(); }
  const_iterator end() const{ return _nodes.end(); }
  size_t size() const{ return _nodes.size(); }
  Node* new_node(std::string const&);
  Node const* operator[](std::string const& key) const;
};
/*--------------------------------------------------------------------------*/
class Node;
class File;
// TODO: decide if it is a Block, or has-a Block aka Scope or whetever.
class Module :public Block {
  typedef std::map<std::string, Probe*> Probe_Map;
public:
  ~Module();
private: // verilog input data
  File const* _file{NULL};
  String_Arg	_identifier;
  New_Port_List	_ports;
  Port_3_List_3	_input;
  Port_3_List_3	_output;
  Port_3_List_3	_inout;
  Port_3_List_3	_ground;
  Net_Declarations _net_decl;
  Branch_Declarations _branch_decl;
  Variable_List_Collection _variables;
  Parameter_List_Collection _parameters;
  Element_2_List _element_list;
  Port_1_List	_local_nodes;
  Attribute_Instance _attribute_dummy;
  AnalogList	_analog_list;
  Code_Block		_validate;
private: // elaboration data
  Filter_List _filters;
  Probe_Map _probes;
  Branch_Names _branch_names;
  Branch_Map _branches;
  Node_Map _nodes;
public:
  Module(){}
public:
  File const* file() const{ return _file; }; // owner?
  void parse(CS& f);
  void dump(std::ostream& f)const;
  const String_Arg&	  identifier()const	{return _identifier;}
  const New_Port_List&	  ports()const		{return _ports;}
  const Port_3_List_3&	  input()const		{return _input;}
  const Port_3_List_3&	  output()const		{return _output;}
  const Port_3_List_3&	  inout()const		{return _inout;}
  const Port_3_List_3&	  ground()const		{return _ground;}
  const Net_Declarations& net_declarations()const{return _net_decl;}
  const Branch_Declarations& branch_declarations()const{return _branch_decl;}
  const Parameter_List_Collection& parameters()const	{return _parameters;}
  const Variable_List_Collection& variables()const	{return _variables;}
  const Element_2_List&	  element_list()const	{return _element_list;}
  const Element_2_List&	  circuit()const	{return _element_list;}
  const Port_1_List&	  local_nodes()const	{return _local_nodes;}
//  const Code_Block&	 tr_eval()const		{return _tr_eval;}
  const AnalogList& analog_list() const {return _analog_list;}
  const Code_Block&	validate()const	{return _validate;}
    	size_t		min_nodes()const	{return ports().size();}
    	size_t		max_nodes()const	{return ports().size();}
    	size_t		net_nodes()const	{return ports().size();}
public:
  const Probe_Map&	probes()const		{return _probes;}
  const Filter_List&	filters()const		{return _filters;}
  const Node_Map&	nodes()const		{return _nodes;}
  const Branch_Names&	branch_names()const	{return _branch_names;}
  const Branch_Map&	branches()const		{return _branches;}
  bool sync()const;
  bool has_submodule()const;
  bool has_analog_block()const;
private: // misc
  CS& parse_analog(CS& cmd);

private:
  Probe const* new_probe(std::string const&, std::string const&, std::string const&) override;
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br) override;

  Filter const* new_filter(std::string const&, Deps const&) override;
  Branch_Ref new_branch(std::string const&, std::string const&) override;
  Branch_Ref new_branch(Node const*, Node const*) override;
  Branch_Ref const& new_branch_name(std::string const& n, Branch_Ref const& b) override;
  Node* new_node(std::string const& p) override;
  Node const* node(std::string const& p) const override;
  Branch_Ref branch(std::string const& p) const override;
}; // Module
typedef Collection<Module> Module_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class BlockRealIdentifier : public Variable{
public:
  explicit BlockRealIdentifier() : Variable() { }
public:
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class ListOfBlockRealIdentifiers : public LiSt<BlockRealIdentifier, '\0', ',', ';'>{
public:
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
// analog_procedural_assignment
class Assignment : public Variable {
protected:
  Variable const* _lhs{NULL};
  Expression* _rhs{NULL}; // const?
public:
  explicit Assignment() : Variable() {}
  ~Assignment();
public:
  bool is_module_variable()const override;
  std::string const& lhsname()const {
    assert(_lhs);
    return _lhs->name();
  }
  void set_lhs(Variable const* v);
  Expression const* rhs()const {return _rhs;}
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class Contribution : public Base {
  Block* _owner{NULL};
protected: // BUG
  std::string _name;
  Expression* _rhs{NULL}; // const?
  Deps _deps;
protected:
  Branch_Ref _branch;
  void set_pot_source();
  void set_flow_source();
  Deps& deps() { return _deps; }
public:
  Contribution(std::string const& lhsname /*TODO*/)
    : Base(), _name(lhsname), _branch(NULL) {}
  ~Contribution(){
    delete _rhs;
  }

  void parse(CS&)override;
  void dump(std::ostream&)const override;
  void set_owner(Block* c) {_owner=c;}
  Block const* owner() const{ return _owner; }
  Block* owner(){ return _owner; }
  Deps const& deps() const { return _deps; }
  Expression const* rhs()const {return _rhs;}
  std::string const& name() const{return _name;}
  Branch const* branch() const{return _branch;}
  bool reversed() const{ return _branch.is_reversed() ;}
  Branch_Ref new_branch(std::string const& p, std::string const& n) {
    assert(owner());
    return owner()->new_branch(p, n);
  }
  Branch_Ref new_branch(Node const* p, Node const* n) {
    assert(owner());
    return owner()->new_branch(p, n);
  }
};
/*--------------------------------------------------------------------------*/
class PotContribution : public Contribution {
public:
  PotContribution(std::string const& lhsname) : Contribution(lhsname) {}
public:
  void parse(CS&)override;
};
/*--------------------------------------------------------------------------*/
class FlowContribution : public Contribution {
public:
  FlowContribution(std::string const& lhsname) : Contribution(lhsname) {}
public:
  void parse(CS&)override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Node :public Base {
  std::string _name;
  int _number{-1};
  std::string _short_to;
  std::string _short_if;
  Discipline const* _discipline{NULL};
  Nature const* _nature{NULL};
public:
  void parse(CS&)override{};
  void dump(std::ostream&)const{};
  Node() {}
  Node(CS& f) {parse(f);}
  Node(std::string const& f, int n) : _name(f), _number(n) {}
  const std::string& name()const	{return _name;}
  std::string code_name()const	{return "n_" + _name;}
  int number()const	{return _number;}
//  const std::string& short_to()const 	{return _short_to;}
//  const std::string& short_if()const 	{return _short_if;}
  void set_discipline(Discipline const* d){ _discipline = d; }

  Discipline const* discipline() const{  return _discipline; }
  Nature const* nature() const{ return _nature; }
};
extern Node mg_ground_node;
/*--------------------------------------------------------------------------*/
class Node_List : public List<Node> {
public:
	explicit Node_List() : List<Node>() {}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class File : public Block {
  std::string	_name;
  std::string   _cwd;
  std::string   _include_path;
  CS		_file;
  Head		_head;
  Code_Block	_h_headers;
  Code_Block	_cc_headers;
  // Model_List	_model_list;
  // Device_List	_device_list;
  Code_Block	_h_direct;
  Code_Block	_cc_direct;
  Define_List	_define_list;
  Nature_List	_nature_list;
  Discipline_List _discipline_list;
  Module_List	_module_list;
  Module_List	_macromodule_list;
  Module_List	_connectmodule_list;
  Attribute_Instance _attribute_dummy;
public:
  std::string preprocess(const std::string& file_name);
  std::string include(const std::string& file_name);
public: // build
  File();
  void read(std::string const&);
  void define(std::string const&);
  void add_include_path(std::string const&);
  void parse(CS& f) override;

public: // readout
  const std::string& name()const	{return _name;}
  const std::string  fullstring()const	{return _file.fullstring();}
  const Head&	     head()const	{return _head;}
  const Code_Block&  h_headers()const	{return _h_headers;}
  const Code_Block&  cc_headers()const	{return _cc_headers;}
  const Module_List& modules()const	{return _module_list;}
  const Code_Block&  h_direct()const	{return _h_direct;}
  const Code_Block&  cc_direct()const	{return _cc_direct;}

  const Define_List&	 define_list()const	{return _define_list;}
  const Nature_List&	 nature_list()const	{return _nature_list;}
  const Discipline_List& discipline_list()const	{return _discipline_list;}
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
class Filter : public Element_2 /*?*/ {
  std::string _name; // BUG?
  Deps _deps;
  Branch_Ref _branch;
  Probe const* _prb=NULL;
public:
  explicit Filter() : Element_2() {}
  explicit Filter(std::string const& name, Deps const& d)
    : Element_2(), _deps(d) {
    _name = name;
    }

  void set_output(Branch_Ref const& x);
//  Branch_Ref const& branch() const{ return _branch; }
  std::string name() const {
    return _name;
  }
  std::string branch_code_name() const {
    assert(_branch);
    return _branch->code_name();
  }
  std::string code_name() const {
    return "_f_" + _name; // name()?
  }
  Deps const& deps()const { return _deps; }

  size_t num_states()const override;
  size_t num_nodes()const override;
  std::string state()const override;
  std::string short_label()const;
  Probe const* prb() const;


public: // make it look like an Element_2?
//  std::string omit() const { return ""; }
  std::string const& dev_type() const;
};
/*--------------------------------------------------------------------------*/
void resolve_symbols(Expression const& e, Expression& E, Block* scope, Deps*d=NULL);
void make_cc_expression(std::ostream& o, Expression const& e);
/*--------------------------------------------------------------------------*/
inline bool Variable::is_node(std::string const& n) const
{
  assert(_owner);
  return _owner->node(n);
}
/*--------------------------------------------------------------------------*/
inline void Variable::new_var_ref()
{
  assert(_owner);
  _owner->new_var_ref(this);
}
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
