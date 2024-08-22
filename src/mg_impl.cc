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
#include "mg_.h" //BUG
#include "mg_in.h"
#include "mg_func.h" // TODO
#include "mg_discipline.h"
#include "mg_token.h" // TData
#include "mg_options.h" // TData
/*--------------------------------------------------------------------------*/
Dep mg_const_dep(NULL);
/*--------------------------------------------------------------------------*/
bool ConstantMinTypMaxExpression::empty() const
{ untested();
  return _e.is_empty();
}
/*--------------------------------------------------------------------------*/
bool Module::has_submodule() const
{ untested();
  assert(circuit());
  return !circuit()->element_list().is_empty();
}
/*--------------------------------------------------------------------------*/
#if 1
#endif
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::new_branch(Branch_Ref const& b, std::string name)
{ untested();
  assert(name!="");
  assert(b);
  //return new_branch(b->p(), b->n(), name);
  auto m = prechecked_cast<Module*>(owner());
  assert(m);

  auto it = _names.find(name);
  if(it != _names.end()){ untested();
    throw Exception("already there" + name);
  }else{ untested();
    trace1("new named branch?", name);
    auto n = new Named_Branch(b, name, m);
    _names.push_back(n);
    _brs.push_back(n);
    if(b.is_reversed()){ untested();
    }else{ untested();
    }
    return Branch_Ref(n, b.is_reversed());
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::new_branch(Node_Ref a, Node_Ref b)
{ untested();
  std::pair<Node const*, Node const*> p(a, b);
  auto m = prechecked_cast<Module*>(owner());
  assert(m);

  auto ii = _m.find(p);
  if(ii!=_m.end()){ untested();
    return Branch_Ref(ii->second);
  }else{ untested();
    std::pair<Node const*, Node const*> r(b, a);
    auto i = _m.find(r);
    if(i!=_m.end()){ untested();
      return Branch_Ref(i->second, true);
    }else{ untested();
      auto nb = new Branch(a, b, m);
      nb->set_owner(owner());
      _brs.push_front(nb); // front?
      _m[p] = nb;
      // br.set_owner(this); // eek.
      return Branch_Ref(nb);
    }
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch(Node* p, Node* n = NULL)
{ untested();
  assert(_circuit);
  Branch_Ref br = _circuit->branches().new_branch(p, n);
  assert(br);
  assert(((Branch const*) br)->owner() == this);
  return br;
}
/*--------------------------------------------------------------------------*/
size_t Module::num_branches() const
{ untested();
  assert(_circuit);
  return _circuit->branches().size();
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch(std::string const& p, std::string const& n)
{ untested();
  trace2("new_branch", p,n);
  if(p==""){ untested();
    throw Exception("syntax error");
  }else if(p[0] == '<'){ untested();
    incomplete();
    assert(0); // wrong place.
  }else{ untested();
  }

  assert(_circuit);
  Branch_Ref a(_circuit->branches().lookup(p) /*, polarity?*/);

  if(a){ untested();
    if(n!=""){ untested();
      throw Exception("syntax error");
    }else{ untested();
      // polarity??
      return a;
    }
  }else{
    incomplete();
    Node_Ref pp = node(p);

    if(pp){
    }else{
      throw Exception_No_Match(p + " does not exist");
    }

    Node_Ref nn;
    if(n==""){ untested();
      nn = &Node_Map::mg_ground_node;
    }else if( nn = node(n) ){
    }else{
      throw Exception_No_Match(n + " does not exist");
    }

    return new_branch(pp.mutable_node(*this), nn.mutable_node(*this));
  }
}
/*--------------------------------------------------------------------------*/
Node* Node_Map::new_node(std::string const& p, Block* owner)
{ untested();
  Node*& cc = _map[p];
  if(cc) { untested();
  }else{ untested();
    // new_ref here?
    cc = new Node(p, int(_nodes.size()));
    owner->new_var_ref(cc);
    _nodes.push_back(cc);
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Node_Map::Node_Map()
{ untested();
  assert(mg_ground_node.number() == 0);
  _nodes.push_back(&mg_ground_node);
}
/*--------------------------------------------------------------------------*/
Node_Map::~Node_Map()
{ untested();
  size_t i = 1;
  for(; i < _nodes.size(); ++i){ untested();
    delete _nodes[i];
  }
}
/*--------------------------------------------------------------------------*/
Node_Ref Node_Map::operator[](std::string const& key) const
{ untested();
  auto i = _map.find(key);
  if(i != _map.end()) { untested();
    return i->second;
  }else{ untested();
    throw Exception("no such node " + key );
  }
}
/*--------------------------------------------------------------------------*/
Node* Module::new_node(std::string const& p)
{ untested();
  assert(_circuit);
  Node* n = _circuit->nodes().new_node(p, this);
  // new_var_ref(n);
  return n;
}
/*--------------------------------------------------------------------------*/
Node_Ref Module::node(std::string const& p) const
{ untested();
  try{ untested();
    assert(_circuit);
    return _circuit->nodes()[p];
  }catch(Exception const&){ untested();
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::lookup_branch(std::string const& p) const
{ untested();
  assert(_circuit);
  return _circuit->branches().lookup(p);
}
/*--------------------------------------------------------------------------*/
void Filter::new_deps()
{ untested();
  assert(!_deps);
  _deps = new TData;
}
/*--------------------------------------------------------------------------*/
Filter::Filter(std::string const& name)
  : Element_2(), _name(name)
{ untested();
  new_deps();
}
/*--------------------------------------------------------------------------*/
Filter::~Filter()
{ untested();
  delete _deps;
  _deps = NULL;
}
/*--------------------------------------------------------------------------*/
std::string Filter::code_name()const
{ untested();
  return "_f_" + _name; // name()?
}
/*--------------------------------------------------------------------------*/
// number of *all* branches in the module.
size_t Filter::num_branches() const
{ untested();
  return 0;
}
/*--------------------------------------------------------------------------*/
bool Branch::is_detached_filter() const
{ untested();
  auto f = dynamic_cast<MGVAMS_FILTER const*>( _ctrl);
  return f && f->is_standalone();
}
/*--------------------------------------------------------------------------*/
void Branch::new_deps()
{ untested();
  assert(!_deps);
  _deps = new TData;
  assert(_deps->is_linear());
}
/*--------------------------------------------------------------------------*/
size_t Branch::num_branches() const
{ untested();
  auto m = prechecked_cast<Module const*>(owner());
  assert(m);
  return m->num_branches();
}
/*--------------------------------------------------------------------------*/
void Branch::add_dep(Dep const& b)
{ untested();
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
{ untested();
  return _has_pot_probe;
}
/*--------------------------------------------------------------------------*/
bool Branch::has_flow_probe() const
{ untested();
  return _has_flow_probe;
}
/*--------------------------------------------------------------------------*/
bool Branch::is_generic()const
{ untested();
  if(!is_direct()){ untested();
    if(has_pot_source()){ untested();
      return true;
    }else{ untested();
      incomplete();
    }
  }else if(has_flow_probe()){ untested();
    // return _selfdep;
  }else if(has_pot_source()){ untested();
    if(_selfdep){ untested();
      return true;
    }else{ untested();
    }
  }else if(has_flow_source()){ untested();
  }else{ untested();
  }
  return false;
}
/*--------------------------------------------------------------------------*/
std::string Branch::dev_type()const
{ untested();
//  if( .. attribute .. )?
  if(is_filter()) { untested();
    std::string label = "va_" + _ctrl->label();
    auto pos = label.find_last_of("_");
    return label.substr(0, pos);
  }else if(!is_direct()){ untested();
    if(has_pot_source()){ untested();
      return "va_pot_br";
    }else{ untested();
      return "incomplete_dev_type";
    }
  }else if(has_flow_probe()) { untested();
    return "va_sw"; // ?
  }else if(has_pot_source()){ untested();
    if(_selfdep){ untested();
      return "va_pot_br";
    }else if(has_always_pot() && !has_flow_source()) { untested();
      return "va_pot";
    }else{ untested();
      return "va_sw";
    }
  }else if(has_flow_source()){ untested();
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
{ untested();
  if(_br){ untested();
    _br->attach(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch_Ref&& b)
    : Base(),
      _br(b._br),
      _r(b._r)
{ untested();
  if(_br){ untested();
    _br->attach(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Branch* b, bool reversed)
  : _br(b), _r(reversed)
{ untested();
  if(_br){ untested();
    _br->attach(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::Branch_Ref(Named_Branch* b)
  : _br(b), _r(b->is_reversed())
{ untested();
  if(_br){ untested();
    _br->attach(this);
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref::~Branch_Ref()
{ untested();
  if(_br){ untested();
    _br->detach(this);
    _br = NULL;
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
std::string Branch_Ref::code_name()const
{ untested();
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
{ untested();
  operator=(o);

  if(_br) { untested();
//    assert(_br->has(this));
  }else{ untested();
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
Branch_Ref& Branch_Ref::operator=(Branch_Ref const& o)
{ untested();
  if(_br) { untested();
    _br->detach(this);
  }else{ untested();
  }

  _br = o._br;
  _r = o._r;
  //_name = o._name;

  if(_br) { untested();
    _br->attach(this);
  }else{ untested();
  }

  return *this;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// has_source? is_source?
bool Branch::has_element() const
{ untested();
  if(is_short()){ untested();
    return false;
  }else if( has_flow_source() ){ untested();
    return true;
  }else if( has_pot_source() ){ untested();
    return true;
  }else if( has_flow_probe() ){ untested();
    return true;
  }else{ untested();
    return false;
  }
}
/*--------------------------------------------------------------------------*/
bool Branch::has_pot_source() const
{ untested();
  return _has_pot_src; //  || _has_flow_probe;
}
/*--------------------------------------------------------------------------*/
size_t Filter::num_states() const
{ untested();
  return size_t(_num_states);
}
/*--------------------------------------------------------------------------*/
// BUG: delegate to branch
size_t Filter::num_nodes() const
{ untested();
  // slew? BUG
  return 0;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Discipline const* Branch::discipline() const
{ untested();
  assert(_p);
  assert(_n);
  if(_n == &Node_Map::mg_ground_node){ untested();
    return _p->discipline();
  }else if(!_p->discipline()){ untested();
    return _n->discipline();
  }else if(_p->discipline() == _n->discipline()){ untested();
    return _p->discipline();
  }else{itested();
    incomplete(); // no default.
    // Make sure to specify some discipline for now.
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Nature const* Branch::nature() const
{ untested();
//  source?
  return NULL;
}
/*--------------------------------------------------------------------------*/
void Node::set_to(Node* p)
{ untested();
  if(number()==p->number()) { untested();
  }else{ untested();
    _number = p->number();
    Node* n = _next;
    _next = p->_next;
    p->_next = this;
    n->set_to(p);
  }
}
/*--------------------------------------------------------------------------*/
void Node_Map::set_short(Node const* p, Node const* n)
{ untested();
  if(p->number() > n->number()){ untested();
    std::swap(p, n);
  }else{ untested();
  }

  if(p->number() == n->number()){ untested();
  }else{ untested();
    int j = n->number();
    int i = p->number();
    assert(i < j);

    _nodes[j]->set_to(_nodes[i]);

    assert(p->number() == n->number());
  }
}
/*--------------------------------------------------------------------------*/
// void Circuit::setup_nodes()?
void Module::setup_nodes()
{ untested();
  assert(_circuit);
  for(auto& br : _circuit->branches()){ untested();
    if(br->is_short()) { untested();
    }else if(br->req_short()) { untested();
      trace4("short", br->name(), br->p()->number(), br->n()->number(), _circuit->net_nodes());
      if(br->p()->number() > int(_circuit->net_nodes())
       ||br->n()->number() > int(_circuit->net_nodes())) { untested();
	_circuit->nodes().set_short(br->p(), br->n());
      }else{ untested();
      }
    }else{ untested();
    }
  }
}
/*--------------------------------------------------------------------------*/
Token* Module::new_token(FUNCTION const* f_, size_t num_args)
{ untested();
  auto f = prechecked_cast<FUNCTION_ const*>(f_);
  assert(f);
//  if(dynamic_cast<MGVAMS_FUNCTION const*>(f)){ untested();
    if(f->static_code()){ untested();
      // return f->new_token(*this, num_args);
      install(f);
      return new Token_CALL(f->label(), f);
    }else{ untested();
      return f->new_token(*this, num_args);
    }
//   }else{ untested();
//     return f->new_token(*this, num_args);
//   }
}
/*--------------------------------------------------------------------------*/
// std::string Variable_Decl::code_name() const
std::string Variable_Decl::code_name() const
{ untested();
  if(is_real()){ untested();
    return "_v_" + name();
  }else if(is_int()){ untested();
    return "_v_" + name();
  }else{ untested();
    return "_v_" + name();
  }
  incomplete();
  return name(); //?
}
/*--------------------------------------------------------------------------*/
void Branch::set_direct(bool d)
{ untested();
  _direct = d;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void Branch::attach(Branch_Ref* r)
{ untested();
  assert(r);
#ifndef NDEBUG
  for(auto i : _refs){ untested();
    assert(i != r);
  }
#endif
  _refs.push_back(r);
}
/*--------------------------------------------------------------------------*/
void Branch::detach(Branch_Ref* r)
{ untested();
  assert(r);
  for(auto& i : _refs){ untested();
    if(i == r){ untested();
      i = _refs.back();
      _refs.resize(_refs.size()-1);
      return;
    }else{ untested();
    }
  }
  assert(0);
  unreachable(); // cleanup is out of order?
}
/*--------------------------------------------------------------------------*/
bool Module::sync() const
{ untested();
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

  if(has_submodule() && has_analog_block()){ untested();
  }else if(has_analog_block()) { untested();
  }else if(has_submodule()) { untested();
  }
  return has_analog_block();
}
/*--------------------------------------------------------------------------*/
std::string Filter::state()const
{ untested();
  if(Element_2::state().size()){ untested();
    return Element_2::state();
  }else{ untested();
    // BUG?
    return "_st" + branch_code_name();
  }
}
/*--------------------------------------------------------------------------*/
std::string Filter::short_label()const
{ untested();
  return name();
}
/*--------------------------------------------------------------------------*/
Block* Block::scope() const
{ untested();
  if(auto ob = dynamic_cast<Owned_Base*>(_owner)){ untested();
    return ob->scope();
  }else if(auto b = dynamic_cast<Block*>(_owner)){ untested();
    // incomplete?
    return b;
  }else{ untested();
    unreachable();
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
Block* Block::scope()
{ untested();
  if(auto ob = dynamic_cast<Owned_Base*>(_owner)){ untested();
    return ob->scope();
  }else if(auto b = dynamic_cast<Block*>(_owner)){ untested();
    // incomplete?
    return b;
  }else if(dynamic_cast<File const*>(_owner)){ untested();
    return NULL;
  }else if(!_owner){ untested();
    return NULL;
  }else{ untested();
    assert(0);
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
void Block::push_back(Base* c)
{ untested();
  List_Base<Base>::push_back(c);
  if(auto v=dynamic_cast<Variable_Decl const*>(c)){ untested();
    unreachable();
      trace1("reg var_ref", v->name());
    _var_refs[v->name()] = c; // BUG
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
Base const* Block::lookup(CS& f) const
{ untested();
  std::string name;
  size_t here = f.cursor();
  f >> name;
  Base const* b = lookup(name, true);
  if(b){ untested();
  }else{ untested();
    trace1("cannot lookup", name);
    f.reset_fail(here);
  }
  return b;
}
/*--------------------------------------------------------------------------*/
Base const* Block::lookup(std::string const& k, bool recurse) const
{ untested();
  Block* b = const_cast<Block*>(this);
  return b->lookup(k, recurse);
}
/*--------------------------------------------------------------------------*/
Base* Block::lookup(std::string const& k, bool recurse)
{ untested();
  trace2("lookup", _owner, k);
  const_iterator f = _var_refs.find(k);
  if(f != _var_refs.end()) { untested();
    return f->second;
  }else if(!recurse) { untested();
//    assert(dynamic_cast<Module const*>(this) || dynamic_cast<AnalogFunction const*>(this));
    return NULL;
  }else if(k[0] == '<'){ untested();
    if(auto m = dynamic_cast<Module*>(this)){ untested();
      std::string portname = k.substr(1, k.size()-2);
      return m->find_port(portname);
    }else{ untested();
      return scope()->lookup(k, true);
    }
  }else if(scope()) { untested();
    assert(scope() != this);
    return scope()->lookup(k, true);
  }else if(dynamic_cast<File const*>(this)){ untested();
  }else if(auto st = dynamic_cast<Statement*>(owner())){ untested();
    assert(st->scope());
    return st->scope()->lookup(k, true);
  }else{ untested();
    assert(dynamic_cast<File const*>(this));
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
DDeps::const_iterator DDeps::begin() const
{ untested();
  return _s.begin();
}
/*--------------------------------------------------------------------------*/
DDeps::const_iterator DDeps::end() const
{ untested();
  return _s.end();
}
/*--------------------------------------------------------------------------*/
bool is_true(Expression const& x)
{ untested();
  double e = x.eval();
  return e != NOT_INPUT && e;
}
/*--------------------------------------------------------------------------*/
bool is_false(Expression const& x)
{ untested();
  double e = x.eval();
  return e != NOT_INPUT && !e;
}
/*--------------------------------------------------------------------------*/
bool is_zero(Expression const& x)
{ untested();
  double e = x.eval();
  trace1("is_zero", e);
  return e == 0.;
}
/*--------------------------------------------------------------------------*/
TData* copy_deps(Base const* b)
{ untested();
  if(auto t=dynamic_cast<TData const*>(b)){ untested();
    return t->clone();
  }else if(!b) { untested();
    // unary(par_ref)?
  }else{ untested();
    incomplete();
    unreachable();
    assert(0);
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
// bool ValueRangeSpec::is_constant() const
// { untested();
//   if(!_what){ untested();
//     return false;
//   }else{ untested();
//     return _what->is_constant();
//   }
// }
/*--------------------------------------------------------------------------*/
bool ConstExpression::operator==(ConstExpression const& o) const
{ untested();
  double a = _expression.eval();
  if(a == NOT_INPUT){ untested();
    return false;
  }else{ untested();
    return a == o._expression.eval();
  }
}
/*--------------------------------------------------------------------------*/
double ValueRangeInterval::eval() const
{ untested();
  if(!lb_is_closed()){ untested();
    return NOT_INPUT;
  }else if(!ub_is_closed()){ untested();
    return NOT_INPUT;
  }else if(_ub == _lb){ untested();
    return _ub.expression().eval();
  }else{ untested();
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
void Node::set_to_ground(Module*)
{ untested();
  _fanout.clear();
  _number = 0;
}
/*--------------------------------------------------------------------------*/
void Module::set_to_ground(Node const* n)
{ untested();
  trace1("stc", n->number());
  assert(_circuit);
  if(n->number()){ untested();
    assert(*(_circuit->nodes().begin() + n->number()) == n);
    (*(_circuit->nodes().begin() + n->number()))->set_to_ground(this);
  }else{ untested();
    // already ground
  }
}
/*--------------------------------------------------------------------------*/
Branch const* Branch::output() const
{ untested();
  if(auto f = dynamic_cast<MGVAMS_FILTER const*>(_ctrl)){ untested();
    return f->output();
  }else{ untested();
  }
  return this;
}
/*--------------------------------------------------------------------------*/
Node_Ref Branch::p() const
{ untested();
  if(auto f = dynamic_cast<MGVAMS_FILTER const*>(_ctrl)){ untested();
    return f->p();
  }else{ untested();
  }
  assert(_p); return _p;
}
/*--------------------------------------------------------------------------*/
Node_Ref Branch::n() const
{ untested();
  if(auto f = dynamic_cast<MGVAMS_FILTER const*>(_ctrl)){ untested();
    return f->n();
  }else{ untested();
  }
  assert(_n); return _n;
}
/*--------------------------------------------------------------------------*/
bool Assignment::has_sensitivities()const
{ untested();
  return data().has_sensitivities();
}
/*--------------------------------------------------------------------------*/
bool Parameter_2_List::is_local()const
{ untested();
  // really? ask *begin?
  return _is_local;
}
/*--------------------------------------------------------------------------*/
double /*?*/ Parameter_2::eval() const
{ untested();
  if(is_local()) { untested();
    return _default_val.value();
  }else if (value_range_list().size() == 1) { untested();
    return (*value_range_list().begin())->eval();
  }else{ untested();
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
double ValueRange::eval() const
{ untested();
  if(spec()){ untested();
    return spec()->eval();
  }else{ untested();
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
void Module::parse_ports(CS& f)
{ untested();
  assert(_circuit);
  _circuit->parse_ports(f);
}
/*--------------------------------------------------------------------------*/
void Module::new_circuit()
{ untested();
  assert(!_circuit);
  _circuit = new Circuit();
}
/*--------------------------------------------------------------------------*/
void Module::new_filter()
{ untested();
  assert(_circuit);
  _circuit->new_filter();
}
/*--------------------------------------------------------------------------*/
Circuit::~Circuit()
{ untested();
  _filters.clear();
  _branch_decl.clear();
  _branches.clear();
}
/*--------------------------------------------------------------------------*/
void Branch_Map::clear()
{ untested();
  while(!_names.is_empty()){ untested();
    delete _names.back();
    _names.pop_back();
  }
  for(auto& x : _m){ untested();
    delete x.second;
//      x = NULL;
  }
  _m.clear();
  _brs.clear();
  // _names.clear();
}
/*--------------------------------------------------------------------------*/
DDeps const& Branch::ddeps()const
{ untested();
  assert(_deps);
  return _deps->ddeps();
}
/*--------------------------------------------------------------------------*/
void Branch::reg_stmt(AnalogStmt const* r)
{ untested();
  assert(r);
#ifndef NDEBUG
  for(auto i : _stmts){ untested();
    assert(i != r);
  }
#endif
  _stmts.push_back(r);
}
/*--------------------------------------------------------------------------*/
void Branch::dereg_stmt(AnalogStmt const* r)
{ untested();
  assert(r);
  for(auto& i : _stmts){ untested();
    if(i == r){ untested();
      i = _stmts.back();
      _stmts.resize(_stmts.size()-1);
      return;
    }else{ untested();
    }
  }
  assert(0);
  unreachable(); // cleanup is out of order?
}
/*--------------------------------------------------------------------------*/
ckt_attr attr;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
