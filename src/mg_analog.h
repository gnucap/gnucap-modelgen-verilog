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
typedef Collection<Statement> AnalogList;
/*--------------------------------------------------------------------------*/
class AnalogStmt : public Statement {
public:
  ~AnalogStmt();
  virtual TData const& deps()const = 0;
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
private:
  friend Probe const* probe(Dep const&);
  bool is_reversed() const { untested();
    incomplete();
    return _br.is_reversed();
  }
public:
  Nature const* nature() const;
  Discipline const* discipline() const;

  bool same_data(Probe const& o) const{ untested();
    return branch() == o.branch() && _type == o._type;
  }
private:
  void stack_op(Expression*)const override { untested();
    throw Exception("invalid");
  }
  Token* new_token(Module&, size_t)const override;
public:
  bool propagate_rdeps_(RDeps const&)const;
  void set_used_in(Base const*b)const{
    _br.set_used_in(b);
  }
  void unset_used_in(Base const*b)const{
    _br.unset_used_in(b);
  }
private:
  void make_cc_dev(std::ostream&)const override;
  void make_cc_common(std::ostream&)const override;
  Data_Type const* return_type()const override { untested();
    static Data_Type_Real r; return &r;
  }
}; // Probe
/*--------------------------------------------------------------------------*/
// BUG: implementation detail
class AF_Arg_List : public Owned_Base {
  typedef LiSt<Token_ARGUMENT, '\0', ',', ';'> list_t;
  typedef list_t::const_iterator const_iterator;
  direction_t _direction;
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
public:
  explicit AnalogSeqBlock() : SeqBlock((Block*)nullptr) {}
  explicit AnalogSeqBlock(CS& cmd, Statement* owner) : SeqBlock(owner) { untested();
    set_owner(owner);
    parse(cmd);
  }
public:
  // void parse(CS& cmd)override; // SeqBlock
  operator bool()const{ return size() || identifier() !=""; }
//  void set_owner(Statement* owner);
private:
  Base* parse_stmt_or_null(CS& file, Block* scope)const override;
}; // AnalogSeqBlock
/*--------------------------------------------------------------------------*/
class AnalogConstruct : public Statement {
  AnalogSeqBlock _block;
public:
  AnalogConstruct() : Statement() { }
  ~AnalogConstruct() { }
private: // Statement
  void submit_variable_access(Variable_Access&)const override;

public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  bool update()override;
  void new_block();
  Block const* block_or_null() const{ return &_block; }
  Block* block(){ return &_block; }
  Block const* block()const{ return &_block; }
  void push_back(Statement*); // push into _block...
  bool is_used_in(Base const*)const override; // BUG?
}; // AnalogConstruct
/*--------------------------------------------------------------------------*/
class AnalogFunctionArgs : public Block {
  Variable_List_Collection _variables;
  std::vector<Token*> _arg_by_idx; // Expression?
  std::vector<TData*> _data;
public:
  explicit AnalogFunctionArgs();
public: // can't resolve these..
  Token const* arg_by_idx(int i)const {assert(i<int(_arg_by_idx.size()));
                                       return _arg_by_idx[i];}
  void push_back(Variable_Stmt* b);
  void parse(CS&)override;
  void set_deps(Block::map const&);
  bool new_var_ref(Base* what)override;
  void dump(std::ostream& f)const override;
  Base* lookup(std::string const& f, bool recurse=true)override;
  Token_ARGUMENT* new_arg(std::string const& name, Base* owner);
};
/*--------------------------------------------------------------------------*/
class AnalogFunctionBody : public AnalogSeqBlock {
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
//  void dump(std::ostream& f)const override { untested(); AnalogSeqBlock::dump(o); }
  Block* scope();
};
/*--------------------------------------------------------------------------*/
class AnalogCtrlStmt : public CtrlStmt {
public:
  AnalogCtrlStmt() : CtrlStmt() { }
  ~AnalogCtrlStmt(){ delete _block; _block = nullptr; }
// void parse(CS& cmd)override; // CtrlStmt
// void dump(std::ostream&)const override; // CtrlStmt
  SeqBlock const& body()const {
    auto r = prechecked_cast<AnalogSeqBlock const*>(_block);
    assert(r);
    return *r;
  }
  SeqBlock& body() {
    auto r = prechecked_cast<AnalogSeqBlock*>(_block);
    assert(r);
    return *r;
  }
private:
  TData const& deps()const override { return _deps;}; // ?
private:
  SeqBlock* make_block()const override {return new AnalogSeqBlock();}
}; // AnalogCtrlStmt
/*--------------------------------------------------------------------------*/
class AnalogSeqStmt : public AnalogCtrlStmt {
public:
  explicit AnalogSeqStmt() : AnalogCtrlStmt() {untested(); }
  explicit AnalogSeqStmt(CS& cmd, Block* owner) : AnalogCtrlStmt() {
//    new_block();
    set_owner(owner);
    parse(cmd);
  }
  ~AnalogSeqStmt() { delete _block; _block = nullptr; }
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  bool update()override { return block().update(); }
  SeqBlock const& block()const { return *prechecked_cast<AnalogSeqBlock*>(_block); }
  TData const& deps()const override { untested(); return block().deps(); }
private:
  AnalogSeqBlock& block() { return *prechecked_cast<AnalogSeqBlock*>(_block); }
  void submit_variable_access(Variable_Access&)const override;
};
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
 // bool update(RDeps const* r) // not virtual
  bool is_final()const {return is_used_in(&final_tag)
    || _rdeps.contains(&final_tag); }
private: // incomplete
  void set_rdeps(); // pull in rdeps from functions
  bool add_rdep(Base const* b) {
    return _rdeps.insert(b).second;
  }
  FUNCTION_ const* function()const;
public:
  RDeps const& rdeps()const { return _rdeps; }
};
/*--------------------------------------------------------------------------*/
class AnalogEvtCtlStmt : public AnalogCtrlStmt {
  AnalogEvtExpression _ctrl;
public:
  explicit AnalogEvtCtlStmt() : AnalogCtrlStmt() { }
  ~AnalogEvtCtlStmt() { }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression_ const& cond()const { return _ctrl; } // override?
  const SeqBlock& code()const { return body(); }
  bool update()override;

  bool is_used_in(Base const* b)const override;
  // TODO bool propagate_rdep(Base const*) override; // block unused tags.
private:
  void submit_variable_access(Variable_Access&)const override;
}; // AnalogEvtCtlStmt
typedef Collection<AnalogEvtCtlStmt> Analog_Events;
/*--------------------------------------------------------------------------*/
class Analog_Function : public /*UserFunction?*/ Statement {
  Token* _variable{nullptr};
  Data_Type _type;
  FUNCTION_ const* _function{nullptr};
  AnalogFunctionArgs _args;
  friend class AnalogFunctionBody; // uuh
  bool update()override;
  std::string _identifier;
protected:
  AnalogFunctionBody _block;
public:
  explicit Analog_Function() {}
  ~Analog_Function();
private:
  void submit_variable_access(Variable_Access&)const override { untested();
    unreachable();
    incomplete();
  }
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  std::string const key()const { assert(_variable); return _variable->name(); }
  std::string code_name()const { assert(_variable); return "af_" + _variable->name(); }
  FUNCTION_ const* function() const{return _function;}
  Token const* variable() const{ return _variable; }
  AnalogFunctionArgs const& header() const{return _args;}
  AnalogFunctionBody const& body() const{return _block;}
  Data_Type const& type()const {return _type;}
  bool is_used_in(Base const*)const override{ untested();
    return true;
  }
//  bool makes_own_scope()const override { untested();return true;}
public: // args
  bool is_output_arg(int I)const;
  TData const* arg_data(int I)const; // needed?
  Token const* arg_token(int I)const;
  Data_Type const* arg_type(int I)const;
private:
  // parse_identifier?
  void set_identifier(std::string const& name) { _identifier = name; }
  AnalogFunctionArgs& header() {return _args;}
}; // Analog_Function
typedef Collection<Analog_Function> Analog_Functions;
/*--------------------------------------------------------------------------*/
class Analog : public Owned_Base /*Block?*/ {
  AnalogList _list; // Statements.
  Analog_Functions _functions;
  Probe_Map* _probes{nullptr};
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

  // kind of submit_variable_access
  void setup_storage(Variable_Access&)const;
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
class CaseGen : public AnalogCtrlStmt {
  ConstExpressionList* _cond{nullptr};
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
  void submit_variable_access(Variable_Access&)const override;
private:
//  SeqBlock const& body()const {return AnalogCtrlStmt::body();}
public:
  ConstExpressionList const* cond_or_null()const {return _cond;}
  bool is_default()const {return !_cond;}
  void set_never() {return body_().set_never() ;}
  void set_always() { untested();return body_().set_always() ;}
  bool is_never()const {return body_().is_never() ;}
  bool is_reachable()const {return body_().is_reachable() ;}
  bool is_always()const {return body_().is_always() ;}
  void calc_reach(Expression const& cond);
  Block& body_() {return body();}
  Block const& body_()const {return body();}

  String_Arg key() const{ untested();return String_Arg("CaseGen");}
  RDeps const& rdeps()const override { untested(); return _rdeps; }
 // bool update() override { untested(); incomplete(); }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class AnalogInitialStmt : public InitialStmt {
public:
  explicit AnalogInitialStmt(Block* o, CS& file) {
    set_owner(o);
    parse(file);
  }
  ~AnalogInitialStmt(){ }
public:
  void parse(CS& file) override;
//  void dump(std::ostream& o)const override;
 //  bool is_used_in(Base const*)const override; // needed?
  // bool update()override;
  SeqBlock* make_block()const override {return new AnalogSeqBlock();}

 // TData const& deps()const override // AnalogCtrlStmt
}; // AnalogInitialStmt
/*--------------------------------------------------------------------------*/
class AnalogSwitchStmt : public CtrlStmt {
  TData _deps; // here?
  ConstExpression_ _ctrl; // Const??
  SwitchBlock _body; // Abuse SeqBlock and turn into CtrlStmt?
  RDeps _rdeps; // here?
  CaseGen const* _always{nullptr};
  CaseGen const* _default{nullptr};
public:
  AnalogSwitchStmt(Block* o, CS& file) {
    set_owner(o);
    parse(file);
  }
  ~AnalogSwitchStmt() { }
protected:
  SwitchBlock const& body()const { return _body;}
  SwitchBlock& body() { return _body;}
private:
  bool have_always()const {return _always;}
  bool have_default()const {return _default;}
  void submit_variable_access(Variable_Access&)const override;
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  ConstExpression_ const& control() const{return _ctrl;}
  SwitchBlock const& cases()const {return _body;}
  bool update()override;
  TData const& deps()const override { return _deps;};
 // TData const& data()const override {return _deps;}; // ctrl?
  RDeps const& rdeps()const override{ return _rdeps; }
private:
  SeqBlock* make_block()const override {return new AnalogSeqBlock();}
}; // AnalogSwitchStmt
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class AnalogConditionalStmt : public ConditionalStmt {
public:
  explicit AnalogConditionalStmt(Block* o, CS& f)
    : ConditionalStmt() {
    set_owner(o);
    parse(f);
  }
  ~AnalogConditionalStmt() {}
private:
  SeqBlock* make_block()const override {return new AnalogSeqBlock();}
  void new_block() override {
    CtrlStmt::new_block();
    _false_part = make_block();
  }
}; // AnalogConditionalStmt
/*--------------------------------------------------------------------------*/
class AnalogWhileStmt : public WhileStmt {
public:
  explicit AnalogWhileStmt() : WhileStmt() {}
  explicit AnalogWhileStmt(CS& f, Block* o);
  ~AnalogWhileStmt() { delete _block; _block = nullptr; }
private:
  SeqBlock* make_block()const override {return new AnalogSeqBlock();}
};
/*--------------------------------------------------------------------------*/
class AnalogForStmt : public ForStmt {
public:
  explicit AnalogForStmt(CS& f, Block* o);
  ~AnalogForStmt() {}
private:
  SeqBlock* make_block()const override {return new AnalogSeqBlock();}
};
/*--------------------------------------------------------------------------*/
// just AssignStatement?
class AnalogProceduralAssignment : public AnalogStmt {
  Assignment _a;
  TData _deps;
public:
  // explicit AnalogProceduralAssignment(Block* o) { untested();
  //   set_owner(o);
  //   _a.set_owner(o);
  // }
  explicit AnalogProceduralAssignment(CS&, Block*);
private:
  void submit_variable_access(Variable_Access& va)const override {
    _a.submit_variable_access(va);
  }
public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  Assignment const& expression()const {return _a;}
  bool is_used_in(Base const*b)const override;
  Statement* deep_copy(Base* no)const override;
  bool propagate_rdeps(RDeps const& r)override;
  bool propagate_rdep(Base const*);
private:
  bool update()override;
  TData const& deps()const override {return _deps;};
public: // dump_annotate
  TData const& data()const {return _a.data();};
  bool has_sensitivities()const {
    return _a.has_sensitivities();
  }
  Sensitivities const& sensitivities()const {return _a.sensitivities();}
  bool is_state_var()const {return _a.is_state_var();}
}; // AnalogProceduralAssignment
/*--------------------------------------------------------------------------*/
// ContributionStatement?
class Contribution : public AnalogStmt {
  std::string _name;
  Nature const* _nature{nullptr};
  Expression_ _rhs;
  Branch_Ref _branch;
  enum{
    t_unknown,
    t_flow,
    t_pot
  } _type{t_unknown};
  bool _short{false};
//  Block* _owner{nullptr};
  TData* _deps{nullptr};
  RDeps _rdeps; // dump_annotate
  Sensitivities _sens;
public:
  Contribution(CS& f, Block* o)
    : AnalogStmt(), _branch(nullptr, false) {
    set_owner(o);
    parse(f);
  }
  ~Contribution();

  std::string val_string()const override {return "contribution " + _name; }
private:
  void set_pot_contrib();
  void set_flow_contrib();
  void set_short();
  void set_always_pot();
  void set_direct(bool d=true);
  TData const& deps()const override; // data?
  void submit_variable_access(Variable_Access& va)const override;
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
inline Probe const* probe_(Dep const& d)
{
  assert(d.probe__());
  return d.probe__();
}
/*--------------------------------------------------------------------------*/
inline Probe const* probe(Dep const& d)
{
  assert(d.probe__());
  // assert(!d.probe__()->is_reversed());
  return d.probe__();
}
/*--------------------------------------------------------------------------*/
inline bool is_flow_probe(Dep const& d)
{
  if(auto pp=dynamic_cast<Token_PROBE const*>(d.item())){
    return pp->is_flow_probe();
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
inline bool is_pot_probe(Dep const& d)
{
  if(auto pp=dynamic_cast<Token_PROBE const*>(d.item())){
    return pp->is_pot_probe();
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
inline std::string /*const&*/ code_name(Dep const& d)
{
  if(auto pp=dynamic_cast<Token_PROBE const*>(d.item())){
    return pp->code_name();
  }else{ untested();
    return d.probe__()->code_name();
  }
}
/*--------------------------------------------------------------------------*/
inline Branch const* branch(Dep const& d)
{
  assert(probe_(d));
  assert(probe_(d)->branch());
  return probe_(d)->branch();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
