/*                             -*- C++ -*-
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
 */
#ifndef GNUCAP_MG_ANALOG_H
#define GNUCAP_MG_ANALOG_H
/*--------------------------------------------------------------------------*/
#include "mg_.h"
#include "mg_code.h"
/*--------------------------------------------------------------------------*/
class AnalogStmt : public Statement {
public:
  ~AnalogStmt();

  virtual TData const& deps()const = 0;
//  Statement* parent_stmt()override { untested();
//    incomplete();
//    return NULL;
//  }
};
/*--------------------------------------------------------------------------*/
// class access_function?
class Probe : public FUNCTION_ {
  Branch_Ref _br;
  mutable RDeps _rdeps;
  enum{
    t_unknown = 0,
    t_flow,
    t_pot
  } _type{t_unknown};
public:
  explicit Probe(std::string const& xs, Branch_Ref b);
  ~Probe();

  std::string const& pname()const { return _br.pname(); }
  std::string const& nname()const { return _br.nname(); }

  bool is_flow_probe()const { return _type == t_flow;}
  bool is_pot_probe()const { return _type == t_pot;}

  std::string code_name()const override;
  std::string code_name_()const; // TODO.
  Branch const* branch()const {
    return _br;
  }
  bool is_reversed() const;
  Nature const* nature() const;
  Discipline const* discipline() const;

  bool operator==(Probe const& o) const{ untested();
    return _br == o._br && _type == o._type;
  }
  bool same_data(Probe const& o) const{
    return branch() == o.branch() && _type == o._type;
  }
  bool operator!=(Probe const& o) const{ untested();
    return !operator==(o);
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override { untested();unreachable(); return "";}
  Token* new_token(Module&, size_t)const override;
public:
  bool propagate_rdeps(RDeps const&)const;
  void set_used_in(Base const*b)const{
    return _br.set_used_in(b);
  }
  void unset_used_in(Base const*b)const{
    return _br.unset_used_in(b);
  }
private:
  void make_cc_dev(std::ostream&)const override;
  void make_cc_common(std::ostream&)const override;
}; // Probe
/*--------------------------------------------------------------------------*/
class AF_Arg_List : public Owned_Base { // was : public LiSt<Analog_Function_Arg, '\0', ',', ';'> 
//  Data_Type _type;
  typedef LiSt<Token_ARGUMENT, '\0', ',', ';'> list_t;
  typedef list_t::const_iterator const_iterator;
  enum{
    a_input,
    a_output,
    a_inout
  } _direction;
  list_t _l;
public:
  explicit AF_Arg_List(CS& cmd, Block* owner) : Owned_Base() {
    set_owner(owner);
    parse(cmd);
  }
  bool is_output() const { return _direction >= a_output; }
//  Data_Type const& type()const { untested();return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  const_iterator begin()const { return _l.begin(); }
  const_iterator end()const { return _l.end(); }
};
/*--------------------------------------------------------------------------*/
class AnalogSeqBlock : public SeqBlock {
protected: // BUG?
  TData _deps;
protected:
public:
  explicit AnalogSeqBlock() : SeqBlock() {}
  explicit AnalogSeqBlock(CS& cmd, Base* owner) : SeqBlock() { untested();
    set_owner(owner);
    parse(cmd);
  }
public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  SeqBlock const& block()const {
    return *this;
  }
public: // sensitivities?
//  void set_never() { untested(); _block.set_never(); }
//  void set_always() { untested(); _block.set_always(); }
//  bool is_never()const { untested();return _block.is_never() ;}
//  bool is_reachable()const { untested();return _block.is_reachable() ;}
//  bool is_always()const { untested();return _block.is_always() ;}
//  void set_sens(Base* s) { untested();_block.set_sens(s);}
//  void clear_vars();

  TData const& deps(){ return _deps;};
  TData const& deps()const { return _deps;};
}; // AnalogSeqBlock
/*--------------------------------------------------------------------------*/
class AnalogSeqStmt : public AnalogStmt {
  AnalogSeqBlock _block; // SeqBlock?
public:
  explicit AnalogSeqStmt() : AnalogStmt() { untested();}
  explicit AnalogSeqStmt(CS& cmd, Block* owner) : AnalogStmt() {
    set_owner(owner);
    parse(cmd);
  }
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override {
    _block.dump(o);
  }
  bool update()override { return _block.update(); }
  AnalogSeqBlock const& block()const { return _block; }
  TData const& deps()const override { untested(); return _block.deps(); }
};
/*--------------------------------------------------------------------------*/
class AnalogCtrlBlock : public AnalogSeqBlock {
public:
  explicit AnalogCtrlBlock() : AnalogSeqBlock() {}
  explicit AnalogCtrlBlock(CS& f, Statement* o) : AnalogSeqBlock() {
    set_owner(o);
    parse(f);
  }

  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  operator bool()const{ return size() || identifier() !=""; }
  void set_owner(Statement* owner);
}; // AnalogCtrlBlock
/*--------------------------------------------------------------------------*/
class AnalogFunctionArgs : public Block {
public: // can't resolve these..
  void parse(CS&)override;
  bool new_var_ref(Base* what)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class AnalogFunctionBody : // public AnalogSeqBlock
			  public AnalogCtrlBlock { //
public: // can't resolve these..
  Branch_Ref new_branch(std::string const&, std::string const&)override { untested();
    return Branch_Ref();
  }
  Branch_Ref new_branch(Node*, Node*)override { untested();
    return Branch_Ref();
  }
  Node_Ref node(std::string const&)const override {
    return Node_Ref();
  }
  Branch_Ref lookup_branch(std::string const&)const override {
    return Branch_Ref();
  }
  Base* lookup(std::string const& f, bool recurse=true)override;

  bool new_var_ref(Base* what)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
#if 1
class VariableLists : public List_Base<Base>{
  void parse(CS&)override { untested();incomplete();}
public:
  void dump(std::ostream& f)const override;
};
#endif
/*--------------------------------------------------------------------------*/
class AnalogCtrlStmt : public AnalogStmt {
  TData _deps; // here?
protected:
  AnalogCtrlBlock _body;
public:
  AnalogCtrlStmt() : _body() { }
  ~AnalogCtrlStmt(){ }
  void dump(std::ostream&)const override;
  void parse(CS& cmd)override;
  AnalogCtrlBlock const& body()const { return _body; }
private:
//  bool update()override { untested();return _body.update();}
  TData const& deps()const override { return _deps;}; // ?
//  TData const& data()const override { return _deps;};
  bool update()override {
    incomplete();
    bool ret = _body.update();
    return AnalogStmt::update() || ret;
  }
  bool propagate_rdeps(RDeps const& incoming)override;
}; // AnalogCtrlStmt
/*--------------------------------------------------------------------------*/
/*
analog_event_control_statement ::= analog_event_control analog_event_statement
analog_event_control ::=
@ hierarchical_event_identifier
| @ ( analog_event_expression )
analog_event_expression ::=
-   expression
- | posedge expression
- | negedge expression
- | hierarchical_event_identifier
- | initial_step [ ( " analysis_identifier "{ , " analysis_identifier " } ) ]
- | final_step [ ( " analysis_identifier " { , " analysis_identifier " } ) ]
- | analog_event_functions
- | analog_event_expression or analog_event_expression
- analog_event_functions ::=
- cross ( analog_expression [ , analog_expression_or_null
-   [ , constant_expression_or_null [ , constant_expression_or_null [ , analog_expression ] ] ] ] )
- | above ( analog_expression [ , constant_expression_or_null
-   [ , constant_expression_or_null [ , analog_expression ] ] ] )
- | timer ( analog_expression [ , analog_expression_or_null
-   [ , constant_expression_or_null [ , analog_expression ] ] ] )
- analog_event_statement ::=
-   { attribute_instance } analog_loop_statement
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
class AnalogEvtExpression : public Expression_ {
  RDeps _rdeps;
public:
  explicit AnalogEvtExpression() : Expression_() {}
  ~AnalogEvtExpression(){ }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
//  Expression const& expression() const{return _expression;};
  bool is_used_in(Base const*)const;
//
private: // incomplete
  void set_rdeps(); // pull in rdeps from functions
  bool add_rdep(Base const* b) {
    return _rdeps.insert(b).second;
  }
  FUNCTION_ const* function()const; // BUG. expressions?
};
/*--------------------------------------------------------------------------*/
class AnalogEvtCtlStmt : public AnalogCtrlStmt {
  AnalogEvtExpression _ctrl;
public:
  ~AnalogEvtCtlStmt() { }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression_ const& cond()const { return _ctrl; } // override?
  const AnalogCtrlBlock& code()const { return _body; }
  bool update()override;

  bool is_used_in(Base const* b)const override;
 // RDeps const& rdeps()const override { untested(); return _ctrl.data().rdeps(); }
}; // AnalogEvtCtlStmt
typedef Collection<AnalogEvtCtlStmt> Analog_Events;
/*--------------------------------------------------------------------------*/
class Analog_Function : public /*UserFunction?*/ Statement {
  Token* _variable{NULL};
  Data_Type _type;
//  AF_Arg_List_Collection _args;
  FUNCTION_ const* _function{NULL};
  // simplify: statement is always a block.
  AnalogFunctionArgs _args;
  AnalogFunctionBody _block;
  VariableLists _vars;
  bool update()override { untested(); return false; }
public:
  ~Analog_Function();
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  std::string const& key()const { assert(_variable); return _variable->name(); }
  std::string code_name()const { assert(_variable); return "af_" + _variable->name(); }
  // std::string code_name()const { untested(); assert(_variable); return "_v_" + _variable->name(); } // _variable->code_name?
  FUNCTION_ const* function() const{return _function;}
  // AF_Arg_List_Collection const& args() const{ untested(); return _args; }
  Token const* variable() const{ return _variable; }
  AnalogFunctionArgs const& args() const{return _args;}
  AnalogFunctionBody const& body() const{return _block;}
  Data_Type const& type()const {return _type;}
  //VariableList const& variables()const { untested();return _vars;}
  bool is_used_in(Base const*)const override{ untested();
    // incomplete();
    return true;
  }
 // RDeps const& rdeps()const override { untested(); return _rdeps; }
};
typedef Collection<Analog_Function> Analog_Functions;
/*--------------------------------------------------------------------------*/
class Analog : public Owned_Base {
  AnalogList _list;
  Analog_Functions _functions;
  Probe_Map* _probes{NULL};
  Analog_Events _events;
public:
  explicit Analog();
  ~Analog();
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;

  bool has_block() const;
  AnalogList const& list()const { return _list; }
  AnalogList const& blocks()const { return _list; }
  Analog_Functions const& functions()const { return _functions; }
  Analog_Events const& events()const { untested(); return _events; }
  void push_back(Base*);

  void new_probe_map(); // analog?
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br, Module* m);
};
/*--------------------------------------------------------------------------*/
inline Analog const& analog(Module const& m)
{
  Analog const* a = prechecked_cast<Analog const*>(&m.analog());
  assert(a);
  return *a;
}
/*--------------------------------------------------------------------------*/
inline AnalogList const& analog_list(Module const& m)
{
  return analog(m).list();
}
/*--------------------------------------------------------------------------*/
class AnalogExpression : public Expression_ {
public:
  explicit AnalogExpression() : Expression_() {}
  ~AnalogExpression();
  void parse(CS& file) override;
//  void dump(std::ostream& o)const override;
//  Block* owner() { untested();return Owned_Base::owner();}
  Expression const& expression() const{ return *this;}
  bool is_true() const;
  bool is_false() const;
//  bool has_sensitivities()const { untested(); return !_sens.empty(); }
};
/*--------------------------------------------------------------------------*/
class AnalogConstExpression : public AnalogExpression {
public:
//  void parse(CS& file) override;
//  void dump(std::ostream& o)const override;
  String_Arg key() const{ untested();return String_Arg("ACE");}
};
typedef LiSt<AnalogConstExpression, '\0', ',', ':'> AnalogConstExpressionList;
/*--------------------------------------------------------------------------*/
class CaseGen : public AnalogCtrlStmt {
  AnalogConstExpressionList* _cond{NULL};
  RDeps _rdeps; // TODO;
private:
  CaseGen() : AnalogCtrlStmt(){ untested(); unreachable(); }
public:
  CaseGen(CS&, Block*, Expression const&, bool, bool);
  ~CaseGen(){
    delete _cond;
  }
  void parse(CS&)override;
  void dump(std::ostream& o)const override;
public:
  AnalogConstExpressionList const* cond_or_null()const {return _cond;}
  bool is_default()const {return !_cond;}
  void set_never() {return _body.set_never() ;}
  void set_always() { untested();return _body.set_always() ;}
  bool is_never()const {return _body.is_never() ;}
  bool is_reachable()const {return _body.is_reachable() ;}
  bool is_always()const {return _body.is_always() ;}
  void calc_reach(Expression const& cond);

  String_Arg key() const{ untested();return String_Arg("CaseGen");}
  RDeps const& rdeps()const override { untested(); return _rdeps; }
 // bool update() override { untested(); incomplete(); }
};
/*--------------------------------------------------------------------------*/
// code?
class System_Task : public Statement {
  AnalogExpression _e; // Analog?
  RDeps _rdeps;
public:
  explicit System_Task(CS&, Block*);
  void parse(CS& o) override;
  void dump(std::ostream&o)const override;
  bool is_used_in(Base const*)const override;
  bool update()override;

  bool has_sensitivities()const {return _e.data().has_sensitivities();}
  Sensitivities const& sensitivities()const {return _e.data().sensitivities();}

  AnalogExpression const& expression()const { return _e; }
  FUNCTION_ const* function()const;
  TData const& data()const { return _e.data(); }
  RDeps const& rdeps()const override { return _rdeps; }
private:
  bool add_rdep(Base const* b) {
    return _rdeps.insert(b).second;
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class AnalogSwitchStmt : public AnalogStmt { // CtrlStmt?
  TData _deps; // here?
  AnalogConstExpression _ctrl; // Const??
  SeqBlock _body;
  RDeps _rdeps; // here?
public:
  AnalogSwitchStmt(Block* o, CS& file);
  ~AnalogSwitchStmt() { }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  AnalogConstExpression const& control() const{return _ctrl;}
  SeqBlock const& cases()const {return _body;}
  bool update()override;
  TData const& deps()const override { return _deps;};
 // TData const& data()const override {return _deps;}; // ctrl?
  RDeps const& rdeps()const override{ return _rdeps; }
}; // AnalogSwitchStmt
/*--------------------------------------------------------------------------*/
class AnalogConditionalStmt : public AnalogCtrlStmt {
  AnalogConstExpression _cond; // Const?
  AnalogCtrlBlock _false_part;
public:
  explicit AnalogConditionalStmt(Block* o, CS& file);
  ~AnalogConditionalStmt(){ }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  AnalogConstExpression const& conditional() const{return _cond;}
  const AnalogCtrlBlock& true_part() const{ return _body; }
  const AnalogCtrlBlock& false_part() const{ return _false_part; }
  bool is_used_in(Base const*)const override;

  TData const& deps()const override{ return _cond.data(); } // ?
}; // AnalogConditionalStmt
/*--------------------------------------------------------------------------*/
class AnalogWhileStmt : public AnalogCtrlStmt {
protected:
  AnalogExpression _cond; // -> AnalogCtrlStmt?
  explicit AnalogWhileStmt() { }
public:
  explicit AnalogWhileStmt(CS& file, Block* o);
  ~AnalogWhileStmt() { }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  AnalogExpression const& conditional()const {return _cond;}
  bool has_body() const{ return _body; }
  const Base& body() const{assert(_body); return _body; }
  virtual bool has_tail() const{ return false; }
  virtual Base const& tail() const{ untested(); return _cond; }
private:
  bool update()override;
};
/*--------------------------------------------------------------------------*/
class AnalogForStmt : public AnalogWhileStmt {
  Assignment* _init{NULL};
  Assignment* _tail{NULL};
public:
  explicit AnalogForStmt(CS& file, Block* o) : AnalogWhileStmt() {
    set_owner(o);
    parse(file);
  }

  void parse(CS& file) override;
  void dump(std::ostream& o)const override;

  bool has_init()const{ return _init; }
  Assignment const& init()const{ assert(_init); return *_init; }

  bool has_tail()const override{ return _tail; }
  Assignment const& tail()const override{ assert(_tail); return *_tail; }
private:
  bool update()override;
};
/*--------------------------------------------------------------------------*/
// just AssignStatement?
class AnalogProceduralAssignment : public AnalogStmt {
  Assignment _a;
  TData _deps;
 // RDeps _rdeps;
public:
  // explicit AnalogProceduralAssignment(Block* o) { untested();
  //   set_owner(o);
  //   _a.set_owner(o);
  // }
  explicit AnalogProceduralAssignment(CS&, Block*);
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  Assignment const& expression()const {return _a;}
  bool is_used_in(Base const*b)const override;
  Statement* deep_copy(Base* no)const override;
  bool propagate_rdeps(RDeps const& r)override;
private:
  bool update()override;
  TData const& deps()const override {return _deps;};
public: // dump_annotate
  TData const& data()const {return _a.data();};
  bool has_sensitivities()const {
    return _a.has_sensitivities();
  }
  Sensitivities const& sensitivities()const {return _a.sensitivities();}
}; // AnalogProceduralAssignment
/*--------------------------------------------------------------------------*/
// ContributionStatement?
class Contribution : public AnalogStmt {
  std::string _name;
  Nature const* _nature{NULL};
  Expression_ _rhs;
  Branch_Ref _branch;
  enum{
    t_unknown,
    t_flow,
    t_pot
  } _type{t_unknown};
  bool _short{false};
//  Block* _owner{NULL};
  TData* _deps{NULL};
  RDeps _rdeps; // dump_annotate
  Sensitivities _sens;
private:
  void set_pot_contrib();
  void set_flow_contrib();
  void set_short();
  void set_always_pot();
  void set_direct(bool d=true);
  TData const& deps()const override; // data?
public:
  Contribution(CS& f, Block* o)
    : AnalogStmt(), _branch(NULL, false) {
    set_owner(o);
    parse(f);
  }
  ~Contribution();
public:

  DDeps const& ddeps() const;
  bool has_sensitivities()const { return !_sens.empty(); }  // BUG? _deps->sens?
  Sensitivities const& sensitivities()const { return _sens; }  // BUG? _deps->sens?
  bool is_pot_contrib() const;
  bool is_flow_contrib() const;
  bool is_short() const { return _short; }
  bool is_direct() const;
  bool is_always() const{ assert(owner()); return owner()->is_always(); }
  bool is_reachable()const { untested();return owner()->is_reachable() ;}
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  TData const& data() const {
    if(_deps) {return *_deps;} else {return _rhs.data();}
  }
  bool has_deps()const { untested();return true;}
  Expression const& rhs()const {return _rhs;}
  std::string const& name() const{return _name;}
  Branch_Ref const& branch_ref() const{return _branch;}
  Branch const* branch() const{return _branch;}
  bool reversed() const{ return _branch.is_reversed() ;}
  Branch_Ref new_branch(std::string const& p, std::string const& n) { untested();
    assert(owner());
    return owner()->new_branch(p, n);
  }
  Branch_Ref new_branch(Node* p, Node* n) { untested();
    assert(owner());
    return owner()->new_branch(p, n);
  }
private:
  Expression& rhs() {return _rhs;}
  bool update()override;
  void add_dep(Dep const&);
public:
  RDeps const& rdeps()const override { return _rdeps; }
//  RDeps const& rdeps()const override { untested();return data().rdeps();}
}; // Contribution
/*--------------------------------------------------------------------------*/
// VarDeclStmt? code?
#if 0
class AnalogDeclareVars : public AnalogStmt {
//  typedef LiSt<Token_VARIABLE, '\0', ',', ';'> list_t;
  typedef LiSt<Token_VAR_REF, '\0', ',', ';'> list_t;
  list_t _l;
  TData _deps;
public:
  explicit AnalogDeclareVars(CS& f, Block* o){ untested();
    set_owner(o);
    parse(f);
  }
  void parse(CS& f)override;
  void dump(std::ostream& o)const override;
  list_t const& list() const{ untested();
    return _l;
  }

private:
  bool update()override;
  TData const& deps()override { return _deps;};
};
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
