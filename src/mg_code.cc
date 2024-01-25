#include "mg_code.h"
#include "mg_deps.h" // BUG?

Variable_Decl::~Variable_Decl()
{
  delete _deps;
  _deps = NULL;
}
/*--------------------------------------------------------------------------*/
void Variable_Decl::new_deps()
{
  assert(!_deps);
  _deps = new Deps;
}
/*--------------------------------------------------------------------------*/
