#include "mg_circuit.h"
#include "mg_in.h" // TODO
#include "mg_.h" // TODO
#include "mg_dump.h"
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
void Circuit::dump(std::ostream& o) const
{
  for(auto const& e : element_list()){
    assert(e);
    if(e->eval()==""){
      o << *e;
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
void New_Port::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
  assert(owner());
  trace2("New_Port::parse", name(), value());
  if(name() == value()){
    set_node(owner()->new_node(value()));
  }else{
    owner()->new_node(value());
  }
}
/*--------------------------------------------------------------------------*/
std::string const& New_Port::node_name() const
{
  if(node()){
    return node()->name();
  }else{
    static std::string inv="(invalid)";
    return inv;
  }
}
/*--------------------------------------------------------------------------*/
int New_Port::node_number() const
{
  if(node()){
    return node()->number();
  }else{
    return -1;
  }
}
/*--------------------------------------------------------------------------*/
std::string New_Port::code_name() const
{
  return "p_" + name();
}
/*--------------------------------------------------------------------------*/
class Wire : public Discipline{
public:
  explicit Wire() : Discipline("wire") {}
}wire;
/*--------------------------------------------------------------------------*/
void Net_Identifier::parse(CS& file)
{
  Port_3::parse(file); // TODO: port_base?
}
/*--------------------------------------------------------------------------*/
Net_Decl_List_Dir::Net_Decl_List_Dir(char what): Net_Decl_List()
{
  if(what=='t'){
    _dir = dir_out;
  }else if(what=='o'){
    _dir = dir_io;
  }else if(what=='p'){
    _dir = dir_in;
  }else{
    unreachable();
  }
}
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
  Net_Decl_List* d = nullptr;

  trace1("Net_Declarations::parse0", f.tail().substr(0,20));
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
  }else if(f.umatch("wire ")){
    auto m = new Net_Decl_List_Discipline();
    m->set_discipline(&wire);

    m->set_owner(owner());
    f >> *m;
    for(auto i : *m){
      i->set_discipline(*ii, mod);
    }

    d = m;

  }else if(f.umatch("reg ")){
    auto m = new Net_Decl_List_Reg();
    m->set_owner(owner());
    f >> *m;
    d = m;
  }else if(f.umatch("ground ")){
    auto m = new Net_Decl_List_Ground();
    m->set_owner(owner());
    f >> *m;
    d = m;
  }else if(f.umatch("inout |input |output ")) {
    trace2("Net_Declarations::parse-", f.last_match(), f.tail().substr(0,20));
    auto m = new Net_Decl_List_Dir(f.last_match()[2]);
    m->set_owner(owner());
    f >> *m;
    d = m;
  }else{
    trace1("Net_Declarations::parse2", f.tail().substr(0,20));
    assert(!f);
  }

  if(d){
    attr.move_attributes(tag_t(&f), tag_t(d));
    push_back(d);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
// 3.6.4 Ground declaration
// Each ground declaration is associated with an already declared net of continuous discipline. The node asso-
// ciated with the net will be the global reference node in the circuit. The net must be assigned a continuous
// discipline to be declared ground.
void Net_Decl_List_Ground::parse(CS& f)
{
  return Net_Decl_List::parse_n_<Net_Identifier_Ground>(f);
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Ground::dump(std::ostream& o) const
{
  o__ "ground ";
  Net_Decl_List::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Net_Decl_Reg::parse(CS& f)
{
  assert(owner());
  Net_Identifier::parse(f);
  Module* mod = prechecked_cast<Module*>(owner());
  assert(mod);
  Node_Ref const& nn = owner()->node(name());
  if(nn) {
    set_node(mod->node(nn));
  }else{ untested();
    throw Exception_CS_("ground: need previously declared net", f);
  }

  Module* m = prechecked_cast<Module*>(owner());
  assert(m);
  assert(node());
  m->set_reg(node());
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Reg::parse(CS& f)
{
  return Net_Decl_List::parse_n_<Net_Decl_Reg>(f);
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Reg::dump(std::ostream& o) const
{
  o__ "reg ";
  Net_Decl_List::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Dir::parse(CS& f)
{
  return Net_Decl_List::parse_n_<Net_Decl_Dir>(f);
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Dir::dump(std::ostream& o) const
{
  static std::string s[4] = {"??", "input", "output", "inout"};
  o__ s[_dir] << " ";
  Net_Decl_List::dump(o);
  o << "\n";
}
/*--------------------------------------------------------------------------*/
void Net_Decl_Dir::parse(CS& f)
{
  Net_Identifier::parse(f);

  assert(owner());
  // incomplete(); but not in use yet.
//  set_dir(owner()->set_dir(name()));
}
/*--------------------------------------------------------------------------*/
void Net_Identifier_Discipline::parse(CS& f)
{
  Net_Identifier::parse(f);

  assert(owner());
  set_node( owner()->new_node(name()));
}
/*--------------------------------------------------------------------------*/
void Net_Identifier_Ground::parse(CS& f)
{
  assert(owner());
  Net_Identifier::parse(f);
  Module* mod = prechecked_cast<Module*>(owner());
  assert(mod);
  Node_Ref const& nn = owner()->node(name());
  if(nn) {
    set_node(mod->node(nn));
  }else{ untested();
    throw Exception_CS_("ground: need previously declared net", f);
  }

  Module* m = prechecked_cast<Module*>(owner());
  assert(m);
  assert(node());
  m->set_to_ground(node());
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Discipline::parse(CS& f)
{
  trace1("Port_Disc_List::parse", f.last_match());
  return Net_Decl_List::parse_n_<Net_Identifier_Discipline>(f);
  // return Net_Identifier, '\0', ',', ';'>::parse(f);
}
/*--------------------------------------------------------------------------*/
void Net_Decl_List_Discipline::dump(std::ostream& o)const
{
  assert(_disc);
  // o__ "";
  print_attributes(o, this);

  o__ _disc->identifier() << " ";
  Net_Decl_List::dump(o);
  o << "\n";
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
    return nullptr;
  }
}
/*--------------------------------------------------------------------------*/
void Circuit::push_back(Filter /*const?*/ * f)
{
  _filters.push_back(f);
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
class Token_POTENTIAL : public Token_PROBE {
public:
  explicit Token_POTENTIAL(std::string const& name, Branch const* b, TData* tdata)
    : Token_PROBE(name, b, tdata) {}

  Probe const* probe()const {incomplete(); return nullptr;}
  std::string val_string()const override;
  std::string code_name()const override {
    assert(_branch); return "_potential"+_branch->code_name();
  }
  bool is_pot_probe()const override { return true;}
};
/*--------------------------------------------------------------------------*/
class Token_FLOW : public Token_PROBE {
public:
  explicit Token_FLOW(std::string const& name, Branch const* b, TData* tdata)
    : Token_PROBE(name, b, tdata) {}

  Probe const* probe()const {incomplete(); return nullptr;}
  std::string val_string()const override;
  std::string code_name()const override {
    assert(_branch); return "_flow"+_branch->code_name();
  }
  bool is_flow_probe()const override {return true;}
};
/*--------------------------------------------------------------------------*/
class Filter_output : public Token_PROBE {
public:
  explicit Filter_output(std::string const& name, Branch const* b, TData* tdata)
    : Token_PROBE(name, b, tdata) {}

  Probe const* probe()const {incomplete(); return nullptr;}
  std::string val_string()const override { untested();
    return "F" + _branch->name();
  }
  bool is_pot_probe()const override {untested(); return true;}//...
  std::string code_name()const override { untested();
    assert(_branch);
    return "_value"+_branch->code_name();
  }
/*--------------------------------------------------------------------------*/
};
/*--------------------------------------------------------------------------*/
std::string Token_FLOW::val_string() const
{
    return "I" + _branch->name();
}
/*--------------------------------------------------------------------------*/
std::string Token_POTENTIAL::val_string() const
{
    return "V" + _branch->name();
}
/*--------------------------------------------------------------------------*/
Base const* Branch::flow_dep()
{
  if(_flow){
  }else{
    _flow = new Token_FLOW("NAME", this, nullptr);
  }
  return _flow;
}
/*--------------------------------------------------------------------------*/
Base const* Branch::potential_dep()
{
  if(_potential){
  }else if(is_filter()){ untested();
    _potential = new Filter_output(code_name(), this, nullptr);
  }else{
    _potential = new Token_POTENTIAL("NAME", this, nullptr);
  }
  return _potential;
}
/*--------------------------------------------------------------------------*/
void Branch::delete_deps()
{
  delete _deps;
  _deps = nullptr;

  delete _flow;
  delete _potential;
  _flow = _potential = nullptr;
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
    _br = nullptr;
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
