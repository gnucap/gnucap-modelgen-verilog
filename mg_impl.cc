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
#include "mg_func.h" // TODO
#include "m_tokens.h" // Deps
#include "mg_options.h" // Deps
/*--------------------------------------------------------------------------*/
Node mg_ground_node;
Dep mg_const_dep(NULL);
/*--------------------------------------------------------------------------*/
bool ConstantMinTypMaxExpression::empty() const
{
  return _e.is_empty();
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
#if 1
Probe const* Module::new_probe(std::string const& xs, Branch_Ref const& br)
{
    trace1("new_probe", br.has_name());
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
  if(xs == flow_xs || xs == "flow"){
    nn = "flow";
  }else if( xs == pot_xs || xs == "potential" ){
    nn = "potential";
//  }else if( xs == "_filter"){ untested();
//   // obsolete?
//    br->set_filter();
  }else{
    trace1("new_probe", xs);
    incomplete(); // port branch?
    nn = xs;
  }

  std::string k = nn + "_" + br.pname() + "_" + br.nname();
  if(br.has_name()){
    k = nn + "_" + br.name();
  }else{
  }
  Probe*& prb = _probes[k];

  if(prb) {
  }else{
    prb = new Probe(nn, br);
  }

  trace1("new_probe", br.has_name());
  return prb;
}
#endif
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::new_branch(Node const* a, Node const* b)
{
  std::pair<Node const*, Node const*> p(a, b);
  auto i = _m.find(p);
  if(i!=_m.end()){
    return Branch_Ref(i->second);
  }else{
    std::pair<Node const*, Node const*> r(b, a);
    auto i = _m.find(r);
    if(i!=_m.end()){
      return Branch_Ref(i->second, true);
    }else{
      auto nb = new Branch(a, b, _m.size());
      nb->set_owner(owner());
      _brs.push_front(nb); // front?
      _m[p] = nb;
  	// br.set_owner(this); // eek.
      return Branch_Ref(nb);
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
  assert(((Branch const*) br)->owner() == this);
  return br;
}
/*--------------------------------------------------------------------------*/
size_t Module::num_branches() const
{
  return branches().size();
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch(std::string const& p, std::string const& n)
{
  trace2("new_branch", p,n);
  if(p==""){ untested();
    throw Exception("syntax error");
  }else if(p[0] == '<'){
    incomplete();
  }else{
  }

  Branch_Ref a = _branch_names.lookup(p);

  if(a){
    if(n!=""){ untested();
      throw Exception("syntax error");
    }else{
      return a;
    }
  }else if(n==""){
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
  if(j){ untested();
    throw Exception("Branch " + n + " already defined\n");
  }else{
    j = r;
    j.set_name(n);
    return j;
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
void Filter::new_deps()
{
  assert(!_deps);
  _deps = new Deps;
}
/*--------------------------------------------------------------------------*/
Filter::Filter(std::string const& name)
  : Element_2(), _name(name)
{
  new_deps();
}
/*--------------------------------------------------------------------------*/
Filter::~Filter()
{
  delete _deps;
  _deps = NULL;
}
/*--------------------------------------------------------------------------*/
std::string Filter::code_name()const
{
  return "_f_" + _name; // name()?
}
/*--------------------------------------------------------------------------*/
// number of *all* branches in the module.
size_t Filter::num_branches() const
{
  return 0;
}
/*--------------------------------------------------------------------------*/
void Branch::new_deps()
{
  assert(!_deps);
  _deps = new Deps;
  assert(_deps->is_linear());
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_branches() const
{
  auto m = prechecked_cast<Module const*>(owner());
  assert(m);
  return m->num_branches();
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_nodes() const
{
  size_t ret=1;

  for(auto i : deps()){
    if(i->branch()->is_short()){ untested();
    }else if(i->branch() == this){
      // self conductance
    }else if(i->is_pot_probe()){
      ++ret;
//     }else if(i->is_filter_probe()){ untested();
//       assert(i->is_pot_probe());
//       unreachable();
//       ++ret;
    }else{
    }
  }
  return 2*ret;
}
/*--------------------------------------------------------------------------*/
void Branch::add_dep(Dep const& b)
{
//  if(b->branch() == this){ untested();
//    _selfdep = true;
//  }else{ untested();
//  }
  // TODO incomplete(); // linear?
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
  static std::string p = "va_pot";
  static std::string pb = "va_pot_br";

  if(is_filter()) {
    return "va_" + _ctrl->label();
  }else if(!is_direct()){
    if(has_pot_source()){
      return pb;
    }else{ untested();
      return "incomplete_dev_type";
    }
  }else if(has_flow_probe()){
    return p;
  }else if(has_pot_source()){
    if(_selfdep){
      return pb;
    }else{
      return p;
    }
  }else if(has_flow_source()){
    return "va_flow";
  }else{ untested();
    return p;
  }
  unreachable();
  return "";
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch_Ref const& b)
    : Owned_Base(b),
      _br(b._br),
      _r(b._r),
      _name(b._name)
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
      _name(b._name)
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
  if(has_name()){
    return "_br_" + *_name;
  }else{
    return _br->code_name();
  }
}
void Branch_Ref::set_name(std::string const& n)
{
  assert(!has_name());
  assert(_br);
  _name = _br->reg_name(n);
}
/*--------------------------------------------------------------------------*/
Branch_Ref& Branch_Ref::operator=(Branch_Ref&& o)
{
  operator=(o);

  if(_br) {
//    assert(_br->has(this));
  }else{ untested();
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
  _name = o._name;

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
std::string const* Branch::reg_name(std::string const&s)
{
  for(auto i: _names){
    assert(i!=s);
  }
  _names.push_back(s);
  return &_names.back();
}
/*--------------------------------------------------------------------------*/
// has_source? is_source?
bool Branch::has_element() const
{
  if(is_short()){ untested();
    return false;
  }else{
    return has_flow_source() || has_pot_source() || has_flow_probe();
  }
}
/*--------------------------------------------------------------------------*/
bool Branch::has_pot_source() const
{
  return _has_pot_src; //  || _has_flow_probe;
}
/*--------------------------------------------------------------------------*/
size_t Filter::num_states() const
{
  return size_t(_num_states);
}
/*--------------------------------------------------------------------------*/
// BUG: delegate to branch
size_t Filter::num_nodes() const
{
    return 0;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
size_t Branch::num_states() const
{
  size_t k = 2;
  // TODO: cleanup
  for(auto i : deps()){
    assert(i);
    // if(i->is_reversed()){ untested();
    //}else
    if(i->branch() == this){
    }else if(i->branch()->is_short()){ untested();
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
Probe::Probe(std::string const& xs, Branch_Ref b) : _br(b)
{
  trace3("::Probe", xs, code_name(), b.has_name());
  // TODO: disciplines.h
  if( (xs == "V") || (xs == "potential") ){
    _type = t_pot;
    _br->set_pot_probe();
  }else if( (xs == "I") || (xs == "flow") ){
    _type = t_flow;
    _br->set_flow_probe();
  }else{ untested();
    unreachable();
  }
}
/*--------------------------------------------------------------------------*/
Probe::~Probe()
{
  if(is_flow_probe()){
    _br->dec_flow_probe();
  }else{
  }
  if(is_pot_probe()){
    _br->dec_pot_probe();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool Probe::is_reversed() const
{
  return _br.is_reversed();
}
/*--------------------------------------------------------------------------*/
Nature const* Branch::nature() const
{ untested();
//  source?
  return NULL;
}
/*--------------------------------------------------------------------------*/
Discipline const* Probe::discipline() const
{
  return _br->discipline();
}
/*--------------------------------------------------------------------------*/
Nature const* Probe::nature() const
{ untested();
  return _br->nature();
}
/*--------------------------------------------------------------------------*/
Module::~Module()
{
  // cleanup
  _branch_names.clear();
  _analog_list.clear();
  _analog_functions.clear();
  _branch_decl.clear();
  for(auto i: _probes){
    delete i.second;
  }
  _probes.clear();
  _filters.clear();
}
/*--------------------------------------------------------------------------*/
Block::~Block()
{
  delete _attributes;
  _attributes = NULL;
}
/*--------------------------------------------------------------------------*/
Token* Module::new_token(FUNCTION_ const* f, size_t num_args)
{
  return f->new_token(*this, num_args);
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
void Branch::set_direct(bool d)
{
  _direct = d;
}
/*--------------------------------------------------------------------------*/
double Assignment::eval() const
{
  return rhs().eval();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_module_variable() const
{
  assert(_lhs);
  return _lhs->is_module_variable();
}
/*--------------------------------------------------------------------------*/
Data_Type const& Assignment::type() const
{
  //assert(_lhs->is_int() == _type.is_int());
  assert(_lhs);
  return _lhs->type();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_int() const
{
  return type().is_int();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Branch::~Branch()
{
  // no, shutting down, not all Refs tidied up.
  assert(!_refs.size());

  // Contributions tidied up
  assert(!_has_pot_src);
  assert(!_has_flow_src);

  // Probes tidied up
  if(_has_pot_probe){ untested();
    unreachable();
  }else if(_has_flow_probe){ untested();
    unreachable();
  }else{
  }
  delete _deps;
  _deps = NULL;
}
/*--------------------------------------------------------------------------*/
void Branch::attach(Branch_Ref* r)
{
  assert(r);
#ifndef NDEBUG
  for(auto i : _refs){
    assert(i != r);
  }
#endif
  _refs.push_back(r);
}
/*--------------------------------------------------------------------------*/
#if 0
bool Branch::has(Branch_Ref* r) const
{ untested();
  assert(r);
  for(auto& i : _refs){ untested();
    if(i == r){ untested();
      return true;
    }else{ untested();
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
  _s.clear();
}
/*--------------------------------------------------------------------------*/
bool Dep::same_data(Dep const& o) const
{
  assert(_prb);
  assert(o._prb);
  return _prb->same_data(*o._prb);
}
/*--------------------------------------------------------------------------*/
std::pair<Deps::const_iterator, bool> Deps::insert(Dep const& x)
{
  for(auto s = begin(); s!=end(); ++s){
    if (s->same_data(x)){
      return std::make_pair(s, false);
    }else{
    }
  }
  _s.push_back(x);
  return std::make_pair(begin()+(int(size())-1), true);
}
/*--------------------------------------------------------------------------*/
bool Module::sync() const
{
#if 0
  // need getattr<bool>?
  if(!has_attr("sync")){ untested();
    true, false? automatic? how
  }else if(attr("sync") == "1"
        || attr("sync") == "yes"){ untested();
    return true;
  }else if(attr("sync") == "0"
        || attr("sync") == "no"){ untested();
  }else{ untested();
    unsupported.
  }
#endif

  if(has_submodule() && has_analog_block()){
    incomplete();
  }else{
  }
  return has_analog_block();
}
/*--------------------------------------------------------------------------*/
std::string Filter::state()const
{
  if(Element_2::state().size()){
    return Element_2::state();
  }else{ untested();
    // BUG?
    return "_st" + branch_code_name();
  }
}
/*--------------------------------------------------------------------------*/
std::string Filter::short_label()const
{
  return name();
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
Base const* Block::lookup(CS& f) const
{
  std::string name;
  size_t here = f.cursor();
  f >> name;
  Base const* b = lookup(name, true);
  if(b){
  }else{
    trace1("cannot lookup", name);
    f.reset_fail(here);
  }
  return b;
}
/*--------------------------------------------------------------------------*/
Base const* Block::lookup(std::string const& k, bool recurse) const
{
  Block* b = const_cast<Block*>(this);
  return b->lookup(k, recurse);
}
/*--------------------------------------------------------------------------*/
Base* Block::lookup(std::string const& k, bool recurse)
{
  trace2("lookup", _owner, k);
  const_iterator f = _var_refs.find(k);
  if(f != _var_refs.end()) {
    return f->second;
  }else if(!recurse) {
    assert(dynamic_cast<Module const*>(this));
    return NULL;
  }else if(_owner) {
    return _owner->lookup(k, true);
  }else{
    assert(dynamic_cast<File const*>(this));
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
Deps::const_iterator Deps::begin() const
{
  return _s.begin();
}
/*--------------------------------------------------------------------------*/
Deps::const_iterator Deps::end() const
{
  return _s.end();
}
/*--------------------------------------------------------------------------*/
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
void Variable_Decl::update_deps(Deps const& d)
{
  assert(&deps() != &d);
  deps().update(d);
}
/*--------------------------------------------------------------------------*/
bool is_true(Expression const& x)
{
  double e = x.eval();
  return e == 1.;
}
/*--------------------------------------------------------------------------*/
bool is_false(Expression const& x)
{
  double e = x.eval();
  return e == 0.;
}
/*--------------------------------------------------------------------------*/
bool is_zero(Expression const& x)
{
  double e = x.eval();
  return e == 0.;
}
/*--------------------------------------------------------------------------*/
Attrib const& Expression_::attrib() const
{
  static Attrib no_attrib;
  if(is_empty()){
    return no_attrib;
  }else if(auto d = dynamic_cast<Attrib const*>(back()->data())){
    return *d;
  }else{
    return no_attrib;
  }
}
/*--------------------------------------------------------------------------*/
Deps const& Expression_::deps() const
{
  static Deps no_deps;
  if(is_empty()){
    return no_deps;
  }else if(auto d = dynamic_cast<Deps const*>(back()->data())){
    return *d;
  }else if(auto d = dynamic_cast<Attrib const*>(back()->data())){ untested();
    return d->deps();
  }else{
    return no_deps;
  }
}
/*--------------------------------------------------------------------------*/
Expression_::~Expression_()
{
//  for(auto i : _deps){ untested();
//    delete i;
//  }
}
/*--------------------------------------------------------------------------*/
Deps* copy_deps(Base const* b)
{
  if(auto t=dynamic_cast<Deps const*>(b)){
    return t->clone();
  }else if(!b) {
    // unary(par_ref)?
  }else{
    incomplete();
    unreachable();
    assert(0);
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
// bool ValueRangeSpec::is_constant() const
// {
//   if(!_what){
//     return false;
//   }else{
//     return _what->is_constant();
//   }
// }
/*--------------------------------------------------------------------------*/
bool ConstExpression::operator==(ConstExpression const& o) const
{
  double a = _expression.eval();
  if(a == NOT_INPUT){
    return false;
  }else{
    return a == o._expression.eval();
  }
}
/*--------------------------------------------------------------------------*/
double ValueRangeInterval::eval() const
{
  if(!lb_is_closed()){
    return NOT_INPUT;
  }else if(!ub_is_closed()){
    return NOT_INPUT;
  }else if(_ub == _lb){
    return _ub.expression().eval();
  }else{ itested();
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
