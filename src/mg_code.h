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
#include "mg_type.h"
/*--------------------------------------------------------------------------*/
template<class A>
void dump_annotate(std::ostream& o, A const& a)
{
  if(!a.scope()->is_reachable()){
    o << " // --\n";
  }else{
    if(a.data().ddeps().size()){
      o << " //";
    }else if(a.has_sensitivities()){
      o << " //";
    }else if(a.rdeps_().size()){
      o << " //";
    }else if(a.data().is_constant()){
      o << " // c";
    }else{ untested();
    }
    if(a.data().ddeps().size()){
      o << " [";
      std::string sep;
      for(const Dep& d : a.data().ddeps()) {
	o << sep << d.name();
	sep = ", ";
      }
      o << "]";
    }else{
    }
#if 1
    if(a.has_sensitivities()){
      o << " s" << a.sensitivities().size();
    }else{
    }
#endif
    if(a.rdeps_().size()){
      o << " r" << a.rdeps_().size() << "{";
      std::set<char> ordered_tags;
      for(Base const* r : a.rdeps_()){
	std::string s = r->val_string();
	if(s.size() == 1){ // HACK
	  ordered_tags.insert(s[0]);
	}else{
	}
      }
      for(char c : ordered_tags){
	o << c;
      }
      o << "}";
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
class Variable_Stmt;
class Variable_List_Collection : public Collection<Variable_Stmt>{
public:
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
};
/*--------------------------------------------------------------------------*/
class Variable_Access;
class Statement : public Owned_Base {
  RDeps _rdeps;
protected:
public: //BUG, mg_base.h: 315
  explicit Statement() : Owned_Base() {}
  virtual Statement* deep_copy(Base*)const // = 0;?
    { untested();unreachable();return nullptr;}

public:
  virtual bool propagate_rdeps(RDeps const&);
  /*virtual?*/ bool propagate_rdep(Base const*);
  virtual bool update() {
    trace2("Statement::update base", typeid(*this).name(), rdeps().size());
    return false;
  }
//  virtual Statement* parent_stmt();
  // Block* scope() { untested(); return Owned_Base::owner(); }
  // Block const* scope() const { untested(); return Owned_Base::owner(); }

  virtual bool is_used_in(Base const*)const;
  virtual Base* owner_() {
    if(scope()){
      return scope()->owner();
    }else{ untested();
      return nullptr;
    }
  }
//  virtual bool makes_own_scope()const  { untested();return false;}
  virtual Block* scope() {return Owned_Base::scope();}
  Block const* scope()const {return const_cast<Statement*>(this)->scope();}

public:
  void parse(CS&)override {unreachable();};
  void dump(std::ostream&)const override{ untested();unreachable();}
  virtual void submit_variable_access(Variable_Access&)const { untested();
    assert(0);
    unreachable();
  }

public: // called from Expression_::resolve
  virtual void push_use(Token_VAR_REF const*){unreachable();};
public:
  virtual RDeps const& rdeps()const {return _rdeps;}
private:
  void set_rdeps(TData const&);
//protected:
//  void store_rdeps(Rdeps const&);
public:
  bool set_used_in(Base const*b);
  void unset_used_in(Base const*){} // later.
  RDeps const& rdeps_()const {return rdeps();} // dump_annotate
protected: // assign.
  bool merge_rdeps(RDeps const&);
protected: // dbg.
 // int rdeps_size()const { untested(); return int(_rdeps.size()); }
 // void set_rdeps(TData const&);
public:
  bool is_ctx_initial()const;
  bool is_ctx_function()const;
  bool is_ctx_event()const;
  bool is_ctx_final()const;
  bool is_reachable()const;
  bool is_always()const;
  bool is_never()const;
}; // Statement
/*--------------------------------------------------------------------------*/
class Token_VAR_REF;
/*--------------------------------------------------------------------------*/
// actually a token?
class Assignment : public Expression_ {
protected:
  TData* _token_data{nullptr};
private:
  Token_VAR_REF* _token{nullptr};
protected:
  Token_VAR_REF* _lhsref{nullptr};
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
  Token_VAR_REF const& token()const { assert(_token); return *_token; }
  Token_VAR_REF* token_hack()const { untested(); /* incomplete(); */ assert(_token); return _token; }
  std::string name() const { assert(_token); return _token->name(); }
  void parse(CS& cmd) override;
  std::string lhsname()const { return lhs().name(); }
  void dump(std::ostream&)const override;
  bool propagate_deps(Token_VAR_REF const&);
  bool propagate_rdeps(RDeps const& incoming);
 // bool update();
  bool update(RDeps const* r=nullptr);

  void parse_rhs(CS& cmd);
  RDeps const& rdeps() const;
  Sensitivities const& sensitivities()const;
  bool has_sensitivities()const;
  Block const* lhs_scope()const { untested(); assert(_lhsref); return _lhsref->scope(); }
  bool is_used_in(Base const*b)const;
  bool is_used()const;
  operator bool() const {return _token;}
  bool is_output_var()const; // BUG
  bool is_state_var()const;
  bool is_common()const;
  bool is_temporary()const;
public: // storage
  void assign_var()const;
  void use_var()const;
  void submit_variable_access(Variable_Access& va)const;
private: // implementation
  bool store_deps(TData const&);
protected:
  std::string code_name()const;
public: // BUG
  Token_VAR_REF const* decl_token()const;
protected:
  void new_token(std::string const&);
  Token_VAR_REF& token() { assert(_token); return *_token; }
}; // Assignment
/*--------------------------------------------------------------------------*/
class Token_PROBE; //bug?
class Node;
class TData;
class Variable_Decl : public Assignment {
  TData _data;
  RDeps _rdeps; // Expression_?
  std::string /*TODO*/ _dimensions;
  enum storage_t{
    st_unknown = 0,
    st_common,
    st_state,
    st_temporary,
    st_override_common,
    st_override_state,
    st_override_temporary
  } _stt{st_unknown};
public:
  explicit Variable_Decl() : Assignment() { }
  explicit Variable_Decl(std::string const& name) : Assignment() {
    // new_data(); // ??
    assert(!_token_data);
    new_token(name);
   // n->_token = new Token_VAR_DECL(s+_token->name(), n, n->_token_data);
  }
  ~Variable_Decl();
  void parse(CS& f)override;
  void dump(std::ostream& f)const override;
  virtual /*?*/ Data_Type const& type()const {
    return _data.type();
  }
  void set_type(Data_Type const& d){ _data.set_type(d); }
  bool propagate_deps(Token_VAR_REF const&);
  bool propagate_rdeps(RDeps const&);
 RDeps const& rdeps()const {return _rdeps;}
public: // storage
  void set_common_var() { if(is_override_var()) {}else{_stt = st_common;} }
  void set_temporary_var() { if(is_override_var()) {}else{_stt = st_temporary;} }
  void set_state_var() { if(is_override_var()) {}else{_stt = st_state;} }
  void set_common_override() { untested();_stt = st_override_common;}
  void set_temporary_override() { untested();_stt = st_override_temporary;}
  void set_state_override() {_stt = st_override_state;};
  bool is_common()const {/*assert(_stt);*/ return _stt==st_common || _stt==st_override_common;}
  bool is_temporary()const {/*assert(_stt);*/ return _stt==st_temporary || _stt==st_override_temporary;}
  bool is_state_var()const {/*assert(_stt);*/ return _stt==st_state || _stt==st_override_state;}
private:
  bool is_override_var()const {/*assert(_stt);*/ return _stt>=st_override_common;}
// public: // manipulate storage
//   void init_var() { untested();_stt.init();}
//   void assign_var() { untested();_stt.assign();}
//   void use_var() { untested();_stt.use();}
protected:
  bool has_deps()const;
private:
  void new_deps();
  void new_data();
public:
  String_Arg key()const { return String_Arg(name()); }
  bool is_real()const { untested(); return type().is_real(); }
  bool is_int()const { untested(); return type().is_int(); }
  std::string const identifier()const { untested();return name();}
  std::string code_name()const;

  virtual Base const* value()const { untested(); unreachable(); return nullptr;}
  Block const* scope() const;
//  TData const& deps()const { untested(); assert(_data); return *_data; }
  Variable_Decl* deep_copy(Base* owner, std::string prefix="") const;
  void update();
private:
  TData& token_data() { assert(_token_data); return *_token_data; }
  TData& data() { return _data; }
public: // code gen
  TData const& data()const { return _data; }
protected:
  void new_var_ref();
}; // Variable_Decl
/*--------------------------------------------------------------------------*/
// Verilog needs variable declarations at the top of a (named) block.
// Technically, they are just Statements and could be anywhere.
// A Variable_Statement can host multiple declarations.
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
    { untested();unreachable();return nullptr;}
  bool update() override;
  bool is_used_in(Base const* b)const override;
  // RDeps const& rdeps()const override{ untested();
  //   static RDeps r; return r;
  // }
private:
  void submit_variable_access(Variable_Access&)const override;
}; // Variable_Stmt
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// class AnalogExpression?
class ConstantMinTypMaxExpression : public Base {
  Expression_ _e;
  Block* _owner{nullptr};
public:
  explicit ConstantMinTypMaxExpression() : Base(){}
  ~ConstantMinTypMaxExpression();
  void set_owner(Block* b){ _owner = b; }
public:
  Expression const& expression()const{ return _e; }
  bool empty()const;
  void parse(CS& f)override;
  void dump(std::ostream& o)const override;
  Base const* value() const;
};
/*--------------------------------------------------------------------------*/
class Sensitivities;
class Variable_Access;
class SeqBlock : public Block {
  Sensitivities* _sens{nullptr}; // here?
  Variable_List_Collection _variables;
  enum context{
    ctx_unknown = 0,
    ctx_default = 1,
    ctx_function = 2,
    ctx_event = 4,
    ctx_initial = 8,
    ctx_final = 16
  };
  int _ctx{0};
  Variable_Access* _variable_access{nullptr};
public:
  explicit SeqBlock(Base const* owner) : Block() {
    new_variable_access();
    (void)owner;
    incomplete();
  }
  explicit SeqBlock(Block const* scope) : Block() {
    new_variable_access();
    if(auto b = dynamic_cast<SeqBlock const*>(scope)){ untested();
      _ctx = b->_ctx;;
    }else{
      _ctx = ctx_unknown;
    }
  }
  ~SeqBlock();

public:
  void set_owner(Statement* s) {
    init_context(s);
    Block::set_owner(s);
  }
  void parse(CS&)override;
  void dump(std::ostream& o)const override;
  void parse_identifier(CS& f);
  bool has_identifier()const {return _identifier != "";}

  Variable_Access& variable_access() {
    assert(_variable_access); return *_variable_access;
  }
  Variable_Access const& variable_access()const {
    assert(_variable_access); return *_variable_access;
  }
private:
  void new_variable_access();
  void delete_variable_access();

public:


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
  bool is_ctx_initial()const { return _ctx & ctx_initial; }
  bool is_ctx_function()const { return _ctx & ctx_function; }
  bool is_ctx_event()const { return _ctx & ctx_event; }
  bool is_ctx_final()const { return _ctx & ctx_final; }
  void set_ctx_initial() { _ctx |= ctx_initial; }
  void set_ctx_function() { _ctx |= ctx_function; }
  void set_ctx_event() { _ctx |= ctx_event; }
  void set_ctx_final() { _ctx |= ctx_final; }
private:
protected: // bug
  void init_context(Statement* s) {
    // reachability here?
    if(s->is_ctx_event()){
      set_ctx_event();
    }else if(s->is_ctx_function()){
      set_ctx_function();
    }else{
    }
    if(s->is_ctx_initial()){
      set_ctx_initial();
    }else{
    }
    if(s->is_ctx_final()){
      set_ctx_final();
    }else{
    }
  }
public:
  Base* lookup(std::string const& k, bool recurse=true)override;
}; // SeqBlock
/*--------------------------------------------------------------------------*/
class SwitchBlock : public Block {
  Sensitivities* _sens{nullptr}; // here?
public:
  explicit SwitchBlock() : Block() { }
  ~SwitchBlock();
  void parse(CS&)override;
  void dump(std::ostream& o)const override;
  void parse_identifier(CS& f);
  bool has_identifier()const { untested();return _identifier != "";}

public:

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
  Branch_Ref lookup_branch(std::string const& n)const override { untested();
    assert(owner());
    return scope()->lookup_branch(n);
  }
  String_Arg const& identifier() const{ untested(); return _identifier; }
  bool has_sensitivities()const { untested();return _sens;}
  Sensitivities const* sensitivities()const { untested();return _sens;}
  void set_sens(Base* s);
  void merge_sens(Sensitivities const& s);
  map const& variables()const { untested();return _var_refs;}
  bool update();
public:
  Base* lookup(std::string const& k, bool recurse=true)override;
}; // SeqBlock
inline bool Statement::is_used_in(Base const* b) const
{
  // "used in vs rdeps?"
  for(auto const& i : rdeps()){
    if(i == b){
      return true;
    }else{
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
class CtrlStmt : public Statement {
protected:
  explicit CtrlStmt() : Statement() {}
  ~CtrlStmt();

  virtual TData const& deps()const = 0;
};
/*--------------------------------------------------------------------------*/
class InitialStmt : public CtrlStmt {
public:
  explicit InitialStmt(Block* o, CS& file) {
    set_owner(o);
    parse(file);
  }
  ~InitialStmt(){ }
public:
  void parse(CS&) override {incomplete();}
  void dump(std::ostream&)const override {incomplete();}
  bool is_used_in(Base const*)const override;
  bool update()override;

 // TData const& deps()const override // AnalogCtrlStmt
}; // AnalogInitialStmt
/*--------------------------------------------------------------------------*/
// SystemTaskCall
class System_Task : public Statement {
  Expression_ _e; // Analog?
  RDeps _rdeps;
public:
  explicit System_Task(CS&, Block*);
  void parse(CS& o) override;
  void dump(std::ostream&o)const override;
  bool is_used_in(Base const*)const override;
  bool update()override;

  bool has_sensitivities()const { return _e.data().has_sensitivities();}
  Sensitivities const& sensitivities()const { return _e.data().sensitivities();}

  Expression_ const& expression()const { return _e; }
  FUNCTION_ const* function()const;
  TData const& data()const { return _e.data(); }
  RDeps const& rdeps()const override { return _rdeps; }
private:
  bool add_rdep(Base const* b) { return _rdeps.insert(b).second; }
  void submit_variable_access(Variable_Access&)const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
