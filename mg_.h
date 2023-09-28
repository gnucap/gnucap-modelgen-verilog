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
#ifndef GNUCAP_MG_H
#define GNUCAP_MG_H
#include <ap.h>
#include <m_base.h>
#include <m_expression.h>
#include <set>
#include "mg_deps.h" // BUG, Deps
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
class Block;
class Owned_Base : public Base {
  Block* _owner{NULL};
protected:
  explicit Owned_Base() : Base() { }
  explicit Owned_Base(Owned_Base const& b) : Base(), _owner(b._owner) { }
  explicit Owned_Base(Block* o) : Base(), _owner(o) { }
public:
  void set_owner(Block* b){  _owner = b; }
  Block const* owner() const{ return _owner;}
protected:
  Block* owner(){ return _owner;}
};
/*--------------------------------------------------------------------------*/
class Block;
class String_Arg : public Base {
protected:
  std::string	_s;
public:
  String_Arg() {}
  explicit String_Arg(const char* s) : _s(s) {}
  explicit String_Arg(std::string const& s) : _s(s) {}
  String_Arg(String_Arg const& o) : Base(), _s(o._s) {untested();} // needed?
  //String_Arg(const std::string& s) : _s(s) {}
  const String_Arg&  key()const	  {return *this;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override{f << _s;}
  void operator=(const std::string& s)	 {_s = s;}
  void operator+=(const std::string& s)	 {_s += s;}
  bool operator!=(const std::string& s)const {return _s != s;}
  bool operator==(const String_Arg& s)const {return _s == s._s;}
  bool operator!=(const String_Arg& s)const {return _s != s._s;}
  bool			is_empty()const	 {return _s.empty();}
  std::string		lower()const	 {return to_lower(_s);}
  const std::string&	to_string()const {return _s;}
  void set_owner(Block*){
    // incomplete();
  }
};
/*--------------------------------------------------------------------------*/
class Raw_String_Arg : public String_Arg {
public:
  void parse(CS& f)override;
};
/*--------------------------------------------------------------------------*/
class C_Comment : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ incomplete();}
};
/*--------------------------------------------------------------------------*/
class Cxx_Comment : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ incomplete();}
};
/*--------------------------------------------------------------------------*/
class Skip_Block : public Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ incomplete();}
};
/*--------------------------------------------------------------------------*/
/* A "Collection" differs from a "LiSt" in how it is parsed.
 * Each parse of a "Collection" creates one more object and stores
 * it in the Collection.  The size of the Collection therefore grows by 1.
 * A "Collection" is often parsed many times.
 * Each parse of a "LiSt" creates a bunch of objects, and stores them.
 * A list has opening and closing delimeters, usually {}.
 * A "LiSt" is usually parsed once.
 */
class Attribute_Instance;
template <class T, char BEGIN, char SEP, char END, char END2='\0', char END3='\0'>
class LiSt : public List_Base<T> {
  Block* _owner{NULL};
  Attribute_Instance const* _attributes{NULL};
public:
  using List_Base<T>::size;
  using List_Base<T>::push_back;
  using List_Base<T>::begin;
  using List_Base<T>::end;
  typedef typename List_Base<T>::const_iterator const_iterator;
public:
  ~LiSt();

  void set_owner(Block* b){ _owner = b; }
  Block const* owner() const{return _owner;}
  Block* owner(){return _owner;}
  void parse(CS& file) override{
    parse_n(file);
  }

// protected:??
  void parse_n(CS& file, size_t max=-1ul) {
    parse_n_<T>(file, max);
  }
protected:
  template<class TT>
  void parse_n_(CS& file, size_t max=-1ul) {
    int paren = !BEGIN || file.skip1b(BEGIN);
    size_t here = file.cursor();
    for (;;) {
      if (file.stuck(&here)) {
  	file.skipbl();
	paren -= file.skip1(END); //!!
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
	p->set_owner(owner());
	file >> *p;
	if (file.stuck(&here)) { untested();
	  delete p;
	  file.warn(0, "not valid here");
	  break;
	}else if (max==size()){
	  throw Exception_Too_Many(int(size()+1), int(max), 0);
	}else{
	  push_back(p);
	}
      }else{ untested();
      }
    }
  }
public: // Block?
  void set_attributes(Attribute_Instance const* a) {
    assert(!_attributes);
    _attributes = a;
  }
protected: // base class?
  bool has_attributes() const{
    return _attributes;
  }
  Attribute_Instance const& attributes()const {
    assert(_attributes);
    return *_attributes;
  }
//  void dump_attributes(std::ostream& f)const;
public:
  void dump(std::ostream& f)const override {
    // dump_attributes(f);
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
    //file >> s;
    s = file.ctos(":,.`()[];*/+-", "", "");
    const_iterator x = find(s);
    if (x == end()) {
      file.reset(here);
    }else{
    }
    return x;
  }
};
typedef LiSt<String_Arg, '(', ',', ')'> String_Arg_List;
typedef LiSt<Raw_String_Arg, '(', ',', ')'> Raw_String_Arg_List;
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
class Collection : public List_Base<T> {
  Block* _owner{NULL};
  File const* _file{NULL};
public:
  using List_Base<T>::push_back;
  using List_Base<T>::begin;
  using List_Base<T>::end;
  using List_Base<T>::back;
  using List_Base<T>::is_empty;
  using List_Base<T>::pop_back;
  using List_Base<T>::erase;
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
    s = file.ctos(":,.`()[];*/+-");
    trace1("collectionfind", s);
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
  const String_Arg& operator[](const char* s) const {
    return operator[](String_Arg(s));
  }
  void clear() {
    while(!is_empty()){
      erase(begin());
    }
  }
};
/*--------------------------------------------------------------------------*/
class Key : public Base {
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
class Bool_Arg : public Base {
  bool _s;
public:
  void parse(CS& f)override {_s = true; f.skip1b(";");}
  void dump(std::ostream& f)const override {untested();f << _s;}
  Bool_Arg() :_s(false) {}
  operator bool()const {return _s;}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Probe; // Dep?
class DEP_STACK;
class FUNCTION_;
class Branch_Ref;
class Symbolic_Expression : public Expression {
  Block* _owner{NULL};
  Deps _deps;
public:
  explicit Symbolic_Expression() : Expression() {}
  ~Symbolic_Expression();
  void resolve_symbols(Expression const& e, Block* scope, Deps* deps=NULL);
  Symbolic_Expression& operator=(Symbolic_Expression const&);
public:
  Deps const& deps()const { return _deps; }
  Deps& deps() { return _deps; }
private: // all the same eventually?
  Token* resolve_xs_function(std::string const& n, DEP_STACK& ds, Block* o);
  Token* resolve_function(FUNCTION_ const* filt, DEP_STACK& ds, Block* o);
  Token* resolve_system_task(FUNCTION_ const* t, DEP_STACK& ds, Block* o);
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br);
};
/*--------------------------------------------------------------------------*/
// class AnalogExpression?
class ConstantMinTypMaxExpression : public Base {
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
class Parameter_Base : public Base {
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
  bool operator!=(const std::string& s)const {return _name != s;}
protected:
  Block* owner(){ return _owner; }
};
// typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
/*--------------------------------------------------------------------------*/
class Parameter_1 : public Parameter_Base {
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
  Parameter_1() :Parameter_Base() {}
};
typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
/*--------------------------------------------------------------------------*/
class ValueRangeSpec : public Owned_Base {
  // incomplete();
};
/*--------------------------------------------------------------------------*/
class ValueRangeConstant : public ValueRangeSpec {
  std::string _cexpr;
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
  std::string const& expr()const {return _cexpr;}
};
/*--------------------------------------------------------------------------*/
class ValueRangeStrings : public ValueRangeSpec {
};
/*--------------------------------------------------------------------------*/
class ConstExpression : public Owned_Base {
  Expression* _expression{NULL};
public:
  explicit ConstExpression() : Owned_Base() {}
  ConstExpression(CS& f, Block* o) : Owned_Base(o) { untested();
    set_owner(o);
    parse(f);
  }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression const& expression() const{assert(_expression); return *_expression;};
};
/*--------------------------------------------------------------------------*/
class Attribute_Spec : public Owned_Base{
  String_Arg _key;
//  typedef ConstExpression value_type; TODO
public:
//  typedef Raw_String_Arg value_type;
  // typedef String_Arg value_type;
  typedef std::string value_type;
private:
  value_type* _expr{NULL};
public:
  void parse(CS& f)override;
  void dump(std::ostream&)const override;
public:
  Attribute_Spec() : Owned_Base() {untested();}
  Attribute_Spec(CS& f, Block* o) : Owned_Base(o) {
    set_owner(o);
    parse(f);
  }
  ~Attribute_Spec(){
    delete _expr;
    _expr = NULL;
  }
  bool operator==(Attribute_Spec const& o) const{ untested();
    return o._key == _key;
  }
  bool operator!=(String_Arg const& o) const{ untested();
    return o != _key;
  }

  String_Arg const& key() const{
    return _key;
  }
  bool has_expression() const{
    return _expr;
  }
  value_type const* expression_or_null() const{
    return _expr;
  }
  value_type const& expression() const{
    assert(_expr);
    return *_expr;
  }
};
/*--------------------------------------------------------------------------*/
class Attribute_Instance : public Collection<Attribute_Spec> {
//  std::set<Attribute_Spec*, getkey>;
  typedef Attribute_Spec::value_type value_type;
public:
  Attribute_Instance() : Collection<Attribute_Spec>() { untested(); }
  Attribute_Instance(CS& f, Block* o) : Collection<Attribute_Spec>() {
    set_owner(o);
    parse(f);
  }
  void parse(CS& f)override;
  void dump(std::ostream&)const override;
  void clear();
//  bool empty() const{ return _m.empty(); }
  value_type const* find(String_Arg const&) const;
private:
};
/*--------------------------------------------------------------------------*/
class Attribute_Stash : public Owned_Base {
  Attribute_Instance* _a{NULL};
public:
  Attribute_Stash() : Owned_Base() { }
  void parse(CS& f)override;
  void dump(std::ostream&)const override{ unreachable(); }
  bool is_empty() const{
    return !_a;
  }
  Attribute_Instance const* detach(){
    Attribute_Instance const* r = _a;
    _a = NULL;
    return r;
  }
private:
};
/*--------------------------------------------------------------------------*/
class ValueRangeInterval : public ValueRangeSpec {
  ConstExpression _lb;
  ConstExpression _ub;
  bool _lb_is_closed;
  bool _ub_is_closed;
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
  bool lb_is_closed()const {return _lb_is_closed;}
  bool ub_is_closed()const {return _ub_is_closed;}
  ConstExpression const& lb()const {return _lb;}
  ConstExpression const& ub()const {return _ub;}
};
/*--------------------------------------------------------------------------*/
class ValueRange : public Owned_Base {
  enum{
    vr_FROM,
    vr_EXCLUDE
  } _type;
  ValueRangeSpec* _what{NULL};
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
  bool is_from() const{return _type == vr_FROM;}
  bool is_exclude() const{return _type == vr_EXCLUDE;}
  ValueRangeSpec const* spec() const{ return _what; }
};
typedef LiSt<ValueRange, '\0', '\0', '\0', ',', ';'> ValueRangeList;
/*--------------------------------------------------------------------------*/
// parameter type name = value ;
class Aliasparam;
class Parameter_2 : public Parameter_Base {
  ValueRangeList _value_range_list;
  std::list<Aliasparam const*> _aliases;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Parameter_2() :Parameter_Base() {}
  void set_type(std::string const& a){_type=a;}
  void add_alias(Aliasparam const*);
  ValueRangeList const& value_range_list()const { return _value_range_list; }
  std::list<Aliasparam const*> const& aliases()const {return _aliases;}
};
/*--------------------------------------------------------------------------*/
class Parameter_2_List : public LiSt<Parameter_2, '\0', ',', ';'> {
  String_Arg _type;
  bool _is_local{false};
  Attribute_Instance const* _attributes{NULL};
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
class Branch;
class Branch_Ref : public Owned_Base {
  Branch* _br{NULL};
  bool _r;
  std::string const* _name{NULL};
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
  std::string code_name()const;
  void set_name(std::string const&);
  bool has_name()const{
    return _name;
  }

  operator Branch*() const{ return _br; }
  Branch* operator->() const{ return _br; }
  bool operator==(Branch_Ref const& o) const{ return _br==o._br && _r==o._r; }
  bool is_reversed() const{return _r;}

  std::string const& pname() const;
  std::string const& nname() const;
};
/*--------------------------------------------------------------------------*/
class Data_Type : public Base{
protected:
  typedef enum{ t_default, t_real, t_int } type_t;
  type_t _type;
  Data_Type(type_t type) : Base(), _type(type){}
public:
  Data_Type(Data_Type const& t) : Base(), _type(t._type){}
  Data_Type() : Base(), _type(t_default){}
  Data_Type& operator=(Data_Type const& o){ _type = o._type; return *this;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  bool is_real() const{ return _type==t_real; }
  bool is_int() const{ return _type==t_int; }
};
/*--------------------------------------------------------------------------*/
class Data_Type_Real : public Data_Type{
public:
  Data_Type_Real() : Data_Type(t_real){}
};
/*--------------------------------------------------------------------------*/
class Data_Type_Int : public Data_Type{
public:
  Data_Type_Int() : Data_Type(t_int){}
};
/*--------------------------------------------------------------------------*/
class Token_PROBE; //bug?
class Node;
class Expression;
class Variable : public Owned_Base {
protected:
  String_Arg _name;
public:
  explicit Variable() : Owned_Base() {}
  Variable(std::string const& name)
   : Owned_Base()
   ,_name(name)
  {}
  ~Variable() {
  }
public:
//  void set_type(Data_Type d){ _type=d; }
  virtual Data_Type const& type() const = 0;
  // bool is_int() const { return _type.is_int(); }
  virtual Deps const& deps()const = 0;
  virtual Deps& deps() = 0;
  String_Arg const& identifier()const {return _name;}
  std::string const& name()const {return _name.to_string();}
  virtual std::string code_name()const;
  virtual bool is_module_variable() const;

  // void parse(CS&) override;
  // void dump(std::ostream& o)const override;
  virtual Branch_Ref new_branch(std::string const& p, std::string const& n);
  virtual Branch_Ref new_branch(Node const* p, Node const* n);

  virtual void update_deps(Deps const&);
protected:
  void new_var_ref();
};
/*--------------------------------------------------------------------------*/
class Variable_Decl : public Variable {
  Deps _deps;
  Data_Type _type;
  Attribute_Instance const* _attributes{NULL};
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Variable_Decl() : Variable() {}
  bool has_attributes() const{
    return _attributes;
  }
  void set_attributes(Attribute_Instance const* a) {
    _attributes = a;
  }
  Data_Type const& type()const override{
    return _type;
  }
  void set_type(Data_Type const& d){ _type=d; }
  Deps const& deps()const override { return _deps; }
  Deps& deps()override { return _deps; }

protected:
//  void set_type(std::string const& a){_type=a;}
};
/*--------------------------------------------------------------------------*/
class Variable_List : public LiSt<Variable_Decl, '\0', ',', ';'> {
  String_Arg _type; // ENUM?
public:
  String_Arg const& type()const {return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Variable_List_Collection : public Collection<Variable_List>{
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Aliasparam : public Owned_Base {
  Parameter_2 const* _param{NULL};
  std::string _name;
public:
  explicit Aliasparam() : Owned_Base() {}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  std::string const& name()const {
    return _name;
  }
  bool operator!=(const std::string& s)const {return _name != s;}
private:
  std::string param_name()const {
    assert(_param);
    return _param->name();
  }
};
typedef  Collection<Aliasparam> Aliasparam_Collection;
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
class Parameter_3 : public Parameter_Base {
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Parameter_3() :Parameter_Base() {}
};
typedef LiSt<Parameter_3, '(', ',', ')'> Parameter_3_List;
/*--------------------------------------------------------------------------*/
#if 0
class Code_Block : public Base {
  std::string _s;
  Block* _owner{NULL};
public:
  void set_owner(Block* c) { _owner = c; }
  void parse(CS& f)override;
  void dump(std::ostream& f)const override{f << _s;}
  Code_Block() {}
  bool is_empty()const {return _s.length() < 2;}
};
#endif
/*--------------------------------------------------------------------------*/
class Parameter_Block : public Base {
  String_Arg	 _unnamed_value;
  Parameter_1_List _override;
  Parameter_1_List _raw;
  Parameter_1_List _calculated;
  // Code_Block	 _code_pre;
  // Code_Block	 _code_mid;
  // Code_Block     _code_post;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  const String_Arg&	unnamed_value()const	{return _unnamed_value;}
  const Parameter_1_List& override()const 	{return _override;}
  const Parameter_1_List& raw()const		{return _raw;}
  const Parameter_1_List& calculated()const	{return _calculated;}
 // const Code_Block&	code_pre()const		{return _code_pre;}
 // const Code_Block&	code_mid()const		{return _code_mid;}
 // const Code_Block&	code_post()const	{return _code_post;}
  bool is_empty()const {untested();
    return (calculated().is_empty()
				// && code_post().is_empty()
				// && code_mid().is_empty()
				&& override().is_empty()
				&& raw().is_empty()
				// && code_pre().is_empty());
    );}
  void fill_in_default_values();
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/// Some kind of sequential block with scope for parameters, variables..
class Filter;
class Task;
class Node;
class Nature;
class FUNCTION_;
class Block : public List_Base<Base> {
  typedef std::map<std::string, Base*> map; // set?
  typedef map::const_iterator const_iterator;
protected:
  Attribute_Instance const* _attributes{NULL}; // Base class?
  map _var_refs;
private:
  Block* _owner{NULL};
public:
  Block const* owner() const{ return _owner;}
  Block* owner(){ return _owner;}
  ~Block();
public:
  bool has_attributes() const{
    return _attributes;
  }
  Attribute_Instance const& attributes()const {
    assert(_attributes);
    return *_attributes;
  }
  void new_var_ref(Base* what);

  virtual Node* new_node(std::string const& p){ untested();
    assert(_owner);
    return _owner->new_node(p);
  }
  virtual Node const* node(std::string const& p) const{ untested();
    assert(_owner);
    return _owner->new_node(p); // new??
  }

  virtual Token* new_token(FUNCTION_ const* f, Deps& d, size_t num_args) {
    assert(_owner);
    return _owner->new_token(f, d, num_args);
  }

  virtual Branch_Ref new_branch(std::string const&, std::string const&) {
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
  Base* resolve(std::string const& k);

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
  void parse(CS&)override{incomplete();}
  void dump(std::ostream& o)const override;

  Branch_Ref new_branch(std::string const& p, std::string const& n)override {
    assert(owner());
    return owner()->new_branch(p, n);
  }
  Branch_Ref new_branch(Node const* p, Node const* n)override {
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
class Eval : public Base {
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
class Function : public Eval {
public:
  void set_owner(Block const*) { }
  void parse(CS& f);
  void dump(std::ostream& f)const override;
  Function() :Eval() {}
};
typedef Collection<Function> Function_List;
#endif
/*--------------------------------------------------------------------------*/
class Port_1 : public Base {
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
class Port_3 : public Owned_Base {
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
class New_Port : public Port_3 {
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
class Net_Identifier : public Port_3 {
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
  Attribute_Instance const* _attributes{NULL};
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
//  Branch_Ref _branch_ref;
public:
  void parse(CS& f)override;
  // void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Element_1 : public Base {
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
class Element_2 : public Owned_Base {
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
  void set_dev_type(std::string const& s){_module_or_paramset_identifier = s;}
  void set_state(std::string const& s){_state = s;}
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
class Arg : public Base {
  String_Arg _identifier;
public:
  Arg() {}
  void set_owner(Block*){
    incomplete();
  }
  String_Arg const& identifier() const{return _identifier;}
  void parse(CS& f) override;
  void dump(std::ostream& f)const override {f << "      " << identifier() << ";\n";}
};
typedef LiSt<Arg, '{', '#', '}'> Arg_List;
/*--------------------------------------------------------------------------*/
class Analog_Function_Arg : public Variable {
//  String_Arg _identifier;
  Deps _deps; // really?
public:
  String_Arg const& identifier()const { return _name; }
  std::string name()const { return _name.to_string(); }
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  std::string code_name()const override;
  Deps const& deps()const { return _deps; }
  Deps& deps() { return _deps; }

  Data_Type const& type()const override{
    static Data_Type_Real r;
    return r; // for now.
  }
};
typedef Collection<Analog_Function_Arg> Analog_Function_Args;
/*--------------------------------------------------------------------------*/
class AF_Arg_List : public LiSt<Analog_Function_Arg, '\0', ',', ';'> {
//  Data_Type _type;
  enum{
    a_input,
    a_output,
    a_inout
  } _direction;
public:
  bool is_output() const { return _direction >= a_output; }
//  Data_Type const& type()const {return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class AF_Arg_List_Collection : public Collection<AF_Arg_List>{
public:
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Args : public Base {
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
class Circuit : public Base {
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
class Probe : public Base {
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
  Branch_Ref _br;
  enum{
    t_unknown = 0,
    t_flow,
    t_pot
  } _type;
public:
  explicit Probe(std::string const& xs, Branch_Ref b);
//  std::string const& name()const {return _name;}
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override {incomplete();}

  std::string const& pname() const{ return _br.pname(); }
  std::string const& nname() const{ return _br.nname(); }

  bool is_flow_probe() const{ return _type == t_flow;}
  bool is_pot_probe() const{ return _type == t_pot;}

  std::string code_name() const;
  Branch const* branch() const{
    return _br;
  }
  bool is_reversed() const;
  Nature const* nature() const;

  bool operator==(Probe const& o) const{
    return _br == o._br && _type == o._type;
  }
  bool operator!=(Probe const& o) const{
    return !operator==(o);
  }
}; // Probe
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Head : public Base {
  std::string _s;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override {f << _s;}
  Head() {}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Attribute : public Base {
  String_Arg _name;
  String_Arg _value;
public:
  void set_owner(Block const*){}
  void parse(CS& f)override {f >> _name >> '=' >> _value >> ';';}
  void dump(std::ostream& f)const override
	  {f << "  " << name() << " = \"" << value() << "\";\n";}
  Attribute() {}
  const String_Arg&  key()const	  {return _name;}
  const String_Arg&  name()const  {return _name;}
  const String_Arg&  value()const {return _value;}
};
typedef Collection<Attribute> Attribute_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Define_List;
class Define : public Base {
  String_Arg _name;
  size_t _num_args;

  std::string _value_tpl;
  std::vector<std::pair<size_t, size_t>> _pos;
public:
  void set_owner(Block const*){}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Define(){}
  const String_Arg&  key()const   {return _name;}
  const String_Arg&  name()const  {return _name;}

  std::string substitute(String_Arg_List const&, Define_List const&) const; // HERE?
  void preprocess(Define_List const&);
//  size_t num_args()const { return _args.size(); }
  size_t num_args()const { return _num_args; }
private:
  void stash(std::string const&, String_Arg_List const&);
};
/*--------------------------------------------------------------------------*/
class Define_List : public Collection<Define>{
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Nature : public Base {
  String_Arg		_identifier;
  String_Arg		_parent_nature;
  Attribute_List	_attributes;
public:
  void set_owner(Block const*){}
  void parse(CS& f)override;
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
class Discipline : public Base {
  String_Arg	_identifier;
  String_Arg	_potential_ident;
  String_Arg	_domain_ident;
  String_Arg	_flow_ident;
  Nature const* _flow{NULL};
  Nature const* _potential{NULL};
  Block const* _owner;
public:
  void set_owner(Block const* c) {_owner=c;}
  Block const* owner() {return _owner;}
  const String_Arg&  key()const	  {return _identifier;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Discipline() {}
  const String_Arg&  identifier()const	    {return _identifier;}
  const String_Arg&  potential_ident()const {return _potential_ident;}
  const String_Arg&  domain_ident()const    {return _domain_ident;}
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
  size_t _number;
  std::list<std::string> _names;
  bool _direct{true};
  bool _selfdep{false};
public:
  explicit Branch(Node const* p, Node const* n, size_t number)
    : Element_2(), _p(p), _n(n), _number(number) {
    assert(p);
    assert(n);
    //_code_name = "_b_" + p->name() + "_" + n->name();
  }
  explicit Branch(){}
  Branch( Branch const&) = delete;
  ~Branch();
  std::list<std::string> const& names()const{
    return _names;
  }
  std::string name()const;
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override;
  std::string const* reg_name(std::string const&);
  Node const* p() const{ assert(_p); return _p; }
  Node const* n() const{ assert(_n); return _n; }
  bool is_short() const{ return _p == _n; }
  bool is_direct() const{ return _direct; }
  bool is_generic() const;
  std::string code_name()const override;
  std::string short_label()const override { return code_name();}
//  std::string name_of_module_instance()const  {return code_name();}
  std::string const& omit()const override;
  const std::string& dev_type()const override;
  Deps const& deps()const { return _deps; } // delete?
  void add_probe(Probe const*);
  size_t num_nodes()const override;
  std::string state()const override;
  virtual bool has_element() const;
  void set_flow_probe(){ _has_flow_probe=true; }
  void set_pot_probe(){ _has_pot_probe=true; }
  void set_flow_source(){ _has_flow_src=true; }
  void set_pot_source(){ _has_pot_src=true; }
  void set_filter(){ _is_filter=true; }
  void set_direct(bool d=true);
  void set_selfdep(bool d=true) {_selfdep = d; }
  bool has_flow_probe() const;
  bool has_pot_probe() const;
  bool has_flow_source() const { return _has_flow_src; }
  bool is_filter() const { return _is_filter; }
  bool has_pot_source() const;
  size_t num_states() const override;
  Discipline const* discipline()const override;
  Nature const* nature()const override;
public:
//  bool has(Branch_Ref*) const;
  void attach(Branch_Ref*);
  void detach(Branch_Ref*);
  size_t number() const{return _number;}
  size_t num_branches() const;
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
class Branch_Map : public Owned_Base {
  typedef std::pair<Node const*, Node const*> key;
  typedef std::map<key, Branch*> map; // set?
				      //
  typedef std::list<Branch /* const?? */ *> list;
  typedef list::const_iterator const_iterator;
  list _brs;
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
  const_iterator begin() const{ return _brs.begin(); }
  const_iterator end() const{ return _brs.end(); }
  size_t size() const{ return _brs.size(); }

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
  typedef nodes::const_reverse_iterator const_reverse_iterator;
  nodes _nodes;
  map _map;
public:
  explicit Node_Map();
  ~Node_Map();
public:
  const_reverse_iterator rbegin() const{ return _nodes.rbegin(); }
  const_reverse_iterator rend() const{ return _nodes.rend(); }
  const_iterator begin() const{ return _nodes.begin(); }
  const_iterator end() const{ return _nodes.end(); }
  size_t size() const{ return _nodes.size(); }
  Node* new_node(std::string const&);
  Node const* operator[](std::string const& key) const;
};
/*--------------------------------------------------------------------------*/
class Analog_Function : public Block{
  Analog_Function_Arg _variable;
  Data_Type _type;
  AF_Arg_List_Collection _args;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  String_Arg const& identifier()const { return _variable.identifier(); }
  std::string code_name()const { return "af_" + identifier().to_string(); }
public:
  Branch_Ref new_branch(std::string const&, std::string const&)override { untested();
    return Branch_Ref();
  }
  Branch_Ref new_branch(Node const*, Node const*)override {
    return Branch_Ref();
  }
  Node const* node(std::string const&)const override {
    return NULL;
  }
  Branch_Ref branch(std::string const&)const override {
    return Branch_Ref();
  }
  AF_Arg_List_Collection const& args() const{ return _args; }
  Analog_Function_Arg const& variable() const{ return _variable; }
};
typedef Collection<Analog_Function> Analog_Functions;
/*--------------------------------------------------------------------------*/
class Node;
class File;
class Module : public Block {
  typedef std::map<std::string, Probe*> Probe_Map;
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
  Analog_Functions _analog_functions;
  Variable_List_Collection _variables;
  Parameter_List_Collection _parameters;
  Aliasparam_Collection _aliasparam;
  Element_2_List _element_list;
  Port_1_List _local_nodes;
  Attribute_Stash _attribute_stash;
  AnalogList _analog_list;
  // Code_Block		_validate;
private: // merge?
  Filter_List _filters;
  std::list<FUNCTION_ const*> _func;
  std::set<FUNCTION_ const*> _funcs;
  size_t _num_evt_slots{0};
  bool _has_analysis{false};
private: // elaboration data
  Probe_Map _probes;
  Branch_Names _branch_names;
  Branch_Map _branches;
  Node_Map _nodes;
public:
  Module(){}
  ~Module();
public:
  File const* file() const{ return _file; }; // owner?
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  const String_Arg&	  identifier()const	{return _identifier;}
  const New_Port_List&	  ports()const		{return _ports;}
  const Port_3_List_3&	  input()const		{return _input;}
  const Port_3_List_3&	  output()const		{return _output;}
  const Port_3_List_3&	  inout()const		{return _inout;}
  const Port_3_List_3&	  ground()const		{return _ground;}
  const Net_Declarations& net_declarations()const{return _net_decl;}
  const Branch_Declarations& branch_declarations()const{return _branch_decl;}
  const Parameter_List_Collection& parameters()const	{return _parameters;}
  const Aliasparam_Collection& aliasparam()const	{return _aliasparam;}
  const Variable_List_Collection& variables()const	{return _variables;}
  const Element_2_List&	  element_list()const	{return _element_list;}
  const Element_2_List&	  circuit()const	{return _element_list;}
  const Port_1_List&	  local_nodes()const	{return _local_nodes;}
//  const Code_Block&	 tr_eval()const		{return _tr_eval;}
  const AnalogList& analog_list() const {return _analog_list;}
  const Analog_Functions& analog_functions() const {return _analog_functions;}
//   const Code_Block&	validate()const	{return _validate;}
    	size_t		min_nodes()const	{return ports().size();}
    	size_t		max_nodes()const	{return ports().size();}
    	size_t		net_nodes()const	{return ports().size();}
  bool has_events()const {return _num_evt_slots;}
  bool has_analysis()const {return _has_analysis;}
  void new_evt_slot() { ++_num_evt_slots; }
  size_t num_evt_slots()const {return _num_evt_slots; }
public:
//  const Probe_Map&	probes()const		{return _probes;}
  const Filter_List&	filters()const		{return _filters;}
  const Node_Map&	nodes()const		{return _nodes;}
  const Branch_Names&	branch_names()const	{return _branch_names;}
  const Branch_Map&	branches()const		{return _branches;}
  size_t num_branches() const;
  bool sync()const;
  bool has_submodule()const;
  bool has_analog_block()const;
  Attribute_Stash& attribute_stash() {
    return _attribute_stash;
  }
  void set_analysis() {_has_analysis = true; }
  void set_attributes(Attribute_Instance const* a) {
    assert(!_attributes);
    _attributes = a;
  }
  void push_back(FUNCTION_ const* f);
  void push_back(Filter /*const*/ * f);
  void install(FUNCTION_ const* f);
  std::list<FUNCTION_ const*> const& func()const {return _func;}
  std::set<FUNCTION_ const*> const& funcs()const {return _funcs;}
private: // misc
  CS& parse_analog(CS& cmd);

public: // BUG
  Probe const* new_probe(std::string const&, std::string const&, std::string const&);
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br);
  Branch_Ref new_branch(std::string const&, std::string const&) override;
private:

  Token* new_token(FUNCTION_ const*, Deps&, size_t na) override;
  Branch_Ref const& new_branch_name(std::string const& n, Branch_Ref const& b) override;
  Node const* node(std::string const& p) const override;
  Branch_Ref branch(std::string const& p) const override;
public: //filters may need this..
  Node* new_node(std::string const& p) override;
  Branch_Ref new_branch(Node const*, Node const*) override;
}; // Module
typedef Collection<Module> Module_List;
/*--------------------------------------------------------------------------*/
class Paramset : public Block {
  std::string _name;
  std::string _proto_name;
  Attribute_Stash _attribute_stash;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
public: // Block?
  void set_attributes(Attribute_Instance const* a) {
    assert(!_attributes);
    _attributes = a;
  }
  bool has_attributes() const{
    return _attributes;
  }
};
typedef Collection<Paramset> Paramset_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class BlockVarIdentifier : public Variable_Decl{
public:
  explicit BlockVarIdentifier() : Variable_Decl() { }
public:
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class ListOfBlockIntIdentifiers : public LiSt<BlockVarIdentifier, '\0', ',', ';'>{
public:
  ListOfBlockIntIdentifiers(CS& f, Block* o){
    set_owner(o);
    parse(f);
    for(auto i : *this){
      i->set_type(Data_Type_Int());
    }
  }
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class ListOfBlockRealIdentifiers : public LiSt<BlockVarIdentifier, '\0', ',', ';'>{
public:
  ListOfBlockRealIdentifiers(CS& f, Block* o){
    set_owner(o);
    parse(f);
    for(auto i : *this){
      i->set_type(Data_Type_Real());
    }
  }
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
// analog_assignment
// VariableAssignment?
class Assignment : public Variable {
protected:
  Variable* _lhs{NULL};
  Symbolic_Expression _rhs;
public:
  explicit Assignment(CS& f, Block* o);
  explicit Assignment() : Variable() {}
  ~Assignment();
public:
  bool is_module_variable()const override;
  bool is_int() const;
  Data_Type const& type()const;
  std::string const& lhsname()const {
    assert(_lhs);
    return _lhs->name();
  }
  Symbolic_Expression const& rhs()const {return _rhs;}
  Variable const& lhs() const{
    assert(_lhs);
    return *_lhs;
  }
  Deps const& deps()const { return _rhs.deps(); }
  Deps& deps() { return _rhs.deps(); }
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override;
  void update_deps(Deps const&) override;
// protected:
  void set_lhs(Variable* v);
};
/*--------------------------------------------------------------------------*/
// ContributionStatement?
class Contribution : public Owned_Base {
  std::string _name;
  Nature const* _nature{NULL};
  Symbolic_Expression _rhs;
  Branch_Ref _branch;
  enum{
    t_unknown,
    t_flow,
    t_pot
  } _type;
private:
  void set_pot_contrib();
  void set_flow_contrib();
  void set_direct(bool d=true);
  Deps& deps() { return _rhs.deps(); }
public:
  Contribution(CS& f, Block* o)
    : Owned_Base(o), _branch(NULL) {
    parse(f);
  }
  ~Contribution(){
  }

  bool is_pot_contrib() const;
  bool is_flow_contrib() const;
  bool is_direct() const;
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Deps const& deps() const { return _rhs.deps(); }
  Expression const& rhs()const {return _rhs;}
  std::string const& name() const{return _name;}
  Branch_Ref const& branch_ref() const{return _branch;}
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
private:
  Expression& rhs() {return _rhs;}
}; // Contribution
/*--------------------------------------------------------------------------*/
class Node : public Base {
  std::string _name;
  int _number{-1};
  std::string _short_to;
  std::string _short_if;
  Discipline const* _discipline{NULL};
  Nature const* _nature{NULL};
public:
  void parse(CS&)override {};
  void dump(std::ostream&)const override {};
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
class Preprocessor : public CS {
  Define_List	_define_list;
  std::string _cwd;
  std::string _include_path;
  std::basic_ostream<char>* _diag{NULL};
private:
  std::string _stripped_file; // DUP?
public:
  explicit Preprocessor();
  void read(std::string const& file_name);
  void define(std::string const&);
  const Define_List&	 define_list()const	{return _define_list;}
  void dump(std::ostream&)const;
  void add_include_path(std::string const&);
  void set_diag(std::basic_ostream<char>& o){
    _diag = &o;
  }
private:
  void parse(CS& file);
  void include(const std::string& file_name);
  std::basic_ostream<char>& diag() {
    if(_diag){
      return *_diag;
    }else{
      return std::cerr;
    }
  }
};
/*--------------------------------------------------------------------------*/
class File : public Block {
  std::string	_name;
  std::string   _cwd;
  CS		_file;
  Head		_head;
  Nature_List	_nature_list;
  Discipline_List _discipline_list;
  Module_List	_module_list;
  Module_List	_macromodule_list;
  Module_List	_connectmodule_list;
  Paramset_List _paramset_list;
  Attribute_Instance const* _attributes{NULL};
  Attribute_Stash _attribute_stash;
public: // build
  File();
  ~File(){
    delete _attributes;
    _attributes = NULL;
  }

  void read(std::string const&);
  void parse(CS& f) override;
  Attribute_Stash& attribute_stash() {
    return _attribute_stash;
  }
  void dump(std::ostream&)const override;

public: // readout
  const std::string& name()const	{return _name;}
  const std::string  fullstring()const	{return _file.fullstring();}
  const Head&	     head()const	{return _head;}
//  const Code_Block&  h_headers()const	{return _h_headers;}
//  const Code_Block&  cc_headers()const	{return _cc_headers;}
  const Module_List& modules()const	{return _module_list;}
//  const Code_Block&  h_direct()const	{return _h_direct;}
//  const Code_Block&  cc_direct()const	{return _cc_direct;}

  const Nature_List&	 nature_list()const	{return _nature_list;}
  const Discipline_List& discipline_list()const	{return _discipline_list;}
  const Module_List&	 module_list()const	{return _module_list;}
  const Module_List&	 macromodule_list()const	{return _macromodule_list;}
  const Module_List&	 connectmodule_list()const	{return _connectmodule_list;}
  const Paramset_List&	 paramset_list()const	{return _paramset_list;}

};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Task : public Owned_Base {
public:
  void parse(CS&)override {unreachable();}
  void dump(std::ostream&)const override {unreachable();}
};
/*--------------------------------------------------------------------------*/
// TODO: merge with Element_2?
class Filter : public Element_2 {
  std::string _name; // BUG?
  Deps _deps;
  Branch_Ref _branch;
  Probe const* _prb=NULL;
  int _num_states{0};
public:
  explicit Filter() : Element_2() {}
  explicit Filter(std::string const& name, Deps const& d)
    : Element_2(), _deps(d) {
    _name = name;
    }
  void parse(CS&) override{unreachable();}
  void dump(std::ostream&)const override {unreachable();}

  void set_num_states(int s) {_num_states = s; }
  void set_output(Branch_Ref const& x);
//  Branch_Ref const& branch() const{ return _branch; }
  std::string name() const {
    return _name;
  }
  std::string branch_code_name() const {
    assert(_branch);
    return _branch->code_name();
  }
  size_t num_branches()const;
  std::string code_name()const override;
  Deps const& deps()const { return _deps; }

  size_t num_states()const override;
  size_t num_nodes()const override;
  std::string state()const override;
  std::string short_label()const override;
//  Probe const* prb() const;
  bool has_branch() const {
    return _branch;
  }
}; // Filter
/*--------------------------------------------------------------------------*/
void make_cc_expression(std::ostream& o, Expression const& e);
void make_cc_event_cond(std::ostream& o, Expression const& e);
/*--------------------------------------------------------------------------*/
inline void Variable::new_var_ref()
{
  assert(owner());
  owner()->new_var_ref(this);
}
/*--------------------------------------------------------------------------*/
/*
analog_event_control_statement ::= analog_event_control analog_event_statement
analog_event_control ::=
@ hierarchical_event_identifier
| @ ( analog_event_expression )
analog_event_expression ::=
...
- analog_event_statement ::=
- { attribute_instance } analog_loop_statement
- | { attribute_instance } analog_case_statement
- | { attribute_instance } analog_conditional_statement
- | { attribute_instance } analog_procedural_assignment
- | { attribute_instance } analog_event_seq_block
- | { attribute_instance } analog_system_task_enable
- | { attribute_instance } disable_statement
- | { attribute_instance } event_trigger
- | { attribute_instance } ;
*/
/*--------------------------------------------------------------------------*/
class AnalogEvtExpression : public Owned_Base {
  Expression* _expression{NULL};
public:
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression const& expression() const{assert(_expression); return *_expression;};
};
/*--------------------------------------------------------------------------*/
class AnalogEvtCtlStmt : public Owned_Base {
  AnalogEvtExpression _ctl;
  Base* _stmt{NULL};
public:
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Base const* stmt_or_null() const{ return _stmt; }
  Expression const& cond() const{ return _ctl.expression(); }
};
/*--------------------------------------------------------------------------*/
template <class T, char BEGIN, char SEP, char END, char END2, char END3>
LiSt<T, BEGIN, SEP, END, END2, END3>::~LiSt(){
  delete _attributes;
  _attributes = NULL;
}
/*--------------------------------------------------------------------------*/
#if 0
template <class T, char BEGIN, char SEP, char END, char END2, char END3>
void LiSt<T, BEGIN, SEP, END, END2, END3>::dump_attributes(std::ostream& f)const
{
  if(_attributes){ untested();
    f << *_attributes;
  }else{ untested();
  }
}
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
