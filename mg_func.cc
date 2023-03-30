
#include <gnucap/u_function.h>
#include <gnucap/globals.h>

namespace{

class DUMMY : public FUNCTION {
  std::string eval(CS&, const CARD_LIST*)const override{
	  unreachable();
	  return "";
  }
} dummy;
DISPATCHER<FUNCTION>::INSTALL d0(&function_dispatcher, "abs", &dummy);
DISPATCHER<FUNCTION>::INSTALL d1(&function_dispatcher, "exp", &dummy);

// not really functions, but syntactically so.
DISPATCHER<FUNCTION>::INSTALL d2(&function_dispatcher, "ddt", &dummy);
DISPATCHER<FUNCTION>::INSTALL d3(&function_dispatcher, "idt", &dummy);

}
