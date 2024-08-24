/*                        -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
 * Author: Felix Salfelder
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
 * delay line
 */
#include <globals.h>
#include <e_compon.h>
#include <e_node.h>
#include <e_elemnt.h>
#include <u_nodemap.h>
#include "e_va.h"
#include <u_limit.h>
#include <m_wave.h>
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class NATURE_current : public NATURE {
  double abstol()const override { untested();return 1e-12;}
}_N_current;
class NATURE_voltage : public NATURE {
  double abstol()const override { untested();return 1e-6;}
}_N_voltage;
class DISCIPLINE_electrical : public DISCIPLINE {
public:
  NATURE const* flow()const override{ untested();
    return &_N_current;
  }
  NATURE const* potential()const override{ untested();
    return &_N_voltage;
  }
}_D_electrical;
/*--------------------------------------------------------------------------*/
class _COMMON_VASRC_electrical : public COMMON_VASRC {
public:
  _COMMON_VASRC_electrical(int i) : COMMON_VASRC(i){}
private:
  _COMMON_VASRC_electrical(_COMMON_VASRC_electrical const&p)     : COMMON_VASRC(p){ untested();}
  COMMON_COMPONENT* clone()const override{ untested();
    return new _COMMON_VASRC_electrical(*this);
  }
  std::string name()const override{untested(); return "electrical";}
  DISCIPLINE const* discipline()const override { untested();return &_D_electrical;}
public:
};
static _COMMON_VASRC_electrical _C_V_electrical(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_DELAY : public COMMON_COMPONENT {
public:
  PARAMETER<double> _delay{0.};
  PARAMETER<double> _maxdelay{0.};
  std::string name()const override {return "va_delay";}
public:
  ~COMMON_DELAY();
  COMMON_DELAY(int x) : COMMON_COMPONENT(x) {}
  COMMON_DELAY(COMMON_DELAY const& x) : COMMON_COMPONENT(x),
    _delay(x._delay), _maxdelay(x._maxdelay) {}
  COMMON_DELAY* clone()const override {return new COMMON_DELAY(*this);}

  bool operator==(const COMMON_COMPONENT& x)const override;
  void set_param_by_index(int I, std::string& Value, int Offset)override;
  int set_param_by_name(std::string Name, std::string Value)override;
  void precalc_last(const CARD_LIST* par_scope)override;
}; //COMMON_DELAY
COMMON_DELAY cdel(CC_STATIC);
/*--------------------------------------------------------------------------*/
class DELAY : public ELEMENT {
private:
  WAVE _forward;
  double _out0;
  double _out1;
  COMPLEX _y12;
  int _n_ports{2};
private:
  double* _ctrl_in{NULL};
  ELEMENT* _input{NULL}; // needed in ac
  double _old_output{0};
private: // construct
  explicit DELAY(DELAY const&);
public:
  explicit DELAY(COMMON_COMPONENT*);
  ~DELAY() {
    if (net_nodes() > NODES_PER_BRANCH) { untested();
      delete [] _n;
    }else{
      // it is part of a base class
    }
    delete (CARD*)_input;
  }
  CARD* clone()const override;
  std::string dev_type()const override{assert(has_common()); return common()->name();}
private: // ELEMENT, pure
  void tr_iwant_matrix() override;
  void ac_iwant_matrix() override;
  double tr_involts()const override;
  double tr_involts_limited()const override { untested(); incomplete(); return 0.; }
  COMPLEX ac_involts()const override { untested(); incomplete(); return 0.; }
private: // BASE_SUBCKT
  void	  tr_begin()override;
  void	  tr_restore()override	{ untested();
    ELEMENT::tr_restore();
    incomplete();
  }
  void	  dc_advance()override;
  void	  tr_advance()override;
  void	  tr_regress()override;
  bool	  tr_needs_eval()const override;
  void    tr_queue_eval()override {if(tr_needs_eval()){q_eval();}else{} }
  bool	  do_tr()override;
  void	  tr_load()override;
  TIME_PAIR tr_review()override;

  void tr_accept()override;
  void tr_unload()override;
  void ac_begin()override;
  void do_ac()override;
  void ac_load()override;
  void map_nodes()override;
private: // overrides
  double tr_amps()const override;
  bool is_valid()const override;
  void precalc_first()override;
  void expand()override;
  void precalc_last()override;
  double tr_probe_num(std::string const&)const override;
  int max_nodes()const override {return std::max(4,_n_ports*2);}
  int net_nodes()const override	{return _n_ports*2;}
  int min_nodes()const override {return 4;}
  int int_nodes()const override {return bool(_ctrl_in);}

  int input_idx()const {
    bool linear_input = !_ctrl_in; // TODO.
    int ii;

    if(linear_input){ untested();
      ii = 2;
    }else if(_ctrl_in){
      ii = net_nodes();
    }else{ untested();
      ii = net_nodes();
    }
    return ii;
  }
  int matrix_nodes()const override {return 2;}
  std::string value_name()const override {itested(); return "";}
  bool print_type_in_spice()const override {itested(); return false;}
  std::string port_name(int i)const override;
public: // params
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
private: // node list
  enum {
    n_out0,
    n_out1,
    n_in0,
    n_in1,
  };
}; // DELAY
/*--------------------------------------------------------------------------*/
COMMON_DELAY::~COMMON_DELAY()
{
}
/*--------------------------------------------------------------------------*/
void COMMON_DELAY::precalc_last(const CARD_LIST* par_scope)
{
  COMMON_COMPONENT::precalc_last(par_scope);
  e_val(&_maxdelay, 1. , par_scope);
  e_val(&_delay, 1. , par_scope);
}
/*--------------------------------------------------------------------------*/
bool COMMON_DELAY::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_DELAY* p = dynamic_cast<const COMMON_DELAY*>(&x);
  return (p
    && _maxdelay == p->_maxdelay
    && _delay == p->_delay
    && COMMON_COMPONENT::operator==(x));
}
/*--------------------------------------------------------------------------*/
int COMMON_DELAY::set_param_by_name(std::string Name, std::string Value)
{
  if(Name == "delay"){
    _delay = Value;
  }else if(Name == "maxdelay"){
    _maxdelay = Value;
  }else{ untested();
    COMMON_COMPONENT::set_param_by_name(Name, Value);
  }
  return 0; // incomplete
}
/*--------------------------------------------------------------------------*/
void COMMON_DELAY::set_param_by_index(int I, std::string& Value, int Offset)
{ untested();
	incomplete();
  switch (COMMON_DELAY::param_count() - 1 - I) {
  case 0: _delay = Value;
	  break;
  case 1: _maxdelay = Value;
	  break;
  default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static COMMON_DELAY Default_common(CC_STATIC);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
double DELAY::tr_involts()const
{
  if(_ctrl_in) {
    // return _input->tr_amps();
    return *_ctrl_in;
    // just fpoly?
    // for(int i = 0; i<_n_ports-1; ++i) { untested();
    //   input += _ctrl_in[2+i] * dn_diff(_n[2+2*i].v0(), _n[2+2*i+1].v0());
    // }
  }else{
    return dn_diff(_n[IN1].v0(), _n[IN2].v0());
  }
}
/*--------------------------------------------------------------------------*/
double DELAY::tr_probe_num(std::string const& n) const
{
  return ELEMENT::tr_probe_num(n);
}
/*--------------------------------------------------------------------------*/
DELAY::DELAY(COMMON_COMPONENT* c) : ELEMENT(c)
{
  // build netlist
  // ports:2
  // overrides
}
/*--------------------------------------------------------------------------*/
// seq blocks
/*--------------------------------------------------------------------------*/
bool DELAY::tr_needs_eval()const
{
  trace1("test_lap2::needs_eval?", long_label());
  node_t gnd(&ground_node);
  if (is_q_for_eval()) { untested();
    return false;
  }else if (!converged()) { untested();
    return true;
  }else{
    return _out0!=_out1;
  }
};
/*--------------------------------------------------------------------------*/
bool DELAY::do_tr()
{
  //if (_out0 != _out1) { untested();
  if (!conchk(_out0, _out1, OPT::abstol, OPT::reltol*.01)) {
    q_load();
  }else{
  }
  assert(converged());
  return true;
}
/*--------------------------------------------------------------------------*/
void DELAY::tr_advance()
{
  trace1("DELAY::tr_advance", _sim->_time0);
  ELEMENT::tr_advance();
  try{
    _out0 = _forward.v_out(_sim->_time0).f0;
  }catch(Exception const&){ untested();
    assert(0);
  }
}
/*--------------------------------------------------------------------------*/
void DELAY::tr_regress()
{ untested();
  ELEMENT::tr_regress();
  _out0 = _forward.v_out(_sim->_time0).f0;
}
/*--------------------------------------------------------------------------*/
// typedef DELAY::ddouble ddouble;
/*--------------------------------------------------------------------------*/
void DELAY::tr_begin()
{
  ELEMENT::tr_begin();
  _loss0 = 1.;

  const COMMON_DELAY* c=prechecked_cast<const COMMON_DELAY*>(common());
  assert(c);

  trace1("DELAY::tr_begin", c->_delay);
  _forward.initialize().push(0.-c->_delay, 0.).push(0., 0.);
}
/*--------------------------------------------------------------------------*/
inline void DELAY::dc_advance()
{
  ELEMENT::dc_advance();
  const COMMON_DELAY* c=prechecked_cast<const COMMON_DELAY*>(common());
  assert(c);
  _out0 = _forward.v_out(_sim->_time0).f0;
}
/*--------------------------------------------------------------------------*/
std::string DELAY::port_name(int i)const
{
  assert(i >= 0);
  assert(i < max_nodes());
  static std::string names[] = {"out0", "out1", "in0", "in1" };
  return names[i];
}
/*--------------------------------------------------------------------------*/
DELAY delay(&cdel);
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, "va_absdelay", &delay);
/*--------------------------------------------------------------------------*/
CARD* DELAY::clone()const
{
  return new DELAY(*this);
}
/*--------------------------------------------------------------------------*/
DELAY::DELAY(DELAY const&p) : ELEMENT(p)
{
  if(p._n){
    trace2("laplace", int_nodes(), max_nodes());
    assert(_n);
    assert(int_nodes() + max_nodes() <= NODES_PER_BRANCH); // not expanded yet.
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void DELAY::precalc_first()
{
  ELEMENT::precalc_first();
  if(_input){ untested();
    _input->precalc_first();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool DELAY::is_valid()const
{ untested();
  COMMON_COMPONENT const* c = prechecked_cast<COMMON_COMPONENT const*>(common());
  assert(c);
  return true; // c->is_valid();
}
/*--------------------------------------------------------------------------*/
void DELAY::expand()
{
  assert(common());
  auto c = static_cast</*const*/ COMMON_COMPONENT*>(mutable_common());
  assert(c);

  if(!_ctrl_in) {
  }else if (!subckt()) {
    new_subckt();
  }else{ untested();
  }

#if 0
  // make space for input node.
  if(_ctrl_in && input_idx() >= 4){ untested();
    auto nn = new node_t[net_nodes() + 1];
    notstd::copy_n(_n, net_nodes(), nn);
    // delete _n; no. it's in ELEMENT
    _n = nn;
  }else{ untested();
  }
#endif

  if(!_ctrl_in) {
//  }else if(linear_input){ untested();
  }else if (!_n[input_idx()].n_()) {
    trace1("expand nmn", input_idx());
    _n[input_idx()].new_model_node(".input", this);
  }else{ untested();
    trace1("expand nmn alreay there?", input_idx());
  }

  if (_sim->is_first_expand()) {
    int n_inputs = _n_ports - 1;
    if(_ctrl_in) {
      std::string input_dev_type = "va_fpoly_g";
      std::vector<node_t> nodes(2*_n_ports);

      node_t gnd;
      gnd.set_to_ground(this);
      nodes[0] = _n[input_idx()];
      nodes[1] = gnd;
      for(int k=2; k<2*n_inputs + 2; ++k){
	nodes[k] = _n[k];
      }
    }else{
      // _input = this;
    }
  }else{ untested();
  }


  assert(!is_constant());
} //expand
/*--------------------------------------------------------------------------*/
void DELAY::precalc_last()
{
  ELEMENT::precalc_last();
  auto c = prechecked_cast<COMMON_DELAY*>(mutable_common());
  assert(c);

  if(_ctrl_in){
  }else{
  }

  _forward.set_delay(c->_delay);
  set_converged();
  assert(!is_constant());
}
/*--------------------------------------------------------------------------*/
void DELAY::set_parameters(const std::string& Label, CARD *Owner,
				   COMMON_COMPONENT *Common, double Value,
				   int n_states, double states[],
				   int n_nodes, const node_t nodes[])
{
  trace2("DELAY::set_parameters", n_states, n_nodes);
  bool first_time = !_ctrl_in;
  _ctrl_in = states;
  auto p = prechecked_cast<COMMON_FILT const*>(Common);
  assert(p);
//  int num_regs = std::max(dens, nums);
  _n_ports = n_nodes/2; // TODO: current ports

  assert(common());
  auto ccc = common()->clone();
  auto cc = prechecked_cast<COMMON_DELAY*>(ccc);
  assert(cc);

  set_label(Label);
  set_owner(Owner);
  set_value(Value);
  attach_common(cc);

  if (first_time) {
    assert(n_states - 1 - n_nodes/2 == 0); // for now.
//    _current_port_names.resize(n_states - 1 - n_nodes/2);
 //   _n_ports = n_nodes/2; // sets num_nodes() = _n_ports*2
    if(net_nodes()+int_nodes()>NODES_PER_BRANCH) {
      _n = new node_t[net_nodes()+int_nodes()];
      trace4("allocnodes", net_nodes(), nodes, int_nodes(), n_nodes);
    }else{ untested();
    }
  }else{untested();
    assert(net_nodes() == n_nodes);
    // assert could fail if changing the number of nodes after a run
  }

  notstd::copy_n(nodes, net_nodes(), _n);
  _loss0 = 1.;
} // set_parameters
/*--------------------------------------------------------------------------*/
void DELAY::ac_begin()
{
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DELAY::do_ac()
{
  trace1("DELAY::do_ac", mfactor());
  COMMON_DELAY const* c = prechecked_cast<COMMON_DELAY const*>(common());
  assert(c);

  double td(c->_delay);
  _acg = std::exp(-_sim->_jomega * td);
}
/*--------------------------------------------------------------------------*/
void DELAY::ac_iwant_matrix()
{
  if(_ctrl_in){
    //trace2("DELAY::ac_iwant_matrix", input_idx(),_n[input_idx()].m_());
    //_sim->_acx.iwant(_n[OUT1].m_(),_n[input_idx()].m_());
    //_sim->_acx.iwant(_n[OUT2].m_(),_n[input_idx()].m_());

    //_input->ac_iwant_matrix();

    assert(is_device());

    for (int ii = 0;  ii < ext_nodes();  ++ii) {
      if (_n[ii].m_() >= 0) {
	for (int jj = 0;  jj < ii ;  ++jj) {
	  _sim->_acx.iwant(_n[ii].m_(),_n[jj].m_());
	}
      }else{itested();
	// node 1 is grounded or invalid
      }
    }
  }else{
    trace0("DELAY::ac_iwant_matrix plain");
    ELEMENT::ac_iwant_matrix_active();
    // ELEMENT::ac_iwant_matrix_extended();
  }
}
/*--------------------------------------------------------------------------*/
void DELAY::tr_iwant_matrix()
{
  ELEMENT::tr_iwant_matrix_passive();
}
/*--------------------------------------------------------------------------*/
void DELAY::map_nodes()
{
  trace3("DELAY::map_nodes", input_idx(), int_nodes(), ext_nodes());
  ELEMENT::map_nodes();

  if(_input){ untested();
    _input->map_nodes();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DELAY::ac_load()
{
  ac_load_shunt();
  if(_ctrl_in){ untested();
    assert(_ctrl_in);
    trace4("acload", input_idx(), _n[input_idx()].m_(), _acg, _n_ports);
    // ac_load_active(); but other input.
//    double mfactor_hack = _ctrl_in[1];
//    _sim->_acx.load_asymmetric(_n[OUT1].m_(), _n[OUT2].m_(),
//	0, _n[input_idx()].m_(), mfactor_hack * _acg);

    auto _values = _ctrl_in;
    for (int i=2; i<=_n_ports; ++i) { untested();
      ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _values[i] * _acg);
    }
  }else{
    trace2("acload", _acg, mfactor());
    ac_load_active();
  }
}
/*--------------------------------------------------------------------------*/
void DELAY::tr_load()
{
  trace4("DELAY::trload", _out0, _out1, _loss0, _loss1);

  if(_ctrl_in){
    return; // incomplete
  }else{
    tr_load_shunt();
  }

    //BUG// explicit mfactor
  double lvf = NOT_VALID; // load value, forward
  if (!_sim->is_inc_mode()) {
    const COMMON_DELAY* c = prechecked_cast<const COMMON_DELAY*>(common());
    assert(c);
    // if loss0?
    // _sim->_aa.load_symmetric(_n[OUT1].m_(), _n[OUT2].m_(), mfactor());
    lvf = _out0;
  }else{
    lvf = dn_diff(_out0, _out1);
  }
  trace1("DELAY::trload", lvf);
  if (lvf != 0.) {
    if (_n[OUT1].m_() != 0) { untested();
      _n[OUT1].i() -= mfactor() * lvf;
    }else{
    }
    if (_n[OUT2].m_() != 0) {
      _n[OUT2].i() += mfactor() * lvf;
    }else{ untested();
    }
  }else{
  }
  _out1 = _out0;
}
/*--------------------------------------------------------------------------*/
void DELAY::tr_accept()
{
  ELEMENT::tr_accept();
  trace2("DELAY::accept", _sim->_time0, tr_involts());
  if(_sim->_time0){
    _forward.push(_sim->_time0, tr_involts());
  }else{
  }
}
/*--------------------------------------------------------------------------*/
TIME_PAIR DELAY::tr_review()
{
  q_accept();
  const COMMON_DELAY* c=prechecked_cast<const COMMON_DELAY*>(common());
  assert(c);
  return TIME_PAIR(_sim->_time0 + c->_delay, NEVER);
}
/*--------------------------------------------------------------------------*/
void DELAY::tr_unload()
{ untested();
  incomplete();
}
/*--------------------------------------------------------------------------*/
double DELAY::tr_amps() const
{
  return _out0;
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
