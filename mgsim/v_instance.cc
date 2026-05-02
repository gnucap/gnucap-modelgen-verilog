/*                              -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2022, 2023 Felix Salfelder
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
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
 *------------------------------------------------------------------
 * module stuff
 */
#include <globals.h>
#include <io_trace.h>
#include <u_nodemap.h>
#include <e_node.h>
#include <e_paramlist.h>
#include <e_subckt.h>
#include <e_model.h>
#include <e_hsparam.h>
#include <c_comand.h>
#include <set>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
// components with one node are unlikely.
const size_t node_capacity_floor = 2;
/*--------------------------------------------------------------------------*/
static void grow_nodes(int Index, node_t*& n, int& capacity, int capacity_floor)
{
  if(Index < capacity){
  }else{
    int new_capacity = std::max(capacity, capacity_floor);
    while(new_capacity <= Index) {
      assert(new_capacity < new_capacity * 2);
      new_capacity *= 2;
    }
    node_t* new_nodes = new node_t[new_capacity];
    for(int i=0; i<capacity; ++i){
      new_nodes[i] = n[i];
    }
    delete[] n;
    n = new_nodes;
    capacity = new_capacity;
  }
}
/*--------------------------------------------------------------------------*/
class DEV_INSTANCE_PROTO;
class COMMON_INSTANCE : public COMMON_COMPONENT {
  DEV_INSTANCE_PROTO* _proto{nullptr};
public:
  PARAM_LIST _params;
  std::vector<std::string> _port_names;
public:
  COMMON_INSTANCE(int x) : COMMON_COMPONENT(x) {}
  COMMON_INSTANCE(COMMON_INSTANCE const& p) :
    COMMON_COMPONENT(p),
    _params(p._params),
    _port_names(p._port_names) {}
  COMMON_INSTANCE* clone()const override { return new COMMON_INSTANCE(*this); }
  std::string name()const override {unreachable(); return "??";}
  bool operator==(COMMON_COMPONENT const& p)const override {
    if(this == &p){ untested();
      return true;
    }else{
    }

    auto x = prechecked_cast<COMMON_INSTANCE const*>(&p);
    return x
        && _proto == x->_proto
        && _params == x->_params
        && _port_names == x->_port_names
        && _modelname == x->_modelname;
  }
#if __cplusplus >= 202002L
  bool operator<(COMMON_COMPONENT const& p)const override {
    return compare(p) < 0;
  }
  int compare(COMMON_COMPONENT const& p)const override {
    if(this == &p){
      return 0;
    }else{
    }
    int c;
    if((c = COMMON_COMPONENT::compare(p))) {
      return c;
    }else{
    }

    auto x = prechecked_cast<COMMON_INSTANCE const*>(&p);
    assert(x);

    if(intptr_t cp = intptr_t(_proto) - intptr_t(x->_proto)) { untested();
      return cp<0?-1:1;
    }else if((c = _params.compare(x->_params))) {
      return c;
    }else{
    }
    auto ord = _port_names <=> x->_port_names;
    if(ord == std::weak_ordering::less){
      return -1;
    }else if(ord == std::weak_ordering::greater){
      return 1;
    }else{
    }
    return 0;
  }
  bool has_less() const override {return true;}
#endif // __cplusplus >= 202002L
  int set_param_by_name(std::string Name, std::string Value) override {
    if(_params.find(Name) == _params.end()){
      return _params.set(Name, Value);
    }else{
      throw(Exception_Clash("parameter " + Name + " already set"));
    }
  }
  bool param_is_printable(int)const override { return true; }
  std::string param_name(int i)const override
    { assert (i < int(_params.size())); return _params.name(i); }
  std::string param_name(int i, int j)const override
    { assert(!j); return param_name(i);}
  std::string param_value(int i)const override
    { assert (i < int(_params.size())); return _params.value(i); }
  int param_count()const override {return static_cast<int>(_params.size()); }
  void precalc_last(const PARAM_LIST* Scope)override { untested();
    assert(Scope);
    COMMON_COMPONENT::precalc_last(Scope);

    for (int i=0; i<_params.size(); ++i){ untested();
      _params[i].e_val(nullptr, Scope);
    }
  }
public:
  PARAM_LIST* params()override {untested(); return &_params;}
};
/*--------------------------------------------------------------------------*/
static COMMON_INSTANCE* Default_SUBCKT()
{
  static COMMON_INSTANCE ci(CC_STATIC);
  return &ci;
}
/*--------------------------------------------------------------------------*/
// looks like INSTANCE from d_subckt.cc, but isnt.
// this one is never part of a simulation, because of deflation.
// TODO: cleanup/rename?
class INSTANCE : public BASE_SUBCKT {
  friend class DEV_INSTANCE_PROTO; // has to do with _parent.
  node_t* _n{nullptr};
protected: // stub stuff
  INSTANCE const* _cloned_from{nullptr}; // use common/mutable_common instead?
  const COMPONENT* _parent{nullptr};
  mutable DEV_INSTANCE_PROTO* _proto{nullptr}; // use common->proto?
  int _node_capacity{0};
protected:
  explicit	INSTANCE(const INSTANCE&);
public:
  explicit	INSTANCE();
		~INSTANCE();
  CARD*		clone_instance()const override {
    // incomplete();
    return clone();
  }
  CARD*		clone()const override {
    INSTANCE* new_instance = new INSTANCE(*this);

    // BUG?
    new_instance->_parent = (COMPONENT const*)_proto;

    return new_instance;
  }
private: // override virtual
  char		id_letter()const override {return 'X';}
  bool		print_type_in_spice()const override{return true;}
  std::string   value_name()const override {return "#";}
protected:
  int		max_nodes()const override {
    // INT_MAX results in arithmetic overflow in lang_spice
    // (does not seem to work with lang_spice anyway)
    return INT_MAX-2;
  }
public: // ?
  void build_proto() const;
private:

protected:
  void set_port_by_index(int Index, std::string& Value)override;

  // override. the base class does not know about _parent.
  int set_port_by_name(std::string& name, std::string& ext_name)override;
  int		min_nodes()const override	{return 0;}
  int		ext_nodes()const override	{return net_nodes();}
  int		matrix_nodes()const override	{ untested();return 0;}
protected:
  int		net_nodes()const override	{return _net_nodes;}
  void		precalc_first()override;
private:
  bool		makes_own_scope()const override { untested();return false;}

protected:
  void		expand()override;
  CARD*		deflate()override;
private:
  void		expand_sift();
  void		precalc_last()override{ untested();
    trace1("INSTANCE::precalc_last", long_label());
    // unreachable();
  }
  double	tr_probe_num(const std::string&)const override { untested();unreachable(); return 0.;}
  int param_count_dont_print()const override {return 0;}
private: // overrides
  void set_parameters(const std::string& Label, CARD* Parent,
			      COMMON_COMPONENT* Common, double Value,
			      int state_count, double state[],
			      int node_count, const node_t nodes[]) override{
    if(node_count){
      grow_nodes(node_count-1, _n, _node_capacity, node_capacity_floor);
      _net_nodes = short(node_count);
    }else{ untested();
    }
    COMPONENT::set_parameters(Label, Parent, Common, Value, state_count, state,
	node_count, nodes);
  }
  std::string param_name(int i, int j) const override { untested();
    if(j==0){ untested();
      return param_name(i);
    }else{ untested();
      return "";
    }
  }
   std::string param_name(int i) const override {
     // assert(i<int(_params.size()));
     std::string n = BASE_SUBCKT::param_name(i);
     if(n[0]=='*'){
       return "";
     }else{
       return n;
     }
   }
  void set_param_by_index(int I, std::string& Value, int i) override {
    assert(i==0);
    assert(I>=0);
    // BASE_SUBCKT::set_param_by_index(I, Value, 0);
    COMMON_COMPONENT* cc = common()->clone();
    auto cp = prechecked_cast<COMMON_INSTANCE*>(cc);
    assert(cp);
    trace2("spbi", I, Value);

    cp->_params.set("*" + to_string(cp->param_count()), Value);
    attach_common(cp);
  }

private:
  void collect_overloads(INSTANCE* scope) const;
  void collect_overloads_from_scope(std::string const& modelname,
      CARD_LIST const& scope, INSTANCE*)const;
  CARD* prepare_overload(CARD* proto, std::string modelname, INSTANCE* p)const;
  COMPONENT* set_overload_params(COMPONENT* cur)const;

  node_t& n_(int i)const override {
    assert(_n); assert(i>=0); assert(i<_node_capacity); return _n[i];
  }
protected:
  std::string port_name(int i)const override {
    auto c = prechecked_cast<COMMON_INSTANCE const*>(common());
    assert(c);
    if(size_t(i)>=c->_port_names.size()){ untested();
      return ""; // it has no name.
    }else if(c->_port_names[i][0] == '*'){
      return ""; // it has no name.Q
    }else{
      return c->_port_names[i];
    }
  }
protected:
  static std::set<INSTANCE*> & protos() {
    static std::set<INSTANCE*> p;
    return p;
  }
} p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "instance|device_stub|__stub", &p1);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DEV_INSTANCE_PROTO : public INSTANCE {
  explicit	DEV_INSTANCE_PROTO(const DEV_INSTANCE_PROTO&p)
   : INSTANCE(p) { untested();
     new_subckt();
     subckt()->set_verilog_math();
   }
public:
  explicit	DEV_INSTANCE_PROTO() : INSTANCE() {
    new_subckt();
    subckt()->set_verilog_math();
    protos().insert(this);
  }
  ~DEV_INSTANCE_PROTO(){
    protos().erase(this);
  }
private:
  CARD* clone() const override;
  CARD* clone_instance()const override{ untested(); return clone();}
public:
  void precalc_first() override { untested(); unreachable(); }
  void precalc_last() override { untested(); unreachable(); }
  CARD_LIST*	   scope()override { return subckt(); }
  const CARD_LIST* scope()const	override { return subckt(); }

  bool do_tr()override { untested(); unreachable(); return true; }

  void ac_final()override {untested();}
  void dc_final()override {untested();}
  void tr_final()override {untested();}

public:
  std::string port_name(int i)const override {
    auto c = prechecked_cast<COMMON_INSTANCE const*>(common());
    assert(c);
    if(size_t(i)>=c->_port_names.size()){
      trace1("port_name0", i);
      return ""; // it has no name.Y
    }else{
      return c->_port_names[i];
    }
  }
  void set_port_by_index(int Index, std::string& Value)override {
    grow_nodes(Index, _n, _node_capacity, node_capacity_floor);
    BASE_SUBCKT::set_port_by_index(Index, Value);
  }

  int set_port_by_name(std::string&name, std::string&ext_name)override {
    int i = net_nodes();
    auto* cc = prechecked_cast<COMMON_INSTANCE*>(mutable_common()->mutable_clone());
    cc->_port_names.push_back(name);
    attach_common(cc);

    if(subckt()){
    }else{ untested();
    }
    assert(scope());

    if(_proto){ untested();
      { // INSTANCE::set_port_by_index(i, ext_name);
	grow_nodes(i, _n, _node_capacity, node_capacity_floor);
	BASE_SUBCKT::set_port_by_index(i, ext_name);
      }
      _proto->set_port_by_index(i, name);
      assert(scope()!=subckt());
    }else{
      trace4("INSTANCE::pbn proto", long_label(), net_nodes(), name, ext_name);
      set_port_by_index(i, ext_name);
    }

    return i; // TODO: test.
  }
//  int		max_nodes()const	{ untested(); return int(_nodes.size());}

  int set_param_by_name(std::string name, std::string value)override { untested();
    trace3("proto:spbn", long_label(), name, value);
    return 0.; // incomplete.
  }

  static void cleanup();

}pp; // DEV_INSTANCE_PROTO
DISPATCHER<CARD>::INSTALL dd(&device_dispatcher, "instance_proto", &pp);
/*--------------------------------------------------------------------------*/
class attributes_xs : public CKT_BASE{
public:
  ATTRIB_LIST_p& set(tag_t t){
    return set_attributes(t);
  }
  bool has_attributes(tag_t x)const {return CKT_BASE::has_attributes(x); }
  const ATTRIB_LIST_p& attributes(tag_t x)const {return CKT_BASE::attributes(x); }
  ATTRIB_LIST_p&   set_attributes(tag_t x)	{return CKT_BASE::set_attributes(x); }
}attr;
/*--------------------------------------------------------------------------*/
CARD* INSTANCE::prepare_overload(CARD* model, std::string modelname, INSTANCE* Proto) const
{
  assert(Proto);
  assert(Proto->subckt());
  assert(Proto->scope()==Proto->subckt());
  assert(model);
  CARD* cl = model->clone_instance();
  COMPONENT* cur = prechecked_cast<COMPONENT*>(cl);
  assert(cur || !cl);

  if(cl && has_attributes(cl->id_tag())) {
    trace2("INSTANCE::prepare_overload attr?", modelname, attributes(cl->id_tag())->string(tag_t()));
  }else if(cl && has_attributes(model->id_tag())) {
    trace2("INSTANCE::prepare_overload attr?", modelname, attributes(model->id_tag())->string(tag_t()));
    attr.set(cl->id_tag()) = attributes(model->id_tag());
  }else{
    trace1("INSTANCE::prepare_overload no attr", modelname);
  }

  if(!cl){ untested();
    return nullptr;
  }else if(!cur->common()){
    cur->set_dev_type(modelname);
  }else if(auto m=dynamic_cast<MODEL_CARD const*>(model)){
    // bypass spice-style find_model
    trace3("prepare_overload bypass", Proto->short_label(), Proto->net_nodes(), _parent);
    assert(cur->common());
    COMMON_COMPONENT* cc = cur->common()->clone();
    cc->attach(m);
    cc->set_modelname("");
    cur->attach_common(cc);
  }else{
    cur->set_dev_type(modelname);
  }

  std::string label = short_label() + ":p" + std::to_string(Proto->subckt()->size());

  cur->set_owner(Proto);
  cur->set_label(label);
  assert(cur->is_device());

  try {
    for(int i=0; i<Proto->net_nodes(); ++i){
      std::string nn = Proto->port_name(i);
      std::string v = Proto->port_value(i); // .name(value) ..

      trace3("prep", i, nn, v);
      if(nn[0] == '*'){
	cur->set_port_by_index(i, v);
      }else{
	cur->set_port_by_name(v, v);
      }

      if(i<cur->net_nodes()){
	// OK
      }else if(i<cur->net_nodes()+cur->num_current_ports()){
	std::string branch_name = Proto->port_value(i); // v.substr(1);
	cur->set_port_by_index(i, branch_name);
	// assert(0); // later
      }else{ untested();
      }
    }
    if(Proto->net_nodes() < cur->min_nodes()){ untested();
      throw Exception("not enough nodes, have "
	    + std::to_string(Proto->net_nodes()) + " need "
	    + std::to_string(cur->min_nodes()) +"\n");
    }else{
    }
  }catch(Exception const& e){
    trace1("discard", long_label());
    // TODO: include proto name attribute
    error(bLOG, long_label() + " discarded (ports): " + e.message() + "\n");
    cur->purge();
    delete (CARD*) cur;
    cur = nullptr;
  }

  if(cur){
    cur = set_overload_params(cur);
  }else{
  }
  if(cur){
    Proto->subckt()->push_back(cur);
  }else{
  }

  return cur;
} // prepare_overload
/*--------------------------------------------------------------------------*/
COMPONENT* INSTANCE::set_overload_params(COMPONENT* cur) const
{
  try{
    COMMON_COMPONENT const* c = common();
    for(int i=0; i<int(c->param_count()); ++i){
      std::string name = c->param_name(i);
      std::string value = c->param_value(i);
      if(!param_is_printable(i)){ untested();
      }else if(name[0] == '*'){
	cur->set_param_by_index(i, value, 0);
      }else{
	try{
	  cur->set_param_by_name(name, value);
	}catch(Exception_Clash const& e){ untested();
	  throw(Exception_Clash("parameter " + name + " already set"));
	}
      }
    }
  }catch(Exception const& e){
    trace1("discard", long_label());
    // TODO: include proto name attribute
    error(bLOG, long_label() + " discarded (params): " + e.message() + "\n");
    assert(cur);
    cur->purge();
    delete (CARD*) cur;
    cur = nullptr;
  }
  return cur;
}
/*--------------------------------------------------------------------------*/
static std::string get_description(tag_t T)
{
  std::string desc = "";
  if(attr.has_attributes(T)) {
    auto const& a = attr.attributes(T);
    if(a){
      desc = a->operator[](std::string("desc"));
      if(desc == "0"){ untested();
	desc = "";
      }else{
      }
    }else{ untested();
    }
  }else{
  }
  return desc;
}
/*--------------------------------------------------------------------------*/
static void describe_if(tag_t t, std::string const& desc)
{
  if(OPT::picky <= bDEBUG) {
    std::string current = get_description(t);

    if(current.size()){
    }else{
      attr.set_attributes(t).add_to("desc=\"" +desc+ "\"", t);
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void INSTANCE::collect_overloads_from_scope(std::string const& modelname,
    CARD_LIST const& Scope, INSTANCE* Proto) const
{
  if(dynamic_cast<DEV_INSTANCE_PROTO const*>(Proto)){
  }else{ untested();
  }

  {
    CARD_LIST::const_iterator i = Scope.find_(modelname);
    while(i != Scope.end()) {
      std::string desc = get_description((**i).id_tag());

      CARD* p = prepare_overload(*i, modelname, Proto);

      if(&Scope == &CARD_LIST::card_list){
	describe_if(p->id_tag(), modelname + " from top level" + desc);
      }else{
	describe_if(p->id_tag(), "nested " + modelname + " " + desc);
      }

      i = Scope.find_again(modelname, ++i);
    }
  }
}
/*--------------------------------------------------------------------------*/
void INSTANCE::collect_overloads(INSTANCE* Proto) const
{
  if(dynamic_cast<DEV_INSTANCE_PROTO const*>(Proto)){
  }else{ untested();
  }
  if(Proto->subckt()->size()){
    // device has multiple instances
    // BUG: cleanup cache if is_first_expand()
    // incomplete();
    return;
  }else{
  }

  auto const* c = prechecked_cast<COMMON_INSTANCE const*>(Proto->common());
  assert(c);
  assert(c->modelname()!="");
  std::string modelname = c->modelname();

  assert(Proto->scope()==Proto->subckt());
  assert(!Proto->scope()->size());

#ifdef DO_TRACE
  for(auto n : *(Proto->subckt()->nodes())){
    trace1("node", n.first);
  }
#endif

  if (_parent){
    // getting here in modelgen...?
  }else if (modelname == "") { untested();
    throw Exception(Proto->long_label() + ": missing args -- need model name");
  }else if(Proto->subckt()->size()){ untested();
    // how to make reruns safe?
  }else{
    assert(Proto->subckt());
    CARD_LIST const& toplevel = CARD_LIST::card_list;
#if 1
    // assert(owner());
    assert(scope());
    collect_overloads_from_scope(modelname, *scope(), Proto);
#else
    try{ untested();
      CARD const* p = owner()->find_in_my_scope(modelname);
      error(bLOG, long_label() + ": " + modelname + " from my scope\n");
      prepare_overload(p, modelname, Proto);
    }catch (Exception_Cant_Find& e) { untested();
      trace1("no local", modelname);
    }
#endif

    if(&toplevel != scope()){
      collect_overloads_from_scope(modelname, toplevel, Proto);
    }else{
    }

    std::string extended_name = modelname;
    int bin_count = 0;
#if 0
    MODEL_CARD* m = model_dispatcher[modelname];
    while(m){ untested();
      CARD* p = prepare_overload(m, modelname, Proto);
      describe_if(p->id_tag(), extended_name + " from model_dispatcher");
      extended_name = modelname + ':' + to_string(bin_count++);
      m = model_dispatcher[extended_name];
    }
#endif

    CARD* p = device_dispatcher[modelname];
    extended_name = modelname;
    bin_count = 0;
    while(p){
      p = prepare_overload(p, modelname, Proto);
      describe_if(p->id_tag(), extended_name + " from device_dispatcher");
      extended_name = modelname + ':' + to_string(bin_count++);
      p = device_dispatcher[extended_name];
    }

  }

  if(size_t s = Proto->subckt()->size()){
    error(bTRACE, long_label() + ": " + std::to_string(s) + " candidate" + (s>1?"s":"") +
	" found for " +modelname+ "\n");
  }else{
    error(bDANGER, long_label() + ": no candidates found for " +modelname+ "\n");
    // not in precalc
    // throw Exception(long_label() + ": no candiates found for " + modelname);
  }
} // collect_overloads
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
CARD* INSTANCE::deflate()
{
//  return this; // keep it all. for debugging
  trace1("INSTANCE::deflate", long_label());
  CARD_LIST* s = subckt();
  assert(s);
  assert(_parent);
  assert(scope()!=s);
  assert(s->size() && "no candidates in deflate?!");

  if(s->size()){
    auto i = s->begin();
    ++i;
    while(i!=s->end()){ untested();
      auto j = i;
      ++i;
      error(bWARNING, long_label() + " too many candidates.\n");
      s->erase(j);
    }
  }else{ untested();
    // this isnt the correct exception.
    throw Exception_Type_Mismatch("ERROR", long_label(), dev_type());
  }
  assert(s->size()==1); // a random one, for now.

  CARD_LIST::iterator i = s->begin();
  if(i!=s->end()){
    assert(*i);
    auto c = prechecked_cast<COMPONENT*>(*i);
    assert(c);
    {
      *i = nullptr;
      s->erase(i);
      c->set_owner(nullptr);
      c->set_owner(owner());
      c->set_label(short_label());
      c->set_dev_type(dev_type()); // really?
    }

    // rewire nodes.
    assert(c);
    int h = _parent->subckt()->nodes()->how_many();
    for(int a = 0; a < _parent->net_nodes(); ++a) {
      trace3("rewire p", a, _parent->port_name(a), _parent->port_value(a));
    }
    trace3("rewire", long_label(), h, net_nodes());
#if 0
    for(int ii=0; ii<net_nodes(); ++ii){ untested();
      std::string nn = _n[ii].n_()->short_label();
      trace4("rewire", long_label(), ii, nn, c->n_(ii).n_()->short_label());
      trace4("rewire", long_label(), ii, c->n_(ii).e_(), n_(ii).e_());
    }
#endif
    for(int ii=0; ii<c->net_nodes(); ++ii){
      if(ii < c->net_nodes()) {
	if( _parent->port_name(ii)[0] == '*'){
	  trace3("rewire do", ii, c->n_(ii).e_(), _parent->port_name(ii));
//	  assert(ii == c->n_(ii).e_());
	  c->n_(ii) = n_(ii); // why?
	 // c->n_(ii) = n_(c->n_(ii).e_());
	}else if(c->n_(ii).e_() != -1){
	  c->n_(ii) = n_(c->n_(ii).e_());
	  // c->n_(ii) = n_(ii); // why?
	}else{
	}
      }else if(ii < c->net_nodes()+c->num_current_ports()){ untested();
      }else{ untested();
      }
    }

    assert(c->dev_type()!="");
    CARD* deflated = c->deflate();
    if(deflated == c){
      assert(deflated->owner() == owner());
    }else{ untested();
      assert(deflated->owner() == owner());
      // a paramset?
      delete (CARD*) c;
    }
    // assert(deflated->dev_type()==dev_type()); ?
    return deflated;
  }else{ untested();
    throw Exception_No_Match(dev_type());
  }
} // INSTANCE::deflate
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
INSTANCE::INSTANCE()
  :BASE_SUBCKT()
{
  attach_common(Default_SUBCKT());
  assert(_n == nullptr);
}
/*--------------------------------------------------------------------------*/
INSTANCE::INSTANCE(const INSTANCE& p) :
  BASE_SUBCKT(p),
  _cloned_from(&p),
  _parent(p._parent)
{
  trace2("INSTANCE::INSTANCE", p.short_label(), p._net_nodes);
  assert(_net_nodes == p._net_nodes);
  _node_capacity = net_nodes();
  if(_node_capacity){
    _n = new node_t[_node_capacity];
  }else{
    assert(_n == nullptr);
  }
  if(p.is_device()){
    for (int ii = 0;  ii < net_nodes();  ++ii) {
      _n[ii] = p._n[ii];
    }
  }else{ untested();
    for (int ii = 0;  ii < net_nodes();  ++ii) { untested();
      assert(!_n[ii].n_());
    }
  }

  assert(!subckt());

  set_label(p.short_label());
  set_dev_type(p.dev_type());

  if(_parent){ untested();
  }else if(!common()){ untested();
    unreachable();
  }else if(common()->has_model()){ untested();
    trace2("INSTANCE::INSTANCE has model", p.long_label(), p._net_nodes);
    incomplete();
  }else if(&p == &p1){
    // TODO:: use dispatcher["instance_proto"]?
    trace2("INSTANCE::INSTANCE no model", p.short_label(), p._net_nodes);
    _proto = new DEV_INSTANCE_PROTO();
   // _proto->set_owner(nullptr); // reset
   // assert(!_proto->owner());
    assert(_proto->common());
    assert(_proto->subckt());
    assert(_proto->subckt()->params());
  }else{
  }
}
/*--------------------------------------------------------------------------*/
INSTANCE::~INSTANCE()
{
  delete _proto;
}
/*--------------------------------------------------------------------------*/
static int eff_param_count(CARD const* x)
{
  auto c = prechecked_cast<COMPONENT const*>(x);
  assert(c);
  auto common = c->common();
  if(!common){ untested();
    // assert(!c->param_count());
    return c->param_count();
  }else if(HS_PARAM const* h = common->hsparam()){
    return(c->param_count() - h->param_count());
  }else{
    return c->param_count();
  }
}
/*--------------------------------------------------------------------------*/
static std::string param_count_string(CARD const* c)
{
  return to_string(eff_param_count(c));
}
/*--------------------------------------------------------------------------*/
void INSTANCE::expand()
{
  BASE_SUBCKT::expand();
  auto c = prechecked_cast<COMMON_INSTANCE*>(mutable_common());
  assert(c);

  assert(_parent);
  assert(_parent->subckt());
  assert(_parent->subckt()->nodes());
  trace3("INSTANCE::expand", long_label(), _parent->net_nodes(),  _parent->subckt()->nodes()->how_many());
  if(_parent->net_nodes() <= _parent->subckt()->nodes()->how_many()){
    // module
  }else{
    // modelgen
  }
  assert(_parent->subckt()->params());

  if(!_parent->scope()->size()){
    std::string modelname = c->modelname();
    throw Exception(long_label() + ": no valid prototype found for " + modelname);
  }else {
    assert(_sim->is_first_expand());
    // PARAM_LIST const* pl = _parent->subckt()->params();
    // assert(pl);
    // c->_params.set_try_again(pl);

    // here: candidate has too many ports.
    renew_subckt(_parent, &(c->_params)); // pass owner?
    assert(scope()!=subckt());
    // subckt()->attach_params(&(c->_params), scope());
  }

  trace3("INSTANCE::expand sckt in", long_label(), subckt()->size(), _sim->is_first_expand());
  // assert(subckt()->size());
  subckt()->set_owner(nullptr);
  subckt()->set_verilog_math();
  subckt()->set_owner(owner()); // TODO: renew_subckt with alternative owner?
  subckt()->precalc_first(); // here?

  expand_sift();
  subckt()->expand();
}
/*--------------------------------------------------------------------------*/
// sift. move to CARD_LIST::expand?
void INSTANCE::expand_sift()
{
  // tie break rules
  // - The paramset with the fewest number of un-overridden parameters shall be selected.
  // - The paramset with the greatest number of local parameters with specified ranges shall be selected.
  // - The paramset with the fewest ports not connected in the instance line shall be selected.
  COMPONENT* gotit = nullptr;
  int gval = 0;
  for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ){
    CARD const* s = *i;
    COMPONENT const* d = dynamic_cast<COMPONENT const*>(s);
    assert(d);
    CARD_LIST::iterator j = i;
    ++i;

    std::string desc = get_description(s->id_tag());
    int dval = d->is_valid();

    // error(bTRACE, long_label() + " got \"" + desc + "\" valid " + to_string(dval) + ".\n");
    if(!dval){
      error(bDEBUG, long_label() + " dropped invalid candidate: \"" + desc + "\".\n");
    }else if(!gotit){
//      error(bTRACE, long_label() + " found valid candidate.\n");
      gotit = prechecked_cast<COMPONENT*>(*j);
      gval = dval;
      assert(gotit);
      *j = nullptr;
    }else if(eff_param_count(d) > eff_param_count(gotit)){ untested();
      if(desc.size()){ untested();
	error(bLOG, long_label() + " rejecting candidate, more params: \"" + desc + "\".\n");
      }else{ untested();
	error(bLOG, long_label() + " tie break: " + param_count_string(gotit) + " vs. " +
	    param_count_string(d) + "\n");
      }
    }else if(eff_param_count(d) < eff_param_count(gotit)){
      if(desc.size()){
	error(bLOG, long_label() + " found fewer params in \"" + desc + "\".\n");
      }else{
	error(bLOG, long_label() + " tie break: " + param_count_string(gotit) + " vs. " +
	    param_count_string(d) + "\n");
      }
      assert(gotit);
      gotit->purge();
      delete (CARD*) gotit;
      gotit = prechecked_cast<COMPONENT*>(*j);
      gval = dval;
      assert(gotit);
      *j = nullptr;
    }else if(gval < dval) {
      error(bLOG, long_label() + " valid value tie break: " + to_string(gval) + " vs. " +
	  to_string(dval) + "\n");
      assert(gotit);
      gotit->purge();
      delete (CARD*) gotit;
      gotit = prechecked_cast<COMPONENT*>(*j);
      gval = dval;
      assert(gotit);
      *j = nullptr;
    }else if(d->max_nodes() > gotit->max_nodes()){ untested();
      error(bLOG, long_label() + " port tie break: " + to_string(gotit->max_nodes()) + " vs. " +
	  to_string(d->max_nodes()) + "\n");
    }else if(d->max_nodes() < gotit->max_nodes()){
      error(bLOG, long_label() + " port tie break: " + to_string(gotit->max_nodes()) + " vs. " +
	  to_string(d->max_nodes()) + "\n");
      assert(gotit);
      gotit->purge();
      delete (CARD*) gotit;
      gotit = prechecked_cast<COMPONENT*>(*j);
      gval = dval;
      assert(gotit);
      *j = nullptr;
    }else if(desc.size()){
      error(bWARNING, long_label() + ": ambiguous " + dev_type() + ": \"" + desc + "\"\n");
    }else{
      error(bWARNING, long_label() + ": ambiguous overload for in " + dev_type() + "\n");
    }
    subckt()->erase(j);
  }
  if(gotit){
    subckt()->push_back(gotit);
    if(has_attributes(gotit->id_tag())) {
      trace2("INSTANCE got it attr?", gotit->long_label(), attributes(gotit->id_tag())->string(tag_t()));
    }else{
      trace1("INSTANCE got it attr?", gotit->long_label());
    }
  }else{
  }

  if(subckt()->size()==0){
    // reachable?
    throw Exception(long_label() + ": no candidates " + dev_type());
  }else if(subckt()->size()==1){
    COMPONENT* d = dynamic_cast<COMPONENT*>(*subckt()->begin());
    assert(d);
    assert(d->is_valid());
    d->set_label(short_label());
    d->set_dev_type(dev_type()); // make spice happier..
    std::string desc = get_description(d->id_tag());
    if(desc.size()){
      error(bDEBUG, long_label() + " is \"" + desc + "\"\n");
    }else{
      error(bDEBUG, long_label() + " is anonymous " + dev_type() + "\n");
    }
  }else{ untested();
    // TODO: include name attributes, once available
    throw Exception(long_label() + ": ambiguous overload: " + dev_type());
  }
}
/*--------------------------------------------------------------------------*/
// Kludge: build proto in stub, so it only needs doing once.
void INSTANCE::precalc_first()
{
  assert(common());
  trace3("INSTANCE::precalc_first", short_label(), _parent, common()->modelname());
  trace1("INSTANCE::precalc_first", _sim->is_first_expand());

  if(!owner()){
    build_proto();
    _parent = _proto; // common->proto?
  }else if(_cloned_from){
    _cloned_from->build_proto();
  }else{ untested();
  }

  if(_parent){
    trace2("INSTANCE::precalc_first w/ parent", short_label(), _parent->short_label());
  }else{ untested();
  }
  // a device in a module instance
  BASE_SUBCKT::precalc_first();

  assert(!is_constant()); /* because I have more work to do */
}
/*--------------------------------------------------------------------------*/
void INSTANCE::build_proto() const
{
  if(!_proto){ untested();
    // static instance?
  }else{
    assert(scope());
    assert(scope()->params());
    assert(_proto->subckt());
    assert(_proto->subckt()->params());

//    delete _proto;
//    _proto = new DEV_INSTANCE_PROTO;
    _proto->attach_common(common()->clone());
    // HERE: set ports.
    auto c = prechecked_cast<COMMON_INSTANCE const*>(common());
    assert(c);
    for(int i = 0; i<int(c->_port_names.size()); ++i) {
      std::string v = c->_port_names[i];
      if(c->_port_names.size()){
	assert(c->_port_names[i] == v);
	trace2("DBG", v, c->_port_names[i]);
//	_proto->set_port_by_name(v, v);
      }else{
//	_proto->set_port_by_name(v, v);
	trace2("DBG", v,v); // , _port_names[i]);
      }
 //     _proto->set_port_by_name(v, v);
    }

    collect_overloads(_proto);
  }
}
/*--------------------------------------------------------------------------*/
CARD* DEV_INSTANCE_PROTO::clone()const
{ untested();
  INSTANCE* new_instance = dynamic_cast<INSTANCE*>(p1.clone());
  assert(!new_instance->subckt());
  assert(common());

  if (this == &pp){ untested();
    // cloning from static, empty model
    // look out for _parent in expand
  }else if(common()->has_model()){ untested();
    incomplete(); // ?
  }else{ untested();
    new_instance->_parent = this;
  }

  assert(new_instance->is_device());
  return new_instance;
}
/*--------------------------------------------------------------------------*/
void INSTANCE::set_port_by_index(int Index, std::string& Value)
{
  trace3("instance spbi", long_label(), Index, Value);
  grow_nodes(Index, _n, _node_capacity, node_capacity_floor);
  BASE_SUBCKT::set_port_by_index(Index, Value);

  if(!_parent){
    assert(_proto);

    std::string n = "*unnamed_port_" + std::to_string(Index);
    trace3("proto fwd", long_label(), n, Value);
    _proto->set_port_by_name(n, Value);
    assert(common());
    auto* cc = prechecked_cast<COMMON_INSTANCE*>(mutable_common()->mutable_clone());
    assert(cc);
#if 1
    cc->_port_names.resize(Index+1);
    cc->_port_names[Index] = n;
#endif
    attach_common(cc);
  }else{ untested();
    incomplete();
  }
}
/*--------------------------------------------------------------------------*/
int INSTANCE::set_port_by_name(std::string& name, std::string& ext_name)
{
  int i = net_nodes();
  auto* cc = prechecked_cast<COMMON_INSTANCE*>(mutable_common()->mutable_clone());
  cc->_port_names.resize(i+1);
  cc->_port_names[i] = name;
  attach_common(cc);

  if(subckt()){ untested();
  }else{
  }
  assert(scope());

  { // INSTANCE::set_port_by_index(i, ext_name);
    grow_nodes(i, _n, _node_capacity, node_capacity_floor);
    BASE_SUBCKT::set_port_by_index(i, ext_name);
  }

  if(_proto){
    _proto->set_port_by_index(i, name);
    assert(scope()!=subckt());
  }else{ untested();
    trace4("INSTANCE::pbn proto", long_label(), net_nodes(), name, ext_name);
    set_port_by_index(i, ext_name);
  }

  return i; // TODO: test.
}
/*--------------------------------------------------------------------------*/
void DEV_INSTANCE_PROTO::cleanup()
{
  for(auto i : protos() ){
    assert(i);
    assert(i->subckt());
    i->subckt()->erase_all();
  }
}
/*--------------------------------------------------------------------------*/
// TODO: need a better stash and mechanism
class CLEANUP : public CMD {
  void do_it(CS&, CARD_LIST* Scope)override {
    DEV_INSTANCE_PROTO::cleanup(); // TODO: more generic approach
    switch (ENV::run_mode) {
    case rPRE_MAIN:
      // unreachable(); // call from DETACH_HACK
      exit(0);
      break;
    case rINTERACTIVE:
	// fall through
    case rSCRIPT:
	// fall through
    case rBATCH:        command("clear", Scope); exit(0); break;
    case rPRESET:       untested(); /*nothing*/ break;
    }
  }
}p3;
DISPATCHER<CMD>::INSTALL d3(&command_dispatcher, "quit|exit", &p3);
/*--------------------------------------------------------------------------*/
// need this, because "quit" is not called from main.cc
// this effectively breaks "detach_all". don't use it for now.
class DETACH_HACK : public CMD {
  void do_it(CS&, CARD_LIST* Scope)override {
    // CMD::command("detach_all:0", Scope); segfault, currently executed code
    CMD::command("quit", Scope);
  }
}p3b;
DISPATCHER<CMD>::INSTALL d3_hack(&command_dispatcher, "detach_all", &p3b);
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
