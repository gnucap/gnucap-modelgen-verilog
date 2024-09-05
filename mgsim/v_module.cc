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
#include <u_nodemap.h>
#include <e_node.h>
#include <globals.h>
#include <e_paramlist.h>
#include <e_subckt.h>
#include <io_trace.h>
/*--------------------------------------------------------------------------*/
namespace{
/*--------------------------------------------------------------------------*/
// components with one node are unlikely.
const size_t node_capacity_floor = 2;
/*--------------------------------------------------------------------------*/
static void grow_nodes(size_t Index, node_t*& n, size_t& capacity, size_t capacity_floor)
{
  if(Index < capacity){
  }else{
    size_t new_capacity = std::max(capacity, capacity_floor);
    while(new_capacity <= Index) {
      assert(new_capacity < new_capacity * 2);
      new_capacity *= 2;
    }
    node_t* new_nodes = new node_t[new_capacity];
    for(size_t i=0; i<capacity; ++i){
      new_nodes[i] = n[i];
    }
    delete[] n;
    n = new_nodes;
    capacity = new_capacity;
  }
}
/*--------------------------------------------------------------------------*/
static COMMON_PARAMLIST Default_SUBCKT(CC_STATIC);
/*--------------------------------------------------------------------------*/
class DEV_MODULE : public BASE_SUBCKT {
private:
  friend class DEV_SUBCKT_PROTO;
  const BASE_SUBCKT* _parent;
  size_t _node_capacity;
protected:
  explicit	DEV_MODULE(const DEV_MODULE&);
public:
  explicit	DEV_MODULE(COMMON_COMPONENT* c);
		~DEV_MODULE()		{ delete[] _n; _node_capacity = 0; }
  CARD*		clone()const override;
private:
  void		set_port_by_index(int Index, std::string& Value) override;
  // void	set_port_by_name(std::string&, std::string&) override;
private: // override virtual
  bool		is_device()const override	{return _parent;}
  char		id_letter()const override	{ untested();return 'X';}
  bool		print_type_in_spice()const override {return true;}
  std::string   value_name()const override	{ untested();return "#";}
  int		max_nodes()const override;
  int		ext_nodes()const override {return int(_node_capacity);}
  int		min_nodes()const override;
  int		matrix_nodes()const override	{return 0;}
  // int	net_nodes()const override	{ untested();return _net_nodes;}
  void		precalc_first()override;
  bool		makes_own_scope()const override;
  bool		is_valid()const override;
  CARD_LIST*	   scope()override;
  const CARD_LIST* scope()const override	{return const_cast<DEV_MODULE*>(this)->scope();}

  void		expand()override;

private: // no ops for prototype
  void map_nodes()override	{if(is_device()){ BASE_SUBCKT::map_nodes();}else{} }
  void tr_begin()override	{if(is_device()){ BASE_SUBCKT::tr_begin();}else{} }
  void tr_restore()override	{if(is_device()){ BASE_SUBCKT::tr_restore();}else{} }
  void ac_begin()override	{if(is_device()){ BASE_SUBCKT::ac_begin();}else{} }
  void tr_load()override	{if(is_device()){ BASE_SUBCKT::tr_load();}else{} }
  void tr_accept()override	{if(is_device()){ BASE_SUBCKT::tr_accept();}else{} }
  void tr_regress()override	{if(is_device()){ BASE_SUBCKT::tr_regress();}else{} }
  void tr_advance()override	{if(is_device()){ BASE_SUBCKT::tr_advance();}else{} }
  TIME_PAIR tr_review() override{if(is_device()){ return BASE_SUBCKT::tr_review();}else{
                                                  return TIME_PAIR(NEVER, NEVER);}}
  void dc_advance()override	{if(is_device()){ BASE_SUBCKT::dc_advance();}else{} }
  void do_ac()override		{if(is_device()){ BASE_SUBCKT::do_ac();}else{} }
  void ac_load()override	{if(is_device()){ BASE_SUBCKT::ac_load();}else{} }
  void tr_queue_eval()override{
    if(is_device()){
      BASE_SUBCKT::tr_queue_eval();
    }else{
    }
  }
  bool do_tr() override		{if(is_device()){ return BASE_SUBCKT::do_tr();}else{ return true;} }

  bool tr_needs_eval()const override{ untested();
    if(is_device()){untested();
      return BASE_SUBCKT::tr_needs_eval();
    }else{untested();
      return false;
    }
  }
private:
  void		precalc_last()override;
  double	tr_probe_num(const std::string&)const override;
  int param_count_dont_print()const override {return common()->COMMON_COMPONENT::param_count();}

  std::string port_name(int i)const override;
  int set_param_by_name(std::string Name, std::string Value)override;
} p0(&Default_SUBCKT);
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, "module", &p0);
/*--------------------------------------------------------------------------*/
//BUG// fixed limit on number of ports
#define PORTS_PER_SUBCKT 100
/*--------------------------------------------------------------------------*/
class DEV_SUBCKT_PROTO : public DEV_MODULE {
private:
  explicit	DEV_SUBCKT_PROTO(const DEV_SUBCKT_PROTO&p);
public:
  explicit	DEV_SUBCKT_PROTO(COMMON_COMPONENT* c=NULL);
		~DEV_SUBCKT_PROTO(){}
public: // override virtual
  char		id_letter()const override	{untested();return '\0';}
  CARD*		clone_instance()const override;
  bool		print_type_in_spice()const override { untested();unreachable(); return false;}
  std::string   value_name()const override	{untested();incomplete(); return "";}
  std::string   dev_type()const override	{ return "";}
  int		max_nodes()const override	{return PORTS_PER_SUBCKT;}
  int		min_nodes()const override	{return 0;}
  int		matrix_nodes()const override	{untested();return 0;}
  int		net_nodes()const override	{return _net_nodes;}
  CARD*		clone()const override		{return new DEV_SUBCKT_PROTO(*this);}
  bool		is_device()const override	{return false;}
  bool		makes_own_scope()const override	{return true;}
  CARD_LIST*	   scope()override		{untested();return subckt();}
  const CARD_LIST* scope()const override	{return subckt();}
private: // no-ops for prototype
  void precalc_first()override {}
  void expand()override {}
  void precalc_last()override {}
  void map_nodes()override {}
  void tr_begin()override {}
  void tr_load()override {}
  TIME_PAIR tr_review() override { return TIME_PAIR(NEVER, NEVER);}
  void tr_accept()override {}
  void tr_advance()override {}
  void tr_restore()override { untested();}
  void tr_regress()override {}
  void dc_advance()override {}
  void ac_begin()override {}
  void do_ac()override {}
  void ac_load()override {}
  bool do_tr()override { untested(); return true;}
  bool tr_needs_eval()const override {untested(); return false;}
  void tr_queue_eval()override {}
  std::string port_name(int i)const override{ untested();return port_value(i);}
} pp(&Default_SUBCKT);
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "X|subckt", &pp);
/*--------------------------------------------------------------------------*/
DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO(const DEV_SUBCKT_PROTO& p)
  :DEV_MODULE(p)
{
  new_subckt();
  attach_common(NULL);
  attach_common(p.common()->clone());
}
/*--------------------------------------------------------------------------*/
DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO(COMMON_COMPONENT* c)
  :DEV_MODULE(c)
{
  new_subckt();
}
/*--------------------------------------------------------------------------*/
CARD* DEV_SUBCKT_PROTO::clone_instance()const
{
  DEV_MODULE* new_instance = dynamic_cast<DEV_MODULE*>(p0.clone());
  // assert(!new_instance->subckt());

  if (this == &pp){ untested();
    // cloning from static, empty model
    // look out for _parent in expand
    assert(!new_instance->_parent);
    assert(!new_instance->is_device());
    new_instance->_parent = this;
  }else{
    new_instance->_parent = this;
  }
  assert(new_instance->is_device());

  return new_instance;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DEV_MODULE::set_port_by_index(int Index, std::string& Value)
{
  grow_nodes(Index, _n, _node_capacity, node_capacity_floor);
  BASE_SUBCKT::set_port_by_index(Index, Value);
}
/*--------------------------------------------------------------------------*/
int DEV_MODULE::max_nodes() const
{
  if(_parent == &pp){ untested();
    // spice..
    return _parent->max_nodes();
    return PORTS_PER_SUBCKT;
  }else if(_parent){
    return static_cast<CARD const*>(_parent)->net_nodes();
  }else{
    // allow one more, building a prototype.
    return net_nodes()+1;
  }
}
/*--------------------------------------------------------------------------*/
int DEV_MODULE::min_nodes() const
{
  if(_parent){
    return ((CARD const*)_parent)->net_nodes();
  }else{
    return 0;
  }
}
/*--------------------------------------------------------------------------*/
CARD_LIST* DEV_MODULE::scope()
{
  if(_parent == &pp && 0) { untested();
    return subckt();
  }else if(is_device()){
    return COMPONENT::scope();
  }else{
    return subckt();
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_MODULE::is_valid() const
{
  trace1("DEV_MODULE::is_valid", long_label());
  assert(subckt());
  assert(_parent);
  assert(_parent->subckt());
  PARAM_LIST const* params = _parent->subckt()->params();
  PARAMETER<double> v = params->deep_lookup("_..is_valid");
  trace1("DEV_MODULE::is_valid I", v.string());
  double x = v.e_val(1., subckt());
  return x==1.;
}
/*--------------------------------------------------------------------------*/
CARD* DEV_MODULE::clone()const
{
  DEV_MODULE* new_instance = new DEV_MODULE(*this);
  assert(!new_instance->subckt());

  if (this == &p0){
    // cloning from static, empty model
    // has no parent.
    new_instance->new_subckt(); // from DEV_SUBCKT_PROTO::DEV_SUBCKT_PROTO
  }else if(_parent){
    new_instance->_parent = _parent;
    assert(new_instance->is_device());
  }else{
    new_instance->_parent = this;
    assert(new_instance->is_device());
  }

  return new_instance;
}
/*--------------------------------------------------------------------------*/
DEV_MODULE::DEV_MODULE(COMMON_COMPONENT* c)
  :BASE_SUBCKT(c),
   _parent(NULL), _node_capacity(0)
{
  assert(_n == NULL);
}
/*--------------------------------------------------------------------------*/
DEV_MODULE::DEV_MODULE(const DEV_MODULE& p)
  :BASE_SUBCKT(p),
   _parent(p._parent)
{
  trace2("DEV_MODULE::DEV_MODULE", long_label(), net_nodes());
  _node_capacity = net_nodes();
  if(_node_capacity){
    _n = new node_t[_node_capacity];
  }else{
    assert(_n == NULL);
  }
  if(p.is_device()){
    for (int ii = 0;  ii < net_nodes();  ++ii) {
      _n[ii] = p._n[ii];
    }
  }else{
    for (int ii = 0;  ii < net_nodes();  ++ii) {
      assert(!_n[ii].n_());
    }
  }
  assert(!subckt());
}
/*--------------------------------------------------------------------------*/
int DEV_MODULE::set_param_by_name(std::string Name, std::string Value)
{
  assert(_parent);
  assert(_parent->subckt());

  if (Umatch(Name, "$mfactor ")) {
//    m->set_param_by_name("$mfactor", "");
    int x = BASE_SUBCKT::set_param_by_name("$mfactor", Value);
    trace2("DEV_MODULE::spbn", long_label(), Value);
    return x;
  }else if(_parent==&pp) { untested();
    // spice.
    trace2("spice spbn", Name, Value);
    int i = BASE_SUBCKT::set_param_by_name(Name,Value);
    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    for(auto p : c->_params){ untested();
      trace2("spbn param spice", p.first, p.second.string());
    }
    return i;
  }else{
    PARAM_LIST::const_iterator p = _parent->subckt()->params()->find(Name);
    if(p != _parent->subckt()->params()->end()){
      return BASE_SUBCKT::set_param_by_name(Name,Value);
    }else{
      throw Exception_No_Match(Name);
    }
  }
}
/*--------------------------------------------------------------------------*/
std::string DEV_MODULE::port_name(int i)const
{
  if (const DEV_MODULE* p=dynamic_cast<const DEV_MODULE*>(_parent)) {
    if (i<p->net_nodes()){
      return p->port_value(i);
    }else{untested();
      return "";
    }
  }else if(_parent) { untested(); untested();
    // reachable?
    return "";
  }else if(i<net_nodes()) {
    return port_value(i);
  }else{
    return "";
  }
}
/*--------------------------------------------------------------------------*/
void DEV_MODULE::expand()
{
  BASE_SUBCKT::expand();

  if(_parent == &pp){ untested();
    COMMON_PARAMLIST const* c = prechecked_cast<COMMON_PARAMLIST const*>(common());
    assert(c);
    for(auto p : c->_params){ untested();
      trace2("expand param spice", p.first, p.second);
    }
    // first time spice
    assert(c->modelname()!="");
    const CARD* model = find_looking_out(c->modelname());
    if ((_parent = dynamic_cast<const DEV_SUBCKT_PROTO*>(model))) { untested();
      // good
    }else if (dynamic_cast<const BASE_SUBCKT*>(model)) { untested();
      throw Exception_Type_Mismatch(long_label(), c->modelname(), "subckt proto");
    }else{ untested();
      throw Exception_Type_Mismatch(long_label(), c->modelname(), "subckt");
    }
    assert(!_parent->is_device()); // really?
  }else{
  }

  if(!_parent || _parent==&pp){
    // not a device. probably a prototype
  }else{
    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    assert(c);
    assert(_parent);
    assert(_parent->subckt());
    assert(_parent->subckt()->params());
    PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
    assert(pl);
    c->_params.set_try_again(pl);
    for(auto p : c->_params){
      trace3("expand param", p.first, p.second, p.second.string());
    }

    renew_subckt(_parent, &(c->_params));
    subckt()->expand();

#if 1 // move to CARD_LIST::expand?
    for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ++i){
      CARD* d = (*i)->deflate();

      if(d == (*i)){
      }else{
	assert(d->owner() == this);
	delete *i;
	*i = d;
      }
    }
#endif
  }
}
/*--------------------------------------------------------------------------*/
void DEV_MODULE::precalc_first()
{
  BASE_SUBCKT::precalc_first();

  if (subckt()) {
  }else{
    new_subckt();
  }
  trace3("DEV_MODULE::precalc_first", long_label(), my_mfactor(), subckt()->size());

  COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
  assert(c);

  if(_parent == &pp && 0){ untested();
  }else if(_parent){
    PARAM_LIST* pl = const_cast<PARAM_LIST*>(_parent->subckt()->params());
    assert(pl);
    c->_params.set_try_again(pl);

    subckt()->attach_params(&(c->_params), scope());
    subckt()->precalc_first();
    assert(!is_constant()); /* because I have more work to do */
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_MODULE::makes_own_scope() const
{
  return !_parent; //  || _parent == &pp;
}
/*--------------------------------------------------------------------------*/
void DEV_MODULE::precalc_last()
{
  if(_parent == &pp){ untested();
    CARD::precalc_last();
    // its a proto, bypass common clash hotfix
  }else if(is_device()){
    BASE_SUBCKT::precalc_last();
    COMMON_PARAMLIST* c = prechecked_cast<COMMON_PARAMLIST*>(mutable_common());
    assert(c);
    subckt()->attach_params(&(c->_params), scope());

    for(auto p : c->_params){
      trace3("pl", p.first, p.second, p.second.string());
    }

    subckt()->precalc_last();

    assert(!is_constant()); /* because I have more work to do */
  }else{
    CARD::precalc_last();
    // its a proto, bypass common clash hotfix
  }
}
/*--------------------------------------------------------------------------*/
double DEV_MODULE::tr_probe_num(const std::string& x)const
{ untested();
  if (Umatch(x, "p ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"P");
    }
    return power;
  }else if (Umatch(x, "pd ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"PD");
    }
    return power;
  }else if (Umatch(x, "ps ")) {untested();
    double power = 0.;
    assert(subckt());
    for (CARD_LIST::const_iterator
	   ci = subckt()->begin(); ci != subckt()->end(); ++ci) {untested();
      power += CARD::probe(*ci,"PS");
    }
    return power;
  }else{ untested();
    return COMPONENT::tr_probe_num(x);
  }
  /*NOTREACHED*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
