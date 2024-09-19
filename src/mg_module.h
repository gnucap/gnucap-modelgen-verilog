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
/*--------------------------------------------------------------------------*/
class ValueRangeSpec : public Owned_Base {
  // incomplete();
public:
//  virtual bool is_constant()const { untested(); return false; }
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
  Parameter_Base const* _param{NULL};
  std::string _name;
public:
  explicit Aliasparam() : Owned_Base() {}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  std::string const& name()const {
    return _name;
  }
  bool operator!=(const std::string& s)const {return _name != s;}
  Parameter_Base const* param()const { untested();
    assert(_param);
    return _param;
  }
  std::string const& param_name()const {
    assert(_param);
    return _param->name();
  }
  bool is_hs()const;
};
typedef  Collection<Aliasparam> Aliasparam_Collection;
/*--------------------------------------------------------------------------*/
class Port_3;
class Node;
class Probe_Map;
class File;
class Filter; // BUG. probably
class Analog;
class Circuit;
class Module : public Block {
  typedef enum : int{
    if_AC_BEGIN = 0,
    if_TR_BEGIN = 1,
    if_TR_ADVANCE = 2,
    if_TR_REVIEW = 3,
    if_TR_ACCEPT = 4,
    if_SET_EVENT = 5,
    if_COUNT = 6
  } iface_id_t;
public:
  typedef enum : int{
    mm_NONE = 0,
    mm_ANALOG = 1,
    mm_DIGITAL = 2,
    mm_BOTH = 3
  } mode_mask_t;
private: // verilog input data
  File const* _file{NULL};
  Aliasparam_Collection _aliasparam;
  // Element_2_List _element_list;
  // Port_1_List _local_nodes;
  Owned_Base* _analog{NULL};
  Circuit* _circuit{NULL};
//  Block _module_body;
public: // token?
//  rdep_tag const& tr_eval_tag   ()const { untested();return ::tr_eval_tag;}
//  rdep_tag const& tr_review_tag ()const { untested();return ::tr_review_tag;}
//  rdep_tag const& tr_advance_tag()const { untested();return ::tr_advance_tag;}
//  rdep_tag const& tr_accept_tag ()const { untested();return ::tr_accept_tag;}
protected:
  Variable_List_Collection _variables;
  Parameter_List_Collection _parameters;
  // Code_Block		_validate;
  String_Arg	_identifier;
  Module const* _proto{NULL};
private: // merge?
  std::list<FUNCTION_*> _func;
  pSet<FUNCTION_ const> _funcs;
  mode_mask_t _has_analysis{mm_NONE};

  mode_mask_t _has_pid[if_COUNT]{mm_NONE};
  int _times{0}; // _time array size
private: // elaboration data
  void new_analog();
  void new_circuit();
  void delete_analog();
  void delete_circuit();
  void detach_out_vars();
public:
  Module() {
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
protected:
  void dump_parameters(std::ostream& f)const;
  void dump_variables(std::ostream& f)const;
public: // TODO

  const Parameter_List_Collection& parameters()const	{return _parameters;}
  const Aliasparam_Collection& aliasparam()const	{return _aliasparam;}
  bool has_hsparam() const{
    return aliasparam().size();// fixme.
  }
  const Variable_List_Collection& variables()const	{return _variables;}
  const Circuit*	  circuit()const	{return _circuit;}
  const Owned_Base& analog() const {assert(_analog); return *_analog;}
  bool has_analysis()const {return _has_analysis;}

  bool has_events()const    { return _has_pid[if_SET_EVENT];}
  bool has_tr_begin()const  { return _has_pid[if_TR_BEGIN]   || times(); }
  bool has_tr_review()const { return _has_pid[if_TR_REVIEW]  || has_analysis(); }
  bool has_tr_accept()const { return _has_pid[if_TR_ACCEPT]  || has_analysis(); }
  bool has_tr_advance()const{ return _has_pid[if_TR_ADVANCE] || has_analysis()
       || has_analog_block(); // why?
  }

  bool has_ac_begin_analog()const   {untested(); return _has_pid[if_TR_BEGIN]   & mm_ANALOG; }
  bool has_tr_begin_analog()const   { return _has_pid[if_TR_BEGIN]   & mm_ANALOG; }
  bool has_tr_review_analog()const  {untested(); return _has_pid[if_TR_BEGIN]   & mm_ANALOG; }
  bool has_tr_advance_analog()const { return _has_pid[if_TR_ADVANCE] & mm_ANALOG; }
  bool has_tr_accept_analog()const  {untested(); return _has_pid[if_TR_ACCEPT]  & mm_ANALOG; }

  bool has_tr_begin_digital()const   { return _has_pid[if_TR_BEGIN]   & mm_DIGITAL; }
  bool has_tr_review_digital()const  {untested(); return _has_pid[if_TR_BEGIN]   & mm_DIGITAL; }
  bool has_tr_advance_digital()const {untested(); return _has_pid[if_TR_ADVANCE] & mm_DIGITAL; }
  bool has_tr_accept_digital()const  {untested(); return _has_pid[if_TR_ACCEPT]  & mm_DIGITAL; }

  int times()const {return _times;}
  void new_filter();
  Port_3* find_port(std::string const&);
public:
  size_t num_branches() const;
  bool sync()const;
  bool has_submodule()const;
  bool has_analog_block()const;

  void set_set_event (mode_mask_t m=mm_ANALOG) {set_pid(if_SET_EVENT, m);}
  void set_ac_begin  (mode_mask_t m=mm_ANALOG) {set_pid(if_AC_BEGIN, m);}
  void set_tr_begin  (mode_mask_t m=mm_ANALOG) {set_pid(if_TR_BEGIN, m);}
  void set_tr_review (mode_mask_t m=mm_ANALOG) {set_pid(if_TR_REVIEW, m);}
  void set_tr_accept (mode_mask_t m=mm_ANALOG) {set_pid(if_TR_ACCEPT, m);}
  void set_tr_advance(mode_mask_t m=mm_ANALOG) {set_pid(if_TR_ADVANCE, m);}

private:
  void set_pid  (iface_id_t p, mode_mask_t m=mm_ANALOG) {_has_pid[p] = (mode_mask_t)(_has_pid[p] | m);}

public:
  void set_times(int h) {if(h > _times){_times = h;}else{}}
  void set_analysis() {_has_analysis = mm_ANALOG; }
  void push_back(FUNCTION_* f);
  void push_back(Filter /*const*/ * f);
  void push_back(Token* x);
  void push_back(Base* x);
  void install(FUNCTION_ const* f);
 // std::list<FUNCTION_*> const& func()const { untested();return _func;}
  pSet<FUNCTION_ const> const& funcs()const {return _funcs;}
private: // misc
  CS& parse_analog(CS& cmd);
public: // for now.
  void parse_body(CS& f);
  void parse_ports(CS& f);
  virtual Module* deflate() { untested();return this;}
  Parameter_List_Collection& parameters()	{return _parameters;}
  void setup_functions();
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
  bool new_var_ref(Base* what)override;
  Node* node(Node_Ref r) { return r.mutable_node(*this); }
public: //filters may need this..
  Node* new_node(std::string const& p) override;
  Branch_Ref new_branch(Node*, Node*) override;
  void set_to_ground(Node const*);
}; // Module
typedef Collection<Module> Module_List;
/*--------------------------------------------------------------------------*/
class Paramset : public Module {
  // std::string _proto_name; // needed?
  Paramset* _sub{NULL};
public:
  explicit Paramset() : Module() {}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
public: // Block?
  Module* deflate() override;
private:
  CS& parse_stmt(CS& f);
  void expand();
protected:
  bool new_var_ref(Base* what) override;
};
typedef Collection<Paramset> Paramset_List;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
