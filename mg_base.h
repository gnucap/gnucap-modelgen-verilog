
#ifndef MG_BASE_H
#define MG_BASE_H
#include <m_base.h>
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
  Block const* owner()const { return _owner; }
  bool is_reachable()const;
  bool is_always()const;
  bool is_never()const;
public: // virtual
//  virtual void set_reachable();
//  virtual void set_never();
//  virtual void set_always();
protected:
  Block* owner(){ return _owner;}
};
/*--------------------------------------------------------------------------*/
inline std::string to_lower(std::string s)
{ untested();
  for (std::string::iterator i = s.begin(); i != s.end(); ++i) { untested();
    *i = static_cast<char>(tolower(*i));
  }
  return s;
}
/*--------------------------------------------------------------------------*/
class Block;
class String_Arg : public Base {
protected:
  std::string	_s;
public:
  String_Arg() {}
  explicit String_Arg(const char* s) : _s(s) {}
  explicit String_Arg(std::string const& s) : _s(s) {}
  String_Arg(String_Arg const& o) : Base(), _s(o._s) {} // needed?
  String_Arg& operator=(String_Arg const& o) { _s = o._s; return *this; }
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
template <class T>
class Keyed_List : public List_Base<T> {
public:
  using List_Base<T>::begin;
  using List_Base<T>::end;
private:
  void parse(CS&) override {unreachable();}
public:
  typedef typename List_Base<T>::const_iterator const_iterator;

  // List_Base? (see Collection)
  const_iterator find(std::string const& s)const {
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
  const_iterator find(String_Arg const& s)const {
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
/*--------------------------------------------------------------------------*/
class Attribute_Instance;
template <class T, char BEGIN, char SEP, char END, char END2='\0', char END3='\0'>
class LiSt : public Keyed_List<T> {
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
	  delete p;
	  throw Exception_Too_Many(int(size()+1), int(max), 0);
	}else{
	  trace1("insert", p->key());
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
};
/*--------------------------------------------------------------------------*/
class Block;
class Raw_String_Arg : public String_Arg {
public:
  void parse(CS& f)override;
};
/*--------------------------------------------------------------------------*/
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
class Collection : public List_Base<T> { // Keyed_List?
  Block* _owner{NULL};
//  File const* _file{NULL};
public:
  using List_Base<T>::push_back;
  using List_Base<T>::begin;
  using List_Base<T>::end;
  using List_Base<T>::back;
  using List_Base<T>::is_empty;
  using List_Base<T>::pop_back;
  using List_Base<T>::erase;
  typedef typename List_Base<T>::const_iterator const_iterator;
//  typedef typename List_Base<T>::iterator iterator;

  void set_owner(Block* c) { _owner = c; }
  Block const* owner() const{return _owner;}
  Block* owner(){return _owner;}
//  void set_file(File const* f){ _file = f; }
//  File const* file() const{untested(); return _file;}
  void parse(CS& file)override {
    size_t here = file.cursor();
    T* m = new T;
    m->set_owner(_owner);
    file >> *m;
    if (!file.stuck(&here)) {
      push_back(m);
      file.skip(0); // set _ok;
    }else{
      delete m;
      file.warn(bWARNING, "what's this??");
    }
  }
  void dump(std::ostream& f)const override{
    for (const_iterator i = begin(); i != end(); ++i) {
      f << (**i);
    }
  }
  // List_Base? Keyed_List?
  const_iterator find(const String_Arg& s) const {
    return find_again(begin(), s);
  }
  const_iterator find_again(const_iterator beg, const String_Arg& s) const {
    for (const_iterator ii = beg; ii != end(); ++ii) {
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
class Node;
class Module;
class Node_Ref {
  Node* _node{NULL};
public:
  explicit Node_Ref() {}
  Node_Ref(Node* n) : _node(n){}
  Node const* operator->() const{ assert(_node); return _node; }
  operator bool() const{ return _node; }
  operator Node const*() const{ return _node; }
private:
  friend class Module;
  Node* mutable_node(Module&) const{
    assert(_node);
    // assert(_node owned by m);
    return _node;
  }
};
/*--------------------------------------------------------------------------*/
//really? used in Block
class Branch;
class Named_Branch;
class Branch_Ref : public Base {
  Branch* _br{NULL};
  bool _r{false};
public:
  Branch_Ref(Branch_Ref&& b);
  Branch_Ref(Branch_Ref const& b);
  explicit Branch_Ref() : Base() {}
  explicit Branch_Ref(Branch* b, bool reversed=false);
  explicit Branch_Ref(Named_Branch* b);
  ~Branch_Ref();
  operator bool() const{return _br;}
  Branch_Ref& operator=(Branch_Ref const& o);
  Branch_Ref& operator=(Branch_Ref&& o);
public:
  void parse(CS&) override;
  void dump(std::ostream& o)const override;
  std::string code_name()const;
  std::string code_name_()const;
  void set_name(std::string const&);
  bool has_name()const;

  operator Branch*() const{ return _br; }
  Branch* operator->() const{ return _br; }
  bool operator==(Branch_Ref const& o) const{ return _br==o._br && _r==o._r; }
  bool is_reversed() const{return _r;}

  std::string const& pname() const;
  std::string const& nname() const;
  std::string name() const;
  void set_used_in(Base const*)const;
  void unset_used_in(Base const*)const;
};
/// Some kind of ordered block with scope for parameters, variables..
class Task;
class Node;
class Nature;
class Token;
class FUNCTION_;
class Block : public List_Base<Base> /* is Base_,  has-A List? */ {
  typedef std::map<std::string, Base*> map; // set?
  typedef map::const_iterator const_iterator;
  typedef enum{
    r_unknown = -1,
    r_never = 0,
    r_always = 1,
  } block_reach_t;
  block_reach_t _reachable{r_unknown};
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
  bool is_reachable()const { return _reachable; }
  bool is_always()const { return _reachable == r_always; }
  bool is_never()const { return _reachable == r_never; }
//  void set_unreachable() { untested(); _reachable = r_never; }
//  void set_reachable() { untested(); _reachable = r_unknown; }
  void set_always() { _reachable = r_always; }
  void set_never() { _reachable = r_never; }
  void new_var_ref(Base* what);

  virtual Node* new_node(std::string const& p){ untested();
    assert(_owner);
    return _owner->new_node(p);
  }
  virtual Node_Ref node(std::string const& p) const{ untested();
    assert(_owner);
    return _owner->new_node(p); // new??
  }

  virtual Token* new_token(FUNCTION_ const* f, size_t num_args) {
    assert(_owner);
    return _owner->new_token(f, num_args);
  }

#if 1
  virtual Branch_Ref new_branch(std::string const&, std::string const&) {
    unreachable();
    return Branch_Ref(NULL, false);
  }
  virtual Branch_Ref new_branch(Node*, Node*) {
    unreachable();
    return Branch_Ref(NULL, false);
  }
  virtual Branch_Ref lookup_branch(std::string const& n)const {
    assert(_owner);
    return _owner->lookup_branch(n);
  }
  virtual Branch_Ref new_branch_name(std::string const& p, Branch_Ref const& r) { untested();
    assert(_owner);
    return _owner->new_branch_name(p, r);
  }
#endif

  void set_owner_raw(Block* b) {
    _owner = b;
  }
  void set_owner(Block* b){
    assert(!b || !_owner || _owner == b);
    _owner = b;
    if(!b){
    }else if(b->is_never()){
      set_never();
    }else if(b->is_always()){
      set_always();
    }else{
    }
  }
  Base const* lookup(std::string const& k, bool recurse=true) const;
  Base* lookup(std::string const& k, bool recurse=true);
  Base const* lookup(CS&) const;

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
class Parameter_Base : public Base {
  Block* _owner{NULL};
protected:
  std::string _name;
  std::string _type;
  std::string _code_name;
  std::string _user_name;
  std::string _alt_name;
  // ConstantMinTypMaxExpression _default_val;
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
  // const ConstantMinTypMaxExpression& default_val()const 	{return _default_val;}
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
  String_Arg key()const { return String_Arg(_name); }

#if 0
  // needed?
  void fill_in_default_name() { untested();
    if (_user_name.empty()) { untested();
      _user_name = to_upper(_code_name);
    }else{ untested();
    }
  }
#endif
  void set_owner(Block* c) { _owner = c; }
  std::string const& name() const{ return _name; }
  bool operator!=(const std::string& s)const {return _name != s;}
//  virtual bool is_constant()const {untested(); return false;}
  virtual double eval()const {untested(); return NOT_INPUT;}
protected:
  Block* owner(){ return _owner; }
}; // Parameter_Base
/*--------------------------------------------------------------------------*/
inline void String_Arg::parse(CS& f)
{
  f >> _s;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
