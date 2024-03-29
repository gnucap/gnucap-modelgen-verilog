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
#ifndef GNUCAP_MG_ANALOG_H
#define GNUCAP_MG_ANALOG_H
/*--------------------------------------------------------------------------*/
#include "mg_.h"
#include "mg_code.h"
/*--------------------------------------------------------------------------*/
// class access_function?
class Probe : public FUNCTION_ {
  Branch_Ref _br;
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

  bool operator==(Probe const& o) const{
    return _br == o._br && _type == o._type;
  }
  bool same_data(Probe const& o) const{
    return branch() == o.branch() && _type == o._type;
  }
  bool operator!=(Probe const& o) const{
    return !operator==(o);
  }
private:
  std::string eval(CS&, const CARD_LIST*)const override {unreachable(); return "";}
  Token* new_token(Module&, size_t)const override;
public:
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
class Analog_Function_Arg : public Variable_Decl {
public:
  explicit Analog_Function_Arg() : Variable_Decl() {}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  std::string code_name()const override;

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
class Analog_Function : public Block {
  Analog_Function_Arg _variable;
  Data_Type _type;
  AF_Arg_List_Collection _args;
  FUNCTION_ const* _function{NULL};
public:
  ~Analog_Function();
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  std::string const& identifier()const { return _variable.identifier(); }
  std::string code_name()const { return "af_" + identifier(); }
public:
  Branch_Ref new_branch(std::string const&, std::string const&)override { untested();
    return Branch_Ref();
  }
  Branch_Ref new_branch(Node*, Node*)override {
    return Branch_Ref();
  }
  Node_Ref node(std::string const&)const override {
    return Node_Ref();
  }
  Branch_Ref lookup_branch(std::string const&)const override {
    return Branch_Ref();
  }
  FUNCTION_ const* function() const{return _function;}
  AF_Arg_List_Collection const& args() const{ return _args; }
  Analog_Function_Arg const& variable() const{ return _variable; }
  void new_var_ref(Base* what)override;
};
typedef Collection<Analog_Function> Analog_Functions;
/*--------------------------------------------------------------------------*/
class Analog : public Owned_Base {
  AnalogList _list;
  Analog_Functions _functions;
public:
  explicit Analog();
  ~Analog();
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;

  bool has_block() const;
  AnalogList const& list()const { return _list; }
  AnalogList const& blocks()const { return _list; }
  Analog_Functions const& functions()const { return _functions; }
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
class AnalogStmt : public Base {
  std::vector<Base const*> _used_in;
public:
  virtual bool update() = 0;
  void set_used_in(Base const*b); // const;
  void unset_used_in(Base const*b); // const;
  ~AnalogStmt();
//  bool used_in(Base const*)const;
  bool is_used_in(Base const*)const;

  virtual TData const& deps() = 0;
};
/*--------------------------------------------------------------------------*/
class AnalogExpression : public Owned_Base {
protected:
  Expression_ _exp;
public:
  ~AnalogExpression();
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  Block* owner() {return Owned_Base::owner();}
  Expression const& expression() const{ return _exp;}
  bool is_true() const;
  bool is_false() const;
};
/*--------------------------------------------------------------------------*/
class AnalogConstExpression : public AnalogExpression {
public:
//  void parse(CS& file) override;
//  void dump(std::ostream& o)const override;
  String_Arg key() const{return String_Arg("ACE");}
};
typedef LiSt<AnalogConstExpression, '\0', ',', ':'> AnalogConstExpressionList;
/*--------------------------------------------------------------------------*/
// AnalogConstruct but Stmt..?
class AnalogSeqBlock : public AnalogStmt {
protected: // BUG?
  SeqBlock _block;
  TData _deps;
public:
  explicit AnalogSeqBlock() : AnalogStmt() {}
  explicit AnalogSeqBlock(CS& cmd, Block* owner) : AnalogStmt() {
    set_owner(owner);
    parse(cmd);
  }
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* owner) {
    _block.set_owner(owner);
  }

  String_Arg const& identifier()const {
    return _block.identifier();
  }
  SeqBlock const& block()const {
    return _block;
  }
  size_t size()const {
    return _block.size();
  }
public: // sensitivities?
  void set_never() { _block.set_never(); }
  void set_always() { _block.set_always(); }
  bool is_never()const {return _block.is_never() ;}
  bool is_reachable()const {return _block.is_reachable() ;}
  bool is_always()const {return _block.is_always() ;}
  bool update()override;
  void set_sens(Base const* s) {_block.set_sens(s);}
  void clear_vars();

  TData const& deps(){ return _deps;};
};
/*--------------------------------------------------------------------------*/
class AnalogCtrlBlock : public AnalogSeqBlock {
public:
  explicit AnalogCtrlBlock() : AnalogSeqBlock() {}
  explicit AnalogCtrlBlock(CS& f, Block* o) : AnalogSeqBlock() {
    set_owner(o);
    parse(f);
  }

  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  operator bool()const{ return size() || identifier() !=""; }
  void set_owner(Block* owner);
}; // AnalogCtrlBlock
/*--------------------------------------------------------------------------*/
class CaseGen : public Base{
  AnalogConstExpressionList* _cond{NULL};
  AnalogCtrlBlock _code;
  // Expression const* _ctrl{NULL};
public:
  CaseGen() : Base(){ unreachable(); }
  CaseGen(CS&, Block*, Expression const&, bool, bool);
  ~CaseGen(){
    delete _cond;
  }
  void set_owner(Block*) { unreachable(); }
  void parse(CS&)override;
  void dump(std::ostream& o)const override;
public:
  AnalogConstExpressionList const* cond_or_null()const {return _cond;}
  Base const* code_or_null()const {return &_code;}
  bool is_default()const {return !_cond;}
  void set_never() {return _code.set_never() ;}
  void set_always() {return _code.set_always() ;}
  bool is_never()const {return _code.is_never() ;}
  bool is_reachable()const {return _code.is_reachable() ;}
  bool is_always()const {return _code.is_always() ;}
  void calc_reach(Expression const& cond);

  String_Arg key() const{return String_Arg("CaseGen");}
};
typedef LiSt<CaseGen, '\0', '\0', '\0'> AnalogCaseList;
/*--------------------------------------------------------------------------*/
class System_Task : public Owned_Base {
  AnalogExpression _e;
public:
  System_Task(CS&, Block*);
  void parse(CS& o) override;
  void dump(std::ostream&o)const override;

  AnalogExpression const& expression() const{ return _e; }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class AnalogCtrlStmt : public AnalogStmt /* : AnalogSeqBlock? */ {
  TData _deps; // here?
protected:
  AnalogCtrlBlock _body; // use AnalogSeqBlock::_block?
public:
  AnalogCtrlStmt() : _body() {}
  ~AnalogCtrlStmt(){ }
  void dump(std::ostream&)const override;
private:
  bool update()override {return _body.update();}
  TData const& deps(){ return _deps;};
};
/*--------------------------------------------------------------------------*/
class AnalogEvtCtlStmt : public AnalogCtrlStmt {
  AnalogEvtExpression _ctrl;
public:
  ~AnalogEvtCtlStmt() { }
  void set_owner(Block* o) {_ctrl.set_owner(o);}
//  Block const* owner()const {return _ctrl.owner();}
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression const& cond()const{ return _ctrl.expression(); }
  const AnalogCtrlBlock& code() const{ return _body; }
protected:
  Block* owner() {return _ctrl.owner();}
};
/*--------------------------------------------------------------------------*/
class AnalogSwitchStmt : public AnalogCtrlStmt {
  AnalogConstExpression _ctrl; // Const??
  AnalogCaseList _cases; // use _body?
public:
  AnalogSwitchStmt(Block* o, CS& file) {
    set_owner(o);
    parse(file);
  }
  ~AnalogSwitchStmt(){
  }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* o) {_ctrl.set_owner(o);}
  Block* owner() {return _ctrl.owner();}
  AnalogConstExpression const& control() const{return _ctrl;}
  AnalogCaseList const& cases()const {return _cases;}
};
/*--------------------------------------------------------------------------*/
class AnalogConditionalStmt : public AnalogCtrlStmt {
  AnalogConstExpression _cond; // Const??
  AnalogCtrlBlock _false_part;
public:
  AnalogConditionalStmt(Block* o, CS& file) {
    set_owner(o);
    parse(file);
  }
  ~AnalogConditionalStmt(){ }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* o) {_cond.set_owner(o);}
  Block* owner() {return _cond.owner();}
  AnalogConstExpression const& conditional() const{return _cond;}
  const AnalogCtrlBlock& true_part() const{ return _body; }
  const AnalogCtrlBlock& false_part() const{ return _false_part; }
};
/*--------------------------------------------------------------------------*/
class AnalogWhileStmt : public AnalogCtrlStmt {
protected:
  AnalogExpression _cond;
  AnalogWhileStmt(){
  }
public:
  AnalogWhileStmt(CS& file, Block* o) {
    set_owner(o);
    parse(file);
  }
  ~AnalogWhileStmt(){
  }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* o) {_cond.set_owner(o);}
  Block* owner() {return _cond.owner();}
  AnalogExpression const& conditional()const {return _cond;}
  bool has_body() const{ return _body; }
  const Base& body() const{assert(_body); return _body; }
  virtual bool has_tail() const{ return false; }
  virtual Base const& tail() const{ return _cond; }
private:
  bool update()override;
};
/*--------------------------------------------------------------------------*/
class AnalogForStmt : public AnalogWhileStmt {
  Base* _init{NULL};
  Base* _tail{NULL};
public:
  AnalogForStmt(CS& file, Block* o) : AnalogWhileStmt() {
    set_owner(o);
    parse(file);
  }

  void parse(CS& file) override;
  void dump(std::ostream& o)const override;

  bool has_init()const{ return _init; }
  Base const& init()const{ assert(_init); return *_init; }

  bool has_tail()const override{ return _tail; }
  Base const& tail()const override{ assert(_tail); return *_tail; }
private:
  bool update()override;
};
/*--------------------------------------------------------------------------*/
class AnalogProceduralAssignment : public AnalogStmt {
  Assignment _a;
  TData _deps;
public:
  explicit AnalogProceduralAssignment(CS&, Block*);
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  Assignment const& assignment()const {return _a;}
private:
  bool update()override;
  TData const& deps()override {return _deps;};
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
  Block* _owner{NULL};
  TData* _deps{NULL};
  Sensitivities _sens;
private:
  void set_pot_contrib();
  void set_flow_contrib();
  void set_short();
  void set_always_pot();
  void set_direct(bool d=true);
//  TData& deps() { return _rhs.deps(); }
  TData const& deps();
public:
  Contribution(CS& f, Block* o)
    : AnalogStmt(), _branch(NULL, false) {
    set_owner(o);
    parse(f);
  }
  ~Contribution();
private:
  void set_owner(Block* owner) { _owner = owner; }
  Block* owner() {return _owner;}
  Block const* owner()const {return _owner;}
public:

  DDeps const& ddeps() const;
  bool has_sensitivities()const { return !_sens.empty(); }
  bool is_pot_contrib() const;
  bool is_flow_contrib() const;
  bool is_short() const { return _short; }
  bool is_direct() const;
  bool is_always() const{ assert(owner()); return owner()->is_always(); }
  bool is_reachable()const {return owner()->is_reachable() ;}
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  TData const& deps() const {
    if(_deps) {return *_deps;} else {return _rhs.deps();}
  }
  bool has_deps()const {return true;}
  Expression const& rhs()const {return _rhs;}
  std::string const& name() const{return _name;}
  Branch_Ref const& branch_ref() const{return _branch;}
  Branch const* branch() const{return _branch;}
  bool reversed() const{ return _branch.is_reversed() ;}
  Branch_Ref new_branch(std::string const& p, std::string const& n) {
    assert(owner());
    return owner()->new_branch(p, n);
  }
  Branch_Ref new_branch(Node* p, Node* n) {
    assert(owner());
    return owner()->new_branch(p, n);
  }
private:
  Expression& rhs() {return _rhs;}
  bool update()override;
  void add_dep(Dep const&);
}; // Contribution
/*--------------------------------------------------------------------------*/
// AnalogDecl?
class AnalogRealDecl : public AnalogStmt {
  ListOfBlockRealIdentifiers _l;
  TData _deps;
public:
  explicit AnalogRealDecl(CS& f, Block* o){
    _l.set_owner(o);
    parse(f);

    for(auto i : _l){
      i->set_type(Data_Type_Real());
    }
  }
  void parse(CS& f)override {
    f >> _l;
  }
  void dump(std::ostream& o)const override {
    _l.dump(o);
  }
  ListOfBlockRealIdentifiers const& list() const{
    return _l;
  }

private:
  bool update()override;
  TData const& deps(){ return _deps;};
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
