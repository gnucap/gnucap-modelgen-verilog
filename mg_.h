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
#include "mg_expression.h"
#include "mg_attrib.h"
#include "mg_deps.h" // BUG, Deps
#include "mg_func.h" // BUG, Probe
#include "mg_base.h"
#include "mg_code.h"
#include "mg_filter.h"
#include "mg_module.h"
// #include "mg_lib.h"
/*--------------------------------------------------------------------------*/
// TODO: better prefix
#define PS_MANGLE_PREFIX "__"
/*--------------------------------------------------------------------------*/
class Options;
Base& modelgen_opts();
/*--------------------------------------------------------------------------*/
inline void error(const std::string& message)
{untested();
  std::cerr << message << '\n';
  exit(1);
}
/*--------------------------------------------------------------------------*/
class Key : public Base {
  std::string _name;
  std::string _var;
  std::string _value;
public:
  void parse(CS& f)override {f >> _name >> _var >> '=' >> _value >> ';';}
  void dump(std::ostream& f)const override
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
class Probe; // Dep?
class FUNCTION_;
class Branch_Ref;
// m_expression.h?
bool is_true(Expression const& x);
bool is_false(Expression const& x);
bool is_zero(Expression const& x);
/*--------------------------------------------------------------------------*/
#if 0
class Const_Expression : public Expression_ {
public:
  explicit Const_Expression() : Expression_() {}
};
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Parameter_1 : public Parameter_Base {
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
  Parameter_1() :Parameter_Base() {}
};
typedef LiSt<Parameter_1, '{', '#', '}'> Parameter_1_List;
/*--------------------------------------------------------------------------*/
class ConstExpression : public Owned_Base {
  Expression_ _expression;
public:
  explicit ConstExpression() : Owned_Base() {}
  ConstExpression(CS& f, Block* o) : Owned_Base(o) { untested();
    set_owner(o);
    parse(f);
  }
  ~ConstExpression(){
  }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression const& expression() const{ return _expression;};
  bool operator==(ConstExpression const&) const;
};
/*--------------------------------------------------------------------------*/
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
  double eval()const override;
};
/*--------------------------------------------------------------------------*/
// parameter type name = value ;
typedef Parameter_2_List Localparam_List;
// class Localparam_List : public LiSt<Parameter_2, '\0', '\0', ';'> {
// };
/*--------------------------------------------------------------------------*/
class Localparam_List_Collection : public Collection<Localparam_List>{
public:
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
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
class Sensitivities;
class SeqBlock : public Block {
  String_Arg _identifier;
  Sensitivities* _sens{NULL};
public:
  explicit SeqBlock() : Block() {}
  ~SeqBlock();
  void parse(CS&)override{incomplete();}
  void dump(std::ostream& o)const override;
  void parse_identifier(CS& f) { f >> _identifier; }

  Branch_Ref new_branch(std::string const& p, std::string const& n)override {
    assert(owner());
    return owner()->new_branch(p, n);
  }
  Branch_Ref new_branch(Node* p, Node* n)override {
    assert(owner());
    return owner()->new_branch(p, n);
  }
  Node_Ref node(std::string const& n)const override {
    assert(owner());
    return owner()->node(n);
  }
  Branch_Ref lookup_branch(std::string const& n)const override {
    assert(owner());
    return owner()->lookup_branch(n);
  }
  String_Arg const& identifier() const{ return _identifier; }
  bool has_sensitivities()const {return _sens;}
  Sensitivities const* sensitivities()const {return _sens;}
  void set_sens(Base const* s);
  void merge_sens(Sensitivities const& s);
}; // SeqBlock
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// analog_procedural_block analog_statement?
class AnalogConstruct : public Owned_Base {
  Base* _stmt{NULL}; // is a ControlBlock
public:
  ~AnalogConstruct(){
    delete _stmt;
    _stmt = NULL;
  }

public:
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
class Discipline;
// TODO: Port_Base?
class Node;
class Net_Identifier : public Port_3 {
  Block* _owner{NULL};
public:
  Net_Identifier() : Port_3() {}
protected:
  Block* owner(){return _owner;}
public:
  void set_owner(Block* c) { _owner = c; }
  void parse(CS& f) override;
};
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
typedef String_Arg Branch_Identifier;
class List_Of_Branch_Identifiers : public LiSt<Branch_Identifier, '\0', ',', ';'>{
public:
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Branch_Declaration : public Owned_Base{
  List_Of_Branch_Identifiers _list;
  Branch_Ref _br;
public:
  explicit Branch_Declaration() : Owned_Base() {}
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
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
  String_Arg const& key()const { return _identifier; }
};
typedef LiSt<Arg, '{', '#', '}'> Arg_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Args : public Base {
  String_Arg _name;
  String_Arg _type;
  Arg_List   _arg_list;
public:
  void set_owner(Block const*){}
  void parse(CS& f)override {f >> _name >> _type >> _arg_list;}
  void dump(std::ostream& f)const override
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
// Name clash, VAMS_ACCESS == Probe?
// mg_analog.h?
class Probe;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Named_Branch : public Branch {
  size_t _num_states;
  Branch_Ref _br;
  std::string _name;
public:
  // explicit Named_Branch(std::string n) : Branch(NULL, NULL, NULL), _name(n){} // BUG: no transparent compare in c++<14
  explicit Named_Branch(Branch_Ref a, std::string n, Module* m)
     : Branch(a, m), _br(a), _name(n) {
     //  set_label("NBTODO");
//     if(n==""){
//       n = b.state();
//       _sl = b.code_name();
//     }else{
//       n = "_st" + n;
//       _sl = "_br" + n;
//     }
//    set_state(n);
//    _num_states = b.num_states();
//    set_label(b.short_label());
  }
//  size_t num_states()const override {return _num_states;}
  std::string name()const override{return _name;} // label?
  bool has_name()const override {return true;}
  std::string code_name()const override;
//  size_t num_nodes()const override {return _base.num_nodes();}
  std::string key()const {return _name;}
  Branch const* base()const {return _br;}
  bool is_reversed()const {return _br.is_reversed(); }
//  std::string short_label()const override  {return _sl;}
//  Discipline const* discipline()const override { return _base.discipline(); }
private:
  void inc_pot_probe()override {
    Branch::inc_pot_probe();
    assert(_br);
    _br->inc_pot_probe();
  }
  void dec_pot_probe()override {
    assert(_br);
    Branch::dec_pot_probe();
    _br->dec_pot_probe();
  }

};
/*--------------------------------------------------------------------------*/
#if 0
class Branch_Names {
  typedef std::string key;
  typedef std::map<key, Branch_Ref> map; // set?
  typedef map::const_iterator const_iterator;
private:
  map _m;
public:
  explicit Branch_Names() {}
  Branch_Ref const& new_name(std::string const&, Branch_Ref const&);
  Branch_Ref lookup(std::string const&) const;

  void clear();
};
#endif
/*--------------------------------------------------------------------------*/
class Probe_Map : public std::map<std::string, Probe*> {
public:
  ~Probe_Map();
};
/*--------------------------------------------------------------------------*/
class Paramset_Stmt : public Owned_Base {
  Parameter_Base const* _what{NULL};
  Expression_ _rhs;
  bool _overridden{false};
public:
  explicit Paramset_Stmt() : Owned_Base() {}
  void set_parameter(Parameter_Base const* b) {_what = b;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  void set_overridden() {_overridden = true;}
  bool is_overridden()const {return _overridden;}
public:
  std::string name()const;
  Expression_ const& value()const;
};
/*--------------------------------------------------------------------------*/
class Paramset : public Module {
  // std::string _proto_name; // needed?
  Paramset* _sub{NULL};
public:
  explicit Paramset() : Module() {}
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
  Module* deflate() override;
private:
  CS& parse_stmt(CS& f);
  void expand();
};
typedef Collection<Paramset> Paramset_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Node : public Base {
  std::string _name;
  int _number{-1};
  Node const* _short_to{NULL};
  std::string _short_if;
  Discipline const* _discipline{NULL};
  Nature const* _nature{NULL};
  Node* _next{NULL};
  mutable int _use{0};
  std::vector<Element_2 const*> _fanout;
public:
  void parse(CS&)override {};
  void dump(std::ostream&)const override {};
  Node(int n) : _number(n){ assert(!n); _next=this; }
  Node() {untested(); _next=this;}
  Node(CS& f) {parse(f); _next=this;}
  Node(std::string const& f, int n) : _name(f), _number(n) { _next=this;}
public:
  ~Node();
  const std::string& name()const	{return _name;}
  std::string code_name()const	{return "n_" + _name;}
  int number()const	{return _number;}
  Node const* short_to()const {return _short_to;}
  std::string const& short_if()const {return _short_if;}
  void set_discipline(Discipline const* d) {_discipline = d;}

  void set_to_ground(Module*);
  void set_to(Node*);
  void set_to(Node const* n, std::string condition) {
    assert(!_short_to);
    _short_to = n;
    _short_if = condition;
  }

  Discipline const* discipline() const{  return _discipline; }
  Nature const* nature() const{ return _nature; }
public:
  bool is_used()const;
  void inc_use()const {++_use;}
  void dec_use()const {assert(_use); --_use;}
  void connect(Element_2 const*);
  bool is_ground() const{ return !_number; }
}; // Node
/*--------------------------------------------------------------------------*/
class Node_List : public List<Node> {
public:
	explicit Node_List() : List<Node>() {}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Task : public Owned_Base {
public:
  void parse(CS&)override {unreachable();}
  void dump(std::ostream&)const override {unreachable();}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void make_cc_expression(std::ostream& o, Expression const& e, bool deriv=true);
void make_cc_event_cond(std::ostream& o, Expression const& e);
void dump_analog(std::ostream& o, Module const& m);
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
  ~AnalogEvtExpression(){
    delete _expression;
  }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression const& expression() const{assert(_expression); return *_expression;};
  Block* owner() {return Owned_Base::owner();}
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
// inline void Owned_Base::set_reachable()
// {
//   assert(_owner);
//   _owner->set_reachable();
// }
/*--------------------------------------------------------------------------*/
inline bool Owned_Base::is_reachable() const
{
  assert(_owner);
  return _owner->is_reachable();
}
/*--------------------------------------------------------------------------*/
inline bool Branch_Ref::has_name()const
{
  assert(_br);
  return _br->has_name();
}
/*--------------------------------------------------------------------------*/
inline std::string Branch_Ref::name() const
{
  assert(_br);
  return _br->name();
}
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
