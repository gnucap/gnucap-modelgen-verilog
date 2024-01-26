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
 * Verilog-AMS module data structures
 */
/*--------------------------------------------------------------------------*/
#ifndef MG_MODULE_H
#define MG_MODULE_H
#include "mg_base.h"
#include "mg_code.h"
class Aliasparam;
class Variable_List;
class Variable_List_Collection : public Collection<Variable_List>{
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class ValueRangeSpec : public Owned_Base {
  // incomplete();
public:
//  virtual bool is_constant()const { return false; }
  virtual double eval()const { return NOT_INPUT; }
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
class ValueRange : public Owned_Base {
  enum{
    vr_FROM,
    vr_EXCLUDE
  } _type{vr_FROM};
  ValueRangeSpec* _what{NULL};
public:
  void parse(CS& f) override;
  void dump(std::ostream& f)const override;
  bool is_from() const{return _type == vr_FROM;}
  bool is_exclude() const{return _type == vr_EXCLUDE;}
  ValueRangeSpec const* spec() const{ return _what; }
  double eval()const;

  String_Arg key()const { return String_Arg("ValueRange"); }
};
typedef LiSt<ValueRange, '\0', '\0', '\0', ',', ';'> ValueRangeList;
class Aliasparam;
class Parameter_2 : public Parameter_Base {
  ValueRangeList _value_range_list;
  std::list<Aliasparam const*> _aliases;
  bool _is_local{false};
  bool _is_given{false};
  ConstantMinTypMaxExpression _default_val;
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Parameter_2() :Parameter_Base() {}
  void set_type(std::string const& a){_type=a;}
  void set_local( bool x=true ) {_is_local = x;}
  bool is_local()const {return _is_local;}
  void set_given( bool x=true ) {_is_given = x;}
  bool is_given()const {return _is_given;}
  void add_alias(Aliasparam const*);
  ValueRangeList const& value_range_list()const { return _value_range_list; }
  std::list<Aliasparam const*> const& aliases()const {return _aliases;}
  void resolve();
  double eval()const override;
  ConstantMinTypMaxExpression const& default_val()const 	{return _default_val;}
};
/*--------------------------------------------------------------------------*/
class Parameter_2_List : public LiSt<Parameter_2, '\0', ',', ';'> {
  String_Arg _type;
  bool _is_local{false};
  Attribute_Instance const* _attributes{NULL};
public:
  bool is_local()const;
  String_Arg const& type()const {return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Parameter_List_Collection : public Collection<Parameter_2_List>{
public:
  void dump(std::ostream& f)const override;
  size_t count_nonlocal() const;
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
  Parameter_2 const* param()const {
    assert(_param);
    return _param;
  }
  std::string param_name()const {
    assert(_param);
    return _param->name();
  }
};
typedef  Collection<Aliasparam> Aliasparam_Collection;
/*--------------------------------------------------------------------------*/
class Node;
class Probe_Map;
class File;
class Filter; // BUG. probably
class Analog;
class Circuit;
class Module : public Block {
private: // verilog input data
  File const* _file{NULL};
  Variable_List_Collection _variables;
  Aliasparam_Collection _aliasparam;
  // Element_2_List _element_list;
  // Port_1_List _local_nodes;
  Owned_Base* _analog{NULL};
  Circuit* _circuit{NULL};
protected:
  Attribute_Stash _attribute_stash;
  Parameter_List_Collection _parameters;
  // Code_Block		_validate;
  String_Arg	_identifier;
  Module const* _proto{NULL};
private: // merge?
  std::list<FUNCTION_ const*> _func;
  pSet<FUNCTION_ const> _funcs;
  size_t _num_evt_slots{0};
  bool _has_analysis{false};
  bool _has_tr_review{false};
private: // elaboration data
  Probe_Map* _probes{NULL};
  void new_probe_map(); // analog?
  void new_analog();
  void new_circuit();
  void delete_analog();
  void delete_circuit();
public:
  Module() {
    new_probe_map();
    new_analog();
    new_circuit();
  }
  ~Module();
public:
  String_Arg const& key()const	  {return _identifier;}
  File const* file() const{ return _file; }; // owner?
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  const String_Arg&	  identifier()const	{return _identifier;}
public: // TODO

  const Parameter_List_Collection& parameters()const	{return _parameters;}
  const Aliasparam_Collection& aliasparam()const	{return _aliasparam;}
  const Variable_List_Collection& variables()const	{return _variables;}
  const Circuit*	  circuit()const	{return _circuit;}
  const Owned_Base& analog() const {assert(_analog); return *_analog;}
  bool has_events()const {return _num_evt_slots;}
  bool has_analysis()const {return _has_analysis;}
  bool has_tr_review()const {return _has_tr_review;}
  void new_evt_slot() { ++_num_evt_slots; }
  void new_filter();
  size_t num_evt_slots()const {return _num_evt_slots; }
  Port_3* find_port(std::string const&);
public:
  size_t num_branches() const;
  bool sync()const;
  bool has_submodule()const;
  bool has_analog_block()const;
  Attribute_Stash& attribute_stash() {
    return _attribute_stash;
  }
  void set_tr_review() {_has_tr_review = true; }
  void set_analysis() {_has_analysis = true; }
  void set_attributes(Attribute_Instance const* a) {
    assert(!_attributes);
    _attributes = a;
  }
  void push_back(FUNCTION_ const* f);
  void push_back(Filter /*const*/ * f);
  void push_back(Base* x){
    return Block::push_back(x);
  }
  void install(FUNCTION_ const* f);
  void install(Probe const* f);// ?
  std::list<FUNCTION_ const*> const& func()const {return _func;}
  pSet<FUNCTION_ const> const& funcs()const {return _funcs;}
private: // misc
  CS& parse_analog(CS& cmd);
public: // for now.
  void parse_body(CS& f);
  void parse_ports(CS& f);
  virtual Module* deflate() {return this;}
  Parameter_List_Collection& parameters()	{return _parameters;}
  void setup_nodes();

public: // BUG
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br);
  Branch_Ref new_branch(std::string const&, std::string const&) override;
private:

  Token* new_token(FUNCTION const*, size_t na) override;
  Branch_Ref new_branch_name(std::string const& n, Branch_Ref const& b) override;
  Node_Ref node(std::string const& p) const override;
  Branch_Ref lookup_branch(std::string const& p) const override;
public:
  Node* node(Node_Ref r) { return r.mutable_node(*this); }
public: //filters may need this..
  Node* new_node(std::string const& p) override;
  Branch_Ref new_branch(Node*, Node*) override;
  void set_to_ground(Node const*);
}; // Module
typedef Collection<Module> Module_List;
/*--------------------------------------------------------------------------*/
inline void Module::install(FUNCTION_ const* f)
{
  _funcs.insert(f);
}
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
