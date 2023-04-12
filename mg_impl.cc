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
/*--------------------------------------------------------------------------*/
bool ConstantMinTypMaxExpression::empty() const
{
  assert(_e);
  return !_e->size();
}
/*--------------------------------------------------------------------------*/
bool Module::has_analog_block() const
{
  return !_analog_list.is_empty();
}
/*--------------------------------------------------------------------------*/
//Probe const* Module::new_probe(std::string const& xs, Branch_Ref const& b)
Probe const* Module::new_probe(std::string const& xs, std::string const& p,
    std::string const& n)
{
  Branch_Ref br = new_branch(p, n);

  std::string nn;
  // incomplete. discipline.h
  if(xs == "I"){
    br->set_flow_probe();
    nn = "flow";
  }else if( xs == "V" ){
    br->set_pot_probe();
    nn = "potential";
  }else{
    nn = xs;
  }

  // TODO: is this needed?
  std::string k = nn + "_" + p + "_" + n;
  Probe*& prb = _probes[k];

  if(prb) {
  }else{
//    size_t s = _probes.size() - 1;
    prb = new Probe(nn, br);
  }

  if(n==p){
  }else if(br.is_reversed()){
    new_probe(xs, n, p);
  }else{
  }

  return prb;
}
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::new_branch(Node const* a, Node const* b)
{
  std::pair p(a, b);
  const_iterator i = _m.find(p);
  if(i!=_m.end()){
    return Branch_Ref(i->second);
  }else{
    std::pair r(b, a);
    const_iterator i = _m.find(r);
    if(i!=_m.end()){
      return Branch_Ref(i->second, true);
    }else{
      _m[p] = new Branch(a,b);
      return Branch_Ref(_m[p]);
    }
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch(std::string const& p, std::string const& n)
{
  Node const* pp = new_node(p);
  Node const* nn = new_node(n);
  return _branches.new_branch(pp, nn);
}
/*--------------------------------------------------------------------------*/
Node* Module::new_node(std::string const& p)
{
  Node*& cc = _nodes[p];
  if(cc) {
  }else{
    cc = new Node(p);
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Node const* Module::node(std::string const& p) const
{
  auto i = _nodes.find(p);
  if(i != _nodes.end()) {
    return i->second;
  }else{
    return NULL;
    throw Exception("no such node " + p );
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
  if(!has_flow_source() && !has_pot_source()){
    incomplete();
    assert(0);
  }else if(has_flow_source() && has_pot_source()){
    incomplete();
    assert(0);
  }

  static std::string f = "flow_src";
  static std::string p = "va_pot";
  if(has_flow_source()){
    return f;
  }else{
    return p;
  }
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
  return _has_pot_src || _has_flow_probe;
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_states() const
{
  size_t k = 2;
  for(auto i : deps()){
    if(i->is_reversed()){
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
  if(!_p->discipline()){
    incomplete();
    return NULL;
  }else if(_p->discipline() == _n->discipline()){
    return _p->discipline();
  }else{ untested();
    incomplete();
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
static int n_filters;
Filter const* Module::new_filter(std::string const& xs, Deps const&d)
{
  std::string filter_code_name = xs + "_" + std::to_string(n_filters++);
  CS cmd(CS::_STRING, filter_code_name + " short_if=0 short_to=0;");
  Port_1* p = new Port_1;
  cmd >> *p;
  _local_nodes.push_back(p);

  Filter* f = new Filter(filter_code_name, d);
  _filters.push_back(f);
  incomplete();
  //
  return f;
}
/*--------------------------------------------------------------------------*/
bool Variable::is_module_variable() const
{
  assert(_owner);
  assert(_owner->owner());
  if(dynamic_cast<Module const*>(_owner)){ untested();
    return true;
  }else{
    return false;
  }
}
/*--------------------------------------------------------------------------*/
void Contribution::set_pot_source()
{
  assert(_branch);
  _branch->set_pot_source();
}
/*--------------------------------------------------------------------------*/
void Contribution::set_flow_source()
{
  assert(_branch);
  _branch->set_flow_source();
}
/*--------------------------------------------------------------------------*/
bool Assignment::is_module_variable() const
{
  assert(_lhs);
  return _lhs->is_module_variable();
}
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
