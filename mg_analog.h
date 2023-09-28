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
};
typedef LiSt<AnalogConstExpression, '\0', ',', ':'> AnalogConstExpressionList;
/*--------------------------------------------------------------------------*/
class CaseGen : public Base{
  AnalogConstExpressionList* _cond{NULL};
  Base* _code{NULL};
public:
  CaseGen() : Base(){ unreachable(); }
  CaseGen(AnalogConstExpressionList* cond, Base* code) : Base(), _cond(cond), _code(code) {}
  ~CaseGen(){
    delete _cond;
    delete _code;
  }
  void set_owner(Block*) { unreachable(); }
  void parse(CS&)override;
  void dump(std::ostream& o)const override;
public:
  AnalogConstExpressionList const* cond_or_null()const {return _cond;}
  Base const* code_or_null()const {return _code;}
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
class AnalogCtrlStmt : public AnalogStmt {
protected:
  Base* _body{NULL};
public:
  ~AnalogCtrlStmt(){
    delete _body;
    _body = NULL;
  }
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class AnalogSwitchStmt : public AnalogCtrlStmt {
  AnalogConstExpression _cond; // Const??
  AnalogCaseList _cases;
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
  void set_owner(Block* o) {_cond.set_owner(o);}
  Block* owner() {return _cond.owner();}
  AnalogConstExpression const& conditional() const{return _cond;}
  AnalogCaseList const& cases()const {return _cases;}
};
/*--------------------------------------------------------------------------*/
class AnalogSeqBlock : public AnalogStmt {
  String_Arg _identifier;
  SeqBlock _block;
public:
  explicit AnalogSeqBlock(CS& cmd, Block* owner){
    set_owner(owner);
    parse(cmd);
  }
  void parse(CS& cmd)override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* owner) {_block.set_owner(owner);}

  String_Arg const& identifier() const{ return _identifier; }
  SeqBlock const& block() const{ return _block; }
};
/*--------------------------------------------------------------------------*/
class AnalogConditionalStmt : public AnalogCtrlStmt {
  AnalogConstExpression _cond; // Const??
  Base* _false_part{NULL};
public:
  AnalogConditionalStmt(Block* o, CS& file) {
    set_owner(o);
    parse(file);
  }
  ~AnalogConditionalStmt(){
    delete _false_part;
    _false_part = NULL;
  }
public:
  void parse(CS& file) override;
  void dump(std::ostream& o)const override;
  void set_owner(Block* o) {_cond.set_owner(o);}
  Block* owner() {return _cond.owner();}
  AnalogConstExpression const& conditional() const{return _cond;}
  const Base* true_part_or_null() const{ return _body; }
  const Base* false_part_or_null() const{ return _false_part; }
  const Base& true_part() const{assert(_body); return *_body; }
  const Base& false_part() const{assert(_false_part); return *_false_part; }
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
  const Base& body() const{assert(_body); return *_body; }
  virtual bool has_tail() const{ return false; }
  virtual Base const& tail() const{ return _cond; }
};
/*--------------------------------------------------------------------------*/
class AnalogForStmt : public AnalogWhileStmt {
  Base const* _init{NULL};
  Base const* _tail{NULL};
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
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
