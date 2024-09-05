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
#include "mg_deps.h" // BUG
#include "mg_attrib.h"
#include "mg_expression.h"
/*--------------------------------------------------------------------------*/
class Variable_Stmt;
class Variable_List_Collection : public Collection<Variable_Stmt>{
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Statement : public Owned_Base {
  RDeps _rdeps;
protected:
  explicit Statement() : Owned_Base() {}
public:
  virtual Statement* deep_copy(Base*)const // = 0;?
    { untested();unreachable();return NULL;}
  virtual bool propagate_rdeps(RDeps const&);
  bool propagate_rdep(Base const*);
  virtual bool update() = 0;
//  virtual Statement* parent_stmt();
  // Block* scope() { return Owned_Base::owner(); }
  // Block const* scope() const { return Owned_Base::owner(); }

  virtual bool is_used_in(Base const*)const;
  virtual Base* owner_() {
    if(scope()){
      return scope()->owner();
    }else{ untested();
      return NULL;
    }
  }

public:
  virtual RDeps const& rdeps()const { return _rdeps; }
private:
  void set_rdeps(TData const&);
//protected:
//  void store_rdeps(Rdeps const&);
public:
  bool set_used_in(Base const*b);
  void unset_used_in(Base const*b){} // later.
  RDeps const& rdeps_()const {return _rdeps;} // dump_annotate
protected: // dbg.
  int rdeps_size()const { return int(_rdeps.size()); }
 // void set_rdeps(TData const&);
public:
  bool is_reachable()const;
  bool is_always()const;
  bool is_never()const;
};
/*--------------------------------------------------------------------------*/
class Data_Type : public Base{
protected:
  typedef enum{ t_default, t_real, t_int } type_t;
  type_t _type;
  Data_Type(type_t type) : Base(), _type(type){}
public:
  Data_Type(Data_Type const& t) : Base(), _type(t._type){ untested();}
  Data_Type() : Base(), _type(t_default){}
  Data_Type& operator=(Data_Type const& o){ _type = o._type; return *this;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  bool is_real() const{ return _type==t_real; }
  bool is_int() const{ return _type==t_int; }
  operator bool() const {return _type!=t_default;}
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
class Variable_Decl : public Expression_ {
  Data_Type _type;
  TData* _data{NULL};
  Token_VAR_REF* _token{NULL};
  RDeps _rdeps; // Expression_?
  std::string /*TODO*/ _dimensions;
public:
  explicit Variable_Decl() : Expression_() { }
  ~Variable_Decl();
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  virtual /*?*/ Data_Type const& type()const { return _type; }
//  std::string code_name()const override;
  void set_type(Data_Type const& d){ _type=d; }
  bool propagate_deps(Token_VAR_REF const&);
  bool propagate_rdeps(RDeps const&);
  bool is_state_variable()const;
protected:
  void clear_deps();
private:
  void new_deps();
  void new_data();
public:
  String_Arg key()const { return String_Arg(name()); }
//  void set_type(Data_Type d){ untested(); _type=d; }
  bool is_real()const { untested(); return type().is_real(); }
  bool is_int()const { untested(); return type().is_int(); }
  std::string const& identifier()const { untested();return name();}
  std::string const& name()const; //  { untested();return name();}
  virtual std::string code_name()const;

//  virtual bool propagate_deps(Variable const&) = 0;
  virtual double eval()const { untested(); return NOT_INPUT;}
  Block const* scope() const;
  // bool has_deps()const { untested(); return _data; }
  TData const& deps()const { assert(_data); return *_data; }
 // bool is_used_in(Base const*b)const;
  Variable_Decl* deep_copy(Base* owner, std::string prefix="") const;
  Token_VAR_REF const& token()const { assert(_token); return *_token; }
  void update();
protected:
  TData& data() { assert(_data); return *_data; }
  void new_var_ref();
  void new_var_ref_();
}; // Variable_Decl
/*--------------------------------------------------------------------------*/
class Variable_Stmt : public Statement {
  typedef LiSt<Variable_Decl, '\0', ',', ';'> list_;
  typedef list_::const_iterator const_iterator;
  list_ _l;
  Data_Type _type;
public:
  explicit Variable_Stmt() : Statement() {}
  explicit Variable_Stmt(CS& f, Base* o) : Statement() {
    set_owner(o);
    parse(f);
  }
public:
  Data_Type const& type()const {return _type;}
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  Variable_Stmt* deep_copy_(Block* owner, std::string prefix="") const;
//  void set_owner(Block* b){ untested(); Statement::set_owner(b); }
  const_iterator begin()const { return _l.begin(); }
  const_iterator end()const { return _l.end(); }
  Variable_Stmt* deep_copy(Base*)const override
    { untested();unreachable();return NULL;}
  bool update() override;
  bool is_used_in(Base const* b)const override;
  RDeps const& rdeps()const override{
    static RDeps r; return r;
  }
};
/*--------------------------------------------------------------------------*/
class Assignment : public Expression_ {
protected:
  TData* _data{NULL};
  Token_VAR_REF* _token{NULL};
protected:
  Token_VAR_REF* _lhsref{NULL};
public:
  explicit Assignment(CS& f, Base* o);
  explicit Assignment() : Expression_() {}
  ~Assignment();
public:
  //bool has_deps()const { untested(); return _data; }
  TData const& data()const;
  bool is_int() const;
  Data_Type const& type()const;
  Expression_ const& rhs()const {return *this;}
  Token_VAR_REF const& lhs() const{
    assert(_lhsref);
    return *_lhsref;
  }
  void parse(CS& cmd) override;
  void dump(std::ostream&)const override;
  bool propagate_deps(Token_VAR_REF const&);
 // bool update();
  bool update(RDeps const* r=NULL);

  void parse_rhs(CS& cmd);
  RDeps const& rdeps() const { static RDeps r; return r; } // dump_annotate.
  Sensitivities const& sensitivities()const;
  bool has_sensitivities()const;
//  Block const* scope() const;
  bool is_used_in(Base const*b)const;
  bool is_used()const;
  operator bool() const {return _token;}
  bool is_output_var()const;
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
class Sensitivities;
class SeqBlock : public Block {
  Sensitivities* _sens{NULL}; // here?
protected: // AF
  Variable_List_Collection _variables;
public:
  explicit SeqBlock() : Block() {}
  ~SeqBlock();
  void parse(CS&)override{ untested();incomplete();}
//  void dump(std::ostream& o)const override;
  void parse_identifier(CS& f);
  bool has_identifier()const {return _identifier != "";}

  Branch_Ref new_branch(std::string const& p, std::string const& n)override {
    assert(owner());
    return scope()->new_branch(p, n);
  }
  Branch_Ref new_branch(Node* p, Node* n)override { untested();
    assert(owner());
    return scope()->new_branch(p, n);
  }
  Node_Ref node(std::string const& n)const override {
    assert(owner());
    return scope()->node(n);
  }
  Branch_Ref lookup_branch(std::string const& n)const override {
    assert(owner());
    return scope()->lookup_branch(n);
  }
  String_Arg const& identifier() const{ return _identifier; }
  bool has_sensitivities()const {return _sens;}
  Sensitivities const* sensitivities()const {return _sens;}
  void set_sens(Base* s);
  void merge_sens(Sensitivities const& s);
  map const& variables()const {return _var_refs;}
  Variable_List_Collection const& variables_()const {return _variables;}
  bool update();
public:
  bool propagate_rdeps(RDeps const&);
}; // SeqBlock
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
