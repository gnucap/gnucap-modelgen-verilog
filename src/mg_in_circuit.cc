#include "mg_circuit.h"
#include "mg_in.h" // TODO
#include "mg_.h" // TODO
/*--------------------------------------------------------------------------*/
void Port_1::parse(CS& file)
{ untested();
  trace1("Port_1::parse", file.last_match());
  file >> _name;
  size_t here = file.cursor();
  for (;;) { untested();
    ONE_OF
      || ((file >> "short_to =") && (file >> _short_to))
      || ((file >> "short_if =") && (file >> _short_if))
      ;
    if (file.skip1b(";")) { untested();
      break;
    }else if (!file.more()) { untested();untested();
      file.warn(0, "premature EOF (Port_1)");
      break;
    }else if (file.stuck(&here)) { untested();
      break;
    }else{ untested();
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
  Block const* root_scope = owner()->scope();
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
    attr.move_attributes(tag_t(&f), tag_t(d));
    push_back(d);
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
  }else{ untested();
    trace1("port not found", n);
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
bool Branch::is_used_in(Base const* b)const
{
  for(auto& i : _used_in){
    if(i == b){ untested();
      return true;
    }else{
    }
  }
  return false;
}
/*--------------------------------------------------------------------------*/
bool Branch::is_used()const
{
  if(is_filter()){
    assert(_ctrl);
    return _ctrl->has_refs();
  }else if(_use){
    return true;
  }else if(_has_flow_probe) {
    return true;
//  }else if(_used_in.size()) { untested();
//    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
