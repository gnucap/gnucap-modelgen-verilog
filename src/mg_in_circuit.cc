#include "mg_circuit.h"
#include "mg_in.h" // TODO
#include "mg_.h" // TODO
/*--------------------------------------------------------------------------*/
void Port_1::parse(CS& file)
{ untested();
  trace1("Port_1::parse", file.last_match());
  file >> _name;
  size_t here = file.cursor();
  for (;;) {
    ONE_OF
      || ((file >> "short_to =") && (file >> _short_to))
      || ((file >> "short_if =") && (file >> _short_if))
      ;
    if (file.skip1b(";")) {
      break;
    }else if (!file.more()) { untested();untested();
      file.warn(0, "premature EOF (Port_1)");
      break;
    }else if (file.stuck(&here)) { untested();
      break;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void Port_1::dump(std::ostream& out)const
{ untested();
  if (short_to() != "" || short_if() != "") { untested();
    out << name() << " short_to=\"" << short_to() 
	<< "\" short_if=\"" << short_if() << "\";\n";
  }else{ untested();
    out << name() << "; ";
  }
}
/*--------------------------------------------------------------------------*/
void Circuit::parse_ports(CS& f)
{
  f >> _ports;
}
/*--------------------------------------------------------------------------*/
void Circuit::parse(CS&)
{
  assert(owner());
  _ports.set_owner(owner());
  _input.set_owner(owner());
  _output.set_owner(owner());
  _inout.set_owner(owner());
  _ground.set_owner(owner());
  _net_decl.set_owner(owner());
  _branch_decl.set_owner(owner());
  _local_nodes.set_owner(owner());
  _net_decl.set_owner(owner());
  _branches.set_owner(owner());
  _element_list.set_owner(owner());
}
/*--------------------------------------------------------------------------*/
void New_Port::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
  assert(owner());
  owner()->new_node(name());
}
/*--------------------------------------------------------------------------*/
void Net_Declarations::parse(CS& f)
{
  assert(owner()); // Module
  Module* mod = prechecked_cast<Module*>(owner());
  assert(mod);
  Block const* root_scope = owner()->owner();
  assert(root_scope);
  File const* root = dynamic_cast<File const*>(root_scope);
  if(root){
  }else{
    //incomplete();
    //f.reset_fail(f.cursor());
    //return;

    root = prechecked_cast<File const*>(root_scope->owner());
    assert(root);
  }
  auto ii = root->discipline_list().find(f);
  Net_Decl_List* d = NULL;

  if(ii!=root->discipline_list().end()){
//    size_t here = f.cursor();
    auto m = new Net_Decl_List_Discipline();
    m->set_discipline(*ii);

    m->set_owner(owner());
    f >> *m;
    for(auto i : *m){
      i->set_discipline(*ii, mod);
    }

    d = m;
  }else if(f.umatch("ground ")){
    auto m = new Net_Decl_List_Ground();
    m->set_owner(owner());
    f >> *m;
    d = m;
  }else{
    assert(!f);
  }

  if(d){
    push_back(d);

    if(mod->attribute_stash().is_empty()){
    }else if(size()){
      assert(!_attributes);
      d->set_attributes(mod->attribute_stash().detach());
    }else{ untested();
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
Port_3* Circuit::find_port(std::string const& n)
{
  auto p = _ports.find(String_Arg(n));
  if (p != _ports.end()){
    assert(*p);
    return *p;
  }else{
    trace1("port not found", n);
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
