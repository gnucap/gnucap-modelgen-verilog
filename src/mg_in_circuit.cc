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
bool Branch::is_detached_filter() const
{
  auto f = dynamic_cast<MGVAMS_FILTER const*>( _ctrl);
  return f && f->is_standalone();
}
/*--------------------------------------------------------------------------*/
void Branch::new_deps()
{
  assert(!_deps);
  _deps = new TData;
  assert(_deps->is_linear());
}
/*--------------------------------------------------------------------------*/
void Branch::add_dep(Dep const& b)
{
//  if(b->branch() == this){ untested();
//    _selfdep = true;
//  }else{ untested();
//  }
  // TODO incomplete(); // linear?
//  b->branch()->inc_use();
  deps().insert(b);
}
/*--------------------------------------------------------------------------*/
bool Branch::has_pot_probe() const
{
  return _has_pot_probe;
}
/*--------------------------------------------------------------------------*/
bool Branch::has_flow_probe() const
{
  return _has_flow_probe;
}
/*--------------------------------------------------------------------------*/
bool Branch::is_generic()const
{
  if(!is_direct()){
    if(has_pot_source()){
      return true;
    }else{ untested();
      incomplete();
    }
  }else if(has_flow_probe()){
    // return _selfdep;
  }else if(has_pot_source()){
    if(_selfdep){
      return true;
    }else{
    }
  }else if(has_flow_source()){
  }else{ untested();
  }
  return false;
}
/*--------------------------------------------------------------------------*/
std::string Branch::dev_type()const
{
//  if( .. attribute .. )?
  if(is_filter()) {
    std::string label = "va_" + _ctrl->label();
    auto pos = label.find_last_of("_");
    return label.substr(0, pos);
  }else if(!is_direct()){
    if(has_pot_source()){
      return "va_pot_br";
    }else{ untested();
      return "incomplete_dev_type";
    }
  }else if(has_flow_probe()) {
    return "va_sw"; // ?
  }else if(has_pot_source()){
    if(_selfdep){
      return "va_pot_br";
    }else if(has_always_pot() && !has_flow_source()) {
      return "va_pot";
    }else{
      return "va_sw";
    }
  }else if(has_flow_source()){
    return "va_flow";
  }else{ untested();
    return "va_sw";
  }
  unreachable();
  return "";
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch_Ref const& b)
    : Base(),
      _br(b._br),
      _r(b._r)
{
  if(_br){
    _br->attach(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch_Ref&& b)
    : Base(),
      _br(b._br),
      _r(b._r)
{
  if(_br){
    _br->attach(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch* b, bool reversed)
  : _br(b), _r(reversed)
{
  if(_br){
    _br->attach(this);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Named_Branch* b)
  : _br(b), _r(b->is_reversed())
{
  if(_br){
    _br->attach(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::~Branch_Ref()
{
  if(_br){
    _br->detach(this);
    _br = NULL;
  }else{
  }
}
/*--------------------------------------------------------------------------*/
std::string Branch_Ref::code_name()const
{
  assert(_br);
//  if(has_name()){ untested();
//    return "_br_" + *_name;
//  }else{ untested();
  return _br->code_name();
//  }
}
/*--------------------------------------------------------------------------*/
// void Branch_Ref::set_name(std::string const& n)
// { untested();
//   assert(!has_name());
//   assert(_br);
//   _name = _br->reg_name(n);
// }
/*--------------------------------------------------------------------------*/
Branch_Ref& Branch_Ref::operator=(Branch_Ref&& o)
{
  operator=(o);

  if(_br) {
//    assert(_br->has(this));
  }else{
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
Branch_Ref& Branch_Ref::operator=(Branch_Ref const& o)
{
  if(_br) { untested();
    _br->detach(this);
  }else{
  }

  _br = o._br;
  _r = o._r;
  //_name = o._name;

  if(_br) {
    _br->attach(this);
  }else{
  }

  return *this;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// has_source? is_source?
bool Branch::has_element() const
{
  if(is_short()){
    return false;
  }else if( has_flow_source() ){
    return true;
  }else if( has_pot_source() ){
    return true;
  }else if( has_flow_probe() ){
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
bool Branch::has_pot_source() const
{
  return _has_pot_src; //  || _has_flow_probe;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
