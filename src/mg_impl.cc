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
bool ConstantMinTypMaxExpression::empty() const
{
  return _e.is_empty();
}
/*--------------------------------------------------------------------------*/
bool Module::has_submodule() const
{
  assert(circuit());
  return !circuit()->element_list().is_empty();
}
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::new_branch(Branch_Ref const& b, std::string name)
{
  assert(name!="");
  assert(b);
  //return new_branch(b->p(), b->n(), name);
  auto m = prechecked_cast<Module*>(owner());
  assert(m);

  auto it = _names.find(name);
  if(it != _names.end()){ untested();
    throw Exception("already there" + name);
  }else{
    trace1("new named branch?", name);
    auto n = new Named_Branch(b, name, m);
    _names.push_back(n);
    _brs.push_back(n);
    if(b.is_reversed()){
    }else{
    }
    return Branch_Ref(n, b.is_reversed());
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref Branch_Map::new_branch(Node_Ref a, Node_Ref b)
{
  std::pair<Node const*, Node const*> p(a, b);
  auto m = prechecked_cast<Module*>(owner());
  assert(m);

  auto ii = _m.find(p);
  if(ii!=_m.end()){
    return Branch_Ref(ii->second);
  }else{
    std::pair<Node const*, Node const*> r(b, a);
    auto i = _m.find(r);
    if(i!=_m.end()){
      return Branch_Ref(i->second, true);
    }else{
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
{
  assert(_circuit);
  Branch_Ref br = _circuit->branches().new_branch(p, n);
  assert(br);
  assert(((Branch const*) br)->owner() == this);
  return br;
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::new_branch(std::string const& p, std::string const& n)
{
  trace2("new_branch", p,n);
  if(p==""){ untested();
    throw Exception("syntax error");
  }else if(p[0] == '<'){ untested();
    incomplete();
    assert(0); // wrong place.
  }else{
  }

  assert(_circuit);
  Branch_Ref a(_circuit->branches().lookup(p) /*, polarity?*/);

  if(a){
    if(n!=""){ untested();
      throw Exception("syntax error");
    }else{
      // polarity??
      return a;
    }
  }else{
    Node_Ref pp = node(p);

    if(pp){
    }else{
      throw Exception_No_Match(p + " does not exist");
    }

    Node_Ref nn;
    if(n==""){
      nn = &Node_Map::mg_ground_node;
    }else if( (nn = node(n)) ){
    }else{ untested();
      throw Exception_No_Match(n + " does not exist");
    }

    return new_branch(pp.mutable_node(*this), nn.mutable_node(*this));
  }
}
/*--------------------------------------------------------------------------*/
Node* Node_Map::new_node(std::string const& p, Block* owner)
{
  Node*& cc = _map[p];
  if(cc) {
  }else{
    // new_ref here?
    cc = new Node(p, int(_nodes.size()));
    owner->new_var_ref(cc);
    _nodes.push_back(cc);
  }
  return cc;
}
/*--------------------------------------------------------------------------*/
Node_Map::Node_Map()
{
  assert(mg_ground_node.number() == 0);
  _nodes.push_back(&mg_ground_node);
}
/*--------------------------------------------------------------------------*/
Node_Map::~Node_Map()
{
  size_t i = 1;
  for(; i < _nodes.size(); ++i){
    delete _nodes[i];
  }
}
/*--------------------------------------------------------------------------*/
Node_Ref Node_Map::operator[](std::string const& key) const
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
  assert(_circuit);
  Node* n = _circuit->nodes().new_node(p, this);
  // new_var_ref(n);
  return n;
}
/*--------------------------------------------------------------------------*/
Node_Ref Module::node(std::string const& p) const
{
  try{
    assert(_circuit);
    return _circuit->nodes()[p];
  }catch(Exception const&){
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
Branch_Ref Module::lookup_branch(std::string const& p) const
{
  assert(_circuit);
  return _circuit->branches().lookup(p);
}
/*--------------------------------------------------------------------------*/
void Filter::new_deps()
{
  assert(!_deps);
  _deps = new TData;
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
size_t Filter::num_states() const
{
  return size_t(_num_states);
}
/*--------------------------------------------------------------------------*/
// BUG: delegate to branch
size_t Filter::num_nodes() const
{
  // slew? BUG
  return 0;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
Discipline const* Branch::discipline() const
{
  assert(_p);
  assert(_n);
  if(_n == &Node_Map::mg_ground_node){
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
Nature const* Branch::nature() const
{ untested();
//  source?
  return NULL;
}
/*--------------------------------------------------------------------------*/
void Node::set_to(Node* p)
{
  if(number()==p->number()) {
  }else{
    _number = p->number();
    Node* n = _next;
    _next = p->_next;
    p->_next = this;
    n->set_to(p);
  }
}
/*--------------------------------------------------------------------------*/
void Node_Map::set_short(Node const* p, Node const* n)
{
  if(p->number() > n->number()){
    std::swap(p, n);
  }else{
  }

  if(p->number() == n->number()){ untested();
  }else{
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
{
  assert(_circuit);
  for(auto& br : _circuit->branches()){
    if(br->is_short()) {
    }else if(br->req_short()) {
      trace4("short", br->name(), br->p()->number(), br->n()->number(), _circuit->net_nodes());
      if(br->p()->number() > int(_circuit->net_nodes())
       ||br->n()->number() > int(_circuit->net_nodes())) {
	_circuit->nodes().set_short(br->p(), br->n());
      }else{
      }
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
Token* Module::new_token(FUNCTION const* f_, size_t num_args)
{
  auto f = prechecked_cast<FUNCTION_ const*>(f_);
  assert(f);
  Token* t = NULL;


  if(f->has_tr_review()){
    auto c = prechecked_cast<Token_CALL*>(t);
    (void)c;
    // incomplete();
   // m.set_tr_review(); // wrong? because token may be deleted later on.
   //                    // inc/dec?
   //                    // do it in token?
   //
   // c->set_used_in(tr_review_tag);
  }else{
  }

  if(f->static_code()){
    // return f->new_token(*this, num_args);
    install(f);
    t = new Token_CALL(f->label(), f);
  }else{
    t = f->new_token(*this, num_args);
  }

  return t;
}
/*--------------------------------------------------------------------------*/
void Branch::set_direct(bool d)
{
  _direct = d;
}
/*--------------------------------------------------------------------------*/
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
  assert(0);
  unreachable(); // cleanup is out of order?
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
  }else if(has_analog_block()) {
  }else if(has_submodule()) {
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
Block* Block::scope() const
{
  if(auto ob = dynamic_cast<Owned_Base*>(_owner)){
    return ob->scope();
  }else if(auto b = dynamic_cast<Block*>(_owner)){
    // incomplete?
    return b;
  }else{
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
Block* Block::scope()
{
  if(auto ob = dynamic_cast<Owned_Base*>(_owner)){
    return ob->scope();
  }else if(auto b = dynamic_cast<Block*>(_owner)){
    // incomplete?
    return b;
  }else if(dynamic_cast<File const*>(_owner)){ untested();
    return NULL;
  }else if(!_owner){
    return NULL;
  }else{ untested();
    assert(0);
    return NULL;
  }
}
/*--------------------------------------------------------------------------*/
void Block::push_back(Base* c)
{
  List_Base<Base>::push_back(c);
  if(auto v=dynamic_cast<Variable_Decl const*>(c)){ untested();
    unreachable();
      trace1("reg var_ref", v->name());
    _var_refs[v->name()] = c; // BUG
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
//    assert(dynamic_cast<Module const*>(this) || dynamic_cast<AnalogFunction const*>(this));
    return NULL;
  }else if(k[0] == '<'){
    if(auto m = dynamic_cast<Module*>(this)){
      std::string portname = k.substr(1, k.size()-2);
      return m->find_port(portname);
    }else{
      return scope()->lookup(k, true);
    }
  }else if(scope()) {
    assert(scope() != this);
    return scope()->lookup(k, true);
  }else if(dynamic_cast<File const*>(this)){
  }else if(auto st = dynamic_cast<Statement*>(owner())){ untested();
    assert(st->scope());
    return st->scope()->lookup(k, true);
  }else{ untested();
    assert(dynamic_cast<File const*>(this));
  }
  return NULL;
}
/*--------------------------------------------------------------------------*/
void Block::clear_vars()
{
      // for(Dep d : deps().ddeps()) { untested();
      //   (*d)->unset_used_in(this);
      // }
  // detach?
  _var_refs.clear();
}
/*--------------------------------------------------------------------------*/
/// set_vr_ref?
bool Block::new_var_ref(Base* what)
{
  assert(what);
  std::string p;
  assert(!dynamic_cast<Variable_Decl const*>(what));
  auto P = dynamic_cast<Parameter_2 const*>(what);
  auto T = dynamic_cast<Token const*>(what);

  if(T){
    p = T->name();
    // assert(T->data());
  }else if(P){
    p = P->name();
  }else if(auto A = dynamic_cast<Aliasparam const*>(what)){ untested();
    unreachable();
    p = A->name();
  }else if(auto ps = dynamic_cast<Paramset_Stmt const*>(what)){ untested();
    unreachable();
    p = "."+ps->name();
  }else if(auto nn = dynamic_cast<Node const*>(what)){
    p = nn->name();
  }else if(auto blk = dynamic_cast<SeqBlock const*>(what)){
    p = blk->identifier().to_string();
  }else{ untested();
    incomplete();
    assert(false);
  }

  trace3("new_var_ref, stashing", p, this, dynamic_cast<Module const*>(this));
  Base*& s = _var_refs[p];
  bool ret = !s;
  s = what;
  return ret;
}
/*--------------------------------------------------------------------------*/
DDeps::const_iterator DDeps::begin() const
{
  return _s.begin();
}
/*--------------------------------------------------------------------------*/
DDeps::const_iterator DDeps::end() const
{
  return _s.end();
}
/*--------------------------------------------------------------------------*/
bool is_true(Expression const& x)
{
  double e = x.eval();
  return e != NOT_INPUT && e;
}
/*--------------------------------------------------------------------------*/
bool is_false(Expression const& x)
{
  double e = x.eval();
  return e != NOT_INPUT && !e;
}
/*--------------------------------------------------------------------------*/
bool is_zero(Expression const& x)
{
  double e = x.eval();
  trace1("is_zero", e);
  return e == 0.;
}
/*--------------------------------------------------------------------------*/
TData* copy_deps(Base const* b)
{
  if(auto t=dynamic_cast<TData const*>(b)){
    return t->clone();
  }else if(!b) {
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
{
  double a = _expression.eval();
  if(a == NOT_INPUT){ untested();
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
  }else{
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
void Node::set_to_ground(Module*)
{
  _fanout.clear();
  _number = 0;
}
/*--------------------------------------------------------------------------*/
void Module::set_to_ground(Node const* n)
{
  trace1("stc", n->number());
  assert(_circuit);
  if(n->number()){
    assert(*(_circuit->nodes().begin() + n->number()) == n);
    (*(_circuit->nodes().begin() + n->number()))->set_to_ground(this);
  }else{
    // already ground
  }
}
/*--------------------------------------------------------------------------*/
Branch const* Branch::output() const
{
  if(auto f = dynamic_cast<MGVAMS_FILTER const*>(_ctrl)){
    return f->output();
  }else{
  }
  return this;
}
/*--------------------------------------------------------------------------*/
Node_Ref Branch::p() const
{
  if(auto f = dynamic_cast<MGVAMS_FILTER const*>(_ctrl)){
    return f->p();
  }else{
  }
  assert(_p); return _p;
}
/*--------------------------------------------------------------------------*/
Node_Ref Branch::n() const
{
  if(auto f = dynamic_cast<MGVAMS_FILTER const*>(_ctrl)){
    return f->n();
  }else{
  }
  assert(_n); return _n;
}
/*--------------------------------------------------------------------------*/
bool Parameter_2_List::is_local()const
{
  // really? ask *begin?
  return _is_local;
}
/*--------------------------------------------------------------------------*/
double /*?*/ Parameter_2::eval() const
{
  if(is_local()) {
    return _default_val.value();
  }else if (value_range_list().size() == 1) {
    return (*value_range_list().begin())->eval();
  }else{
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
double ValueRange::eval() const
{
  if(spec()){
    return spec()->eval();
  }else{ untested();
    return NOT_INPUT;
  }
}
/*--------------------------------------------------------------------------*/
void Module::parse_ports(CS& f)
{
  assert(_circuit);
  _circuit->parse_ports(f);
}
/*--------------------------------------------------------------------------*/
void Module::new_circuit()
{
  assert(!_circuit);
  _circuit = new Circuit();
}
/*--------------------------------------------------------------------------*/
void Module::new_filter()
{
  assert(_circuit);
  _circuit->new_filter();
}
/*--------------------------------------------------------------------------*/
Circuit::~Circuit()
{
  _filters.clear();
  _branch_decl.clear();
  _branches.clear();
}
/*--------------------------------------------------------------------------*/
void Branch_Map::clear()
{
  while(!_names.is_empty()){
    delete _names.back();
    _names.pop_back();
  }
  for(auto& x : _m){
    delete x.second;
//      x = NULL;
  }
  _m.clear();
  _brs.clear();
  // _names.clear();
}
/*--------------------------------------------------------------------------*/
DDeps const& Branch::ddeps()const
{
  assert(_deps);
  return _deps->ddeps();
}
/*--------------------------------------------------------------------------*/
#if 0
void Branch::reg_stmt(AnalogStmt const* r)
{
  assert(r);
#ifndef NDEBUG
  for(auto i : _stmts){
    assert(i != r);
  }
#endif
  _stmts.push_back(r);
}
/*--------------------------------------------------------------------------*/
void Branch::dereg_stmt(AnalogStmt const* r)
{
  assert(r);
  for(auto& i : _stmts){
    if(i == r){
      i = _stmts.back();
      _stmts.resize(_stmts.size()-1);
      return;
    }else{
    }
  }
  assert(0);
  unreachable(); // cleanup is out of order?
}
#endif
/*--------------------------------------------------------------------------*/
ckt_attr attr;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
