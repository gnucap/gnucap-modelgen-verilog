#include "mg_analog.h"
#include "mg_func.h"
#include "mg_token.h"
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// .. to be sorted.
#include "mg_in.cc"
#include "mg_out.cc"
/*--------------------------------------------------------------------------*/
void Token_FUNCTION::stack_op(Expression*) const
{ untested();
  unreachable();
  incomplete();
}
/*--------------------------------------------------------------------------*/
#if 1
void Token_CALL::stack_op(Expression*) const
{ untested();
  unreachable();
  incomplete();
}
#else
/*--------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------*/
Node_Ref MGVAMS_FILTER::p() const {unreachable(); return nullptr;}
Node_Ref MGVAMS_FILTER::n() const {unreachable(); return nullptr;}
/*--------------------------------------------------------------------------*/
Node_Ref Branch::p() const {return _p;}
Node_Ref Branch::n() const {return _n;}
bool Branch::is_short() const
{
  assert(_p);
  assert(_n);
  return _p->number() == _n->number();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void MGVAMS_FILTER::set_p_to_gnd(Module*) const{ unreachable(); }
void MGVAMS_FILTER::set_n_to_gnd(Module*) const{ untested(); unreachable(); }
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
