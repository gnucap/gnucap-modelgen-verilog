/*                       -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#include "mg_.h"
#include "mg_out.h" // really?
/*--------------------------------------------------------------------------*/
Node mg_ground_node;
/*--------------------------------------------------------------------------*/
bool ConstantMinTypMaxExpression::empty() const
{
  assert(_e);
  return !_e->size();
}
/*--------------------------------------------------------------------------*/
bool Module::has_analog_block() const
{
  return !analog_list().is_empty();
}
/*--------------------------------------------------------------------------*/
bool Module::has_submodule() const
{
  return !element_list().is_empty();
}
/*--------------------------------------------------------------------------*/
Probe const* Module::new_probe(std::string const& xs, std::string const& p,
    std::string const& n)
{
  Branch_Ref br = new_branch(p, n);
  assert(br);
  return new_probe(xs, br);
}
/*--------------------------------------------------------------------------*/
Probe const* Module::new_probe(std::string const& xs, Branch_Ref const& br)
{
  std::string flow_xs;
  std::string pot_xs;

  if(br->discipline()){
    trace2("new_probe", xs, br->discipline()->identifier());
    flow_xs = br->discipline()->flow()->access().to_string();
    pot_xs = br->discipline()->potential()->access().to_string();
  }else{
    // huh?
  }

  std::string nn = xs;
  // incomplete. discipline.h
  if(xs == flow_xs || xs == "flow"){
    br->set_flow_probe();
    assert(br->has_flow_probe());
    nn = "flow";
  }else if( xs == pot_xs || xs == "potential" ){
    br->set_pot_probe();
    nn = "potential";
  }else if( xs == "_filter"){
    br->set_filter();
    br->set_pot_probe();
  }else{ untested();
    unreachable();
    nn = xs;
  }

  // TODO: use new_probe(branch); ?

  // TODO: is this needed?
  std::string k = nn + "_" + br.pname() + "_" + br.nname();
  Probe*& prb = _probes[k];

  if(prb) {
  }else{
    prb = new Probe(nn, br);
  }

  // disambiguation hack. cleanup later.
  if(br.nname()==br.pname()){
  }else if(br.is_reversed()){
    new_probe(xs, br.nname(), br.pname());
  }else{
  }

  return prb;
}
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::new_branch(Node const* a, Node const* b)
{
  std::pair<Node const*, Node const*> p(a, b);
  const_iterator i = _m.find(p);
  if(i!=_m.end()){
    return Branch_Ref(i->second);
  }else{
    std::pair<Node const*, Node const*> r(b, a);
    const_iterator i = _m.find(r);
    if(i!=_m.end()){
      return Branch_Ref(i->second, true);
    }else{
      _m[p] = new Branch(a, b);
  	// br.set_owner(this); // eek.
      return Branch_Ref(_m[p] /*owner?*/);
    }
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref const& Module::new_branch_name(std::string const& n, Branch_Ref const& b)
{
  return _branch_names.new_name(n, b);
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch(Node const* p, Node const* n = NULL)
{
  Branch_Ref br = _branches.new_branch(p, n);
  assert(br);
  br.set_owner(this); // eek.
  return br;
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch(std::string const& p, std::string const& n)
{
  trace2("new_branch", p,n);
  if(p==""){ untested();
    throw Exception("syntax error");
  }else{
  }

  Branch_Ref a = _branch_names.lookup(p);

  if(a){
    if(n!=""){
      throw Exception("syntax error");
    }else{
      return a;
    }
  }else if(n==""){itested();
    Node const* pp = new_node(p); // BUG: existing node??
    return new_branch(pp, &mg_ground_node);
  }else{
    Node const* pp = new_node(p); // BUG: existing node??
    Node const* nn = new_node(n); // BUG: existing node??
    return new_branch(pp, nn);
  }
}
/*--------------------------------------------------------------------------*/
Node* Node_Map::new_node(std::string const& p)
{
  Node*& cc = _map[p];
  if(cc) {
  }else{
    cc = new Node(p, int(_nodes.size()));
    _nodes.push_back(cc);
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Node_Map::Node_Map()
{
}
/*--------------------------------------------------------------------------*/
Node_Map::~Node_Map()
{
  for(auto i: _nodes){
    delete i;
  }
}
/*--------------------------------------------------------------------------*/
Node const* Node_Map::operator[](std::string const& key) const
{
  auto i = _map.find(key);
  if(i != _map.end()) {
    return i->second;
  }else{
    throw Exception("no such node " + key );
  }
}
/*--------------------------------------------------------------------------*/
Node* Module::new_node(std::string const& p)
{
  return _nodes.new_node(p);
}
/*--------------------------------------------------------------------------*/
Node const* Module::node(std::string const& p) const
{
  try{
    return _nodes[p];
  }catch(Exception const&){
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::branch(std::string const& p) const
{
  return _branch_names.lookup(p);
}
/*--------------------------------------------------------------------------*/
Branch_Ref const& Branch_Names::new_name(std::string const& n, Branch_Ref const& r)
{
  Branch_Ref& j = _m[n];
  if(j){
    throw Exception("Branch " + n + " already defined\n");
  }else{
    return j = r;
  }
}
/*--------------------------------------------------------------------------*/
void Branch_Names::clear()
{
  _m.clear();
}
/*--------------------------------------------------------------------------*/
Branch_Ref const& Branch_Names::lookup(std::string const& p) const
{
  static Branch_Ref none;
  const_iterator i = _m.find(p);
  if(i == _m.end()){
    return none;
  }else{
    return i->second;
  }
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_nodes() const
{
  size_t ret=1;

  for(auto i : deps()){
    if(i->is_reversed()){
    }else if(i->branch() == this){
      // self conductance
    }else if(i->is_filter_probe()){
      ++ret;
    }else if(i->is_pot_probe()){
      ++ret;
    }else{
    }
  }
  return 2*ret;
}
/*--------------------------------------------------------------------------*/
void Branch::add_probe(Probe const* b)
{
  _deps.insert(b);
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
const std::string& Branch::dev_type()const
{
  if(!has_flow_source() && !has_pot_source() && !has_flow_probe()){
    incomplete();
    assert(0);
  }else if(has_flow_source() && has_pot_source()){
  }

  static std::string f = "flow_src";
  static std::string p = "va_pot";
  if(has_flow_probe()){
    return p;
  }else if(has_pot_source()){
    return p;
  }else if(has_flow_source()){
    return f;
  }else{
    return p;
  }
}
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

  if(_br) {
    _br->attach(this);
  }else{ untested();
  }

  return *this;
}
/*--------------------------------------------------------------------------*/
std::string const& Branch_Ref::pname() const
{
  assert(_br);
  assert(_br);
  if(_r){
    assert(_br->n());
    return _br->n()->name();
  }else{
    assert(_br->p());
    return _br->p()->name();
  }
}
/*--------------------------------------------------------------------------*/
std::string const& Branch_Ref::nname() const
{
  assert(_br);
  if(_r){
    assert(_br->n());
    return _br->p()->name();
  }else{
    assert(_br->p());
    return _br->n()->name();
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
bool Branch::has_element() const
{
  return has_flow_source() || has_pot_source() || has_flow_probe();
}
/*--------------------------------------------------------------------------*/
bool Branch::has_pot_source() const
{
  return _has_pot_src; //  || _has_flow_probe;
}
/*--------------------------------------------------------------------------*/
size_t Filter::num_states() const
{
  size_t k = 2; // self conductance and what?
  // TODO: cleanup
  for(auto i : deps()){
    trace1("filter deps", i->code_name());
    if(i->is_reversed()){
    }else{
    }

    if(dynamic_cast<Element_2 const*>(i->branch()) == this){
    }else{
      ++k;
    }
  }
  return k;
}
/*--------------------------------------------------------------------------*/
size_t Filter::num_nodes() const
{
  size_t ret=1;

  for(auto i : deps()){
    if(i->is_reversed()){
    }else{
    }
//    }else if(i->branch() == this){
      // self conductance
    if(i->is_filter_probe()){
      ++ret;
    }else if(i->is_pot_probe()){
      ++ret;
    }else{
    }
  }
  return 2*ret;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
size_t Branch::num_states() const
{
  size_t k = 2;
  // TODO: cleanup
  for(auto i : deps()){
    assert(i);
    if(i->is_reversed()){ untested();
    }else if(i->branch() == this){
    }else{
      ++k;
    }
  }
  return k;
}
/*--------------------------------------------------------------------------*/
std::string Branch::state()const
{
  return "_st" + code_name();
}
/*--------------------------------------------------------------------------*/
Discipline const* Branch::discipline() const
{
  assert(_p);
  assert(_n);
  if(_n == &mg_ground_node){
    return _p->discipline();
  }else if(!_p->discipline()){
    return _n->discipline();
  }else if(_p->discipline() == _n->discipline()){
    return _p->discipline();
  }else{itested();
    incomplete(); // no default.
    // Make sure to specify some discipline for now.
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Probe::Probe(std::string const& xs, Branch_Ref b) : _xs(xs), _br(b)
{
  trace2("::Probe", xs, code_name());
  // TODO: disciplines.h
  _is_pot_probe = (_xs == "V") || (_xs == "potential");
  _is_flow_probe = (_xs == "I") || (_xs == "flow");
}
//bool Probe::is_pot_probe() const
//{
//  ...
//}
/*--------------------------------------------------------------------------*/
bool Probe::is_filter_probe() const
{
  assert(_br);
  return _br->is_filter();
}
/*--------------------------------------------------------------------------*/
bool Probe::is_reversed() const
{
  return _br.is_reversed();
}
/*--------------------------------------------------------------------------*/
Nature const* Branch::nature() const
{
//  source?
  return NULL;
}
/*--------------------------------------------------------------------------*/
Nature const* Probe::nature() const
{
  return _br->nature();
}
/*--------------------------------------------------------------------------*/
Module::~Module()
{
  // cleanup references to branches.
  _branch_names.clear();
  _analog_list.clear();
  _branch_decl.clear();
  for(auto i: _probes){
    delete i.second;
  }
  _probes.clear();
  _filters.clear();
}
/*--------------------------------------------------------------------------*/
class idt_Filter: public Filter{
public:
  explicit idt_Filter(std::string const& name, Deps const& d)
    : Filter(name, d) {}
public:
  std::string const& dev_type()const override{
    static std::string dt = "va_idt";
    return dt;
  }
  size_t num_args()const override{return 1;}
};
/*--------------------------------------------------------------------------*/
class ddt_Filter: public Filter{
public:
  explicit ddt_Filter(std::string const& name, Deps const& d)
    : Filter(name, d) {}
public:
  std::string const& dev_type()const override{
    static std::string dt = "va_ddt";
    return dt;
  }
  size_t num_args()const override{return 1;}
};
/*--------------------------------------------------------------------------*/
class ddx_Filter: public Filter{
  Probe const* _ddxprobe{NULL};
public:
  explicit ddx_Filter(std::string const& name, Deps const& d)
    : Filter(name, d) {
      assert(d.size()==1);
      _ddxprobe = *d.begin();
  }
public:
  std::string const& dev_type()const override{
    static std::string dt = "";
    return dt;
  }
  size_t num_args()const override{return 2;}
  void make_cc(std::ostream&, Module const&)const override;
};
/*--------------------------------------------------------------------------*/
void ddx_Filter::make_cc(std::ostream& o, Module const& m) const
{
  o__ "// ddx " <<  _ddxprobe->code_name() << "\n";
  if(_ddxprobe->is_pot_probe()){
  }else{
    incomplete();
  }
  if(_ddxprobe->branch()->n() == &mg_ground_node){
  }else{
    incomplete();
  }
  Node const* p = _ddxprobe->branch()->p();
  o__ "ddouble ret;\n";
  o__ "(void) t1;\n";
  for(auto x : m.branches()){
    if(x.second->has_pot_probe()){
      o__ "// found probe " <<  x.second->code_name() << "\n";
      if(x.second->p() == p){
	o__ "ret.value() += t0[d_potential" << x.second->code_name() << "];\n";
      }else if(x.second->n() == p){
	o__ "ret.value() -= t0[d_potential" << x.second->code_name() << "];\n";
      }else{
      }

    }else{
    }

  }
  o__ "return ret;\n";
}
/*--------------------------------------------------------------------------*/
static int n_filters;
Filter const* Module::new_filter(std::string const& xs, Deps const&d)
{
//  size_t n_filters = _filters.size();
  std::string filter_code_name = xs + "_" + std::to_string(n_filters++);
  CS cmd(CS::_STRING, filter_code_name + " short_if=0 short_to=0;");

  Port_1* p = new Port_1;
  cmd >> *p;
  // _local_nodes.push_back(p);
  Node* n = NULL;

  // TODO: some kind of dispatch.
  Filter* f = NULL;
  if(xs=="ddt"){
    n = new_node(p->name());
    f = new ddt_Filter(filter_code_name, d);
  }else if(xs=="idt"){
    n = new_node(p->name());
    f = new idt_Filter(filter_code_name, d);
  }else if(xs=="ddx"){
    f = new ddx_Filter(filter_code_name, d);
  }else{
    assert(false);
    /// . throw..
  }
  f->set_owner(this);
  trace3("new filter", f->name(), f->num_states(), d.size());
  if(xs == "ddt" || xs == "idt"){
    // BUG. filter constructor?
    Branch* br = new_branch(n, &mg_ground_node);
    assert(br);
    f->set_output(Branch_Ref(br));
  }else{
  }
  _filters.push_back(f);
  return f;
}
/*--------------------------------------------------------------------------*/
inline Branch_Ref Variable::new_branch(std::string const& p, std::string const& n)
{
  assert(owner());
  return(owner()->new_branch(p, n));
}
/*--------------------------------------------------------------------------*/
inline Branch_Ref Variable::new_branch(Node const* p, Node const* n)
{
  assert(owner());
  return(owner()->new_branch(p, n));
}
/*--------------------------------------------------------------------------*/
bool Variable::is_module_variable() const
{
  if(dynamic_cast<Module const*>(owner())){
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
std::string Variable::code_name() const
{
  if(is_module_variable()){
    return "d->_v_" + name();
  }else{
    return "_v_" + name();
  }
}
/*--------------------------------------------------------------------------*/
void Contribution::set_pot_source()
{
  assert(_branch);
  _branch->set_pot_source();
  _type = t_pot;
}
/*--------------------------------------------------------------------------*/
void Contribution::set_flow_source()
{
  assert(_branch);
  _branch->set_flow_source();
  _type = t_flow;
}
/*--------------------------------------------------------------------------*/
bool Contribution::is_flow_contrib() const
{
  return _type == t_flow;
}
/*--------------------------------------------------------------------------*/
bool Contribution::is_pot_contrib() const
{
  return _type == t_pot;
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_module_variable() const
{
  assert(_lhs);
  return _lhs->is_module_variable();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_int() const
{
  //assert(_lhs->is_int() == _type.is_int());
  assert(_lhs);
  return _lhs->is_int();
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch_Ref const& b)
    : Owned_Base(b),
      _br(b._br),
      _r(b._r),
      _alias(b._alias)
{
  if(_br){
    _br->attach(this);
  }else{
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch_Ref&& b)
    : Owned_Base(b),
      _br(b._br),
      _r(b._r),
      _alias(b._alias)
{
  if(_br){
    _br->attach(this);
  }else{
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
Branch_Ref::~Branch_Ref()
{
  if(_br){
    _br->detach(this);
    _br = NULL;
  }else{
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch::~Branch()
{
  // no, shutting down, not all Refs tidied up.
  assert(!_refs.size());
}
/*--------------------------------------------------------------------------*/
void Branch::attach(Branch_Ref* r)
{
  assert(r);
  for(auto i : _refs){
    assert(i != r);
  }
  _refs.push_back(r);
}
/*--------------------------------------------------------------------------*/
#if 0
bool Branch::has(Branch_Ref* r) const
{
  assert(r);
  for(auto& i : _refs){
    if(i == r){
      return true;
    }else{
    }
  }
  return false;
}
#endif
/*--------------------------------------------------------------------------*/
void Branch::detach(Branch_Ref* r)
{
  assert(r);
  for(auto& i : _refs){
    if(i == r){
      i = _refs.back();
      _refs.resize(_refs.size()-1);
      return;
    }else{
    }
  }
  unreachable(); // cleanup is out of order?
}
/*--------------------------------------------------------------------------*/
Deps::~Deps()
{
//  for(auto i : *this){ untested();
//    delete i;
//  }
  std::set<Probe const*>::clear();
}
/*--------------------------------------------------------------------------*/
bool Module::sync() const
{
#if 0
  // need getattr<bool>?
  if(!has_attr("sync")){
    true, false? automatic? how
  }else if(attr("sync") == "1"
        || attr("sync") == "yes"){
    return true;
  }else if(attr("sync") == "0"
        || attr("sync") == "no"){
  }else{
    unsupported.
  }
#endif

  if(has_submodule() && has_analog_block()){ untested();
    incomplete();
  }else{
  }
  return has_analog_block();
}
/*--------------------------------------------------------------------------*/
std::string const& Filter::dev_type() const
{
  // incomplete: which filter?
  static std::string dt = "incomplete";
  return dt;
}
/*--------------------------------------------------------------------------*/
std::string Filter::state()const
{
  return "_st" + branch_code_name();
}
/*--------------------------------------------------------------------------*/
std::string Filter::short_label()const
{
  return name();
}
/*--------------------------------------------------------------------------*/
Probe const* Filter::prb() const
{
  return _prb;
}
/*--------------------------------------------------------------------------*/
void Filter::set_output(Branch_Ref const& x)
{
  _branch = x;
  assert(owner());
  _prb = owner()->new_probe("_filter", _branch);
  // _prb = owner()->new_probe("potential", _branch);
  assert(_prb);
}
/*--------------------------------------------------------------------------*/
void Block::push_back(Base* c)
{
  List_Base<Base>::push_back(c);
  if(auto v=dynamic_cast<Variable const*>(c)){
      trace1("reg var_ref", v->name());
    _var_refs[v->name()] = c;
  }else{
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Base const* Block::resolve(std::string const& k) const
{
  trace2("resolve", _owner, k);
  const_iterator f = _var_refs.find(k);
  if(f != _var_refs.end()) {
    return f->second;
  }else if(_owner) {
    return _owner->resolve(k);
  }else{
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
