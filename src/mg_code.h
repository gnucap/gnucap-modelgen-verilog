/*                       -*- C++ -*-
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
#ifndef MG_CODE_H
#define MG_CODE_H
#include "mg_base.h"
#include "mg_attrib.h"
#include "mg_expression.h"
/*--------------------------------------------------------------------------*/
class Statement : public Base {
public:
  virtual bool propagate_deps(Token const&) = 0;
};
/*--------------------------------------------------------------------------*/
class Data_Type : public Base{
protected:
  typedef enum{ t_default, t_real, t_int } type_t;
  type_t _type;
  Data_Type(type_t type) : Base(), _type(type){}
public:
  Data_Type(Data_Type const& t) : Base(), _type(t._type){}
  Data_Type() : Base(), _type(t_default){}
  Data_Type& operator=(Data_Type const& o){ _type = o._type; return *this;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  bool is_real() const{ return _type==t_real; }
  bool is_int() const{ return _type==t_int; }
};
/*--------------------------------------------------------------------------*/
class Data_Type_Real : public Data_Type{
public:
  Data_Type_Real() : Data_Type(t_real){}
};
/*--------------------------------------------------------------------------*/
class Data_Type_Int : public Data_Type{
public:
  Data_Type_Int() : Data_Type(t_int){}
};
/*--------------------------------------------------------------------------*/
class Token_VAR_REF;
/*--------------------------------------------------------------------------*/
class Token_PROBE; //bug?
class Node;
class TData;
class Expression;
class Variable : public Owned_Base {
protected:
  TData* _data{NULL};
//  String_Arg _name;
  Token_VAR_REF* _token{NULL};
public:
  explicit Variable() : Owned_Base() {}
//  Variable(std::string const& name)
//   : Owned_Base()
//   ,_name(name) {untested();
//  }
  ~Variable() {
  }
public:
  String_Arg key()const { return String_Arg(name()); }
//  void set_type(Data_Type d){ _type=d; }
  virtual Data_Type const& type() const = 0;
  bool is_real()const { return type().is_real(); }
  bool is_int()const { return type().is_int(); }
  std::string const& identifier()const {return name();}
  std::string const& name()const; //  {return name();}
  virtual std::string code_name()const;
  virtual bool is_module_variable()const;

//  virtual bool propagate_deps(Variable const&) = 0;
  virtual double eval()const { return NOT_INPUT;}
  Block const* scope() const;
  bool has_deps()const { return _data; }
  TData const& deps()const { assert(_data); return *_data; }
protected:
  TData& data() { assert(_data); return *_data; }
  void new_var_ref();
}; // Variable
/*--------------------------------------------------------------------------*/
class Variable_Decl : public Variable {
  Data_Type _type;
  Attribute_Instance const* _attributes{NULL};
public:
  Variable_Decl() : Variable() { } // {new_deps(); }
  ~Variable_Decl();
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  bool has_attributes() const{
    return _attributes;
  }
  void set_attributes(Attribute_Instance const* a) {
    _attributes = a;
  }
  Data_Type const& type()const override{
    return _type;
  }
//  std::string code_name()const override;
  void set_type(Data_Type const& d){ _type=d; }
  bool propagate_deps(Token_VAR_REF const&);
protected:
  void clear_deps();
private:
//  TData& deps() { assert(_deps); return *_deps; }
  void new_deps();
//  void set_type(std::string const& a){_type=a;}
};
/*--------------------------------------------------------------------------*/
class BlockVarIdentifier : public Variable_Decl {
public:
  explicit BlockVarIdentifier() : Variable_Decl() { }
public:
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override;
  void update();
};
/*--------------------------------------------------------------------------*/
class ListOfBlockIntIdentifiers : public LiSt<BlockVarIdentifier, '\0', ',', ';'>{
public:
  ListOfBlockIntIdentifiers(CS& f, Block* o){
    set_owner(o);
    parse(f);
    for(auto i : *this){
      i->set_type(Data_Type_Int());
    }
  }
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class ListOfBlockRealIdentifiers : public LiSt<BlockVarIdentifier, '\0', ',', ';'>{
public:
  ListOfBlockRealIdentifiers() {}
  ListOfBlockRealIdentifiers(CS& f, Block* o){
    set_owner(o);
    parse(f);
    for(auto i : *this){
      i->set_type(Data_Type_Real());
    }
  }
  void dump(std::ostream&)const override;
};
/*--------------------------------------------------------------------------*/
class Variable_List : public LiSt<Variable_Decl, '\0', ',', ';'> {
  Data_Type _type;
public:
  Data_Type const& type()const {return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
// class Assignment : public Expression_ ? (later)
class Assignment : public Variable {
protected:
  Token_VAR_REF* _lhsref{NULL};
  Expression_ _rhs;
public:
  explicit Assignment(CS& f, Block* o);
  explicit Assignment() : Variable() {}
  ~Assignment();
public:
  bool is_module_variable()const override;
  bool is_int() const;
  Data_Type const& type()const override;
//  std::string const& lhsname()const {
//    assert(_lhsref);
//    return _lhsref->name();
//  }
  Expression_ const& rhs()const {return _rhs;}
  Token_VAR_REF const& lhs() const{
    assert(_lhsref);
    return *_lhsref;
  }
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override;
  bool propagate_deps(Token_VAR_REF const&);
  bool update();
// protected:
  void set_lhs(Variable* v);

//  TData const& deps()const override;
//private: // ?
  void parse_rhs(CS& cmd);
  double eval()const override;
  Block* owner(){ return Variable::owner();}
  Block const* owner() const{ return Variable::owner();}
  bool has_sensitivities()const;
//  TData& deps()override { return _rhs.deps(); }
  Block const* scope() const;
private: // implementation
  bool store_deps(TData const&);
  std::string code_name()const;
}; // Assignment
/*--------------------------------------------------------------------------*/
// class AnalogExpression?
class ConstantMinTypMaxExpression : public Base {
  Expression_ _e;
  Block* _owner{NULL};
public:
  explicit ConstantMinTypMaxExpression() : Base(){}
  ~ConstantMinTypMaxExpression();
  void set_owner(Block* b){ _owner = b; }
public:
  Expression const& expression()const{ return _e; }
  bool empty()const;
  void parse(CS& f)override;
  void dump(std::ostream& o)const override;
  double value() const;
  void resolve();
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
