/*                             -*- C++ -*-
 * Copyright (C) 2001, 2023 Albert Davis
 *               2023, 2024, 2025 Felix Salfelder
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
#ifndef GNUCAP_MG_DIGITAL_H
#define GNUCAP_MG_DIGITAL_H
/*--------------------------------------------------------------------------*/
#include "mg_code.h"
#include "mg_assign.h"
#include "mg_.h"
/*--------------------------------------------------------------------------*/
class VariableAssignment : Assignment {
};
/*--------------------------------------------------------------------------*/
class ProcContAssign : public Statement {
  VariableAssignment _assign;
public:
  ProcContAssign(){
  }
  ~ProcContAssign(){
  }

private:
  void submit_variable_access(Variable_Access&)const override { untested();
    incomplete();
  }

public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  bool update()override { untested(); incomplete(); return false; }
};
/*--------------------------------------------------------------------------*/
class DigitalStmt : public Statement {
public:
  ~DigitalStmt();

  virtual TData const& deps()const = 0;
//  Statement* parent_stmt()override { untested();
//    incomplete();
//    return nullptr;
//  }
private:
  void submit_variable_access(Variable_Access&)const override{ untested();incomplete();}
};
/*--------------------------------------------------------------------------*/
class DigitalSeqBlock : public SeqBlock {
protected: // BUG?
  TData _deps;
protected:
public:
  explicit DigitalSeqBlock() : SeqBlock((Base*)nullptr) {}
  explicit DigitalSeqBlock(CS& cmd, Statement* owner) : SeqBlock(owner) { untested();
    set_owner(owner);
    parse(cmd);
  }
public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  SeqBlock const& block()const { untested();
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
}; // DigitalSeqBlock
/*--------------------------------------------------------------------------*/
class DigitalSeqStmt : public DigitalStmt {
  DigitalSeqBlock _block; // SeqBlock?
public:
  explicit DigitalSeqStmt() : DigitalStmt() { untested();}
  explicit DigitalSeqStmt(CS& cmd, Block* owner) : DigitalStmt() { untested();
    set_owner(owner);
    parse(cmd);
  }
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override { untested();
    _block.dump(o);
  }
  bool update()override { untested(); return _block.update(); }
  DigitalSeqBlock const& block()const { untested(); return _block; }
  TData const& deps()const override { untested(); return _block.deps(); }
};
/*--------------------------------------------------------------------------*/
class DigitalCtrlBlock : public DigitalSeqBlock {
public:
  explicit DigitalCtrlBlock() : DigitalSeqBlock() {}
  explicit DigitalCtrlBlock(CS& f, Statement* o) : DigitalSeqBlock() {
    set_owner(o);
    parse(f);
  }

  void parse(CS& cmd)override {
    return DigitalSeqBlock::parse(cmd);
  }
  void dump(std::ostream& o)const override {
    return DigitalSeqBlock::dump(o);
  }
  operator bool()const{ return size() || identifier() !=""; }
  void set_owner(Statement* owner);
}; // DigitalCtrlBlock
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// just CtrlStmt?
class DigitalCtrlStmt : public DigitalStmt {
  TData _deps; // here?
  RDeps _rdeps;
protected:
  DigitalCtrlBlock _body;
public:
  DigitalCtrlStmt() : _body() { }
  ~DigitalCtrlStmt(){ }
  void dump(std::ostream&)const override;
  void parse(CS& cmd)override;
  DigitalCtrlBlock const& body()const { untested(); return _body; }
private:
  TData const& deps()const override { return _deps;}; // ?
  RDeps const& rdeps()const override { untested(); return _rdeps;};
//  void submit_variable_access(Variable_Access&)const override;
protected:
  bool update()override { untested();
    bool ret = _body.update();
    return DigitalStmt::update() || ret;
  }
public:
  bool propagate_rdeps(RDeps const& incoming)override;
}; // DigitalCtrlStmt
/*--------------------------------------------------------------------------*/
class AlwaysConstruct : public Statement /* CtrlStatement? */ {
  DigitalCtrlBlock _block;
public:
  AlwaysConstruct(){ }
  ~AlwaysConstruct(){ }
private:
  void submit_variable_access(Variable_Access&)const override {
    incomplete();
  }

public:
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  bool update()override { untested(); incomplete(); return false; }
  void new_block();
  Block const* block_or_null() const{ return &_block; }
  Block* block(){ return &_block; }
  void push_back(Statement*);
  bool is_used_in(Base const*)const override; // BUG?
};
typedef Collection<AlwaysConstruct> AlwaysList; // needed?
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DigitalEvtExpression : public Expression_ {
  RDeps _rdeps;
public:
  explicit DigitalEvtExpression() : Expression_() {}
  ~DigitalEvtExpression(){ }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
//  Expression const& expression() const{return _expression;};
  bool is_used_in(Base const*)const;
 // bool update(RDeps const* r) // not virtual
//
private: // incomplete
  void set_rdeps(); // pull in rdeps from functions
  bool add_rdep(Base const* b) {
    return _rdeps.insert(b).second;
  }
  FUNCTION_ const* function()const;
public:
  RDeps const& rdeps()const { untested(); return _rdeps; }
};
/*--------------------------------------------------------------------------*/
class DigitalEvtCtlStmt : public DigitalCtrlStmt {
  DigitalEvtExpression _ctrl;
public:
  ~DigitalEvtCtlStmt() { }
  void parse(CS&)override;
  void dump(std::ostream&)const override;
  Expression_ const& cond()const { return _ctrl; } // override?
  const DigitalCtrlBlock& code()const { untested(); return _body; }
  bool update()override;

  bool is_used_in(Base const* b)const override;
  // TODO bool propagate_rdep(Base const*) override; // block unused tags.
}; // DigitalEvtCtlStmt
typedef Collection<DigitalEvtCtlStmt> Digital_Events;
/*--------------------------------------------------------------------------*/
class Always : public Owned_Base {
  AlwaysList _list;
  Probe_Map* _probes{nullptr};
  Digital_Events _events;
public:
  explicit Always();
  ~Always();
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;

  bool has_block() const;
  AlwaysList const& list()const { untested(); return _list; }
  AlwaysList const& blocks()const { return _list; }
  Digital_Events const& events()const { untested(); return _events; }
  void push_back(Base*);
};
/*--------------------------------------------------------------------------*/
inline Always const& always(Module const& m)
{ untested();
  Always const* a = prechecked_cast<Always const*>(&m.always());
  assert(a);
  return *a;
}
/*--------------------------------------------------------------------------*/
inline AlwaysList const& always_list(Module const& m)
{ untested();
  return always(m).list();
}
/*--------------------------------------------------------------------------*/
class DigitalExpression : public Expression_ {
public:
  explicit DigitalExpression() : Expression_() { untested();}
  ~DigitalExpression();
  void parse(CS& file) override;
//  void dump(std::ostream& o)const override;
//  Block* owner() { untested();return Owned_Base::owner();}
  Expression const& expression() const{ untested(); return *this;}
  bool is_true() const;
  bool is_false() const;
//  bool has_sensitivities()const { untested(); return !_sens.empty(); }
};
/*--------------------------------------------------------------------------*/
class DigitalConstExpression : public DigitalExpression {
public:
//  void parse(CS& file) override;
//  void dump(std::ostream& o)const override;
  String_Arg key() const{ untested();return String_Arg("ACE");}
};
typedef LiSt<DigitalConstExpression, '\0', ',', ':'> DigitalConstExpressionList;
/*--------------------------------------------------------------------------*/
#if 0
class CaseGen : public DigitalCtrlStmt {
  DigitalConstExpressionList* _cond{nullptr};
  RDeps _rdeps; // TODO;
private:
  CaseGen() : AnalogCtrlStmt(){ untested(); unreachable(); }
public:
  CaseGen(CS&, Block*, Expression const&, bool, bool);
  ~CaseGen(){ untested();
    delete _cond;
  }
  void parse(CS&)override;
  void dump(std::ostream& o)const override;
public:
  DigitalConstExpressionList const* cond_or_null()const { untested();return _cond;}
  bool is_default()const { untested();return !_cond;}
  void set_never() { untested();return _body.set_never() ;}
  void set_always() { untested();return _body.set_always() ;}
  bool is_never()const { untested();return _body.is_never() ;}
  bool is_reachable()const { untested();return _body.is_reachable() ;}
  bool is_always()const { untested();return _body.is_always() ;}
  void calc_reach(Expression const& cond);

  String_Arg key() const{ untested();return String_Arg("CaseGen");}
  RDeps const& rdeps()const override { untested(); return _rdeps; }
 // bool update() override { untested(); incomplete(); }
};
#endif
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class InitialStmt : public DigitalCtrlStmt {
public:
  explicit InitialStmt(Block* o, CS& file) { untested();
    set_owner(o);
    parse(file);
  }
  ~InitialStmt(){ untested(); }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  bool is_used_in(Base const*)const override; // needed?
  bool update()override;
}; // InitialStmt
/*--------------------------------------------------------------------------*/
#if 0 // later, code?
class DigitalSwitchStmt : public DigitalStmt { // CtrlStmt?
  TData _deps; // here?
  DigitalConstExpression _ctrl; // Const??
  SeqBlock _body;
  RDeps _rdeps; // here?
public:
  DigitalSwitchStmt(Block* o, CS& file) { untested();
    set_owner(o);
    parse(file);
  }
  ~DigitalSwitchStmt() { untested(); }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  DigitalConstExpression const& control() const{ untested();return _ctrl;}
  SeqBlock const& cases()const { untested();return _body;}
  bool update()override;
  TData const& deps()const override { return _deps;};
 // TData const& data()const override {return _deps;}; // ctrl?
  RDeps const& rdeps()const override{ untested(); return _rdeps; }
}; // DigitalSwitchStmt
#endif
/*--------------------------------------------------------------------------*/
class DigitalConditionalStmt : public DigitalCtrlStmt {
  DigitalConstExpression _cond; // Const?
  DigitalCtrlBlock _false_part;
public:
  explicit DigitalConditionalStmt(Block* o, CS& file) { untested();
    set_owner(o);
    parse(file);
  }
/*--------------------------------------------------------------------------*/
  ~DigitalConditionalStmt(){ untested(); }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  DigitalConstExpression const& conditional() const{ untested();return _cond;}
  const DigitalCtrlBlock& true_part() const{ untested(); return _body; }
  const DigitalCtrlBlock& false_part() const{ untested(); return _false_part; }
  bool is_used_in(Base const*)const override;
  bool update()override { untested();
    bool ret = _false_part.update();
    return DigitalCtrlStmt::update() || ret;
  }

  TData const& deps()const override{ return _cond.data(); } // ?
}; // DigitalConditionalStmt
/*--------------------------------------------------------------------------*/
class DigitalWhileStmt : public DigitalCtrlStmt {
protected:
  DigitalExpression _cond; // -> DigitalCtrlStmt?
  explicit DigitalWhileStmt() { untested(); }
public:
  explicit DigitalWhileStmt(CS& file, Block* o);
  ~DigitalWhileStmt() { untested(); }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  DigitalExpression const& conditional()const { untested();return _cond;}
  bool has_body() const{ untested(); return _body; }
  const Base& body() const{ untested();assert(_body); return _body; }
  virtual bool has_tail() const{ untested(); return false; }
  virtual Base const& tail() const{ untested(); return _cond; }
private:
  bool update()override;
};
/*--------------------------------------------------------------------------*/
class DigitalForStmt : public DigitalWhileStmt {
  Assignment* _init{nullptr};
  Assignment* _tail{nullptr};
public:
  explicit DigitalForStmt(CS& file, Block* o) : DigitalWhileStmt() { untested();
    set_owner(o);
    parse(file);
  }

  void parse(CS& file) override;
  void dump(std::ostream& o)const override;

  bool has_init()const{ untested(); return _init; }
  Assignment const& init()const{ untested(); assert(_init); return *_init; }

  bool has_tail()const override{ untested(); return _tail; }
  Assignment const& tail()const override{ untested(); assert(_tail); return *_tail; }
private:
  bool update()override;
};
/*--------------------------------------------------------------------------*/
// just AssignStatement?
class DigitalProceduralAssignment : public DigitalStmt {
  Assignment _a;
  TData _deps;
 // RDeps _rdeps;
public:
  // explicit DigitalProceduralAssignment(Block* o) { untested();
  //   set_owner(o);
  //   _a.set_owner(o);
  // }
  explicit DigitalProceduralAssignment(CS&, Block*);
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  Assignment const& expression()const { untested();return _a;}
  bool is_used_in(Base const*b)const override;
  Statement* deep_copy(Base* no)const override;
  bool propagate_rdeps(RDeps const& r)override;
private:
  bool update()override;
  TData const& deps()const override {return _deps;};
public: // dump_annotate
  TData const& data()const {return _a.data();};
  bool has_sensitivities()const { untested();
    return _a.has_sensitivities();
  }
  Sensitivities const& sensitivities()const { untested();return _a.sensitivities();}
  bool is_state_var()const {return _a.is_state_var();}
}; // DigitalProceduralAssignment
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
