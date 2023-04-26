
#include <gnucap/u_function.h>
#include <gnucap/globals.h>
#include <gnucap/u_parameter.h>

namespace{

class DUMMY : public FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "";
  }
} dummy;
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "abs", &dummy);
/*--------------------------------------------------------------------------*/
class exp : public FUNCTION {
public:
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const override {
    PARAMETER<double> x;
    Cmd >> x;
    x.e_val(NOT_INPUT, Scope);
    return to_string(std::exp(x));
  }
} p_exp;
DISPATCHER<FUNCTION>::INSTALL d_exp(&function_dispatcher, "exp", &p_exp);
/*--------------------------------------------------------------------------*/

// not really functions, but syntactically so.
DISPATCHER<FUNCTION>::INSTALL d2(&function_dispatcher, "ddt", &dummy);
DISPATCHER<FUNCTION>::INSTALL d3(&function_dispatcher, "idt", &dummy);

}
