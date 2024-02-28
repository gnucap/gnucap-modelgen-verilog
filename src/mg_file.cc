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
void File::parse(CS& file)
{
//  _module_list.set_file(this); // needed?
			       //
  _attribute_stash.set_owner(this);

  _module_list.set_owner(this);
  _macromodule_list.set_owner(this);
  _connectmodule_list.set_owner(this);
  _nature_list.set_owner(this);
  _discipline_list.set_owner(this);
  _paramset_list.set_owner(this);

  size_t here = _file.cursor();
  for (;;) {
    while(file >> _attribute_stash){ }
    ONE_OF	// description
      || file.umatch(";")
      || ((file >> "module ")	     && (file >> _module_list))
      || ((file >> "macromodule ")   && (file >> _macromodule_list))
      || ((file >> "connectmodule ") && (file >> _connectmodule_list))
      || ((file >> "nature ")	     && (file >> _nature_list))
      || ((file >> "discipline ")    && (file >> _discipline_list))
      || ((file >> "paramset ")      && (file >> _paramset_list))
      ;
    if (_attribute_stash.is_empty()){
    }else{ untested();
      file.warn(bWARNING, "dangling attributes");
    }
    if (!file.more()) {
      break;
    }else if (file.stuck(&here)) { untested();
      throw Exception_CS_("syntax error, need nature, discipline, module or paramset", file);
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
