/*                        -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023, 2024 Felix Salfelder
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
 *------------------------------------------------------------------
 * module circuit data
 */
/*--------------------------------------------------------------------------*/

#ifndef MG_CIRCUIT_H
#define MG_CIRCUIT_H
#include "mg_base.h"
#include "mg_discipline.h"
#include "mg_code.h" // MinTypMaxExpression??
#include "mg_expression.h"
/*--------------------------------------------------------------------------*/
class Discipline;
// TODO: Port_Base?
class Port_3 : public Owned_Base {
  std::string _name;
  std::string _value; // needed?
  Node_Ref _node;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Port_3() {}
  const std::string& name()const  {return _name;}
  const std::string& value()const  {
    if(has_identifier()){
      return _value;
    }else{
      return _name; // _node->name?
    }
  }
  bool has_identifier()const;
  String_Arg key()const { return String_Arg(value()); }
  Node_Ref const& node()const {return _node;}
  void set_node(Node*n){_node = n;}
  void set_discipline(Discipline const* d, Module* owner);
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
/*--------------------------------------------------------------------------*/
class Port_1 : public Base {
  std::string _name;
  std::string _short_to;
  std::string _short_if;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Port_1() {}
  const std::string& name()const	{return _name;}
  const std::string& short_to()const 	{return _short_to;}
  const std::string& short_if()const 	{return _short_if;}
  void set_owner(Block*){
    incomplete();
  }
  String_Arg key()const { return String_Arg(_name); }
};
/*--------------------------------------------------------------------------*/
typedef LiSt<Port_1, '{', '#', '}'> Port_1_List;
/*--------------------------------------------------------------------------*/
// TODO? param_by_index?
class Parameter_3 : public Parameter_Base {
  ConstantMinTypMaxExpression _default_val; // Base?
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Parameter_3() :Parameter_Base() {}
  ConstantMinTypMaxExpression const& default_val()const 	{return _default_val;}
};
typedef LiSt<Parameter_3, '(', ',', ')'> Parameter_3_List;
/*--------------------------------------------------------------------------*/
class Nature;
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
  void set_eval(std::string const& s){_eval = s;}
  void set_state(std::string const& s){_state = s;}
  virtual std::string dev_type()const {return _module_or_paramset_identifier;}
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
  virtual bool is_used()const {return true;} // incomplete.
};
/*--------------------------------------------------------------------------*/
class Dep;
class DDeps;
class Branch : public Element_2 {
 // TerminalPair _ports;
  Node_Ref _p;
  Node_Ref _n;
  TData *_deps{NULL}; // delete? move to _ctrl.
  // TODO: refactor into _ctrl
  size_t _has_flow_probe{0};
  size_t _has_pot_probe{0};
  size_t _has_flow_src{0};
  size_t _has_pot_src{0};
  size_t _has_short{0};
  size_t _has_always_pot{0};
  FUNCTION_ const* _ctrl{NULL};
  std::vector<Branch_Ref*> _refs;
  size_t _number;
//  std::list<std::string> _names;
  bool _direct{true};
  bool _selfdep{false};
  bool _source{false};
  bool _probe{false};
  mutable /*really?*/ int _use{0};
//  std::vector<Branch const*> _controlled_by; // move to FUNCTION_
  std::vector<Base const*> _used_in; //?
public: // use in contributions
  bool is_used()const override;
  void inc_use()const {++_use;}
  void dec_use()const {assert(_use); --_use;}
  void set_used_in(Base const*);
  void unset_used_in(Base const*);
  std::vector<Base const*> const& used_in() {return _used_in;}
public:
  explicit Branch(Node_Ref p, Node_Ref n, Module* m);
  explicit Branch(Branch_Ref p, Module* m);
  Branch( Branch const&) = delete;
  ~Branch();
  virtual std::string name()const; // use label?
  // later.
  void parse(CS&)override {incomplete();}
  void dump(std::ostream&)const override;
  std::string const* reg_name(std::string const&); //?
  Node_Ref p() const;
  Node_Ref n() const;
  bool req_short()const;
  bool is_short() const;
  bool is_direct() const{ return _direct; }
  bool is_generic() const;
  std::string code_name()const override;
  std::string short_label()const override { return code_name();}
//  std::string name_of_module_instance()const  {return code_name();}
  std::string const& omit()const override;
  std::string dev_type()const override;
  void add_dep(Dep const&);
  size_t num_nodes()const override;
  std::string state()const override;
  std::string state(std::string const& n)const;
  virtual bool has_element() const;

  void inc_flow_probe(){ ++_has_flow_probe; }
  virtual void inc_pot_probe(){ ++_has_pot_probe; }
  void inc_flow_source(){ ++_has_flow_src; }
  void inc_pot_source(){ ++_has_pot_src; }
  void inc_short(){ ++_has_short; }
  void inc_always_pot(){ ++_has_always_pot; }

  void dec_flow_probe() { assert(_has_flow_probe); --_has_flow_probe; }
  virtual void dec_pot_probe() { assert(_has_pot_probe); --_has_pot_probe; }
  void dec_flow_source() { assert(_has_flow_src); --_has_flow_src; }
  void dec_pot_source() { assert(_has_pot_src); --_has_pot_src; }
  void dec_short(){ assert(_has_short); --_has_short; }
//  void dec_always_pot(){ assert(_has_always_pot); --_has_always_pot; }

  void set_filter(FUNCTION_ const* f){ _ctrl=f; }
  void set_source(bool d=true) {_source = d; }
  void set_probe(bool d=true) {_probe = d; }
  void set_direct(bool d=true);
  void set_selfdep(bool d=true) {_selfdep = d; }
  bool has_flow_probe() const;
  bool has_pot_probe() const;
  bool has_flow_source() const { return _has_flow_src; }
  bool has_short() const { return _has_short; }
  bool has_always_pot() const { return _has_always_pot; }
  bool is_filter() const { return _ctrl; }
  bool has_pot_source()const;
  bool is_source()const {return _source || has_pot_source() || has_flow_source();}
  bool is_shadow_source()const;
  size_t num_states()const override;
  Discipline const* discipline()const override;
  Nature const* nature()const override;
public:
//  bool has(Branch_Ref*) const;
  void attach(Branch_Ref*);
  void detach(Branch_Ref*);
  size_t number() const{return _number;}
  size_t num_branches() const;

  TData const& deps()const { assert(_deps); return *_deps; } // delete?
  DDeps const& ddeps()const;
  TData& deps() { assert(_deps); return *_deps; } // delete?
						 //
  Branch const* output() const;
  virtual bool has_name()const{return false;}
private:
  void new_deps();
}; // Branch
class Branch;
class Named_Branch;
/*--------------------------------------------------------------------------*/
inline std::string to_upper(std::string s)
{ untested();
  for (std::string::iterator i = s.begin(); i != s.end(); ++i) { untested();
    *i = static_cast<char>(toupper(*i));
  }
  return s;
}
/*--------------------------------------------------------------------------*/
// typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
// .name(value)
/*--------------------------------------------------------------------------*/
// TODO: Port_Base?
class New_Port : public Port_3 {
//  Block* _owner{NULL};
public:
//  void set_owner(Block* c) { _owner = c; }
  void parse(CS& f) override;
  New_Port() : Port_3() {}
};
// list ::= "(" port {"," port} ")"
typedef LiSt<New_Port, '(', ',', ')'> New_Port_List;
/*--------------------------------------------------------------------------*/
class Net_Identifier;
typedef LiSt<Net_Identifier, '\0', ',', ';'> Net_Decl_List;
/*--------------------------------------------------------------------------*/
class Net_Declarations : public Collection<Net_Decl_List>{
  Attribute_Instance const* _attributes{NULL};
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Branch_Declaration;
class Branch_Declarations : public Collection<Branch_Declaration>{
public:
  void parse(CS& f)override;
  // void dump(std::ostream& f)const override;
};
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
  void parse(CS&) override;
  void dump(std::ostream& f)const override;
  Element_1() {untested();}
  Element_1(CS& f) {parse(f);}
  std::string dev_type()const	{return _dev_type;}
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
/*--------------------------------------------------------------------------*/
// the branches used in the model, in probes and sources, deduplicated.
class Branch_Map : public Owned_Base {
  typedef std::pair<Node const*, Node const*> key;
  typedef std::map<key, Branch*> unmap; // set?
				      //
  typedef std::list<Branch /* const?? */ *> list;
  typedef list::const_iterator const_iterator;
private:
  unmap _m;
  Keyed_List<Named_Branch> _names;
  list _brs;
public:
  explicit Branch_Map(){}
  ~Branch_Map(){
    assert(!_m.size());
    assert(!_brs.size());
  }
  //Branch_Ref new_branch(Node const* a, Node const* b, Block* owner);
  const_iterator begin() const{ return _brs.begin(); }
  const_iterator end() const{ return _brs.end(); }
  size_t size() const{ return _brs.size(); }
//  Branch_Ref lookup(std::string const&);
  Branch_Ref lookup(std::string const&) const;

  Branch_Ref new_branch(Node_Ref a, Node_Ref b);
  Branch_Ref new_branch(Branch_Ref const& b, std::string name);
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  void clear();
};
/*--------------------------------------------------------------------------*/
class Node_Map{
  typedef std::map<std::string, Node*> map; // use set??
  typedef std::vector<Node*> nodes;
  typedef nodes::const_iterator const_iterator;
  typedef nodes::const_reverse_iterator const_reverse_iterator;
  nodes _nodes;
  map _map;
public:
  static Node mg_ground_node;
public:
  explicit Node_Map();
  ~Node_Map();
public:
  const_reverse_iterator rbegin() const{ return _nodes.rbegin(); }
  const_reverse_iterator rend() const{ return _nodes.rend(); }
  const_iterator begin() const{ return _nodes.begin(); }
  const_iterator end() const{ return _nodes.end(); }
  size_t size() const{ return _map.size(); }
//   size_t how_many() const{ return _nodes.size() - 1; }
  Node* new_node(std::string const&, Block* owner);
  Node_Ref operator[](std::string const& key) const;
  Node_Ref operator[](int key) const{ return _nodes[key]; }
  void set_short(Node const*, Node const*);
};
/*--------------------------------------------------------------------------*/
typedef Collection<Element_1> Element_1_List;
typedef Collection<Element_2> Element_2_List;
class Filter;
typedef Collection<Filter> Filter_List;
/*--------------------------------------------------------------------------*/
class Circuit : public Owned_Base {
  New_Port_List	_ports;
  Port_3_List_3	_input;
  Port_3_List_3	_output;
  Port_3_List_3	_inout;
  Port_3_List_3	_ground;
  Branch_Declarations _branch_decl;
  Element_2_List _element_list;
  Port_1_List _local_nodes;
  Net_Declarations _net_decl;
  Branch_Map _branches;
  Node_Map _nodes;
  Filter_List _filters;
  size_t _num_filters{0}; // ?
public:
  explicit Circuit();
  ~Circuit();
  void parse(CS&) override;
  void dump(std::ostream&)const override{incomplete();}

  Port_3* find_port(std::string const& n);

  size_t		min_nodes()const	{return ports().size();}
  size_t		max_nodes()const	{return ports().size();}
  size_t		net_nodes()const	{return ports().size();}

  const New_Port_List&	  ports()const		{return _ports;}
  const Port_3_List_3&	  input()const		{return _input;}
  const Port_3_List_3&	  output()const		{return _output;}
  const Port_3_List_3&	  inout()const		{return _inout;}
  const Port_3_List_3&	  ground()const		{return _ground;}
  const Net_Declarations& net_decl()const       {return _net_decl;}
  const Branch_Declarations& branch_decl()const	{return _branch_decl;}
  const Element_2_List&	  element_list()const	{return _element_list;}
  const Node_Map&	nodes()const		{return _nodes;}
  const Branch_Map&	branches()const		{return _branches;}
  const Port_1_List&	  local_nodes()const	{return _local_nodes;}
  const Filter_List&	filters()const		{return _filters;} // incomplete();
  bool	has_filters()const		{return _filters.size() || _num_filters;}
public:
  void parse_ports(CS& f);

public: //TODO
  void push_back(Filter /*const?*/ * f);
  New_Port_List&	  ports()		{return _ports;}
  Port_3_List_3&	  input()		{return _input;}
  Port_3_List_3&	  output()		{return _output;}
  Port_3_List_3&	  inout()		{return _inout;}
  Port_3_List_3&	  ground()		{return _ground;}
  Net_Declarations& net_decl()       {return _net_decl;}
  Branch_Declarations& branch_decl()		{return _branch_decl;}
  Element_2_List&	  element_list()	{return _element_list;}

  Node_Map&	nodes()		{return _nodes;}
  Branch_Map&	branches()	{return _branches;}
  Port_1_List&	local_nodes()	{return _local_nodes;}
  Filter_List&	filters()	{return _filters;} // incomplete();
  void new_filter() { ++_num_filters; }
};
/*--------------------------------------------------------------------------*/
inline Circuit::Circuit()
{
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
