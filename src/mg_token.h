/*                                      -*- C++ -*-
 * Copyright (C) 2023, 2024 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
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
/*--------------------------------------------------------------------------*/
#ifndef MG_TOKEN_H
#define MG_TOKEN_H
#include <m_expression.h>
#include "mg_func.h" // BUG?
#include "mg_base.h"
#include <stack> // BUG
/*--------------------------------------------------------------------------*/
class FUNCTION_;
class Token_FUNCTION : public Token_SYMBOL {
private: // stuff into data?
  FUNCTION_ const* _function{nullptr};
  size_t _num_args{size_t(-1)};
public:
  explicit Token_FUNCTION(const std::string Name, FUNCTION_ const* f)
    : Token_SYMBOL(Name, ""), _function(f) { attach(); }
  ~Token_FUNCTION() { detach(); }
protected:
  explicit Token_FUNCTION(Token_FUNCTION const& P)
    : Token_SYMBOL(P.name(), ""), _function(P._function), _num_args(P._num_args) { attach(); }
  explicit Token_FUNCTION(Token_FUNCTION const& P, Base const* data)
    : Token_SYMBOL(P.name(), data), _function(P._function), _num_args(P._num_args) { attach(); }
protected:
  void stack_op(Expression* e)const override;
public:
  void set_num_args(size_t n) { _num_args = n; } // expression size?
  FUNCTION_ const* f()const { return _function; }
  FUNCTION_ const* function()const { return _function; }
  FUNCTION_ const* operator->()const { assert(_function); return _function; }
  virtual /*?*/ std::string code_name()const;
private:
  void attach();
  void detach();

  // really?
  // friend class Token_CALL;
};
/*--------------------------------------------------------------------------*/
class Token_CALL : public Token_FUNCTION {
private: // stuff into data?
  Expression const* _args{nullptr};
public:
  explicit Token_CALL(const std::string Name, FUNCTION_ const* f, Expression const* e=nullptr)
    : Token_FUNCTION(Name, f), _args(e) { }
  ~Token_CALL() {
    delete _args;
  }
  void stack_op(Expression* E)const override;
private:
  Token* clone()const override { untested();
    return new Token_CALL(*this);
  }

protected:
  explicit Token_CALL(const Token_CALL& P)
    : Token_FUNCTION(P) { }
public:
  explicit Token_CALL(const Token_FUNCTION& P, Base const* data, Expression const* e=nullptr)
    : Token_FUNCTION(P, data), _args(e) { }
public:
  void set_args(Expression* e){ assert(!_args); _args = e; } // needed?
 // size_t num_args() const;
  Expression const* args()const { return _args; }
}; // Token_CALL
/*--------------------------------------------------------------------------*/
class Port_3; // New_Port?
class Token_PORT_BRANCH : public Token_SYMBOL {
  Port_3 const* _port{nullptr}; // use _data?
public:
  Token_PORT_BRANCH(Token_PORT_BRANCH const& s)
    : Token_SYMBOL(s), _port(s._port) {}
  Token_PORT_BRANCH(Token_SYMBOL const& s, Port_3 const* p)
    : Token_SYMBOL(s), _port(p) {}

  Token_PORT_BRANCH* clone()const override { return new Token_PORT_BRANCH(*this); }

  void stack_op(Expression* E)const override{
    E->push_back(clone());
  }
  std::string code_name()const;
};
/*--------------------------------------------------------------------------*/
class Token_NODE : public Token_SYMBOL {
  Node_Ref const _node;
public:
  Token_NODE(Token_NODE const& s)
    : Token_SYMBOL(s), _node(s._node) { }
  Token_NODE(Token_SYMBOL const& s, Node_Ref p)
    : Token_SYMBOL(s), _node(p) { }

  Token_NODE* clone()const override { return new Token_NODE(*this); }

  void stack_op(Expression* E)const override;
  std::string code_name()const;
};
/*--------------------------------------------------------------------------*/
inline void Token_FUNCTION::attach()
{
  assert(_function);
  _function->inc_refs();
}
/*--------------------------------------------------------------------------*/
inline void Token_FUNCTION::detach()
{
  assert(_function);
  _function->dec_refs();
}
/*--------------------------------------------------------------------------*/
inline std::string Token_FUNCTION::code_name()const
{
  assert(f());
  if(f()->code_name() != "") {
    return f()->code_name();
  }else if(f()->label() != "") {itested();
    // incomplete(); // m_va.h, TODO
    return "/*INCOMPLETE*/ va::" + f()->label();
  }else{ untested();
    return "Token_CALL::code_name: incomplete";
  }
}
/*--------------------------------------------------------------------------*/
class TData;
/*--------------------------------------------------------------------------*/
Base* copy_deps(Base const* b);
/*--------------------------------------------------------------------------*/
static Token* copy(Token const* b)
{ untested();
  if(auto t=dynamic_cast<Token const*>(b)){ untested();
    return t->clone();
  }else if(b){ untested();
    return nullptr;
  }else{ untested();
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
class Expression_;
class Token_PARLIST_ : public Token_PARLIST {
  Expression const* _args{nullptr};
public:
  explicit Token_PARLIST_(Token_PARLIST const& p)
    : Token_PARLIST(p) { assert(!_args); }
  explicit Token_PARLIST_(Token_PARLIST_ const& p)
    : Token_PARLIST(p) { assert(!_args); }
  explicit Token_PARLIST_(const std::string Name, Base* L=nullptr)
    : Token_PARLIST(Name, L) { assert(!L); assert(!_args); }
  ~Token_PARLIST_() { delete _args; _args = nullptr; }
public:
  void stack_op(Expression* E)const override;
  Token_PARLIST_* clone()const override{
    return new Token_PARLIST_(*this);
  }
  TData* new_deps()const;
  Expression const* args()const { return _args; }
  void set_args(Expression const* e) {assert(!(_args && e)); _args = e;}
};
/*--------------------------------------------------------------------------*/
#if 1
class Token_ARRAY_ : public Token_ARRAY {
  Expression const* _args{nullptr};
public:
  explicit Token_ARRAY_(Token_ARRAY const& p)
    : Token_ARRAY(p) { assert(!args()); }
  explicit Token_ARRAY_(Token_ARRAY_ const& p)
    : Token_ARRAY(p) { assert(!args()); }
  explicit Token_ARRAY_(const std::string Name, Base* L=nullptr)
    : Token_ARRAY(Name, L) { untested(); assert(!args()); }
public:
  void stack_op(Expression* E)const override;
  Token_ARRAY_* clone()const override{
    auto t = new Token_ARRAY_(*this);
    assert(!t->args());
    return t;
  }
  Expression const* args()const { return _args; }
  size_t size()const { return _args?_args->size():0; }
  void set_args(Expression const* e) {assert(!(_args && e)); _args = e;}
};
#endif
/*--------------------------------------------------------------------------*/
class Token_UNARY_ : public Token_UNARY {
  Token const* _op{nullptr}; // stuff into data?
public:
  explicit Token_UNARY_(const std::string Name, Token const* op, Base const* d=nullptr)
    : Token_UNARY(Name, d), _op(op) {}
  explicit Token_UNARY_(Token_UNARY const& b)
    : Token_UNARY(b.name()) { } // ,( copy_deps(b.data())) { }
  Token* clone()const override { untested();
    return new Token_UNARY_(*this); // name(), copy(data()));
  }
  ~Token_UNARY_(){ delete _op; }

  void stack_op(Expression*)const override;
  void pop(){ untested(); _op = nullptr; }
  Token const* op1()const { return _op; };
};
/*--------------------------------------------------------------------------*/
class Token_BINOP_ : public Token_BINOP {
private: // stuff into data?
  Token const* _op1{nullptr};
  Token const* _op2{nullptr};
public:
  explicit Token_BINOP_(Token_BINOP const& b, Base const* d)
    : Token_BINOP(b.name(), d) { untested(); }
  explicit Token_BINOP_(std::string const& b, Base const* d)
    : Token_BINOP(b, d) { }
  explicit Token_BINOP_(std::string const& b,
      Token const* t1, Token const* t2, Base const* d)
    : Token_BINOP(b, d), _op1(t1), _op2(t2) { }
  explicit Token_BINOP_(Token_BINOP const& b)
    : Token_BINOP(b.name()) { assert(!b.data()); }
  ~Token_BINOP_(){
    delete _op1;
    delete _op2;
  }
  void pop(){
    _op1 = _op2 = nullptr;
  }
  void pop1(){
    _op1 = nullptr;
  }
  void pop2(){
    _op2 = nullptr;
  }

  Token* clone()const override { untested();
    return new Token_BINOP_(name(), copy(_op1), copy(_op2), copy_deps(data()));
  }
  Token const* op1()const { return _op1; };
  Token const* op2()const { return _op2; };

  void stack_op(Expression* E)const override;
//  TData const* op_deps(Base const* t1, Base const* t2)const;
  TData const* op_deps(Token const* d1, Token const* d2)const;
private:
  void stack_op_(Expression* E)const;
};
/*--------------------------------------------------------------------------*/
class Token_TERNARY_ : public Token_TERNARY {
  Token const* _cond{nullptr}; // stuff into data?
public:
  explicit Token_TERNARY_(std::string const&, Token const* Cond,
      Expression const* t1, Expression const* t2, Base const* d=nullptr)
    : Token_TERNARY(t1, t2, d), _cond(Cond) {}
  ~Token_TERNARY_();

  Token* clone()const override { untested();
    unreachable();
    return nullptr;
    // new Token_TERNARY_(name(), dup(true_part()), dup(false_part()), copy(data()));
  }

  void stack_op(Expression* E)const override;
//  TData const* op_deps(Base const* t1, Base const* t2)const;
  Token const* op_deps(Token const* d1, Token const* d2)const;
  Token const* cond()const {return _cond; }
private:
  void stack_op_(Expression* E)const;
};
/*--------------------------------------------------------------------------*/
class Token_SYMBOL_ : public Token_SYMBOL {
public:
  explicit Token_SYMBOL_(Token_SYMBOL const& p)
    : Token_SYMBOL(p) { untested();
  }
  ~Token_SYMBOL_(){ untested();
//    _data?
    incomplete();
  }
public:
  void stack_op(Expression* E)const override;
  Token* clone()const override{ untested();
    return new Token_SYMBOL(*this);
  }
};
/*--------------------------------------------------------------------------*/
class Probe;
class FUNCTION_;
class Token_ACCESS : public Token_SYMBOL {
  mutable /*BUG*/ FUNCTION_ const* _prb;
public:
  explicit Token_ACCESS(const std::string Name, Base const* Data, FUNCTION_ const* f=nullptr)
    : Token_SYMBOL(Name, Data), _prb(f) {}

private:
  explicit Token_ACCESS(const Token_ACCESS& P)
    : Token_SYMBOL(P.name(), copy_deps(P.data())), _prb(P._prb) {}
  Token* clone()const override {
    return new Token_ACCESS(*this);
  }

public:
  void stack_op(Expression* e)const override;
  Probe const* prb() const;
  bool is_short() const;
  bool is_reversed() const;
  std::string code_name()const;
  std::string code_name_()const;
private:
  size_t num_deps() const;
};
/*--------------------------------------------------------------------------*/
class Parameter_Base;
class Token_PAR_REF : public Token_CONSTANT {
  std::string _name;
  Parameter_Base const* _item;
public:
  explicit Token_PAR_REF(const std::string Name, Parameter_Base const* item)
    : Token_CONSTANT(nullptr, ""), _name(Name), _item(item) {}
private:
  explicit Token_PAR_REF(const Token_PAR_REF& P, Base const* d=nullptr)
    : Token_CONSTANT(d, ""), _name(P._name), _item(P._item) {}
  Token* clone()const override { untested();
    return new Token_PAR_REF(*this);
  }
public:
  void stack_op(Expression* e)const override;
public:
  std::string code_name()const { untested();
    return "_p_" + name();
  }
  Parameter_Base const* item()const {
    return _item;
  }
  Parameter_Base const* operator->() const{ return _item; }
  std::string val_string()const override {return _name;}
};
/*--------------------------------------------------------------------------*/
class Token_OUT_VAR : public Token_SYMBOL {
public:
  explicit Token_OUT_VAR(std::string Name)
    : Token_SYMBOL(Name, "") { }
public:
  void stack_op(Expression* e)const override;
private:
  explicit Token_OUT_VAR(Token_OUT_VAR const& p)
    : Token_SYMBOL(p.name(), "") { }
  Token_OUT_VAR* clone()const override{
    return new Token_OUT_VAR(*this);
  }
  // std::string code_name() const { untested(); return "_v_"+name(); }
  void dump(std::ostream& o)const override { untested();
    o << "." << name();
  }
};
/*--------------------------------------------------------------------------*/
class Data_Type;
// Token_VARIABLE?
class Token_VAR_REF : public Token_SYMBOL {
protected:
  Base* _item;
public:
  explicit Token_VAR_REF(std::string Name, Base* item)
    : Token_SYMBOL(Name, ""), _item(item) {}
  explicit Token_VAR_REF(std::string Name, Base* item, Base const* data)
    : Token_SYMBOL(Name, data), _item(item) {}
// protected: //??
  explicit Token_VAR_REF(const Token_VAR_REF& P, Base* d=nullptr)
    : Token_SYMBOL(P.name(), d), _item(P._item) {}
  explicit Token_VAR_REF() : Token_SYMBOL("","")  { untested(); unreachable(); }
  ~Token_VAR_REF() {
    trace1("~Token_VAR_REF", name());
  }
protected:
  Token_VAR_REF(Token_VAR_REF const*p, Base* owner);
public:
  Token_VAR_REF* clone()const override;
public:
  void stack_op(Expression* e)const override;
  Base const* operator->() const{ return _item; }
  virtual Data_Type const& type()const;
  bool propagate_deps(Token_VAR_REF const&);
  TData const& deps()const;
  Block const* scope() const;
  std::string code_name() const { return "_v_"+name(); }
  std::string long_code_name()const;
 // bool is_used() const;

  // LiSt
  std::string key() const { untested();unreachable();return "";}
  void set_owner(Base*){ untested();unreachable();}
  bool is_state_var()const;
private:
  size_t num_deps() const;
}; // Token_VAR_REF
/*--------------------------------------------------------------------------*/
  // TODO split into Base+DECL
class Token_ARGUMENT : public Token_VAR_REF {
public:
  Token * _var{nullptr}; // why not use VAR_REF::_item?
  direction_t _dir{a_unset};
  Data_Type const& type()const override;
public:
  explicit Token_ARGUMENT() : Token_VAR_REF("", nullptr){ untested();unreachable();}
  explicit Token_ARGUMENT(std::string Name, Base* b, Base* tdata)
    : Token_VAR_REF(Name, b, tdata) {}
  void dump(std::ostream& o)const override;
public: // LiSt
  std::string key() const { untested();unreachable();return "";}
  void set_owner(Base*){ untested();unreachable();}
  void set_dir(direction_t const& t) { _dir = t; }
  void set_input() {untested(); _dir = a_input; }
  void set_output() {untested(); _dir = a_output; }
  void set_inout() {untested(); _dir = a_inout; }
  bool is_output()const { return _dir==a_output || _dir==a_inout; }
//protected:
  Base const* item()const {
    return _item;
  }
  void stack_op(Expression* e)const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Variable_Stmt;
class Token_VAR_DECL : public Token_VAR_REF {
  // Variable_Stmt const* _owner{nullptr}; // _item?
  Base const* _default{0};
  // type //
public:
  explicit Token_VAR_DECL() : Token_VAR_REF("",nullptr,nullptr) { untested();unreachable();}
  explicit Token_VAR_DECL(std::string Name, Base* item, Base const* data)
    : Token_VAR_REF(Name, item, data) {}
  ~Token_VAR_DECL() { untested(); delete _default; }
  Data_Type const& type()const override;

//  void set_owner(Variable_Stmt* b){ untested();_item = b;}
  std::string key() const { untested();unreachable();return "";}

  void clear_deps();
  void stack_op(Expression* e)const override;

  void dump(std::ostream& o)const override;
  void set_default(Base const* x) { untested(); assert(!_default); _default=x; }
};
/*--------------------------------------------------------------------------*/
class Token_EVT : public Token_CALL {
public:
  explicit Token_EVT(const std::string Name, FUNCTION_ const* f = nullptr)
    : Token_CALL(Name, f) {}
private:
  explicit Token_EVT(const Token_EVT& P, Base const* data=nullptr)
    : Token_CALL(P, data) { untested();}
  Token* clone()const override { untested();
    return new Token_EVT(*this);
  }
  void stack_op(Expression* e)const override {
    Token_CALL::stack_op(e);
  }
};
/*--------------------------------------------------------------------------*/
class Token_FILTER : public Token_CALL {
public:
  explicit Token_FILTER(const std::string Name, FUNCTION_ const* f)
    : Token_CALL(Name, f) {}
private:
  explicit Token_FILTER(const Token_FILTER& P, Base const* data, Expression const* e = nullptr)
    : Token_CALL(P, data, e) {} // , _item(P._item) {}
  Token* clone()const override { untested(); return new Token_FILTER(*this);}

private:
  void stack_op(Expression* e)const override;
  Branch* branch() const;
#if 0
  Expression_ const* args() const{ untested();
    if(auto a=prechecked_cast<Expression_ const*>(Token_CALL::args())){ untested();
      return a;
    }else{ untested();
      assert(!Token_CALL::args());
      return nullptr;
    }
  }
#endif
};
/*--------------------------------------------------------------------------*/
class Token_PROBE : public Base {
protected:
  TData* _data;
  Branch const* _branch{nullptr};
public:
  explicit Token_PROBE(std::string const&, Branch const* b, TData* tdata)
    : _data(tdata), _branch(b) { }
private: // Base
  void parse(CS&)override {unreachable();}
  void dump(std::ostream&)const override {unreachable();}
public:
  virtual std::string code_name()const = 0;
  virtual bool is_pot_probe()const {return false;}
  virtual bool is_flow_probe()const {return false;}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
