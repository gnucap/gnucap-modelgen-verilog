#include "mg_in.h"
#include "mg_options.h"
#include "mg_error.h"
#include "mg_.h" // TODO
/*--------------------------------------------------------------------------*/

void File::dump(std::ostream& o) const
{
  if (options().dump_nature()){
    o << nature_list() << '\n';
  }else{
  }

  if (options().dump_discipline()){
    o << discipline_list() << '\n';
  }else{
  }
      // keep modules in order?
      //
  if (options().dump_module()){
    o << module_list() << '\n'
      << macromodule_list() << '\n'
      << connectmodule_list() << '\n';
  }else{
  }

  if(paramset_list().is_empty()) {
  }else if (options().dump_paramset()) {
    o << "// paramsets\n";
    o << paramset_list() << '\n';
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
File::File() : _file(CS::_STRING, "")
{
}
/*--------------------------------------------------------------------------*/
/* A.1.2
+ source_text ::=
+	  { untested(); description }
+ description ::=
+	  module_declaration
-	| udp_declaration
-	| config_declaration
-	| paramset_declaration
+	| nature_declaration
+	| discipline_declaration
-	| connectrules_declaration
*/
void File::parse(CS& f)
{
  _module_list.set_owner(this);
  _macromodule_list.set_owner(this);
  _connectmodule_list.set_owner(this);
  _nature_list.set_owner(this);
  _discipline_list.set_owner(this);
  _paramset_list.set_owner(this);

  size_t here = _file.cursor();
  for (;;) {
    parse_attributes(f, &f);
    ONE_OF	// description
      || f.umatch(";")
      || ((f >> "module ")	     && (f >> _module_list))
      || ((f >> "macromodule ")   && (f >> _macromodule_list))
      || ((f >> "connectmodule ") && (f >> _connectmodule_list))
      || ((f >> "nature ")	     && (f >> _nature_list))
      || ((f >> "discipline ")    && (f >> _discipline_list))
      || ((f >> "paramset ")      && (f >> _paramset_list))
      ;
    if (has_attributes(&f)){
      f.warn(bWARNING, "dangling attributes");
    }else{
    }
    if (!f.more()) {
      break;
    }else if (f.stuck(&here)) { untested();
      throw Exception_CS_("syntax error, need nature, discipline, module or paramset", f);
    }else{
    }
  }

  // HACK
  for(auto i: _module_list){
    i->set_owner(this);
  }

#if 1
  std::vector<Module*> tmp;
  for(auto i = _paramset_list.begin(); i!=_paramset_list.end();){
    auto j = i;
    ++j;
    Module* m = (*i)->deflate();
    if(m == *i){
      trace1("undeflated paramset", m->identifier());
    }else{
      trace1("deflated paramset", m->identifier());
      _paramset_list.erase(i);
      tmp.push_back(m); 
//		m->dump(std::cout);
//       _module_list.push_back(m);
    }
    i = j;
  }
   for(auto i: tmp){
     if(auto pp = dynamic_cast<Paramset*>(i)){
       trace1("undeflated paramset1", i->identifier());
       _paramset_list.push_back(pp);
     }else{ untested();
       trace1("deflated paramset1", i->identifier());
       _module_list.push_back(i);
     }
   }
#endif
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
