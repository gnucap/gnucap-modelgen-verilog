
#ifndef MG_TOKEN_H
#define MG_TOKEN_H
#include "m_tokens.h"
#include <m_expression.h>
#include "mg_func.h"
class FUNCTION_;
class Token_CALL : public Token_SYMBOL {
private: // stuff into data?
  FUNCTION_ const* _function{NULL};
  Expression const* _args{NULL};
  size_t _num_args{size_t(-1)};
private:
  Token* clone()const override {
    return new Token_CALL(*this);
  }
public:
  explicit Token_CALL(const std::string Name, FUNCTION_ const* f, Expression const* e=NULL)
    : Token_SYMBOL(Name, ""), _function(f), _args(e) { attach(); }
  ~Token_CALL() { detach(); delete _args; }
protected:
  explicit Token_CALL(const Token_CALL& P)
    : Token_SYMBOL(P.name(), ""), _function(P._function), _num_args(P._num_args) { attach(); }
  explicit Token_CALL(const Token_CALL& P, Base const* data, Expression const* e=NULL)
    : Token_SYMBOL(P.name(), data), _function(P._function), _args(e)
    , _num_args(P._num_args) { attach(); }
private:
  void attach();
  void detach();
public:
  void pop(){ untested(); assert(_args); _args = NULL; }
  void push(Expression const* e){ assert(!_args); _args = e; }
  void stack_op(Expression* e)const override;
  void set_num_args(size_t n){ _num_args = n; } // expression size?
 // size_t num_args() const;
  Expression const* args()const { return _args; }
  virtual /*?*/ std::string code_name() const;
  FUNCTION_ const* f() const{ return _function; }
  bool returns_void() const;
}; // Token_CALL
/*--------------------------------------------------------------------------*/
inline void Token_CALL::attach()
{
  assert(_function);
  _function->inc_refs();
}
/*--------------------------------------------------------------------------*/
inline void Token_CALL::detach()
{
  assert(_function);
  _function->dec_refs();
}
/*--------------------------------------------------------------------------*/
inline bool Token_CALL::returns_void() const
{
//  assert(_function);
  if(_function){
    return _function->returns_void();
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
inline std::string Token_CALL::code_name()const
{
  assert(_function);
  if(_function->code_name()!=""){
    return "/*call1*/" + _function->code_name();
  }else if(_function->label()!=""){
    // incomplete(); // m_va.h, TODO
    return "/*INCOMPLETE*/ va::" + _function->label();
  }else{ untested();
    return "Token_CALL::code_name: incomplete";
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet
