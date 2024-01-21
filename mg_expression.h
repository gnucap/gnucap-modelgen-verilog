// m_expression.h, with monkey patches
#ifndef MG_EXPRESSION_H
#define MG_EXPRESSION_H
#define Token_UNARY KO_Token_UNARY
#define Token_BINOP KO_Token_BINOP
#define Token_SYMBOL KO_Token_SYMBOL
#define Token_TERNARY KO_Token_TERNARY
#include <m_expression.h>
#undef Token_UNARY
#undef Token_BINOP
#undef Token_SYMBOL
#undef Token_TERNARY
/*--------------------------------------------------------------------------*/
class Token_UNARY : public Token
{
protected:
  explicit Token_UNARY(const std::string Name, Base const* Data)
    : Token(Name, Data, "") {}
public:
  explicit Token_UNARY(const std::string Name)
    : Token(Name, NULL, "") {}
//  explicit Token_UNARY(const Token_UNARY& P) : Token(P) {itested();}
  Token* clone()const override {itested();return new Token_UNARY(*this);}
  Token* op(const Token* t1)const;
  void stack_op(Expression*)const override;
};
/*--------------------------------------------------------------------------*/
class Token_BINOP : public Token
{
protected:
  explicit Token_BINOP(const std::string Name, Base const* Data)
    : Token(Name, Data, "") {}
public:
  explicit Token_BINOP(const std::string Name)
    : Token(Name, NULL, "") {}
  // explicit Token_BINOP(const Token_BINOP& P) : Token(P) {}
  Token* clone()const override{return new Token_BINOP(*this);}
  Token* op(const Token* t1, const Token* t2)const;
  void stack_op(Expression*)const override;
};
/*--------------------------------------------------------------------------*/
class Token_SYMBOL : public Token
{
protected:
  explicit Token_SYMBOL(const std::string Name, Base const* Data)
    : Token(Name, Data, "") {}
public:
  explicit Token_SYMBOL(const std::string Name, const std::string Args)
    : Token(Name, NULL, Args) {}
  explicit Token_SYMBOL(const Token_SYMBOL& P) : Token(P) {}
  Token* clone()const  override{return new Token_SYMBOL(*this);}
  void stack_op(Expression*)const override;
};
/*--------------------------------------------------------------------------*/
class Token_TERNARY : public Token {
  Expression const* _true{NULL};
  Expression const* _false{NULL};
protected:
  explicit Token_TERNARY(const std::string Name, Expression const* t,
                         Expression const* f, Base const* Data)
    : Token(Name, Data, ""), _true(t), _false(f) {}
  explicit Token_TERNARY(const std::string Name, Base const* Data)
    : Token(Name, Data, "") {}
public:
  explicit Token_TERNARY(const std::string Name, Expression const* t, Expression const* f)
    : Token(Name, NULL, ""), _true(t), _false(f) {}
  explicit Token_TERNARY(const Token_TERNARY& P) : Token(P) {}
  ~Token_TERNARY();
  Token* clone()const override{return new Token_TERNARY(*this);}
  Token* op(const Token* t1, const Token* t2, const Token* t3)const;
  void stack_op(Expression*)const override;
public:
  Expression const* true_part() const{ return _true; }
  Expression const* false_part() const{ return _false; }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class Deps;
class FUNCTION_;
class Probe;
class Branch_Ref;
class Block;
class Expression_ : public Expression {
  Block* _owner{NULL};
public:
  explicit Expression_() : Expression() {}
  ~Expression_();
  void resolve_symbols(Expression const& e){
    return resolve_symbols_(e);
  }
  void set_owner(Block* b){ _owner = b; }
  void dump(std::ostream& out)const override;
// private:
  Block* owner(){ return _owner; }
private:
  void resolve_symbols_(Expression const& e, Deps* deps=NULL);
public:
  void clear();
  Expression_* clone() const;
  Deps const& deps()const;
  // Attrib const& attrib()const;
  bool update();
//  Deps& deps();
//  Deps const& deps()const { return _deps; }
//  Deps& deps() { return _deps; }
private: // all the same eventually?
  Token* resolve_xs_function(std::string const& n);
  Token* resolve_function(FUNCTION_ const* filt, size_t na);
  Token* resolve_system_task(FUNCTION_ const* t);
  Probe const* new_probe(std::string const& xs, Branch_Ref const& br);
};
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
