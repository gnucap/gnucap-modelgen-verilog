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
  void set_never() { _block.set_never(); }
  void set_always() { _block.set_always(); }
  bool is_never()const {return _block.is_never() ;}
  bool is_reachable()const {return _block.is_reachable() ;}
  bool is_always()const {return _block.is_always() ;}
  bool update()override;
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
  void set_owner(Block* owner) { _block.set_owner_raw(owner); }
};
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
protected:
  AnalogCtrlBlock _body; // use AnalogSeqBlock::_block?
public:
  AnalogCtrlStmt() : _body() {}
  ~AnalogCtrlStmt(){ }
  void dump(std::ostream&)const override;
private:
  bool update()override {incomplete(); return false;}
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
public:
  explicit AnalogProceduralAssignment(CS&, Block*);
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  Assignment const& assignment()const {return _a;}
private:
  bool update()override;
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
  Block* _owner{NULL};
private:
  void set_pot_contrib();
  void set_flow_contrib();
  void set_direct(bool d=true);
//  Deps& deps() { return _rhs.deps(); }
  Deps const& deps() { return _rhs.deps(); }
public:
  Contribution(CS& f, Block* o)
    : AnalogStmt(), _branch(NULL) {
    set_owner(o);
    parse(f);
  }
  ~Contribution();
private:
  void set_owner(Block* owner) { _owner = owner; }
  Block* owner() {return _owner;}
public:

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
  bool update()override;
}; // Contribution
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
