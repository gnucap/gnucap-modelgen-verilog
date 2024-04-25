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
 * Inverse Z transform filter
 * BUG1: current input not currently implemented
 */
#include <globals.h>
#include <e_compon.h>
#include <e_node.h>
#include <e_elemnt.h>
#include <u_nodemap.h>
#include "e_va.h"
#include "e_rf.h"
#include <u_limit.h>

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class NATURE_current : public NATURE {
  double abstol()const override {return 1e-12;}
}_N_current;
class NATURE_voltage : public NATURE {
  double abstol()const override {return 1e-6;}
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
  _COMMON_VASRC_electrical(_COMMON_VASRC_electrical const&p)     : COMMON_VASRC(p){}
  COMMON_COMPONENT* clone()const override{ untested();
    return new _COMMON_VASRC_electrical(*this);
  }
  std::string name()const override{untested(); return "electrical";}
  DISCIPLINE const* discipline()const override {return &_D_electrical;}
public:
};
static _COMMON_VASRC_electrical _C_V_electrical(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_ZIFILTER : public COMMON_RF_BASE {
public:
  PARAMETER<double> _delay{0.}; // "t0"
  PARAMETER<double> _period{1.}; // "T"
  PARAMETER<double> _ttime{.1}; // "\tau"
  std::string name()const override {return "va_zi";}
public:
  ~COMMON_ZIFILTER();
  COMMON_ZIFILTER(int x) : COMMON_RF_BASE(x) {}
  COMMON_ZIFILTER(COMMON_ZIFILTER const& x) : COMMON_RF_BASE(x),
    _delay(x._delay), _period(x._period), _ttime(x._ttime) {}
  COMMON_ZIFILTER* clone()const override {return new COMMON_ZIFILTER(*this);}

  bool operator==(const COMMON_COMPONENT& x)const override;
  void set_param_by_index(int I, std::string& Value, int Offset)override;
  int set_param_by_name(std::string Name, std::string Value)override;
  void precalc_last(const CARD_LIST* par_scope)override;
}; //COMMON_ZIFILTER
COMMON_ZIFILTER czi(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_ZIFILTER_ND : public COMMON_ZIFILTER {
  std::string name()const override {return "va_zi_nd";}
public:
  ~COMMON_ZIFILTER_ND() {}
  COMMON_ZIFILTER_ND(int x) : COMMON_ZIFILTER(x) { set_nx(); set_xd();}
  COMMON_ZIFILTER_ND(COMMON_ZIFILTER_ND const& x) : COMMON_ZIFILTER(x) {}
  COMMON_ZIFILTER_ND* clone()const override {return new COMMON_ZIFILTER_ND(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{
    COMMON_ZIFILTER::precalc_last(par_scope);
    convert_nd();
    reduce_shift();
  }
}; //COMMON_ZIFILTER_ND
COMMON_ZIFILTER_ND czi_nd(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_ZIFILTER_ZD : public COMMON_ZIFILTER {
  std::string name()const override {return "va_zi_zd";}
public:
  ~COMMON_ZIFILTER_ZD() {}
  COMMON_ZIFILTER_ZD(int x) : COMMON_ZIFILTER(x) { set_zx(); set_xd(); }
  COMMON_ZIFILTER_ZD(COMMON_ZIFILTER_ZD const& x) : COMMON_ZIFILTER(x) {}
  COMMON_ZIFILTER_ZD* clone()const override {return new COMMON_ZIFILTER_ZD(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{
    COMMON_ZIFILTER::precalc_last(par_scope);
    convert_nd();
    reduce_shift();
  }
}; //COMMON_ZIFILTER_ZD
COMMON_ZIFILTER_ZD czi_zd(CC_STATIC);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class COMMON_ZIFILTER_NP : public COMMON_ZIFILTER {
  std::string name()const override {return "va_zi_np";}
public:
  ~COMMON_ZIFILTER_NP() {}
  COMMON_ZIFILTER_NP(int x) : COMMON_ZIFILTER(x) { set_nx(); set_xp();}
  COMMON_ZIFILTER_NP(COMMON_ZIFILTER_NP const& x) : COMMON_ZIFILTER(x) {}
  COMMON_ZIFILTER_NP* clone()const override {return new COMMON_ZIFILTER_NP(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{ untested();
    COMMON_ZIFILTER::precalc_last(par_scope);
    convert_nd();
    reduce_shift();
  }
}; //COMMON_ZIFILTER_NP
COMMON_ZIFILTER_NP czi_np(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_ZIFILTER_ZP : public COMMON_ZIFILTER {
  std::string name()const override {return "va_zi_zp";}
public:
  ~COMMON_ZIFILTER_ZP() {}
  COMMON_ZIFILTER_ZP(int x) : COMMON_ZIFILTER(x) { set_zx(); set_xp();}
  COMMON_ZIFILTER_ZP(COMMON_ZIFILTER_ZP const& x) : COMMON_ZIFILTER(x) {}
  COMMON_ZIFILTER_ZP* clone()const override { return new COMMON_ZIFILTER_ZP(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{
    COMMON_ZIFILTER::precalc_last(par_scope);
    convert_nd();
    reduce_shift();
    trace2("zp::pl", _p_den.size(), _p_num.size());
  }
}; //COMMON_ZIFILTER_ZP
COMMON_ZIFILTER_ZP czi_zp(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_ZIFILTER_RP : public COMMON_ZIFILTER {
  std::string name()const override {return "va_zi_rp";}
public:
  ~COMMON_ZIFILTER_RP() {}
  COMMON_ZIFILTER_RP(int x) : COMMON_ZIFILTER(x) { set_rp(); }
  COMMON_ZIFILTER_RP(COMMON_ZIFILTER_RP const& x) = default;
  COMMON_ZIFILTER_RP* clone()const override {return new COMMON_ZIFILTER_RP(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{ untested();
    COMMON_ZIFILTER::precalc_last(par_scope);
    convert_nd();
    reduce_shift();
  }
}; //COMMON_ZIFILTER_RP
COMMON_ZIFILTER_RP czi_rp(CC_STATIC);
/*--------------------------------------------------------------------------*/
class ZFILTER : public ELEMENT {
private:
  int _n_ports{2};
private:
  double* _ctrl_in{NULL};
  double* _regs{NULL};
  double _output{0}; // ELEMENT::_y?
  ELEMENT* _input{NULL}; // needed in ac
  double _old_output{0};
private: // construct
  explicit ZFILTER(ZFILTER const&);
public:
  explicit ZFILTER(COMMON_COMPONENT*);
  ~ZFILTER() {
    if (net_nodes() > NODES_PER_BRANCH) {
      delete [] _n;
    }else{
      // it is part of a base class
    }
    delete[] _regs;
    _regs = NULL;
    delete (CARD*)_input;
  }
  CARD* clone()const override;
  std::string dev_type()const override{assert(has_common()); return common()->name();}
private: // ELEMENT, pure
  void tr_iwant_matrix() override;
  void ac_iwant_matrix() override;
  double tr_involts()const override;
  double tr_involts_limited()const override { incomplete(); return 0.; }
  COMPLEX ac_involts()const override { incomplete(); return 0.; }
private: // BASE_SUBCKT
  void	  tr_begin()override;
  void	  tr_restore()override	{
    ELEMENT::tr_restore();
    incomplete();
  }
  void	  dc_advance()override;
  void	  tr_advance()override;
  void	  tr_regress()override {
    set_not_converged();
    ELEMENT::tr_regress();
  }
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

    if(linear_input){
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
private: // impl
  friend class COMMON_RF_BASE;
}; // ZFILTER
/*--------------------------------------------------------------------------*/
COMMON_ZIFILTER::~COMMON_ZIFILTER()
{
}
/*--------------------------------------------------------------------------*/
void COMMON_ZIFILTER::precalc_last(const CARD_LIST* par_scope)
{
  COMMON_RF_BASE::precalc_last(par_scope);
  e_val(&_period, 1. , par_scope);
  e_val(&_ttime, 0.1 , par_scope);
  e_val(&_delay, 0. , par_scope);
}
/*--------------------------------------------------------------------------*/
bool COMMON_ZIFILTER::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_ZIFILTER* p = dynamic_cast<const COMMON_ZIFILTER*>(&x);
  return (p
    && _period == p->_period
    && _ttime == p->_ttime
    && _delay == p->_delay
    && COMMON_RF_BASE::operator==(x));
}
/*--------------------------------------------------------------------------*/
int COMMON_ZIFILTER::set_param_by_name(std::string Name, std::string Value)
{
  trace2("spbn", Name, Value);
  if(Name == "T"){
    _period = Value;
  }else if(Name == "tau"){
    _ttime = Value;
  }else if(Name == "t0"){ untested();
    _delay = Value;
  }else{
    COMMON_RF_BASE::set_param_by_name(Name, Value);
  }
  return 0; // incomplete
}
/*--------------------------------------------------------------------------*/
void COMMON_ZIFILTER::set_param_by_index(int I, std::string& Value, int Offset)
{
	incomplete();
  switch (COMMON_ZIFILTER::param_count() - 1 - I) {
  case 0: _period = Value;
	  break;
  case 1: _ttime = Value;
	  break;
  case 2: _delay = Value;
	  break;
  default: COMMON_RF_BASE::set_param_by_index(I, Value, Offset);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static COMMON_ZIFILTER Default_common(CC_STATIC);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
double ZFILTER::tr_involts()const
{
  if(_input) {
    // return _input->tr_amps();
    assert(_ctrl_in);
    double input = *_ctrl_in;
    // just fpoly?
    // for(int i = 0; i<_n_ports-1; ++i) { untested();
    //   input += _ctrl_in[2+i] * dn_diff(_n[2+2*i].v0(), _n[2+2*i+1].v0());
    // }
    return input;
  }else{
    return dn_diff(_n[IN1].v0(), _n[IN2].v0());
  }
}
/*--------------------------------------------------------------------------*/
double ZFILTER::tr_probe_num(std::string const& n) const
{ untested();
  auto c = prechecked_cast<COMMON_RF_BASE const*>(common());
  assert(c);
  int num_den = c->den_size();

  if(n[0] == 's'){ untested();
    int idx = atoi(n.substr(1).c_str());
    if(idx < num_den){ untested();
      return _regs[idx];
    }else{ untested();
    }
  }else{ untested();
  }

  if(n == "vin") { untested();
    return tr_involts();
  }else if(n == "conv") { untested();
    return converged();
  }else{ untested();
    return ELEMENT::tr_probe_num(n);
  }
}
/*--------------------------------------------------------------------------*/
ZFILTER::ZFILTER(COMMON_COMPONENT* c) : ELEMENT(c)
{
  // build netlist
  // ports:2
  // overrides
}
/*--------------------------------------------------------------------------*/
// seq blocks
/*--------------------------------------------------------------------------*/
bool ZFILTER::tr_needs_eval()const
{
  trace1("test_lap2::needs_eval?", long_label());
  return false;
  node_t gnd(&ground_node);
  if (is_q_for_eval()) { untested();
    return false;
  }else if (!converged()) { untested();
    return true;
  }else   if(_sim->is_first_iteration()){ untested();
    return true;
  }else{ untested();
    return false;
  }
};
/*--------------------------------------------------------------------------*/
bool ZFILTER::do_tr()
{ untested();
  return ELEMENT::do_tr();
}
/*--------------------------------------------------------------------------*/
void ZFILTER::tr_advance()
{
  ELEMENT::tr_advance();
  trace4("ZFILTER::tr_advance", _sim->_time0, long_label(), tr_involts(), _loss0);
  set_converged();
  ELEMENT* d = this;
  auto c = prechecked_cast<COMMON_ZIFILTER const*>(common());
  assert(c);
  assert( c->_p_num.size());
  assert( c->_p_den.size());
  trace3("ZFILTER::tr_advance", long_label(), c->_delay, c->_period);

  double raw_time = d->_sim->_time0;

  node_t gnd(&ground_node);
  int num_den = int(c->den_size());
  assert(num_den);


  double ev;

  double const& old_value = _old_output;
  double const& new_value = _output;

  if (raw_time <= c->_delay) { untested();
    ev = old_value;
    assert(ev==ev);
  }else{
    double reltime;
    assert(c->_period);
    reltime = fmod(raw_time - c->_delay, c->_period);

    if (reltime < _sim->_dtmin * .5) {
      // event.
      ev = new_value;
      assert(ev==ev);
    }else if (reltime < c->_ttime) {
      double interp = reltime / c->_ttime;
      ev = old_value + interp * (new_value - old_value);
      assert(ev==ev);
    }else{
      ev = new_value;
      assert(ev==ev);
    }
  }
  trace2("ev?", raw_time, c->_period);
  trace3("ev?", raw_time, new_value, old_value);

  //d->q_accept();
  //tr_finish_tdv(d, ev);

  _m0.x = 0.;
  _m0.c1 = 1. * _loss0;
  _m0.c0 = ev * _loss0; // ??
  trace2("test_zi::do_tr", ev, _sim->_time0);
  set_converged(conv_check());
  store_values();
  q_load();
}
/*--------------------------------------------------------------------------*/
// typedef ZFILTER::ddouble ddouble;
/*--------------------------------------------------------------------------*/
void ZFILTER::tr_begin()
{
  ELEMENT::tr_begin();
  _loss0 = 1.;
  _loss1 = 1.;
  _output = 0.; //really?

  COMMON_RF_BASE const* c = prechecked_cast<COMMON_RF_BASE const*>(common());
  assert(c);
  int num_den = c->den_size();
  int num_num = c->num_size();
  int num_regs = std::max(num_den, num_num);
  trace2("ZFILTER::tr_begin", num_num, num_den);
  std::fill_n(_regs, num_regs, 0.);
}
/*--------------------------------------------------------------------------*/
inline void ZFILTER::dc_advance()
{
  // incomplete.
  return ELEMENT::dc_advance();
}
/*--------------------------------------------------------------------------*/
std::string ZFILTER::port_name(int i)const
{
	assert(i >= 0);
	assert(i < max_nodes());
	static std::string names[] = {"out0", "out1", "in0", "in1" };
	return names[i];
}
/*--------------------------------------------------------------------------*/
ZFILTER zi(&czi);
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, "va_zi", &zi);
ZFILTER zi_nd(&czi_nd);
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "va_zi_nd", &zi_nd);
ZFILTER zi_zd(&czi_zd);
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_zi_zd", &zi_zd);
ZFILTER zi_np(&czi_np);
DISPATCHER<CARD>::INSTALL d3(&device_dispatcher, "va_zi_np", &zi_np);
ZFILTER zi_zp(&czi_zp);
DISPATCHER<CARD>::INSTALL d4(&device_dispatcher, "va_zi_zp", &zi_zp);
ZFILTER zi_rp(&czi_rp);
DISPATCHER<CARD>::INSTALL d5(&device_dispatcher, "va_zi_rp", &zi_rp);
/*--------------------------------------------------------------------------*/
CARD* ZFILTER::clone()const
{
  ZFILTER* new_instance = new ZFILTER(*this);
  return new_instance;
}
/*--------------------------------------------------------------------------*/
ZFILTER::ZFILTER(ZFILTER const&p) : ELEMENT(p)
{
  if(p._n){
    trace2("laplace", int_nodes(), max_nodes());
    assert(_n);
    assert(int_nodes() + max_nodes() <= NODES_PER_BRANCH); // not expanded yet.
//    _n = new node_t[int_nodes() + max_nodes()];
//    for (int ii = 0; ii < p.net_nodes(); ++ii) { untested();
//      _n[ii] = p._n[ii];
//    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void ZFILTER::precalc_first()
{
  ELEMENT::precalc_first();
  if(_input){
    _input->precalc_first();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool ZFILTER::is_valid()const
{ untested();
  COMMON_RF_BASE const* c = prechecked_cast<COMMON_RF_BASE const*>(common());
  assert(c);
  return c->is_valid();
}
/*--------------------------------------------------------------------------*/
void ZFILTER::expand()
{
  assert(common());
  auto c = static_cast</*const*/ COMMON_RF_BASE*>(mutable_common());
  assert(c);
  int num_den = c->den_size();
  int num_num = c->num_size();
  int num_regs = std::max(num_den, num_num);
  assert(num_den);
  trace2("ZFILTER::expand", num_den, num_num);

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
    if(!_regs){
      _regs = new double [num_regs];
      trace3("expand", long_label(), c->_p_den.size(), _ctrl_in);
    }else{
    }
    // ...
    int n_inputs = _n_ports - 1;
    if(_ctrl_in) {
      std::string input_dev_type = "va_fpoly_g";
      if (!_input) {
	const CARD* input_dev = device_dispatcher[input_dev_type];
	if(!input_dev){ untested();
	  throw Exception("Cannot find " + input_dev_type + ". Load module?");
	}else{
	}
	_input = dynamic_cast<ELEMENT*>(input_dev->clone());
	_input->_loss0 = 1;
	_input->_loss1 = 1;
	if(!_input){ untested();
	  throw Exception("Cannot use " + input_dev_type + " wrong type");
	}else{
	}
	// subckt()->push_front(_input);
//	_input->set_owner(owner());
      }else{ untested();
      }
      std::vector<node_t> nodes(2*_n_ports);

      node_t gnd;
      gnd.set_to_ground(this);
      nodes[0] = _n[input_idx()];
      nodes[1] = gnd;
      for(int k=2; k<2*n_inputs + 2; ++k){
	nodes[k] = _n[k];
      }

      trace2("expand4 input", long_label(), c->_p_den.size());
      // TODO: current inputs?
      assert(_input);
      _input->set_parameters("in", this, &_C_V_electrical, 0., /*states:*/2+n_inputs, _ctrl_in,
	     int(nodes.size()), nodes.data());
    }else{
      // _input = this;
    }
  }else{ untested();
  }


  assert(!is_constant());
} //expand
/*--------------------------------------------------------------------------*/
void ZFILTER::precalc_last()
{
  ELEMENT::precalc_last();
  auto c = static_cast<COMMON_RF_BASE*>(mutable_common());
  assert(c);

  if(_ctrl_in){
  }else{
  }
  assert( c->_p_num.size());
  assert( c->_p_den.size());
}
/*--------------------------------------------------------------------------*/
void ZFILTER::set_parameters(const std::string& Label, CARD *Owner,
				   COMMON_COMPONENT *Common, double Value,
				   int n_states, double states[],
				   int n_nodes, const node_t nodes[])
{
  trace2("ZFILTER::set_parameters", n_states, n_nodes);
  bool first_time = !_ctrl_in;
  _ctrl_in = states;
  auto p = prechecked_cast<COMMON_FILT const*>(Common);
  assert(p);
  int dens = p->args(2);
  int nums = p->args(1);
  trace2("ZFILTER args", dens, nums);
//  int num_regs = std::max(dens, nums);
  _n_ports = n_nodes/2; // TODO: current ports

  assert(common());
  auto ccc = common()->clone();
  auto cc = prechecked_cast<COMMON_ZIFILTER*>(ccc);
  assert(cc);

  cc->_p_den.resize(dens);
  cc->_p_num.resize(nums);

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


  // _vy0 = states;
  trace4("setnodes", net_nodes(), nodes, dens, n_nodes);
//  assert(net_nodes() == n_nodes);
  notstd::copy_n(nodes, net_nodes(), _n);
  _loss1 = _loss0 = 1.;
} // set_parameters
/*--------------------------------------------------------------------------*/
void ZFILTER::ac_begin()
{
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void ZFILTER::do_ac()
{
  trace1("ZFILTER::do_ac", mfactor());
  COMMON_ZIFILTER const* c = prechecked_cast<COMMON_ZIFILTER const*>(common());
  assert(c);
  assert( c->_p_num.size());
  assert( c->_p_den.size());

  double period = double(c->_period);
  COMPLEX z = std::exp(-_sim->_jomega * period);
  COMPLEX num = evalp(z, c->_p_num.begin(), c->num_size());
  COMPLEX den = evalp(z, c->_p_den.begin(), c->den_size());

  _acg = num/den;
  _acg *= exp(-_sim->_jomega * double(c->_ttime) * .5);
}
/*--------------------------------------------------------------------------*/
void ZFILTER::ac_iwant_matrix()
{
  if(_input){
    trace2("ZFILTER::ac_iwant_matrix", input_idx(),_n[input_idx()].m_());
    _sim->_acx.iwant(_n[OUT1].m_(),_n[input_idx()].m_());
    //_sim->_acx.iwant(_n[OUT1].m_(),_n[IN2].m_()); IN2==gnd.
    _sim->_acx.iwant(_n[OUT2].m_(),_n[input_idx()].m_());
    //_sim->_acx.iwant(_n[OUT2].m_(),_n[IN2].m_()); IN2==gnd.

    _input->ac_iwant_matrix();
  }else{
    trace0("ZFILTER::ac_iwant_matrix plain");
    ELEMENT::ac_iwant_matrix_active();
    // ELEMENT::ac_iwant_matrix_extended();
  }
}
/*--------------------------------------------------------------------------*/
void ZFILTER::tr_iwant_matrix()
{
  ELEMENT::tr_iwant_matrix_passive();
}
/*--------------------------------------------------------------------------*/
void ZFILTER::map_nodes()
{
  trace3("ZFILTER::map_nodes", input_idx(), int_nodes(), ext_nodes());
  ELEMENT::map_nodes();

  if(_input){
    _input->map_nodes();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void ZFILTER::ac_load()
{
  ac_load_shunt();
  if(_input){
    _input->ac_load();
    trace2("acload", input_idx(), _n[input_idx()].m_());
    // ac_load_active(); but other input.
    double mfactor_hack = _ctrl_in[1];
    _sim->_acx.load_asymmetric(_n[OUT1].m_(), _n[OUT2].m_(),
	0, _n[input_idx()].m_(), mfactor_hack * _acg);
  }else{
    trace2("acload", _acg, mfactor());
    ac_load_active();
  }
}
/*--------------------------------------------------------------------------*/
void ZFILTER::tr_load()
{
  trace3("trload", long_label(), input_idx(), _n[input_idx()].m_());
  tr_load_shunt();
  tr_load_source();
}
/*--------------------------------------------------------------------------*/
void ZFILTER::tr_accept()
{
  ELEMENT::tr_accept();
  auto c = prechecked_cast<COMMON_ZIFILTER const*>(common());
  assert(c);
  trace4("ZFILTER::tr_accept", _sim->_time0, long_label(), tr_involts(), c->_period);

  double raw_time = _sim->_time0 + _sim->_dtmin * .01;
  raw_time -= c->_delay;
  int howmany = int(raw_time / c->_period);

  assert(fmod(raw_time - c->_delay, c->_period) < _sim->_dtmin);

  double et = c->_delay + double(howmany+1) * c->_period;
#if 0
  _n[2]->set_event(et, lvUNKNOWN);
#else
  _sim->new_event(et);// BUG: global event.
#endif

  _old_output = _output;

  int num_den = c->den_size();
  int num_num = c->num_size();
  int num_regs = std::max(num_den, num_num);
  trace3("ZFILTER::tr_accept", num_den, num_num, num_regs);
  trace3("ZFILTER::tr_accept", long_label(), tr_involts(), num_regs);

  // double& new_output = _y[0].f1 = 0;
  double new_input = tr_involts();
  int i = num_regs-1;
  for(i=num_regs-1; i>num_den-1; --i){
    _regs[i] = _regs[i-1];
  }
  for(       ; i>0; --i){
    trace3("den", i, c->_p_den[i], c->_p_den.size());
    new_input -= _regs[i] * c->_p_den[i] / c->_p_den[0];
    _regs[i] = _regs[i-1];
  }
  _regs[0] = new_input;

  _output = 0.;
  for(i=0; i<num_num; ++i){
    _output += _regs[i] * c->_p_num[i];
  }
  assert(c->_p_den[0]);
  _output /= c->_p_den[0];
  trace4("accept", _sim->_time0, c->_p_den[0], c->_p_num[0], new_input);
  trace4("accept", _sim->_time0, _output, _old_output, tr_involts());
}
/*--------------------------------------------------------------------------*/
TIME_PAIR ZFILTER::tr_review()
{
  trace1("ZFILTER::tr_review", long_label());
  ELEMENT::tr_review();
  auto c = prechecked_cast<COMMON_ZIFILTER const*>(common());
  assert(c);

  // hack to avoid duplicate events from numerical noise
  double raw_time = _sim->_time0 + _sim->_dtmin * .01;

  if (raw_time <= c->_delay) { untested();
    _time_by.min_event(c->_delay);
  }else{
    double reltime;
    assert(c->_period);
    reltime = fmod(raw_time - c->_delay, c->_period);
      trace2("review", c->_period, reltime);

    double time_offset = raw_time - reltime;
    if (reltime < _sim->_dtmin * .5) {
      q_accept();
      _time_by.min_event(c->_ttime + time_offset);
    }else if (reltime < c->_ttime) {
      _time_by.min_event(c->_ttime + time_offset);
    }else if (reltime < c->_period) {
      trace1("review", c->_period);
      _time_by.min_event(c->_period + time_offset);
    }else{ untested();
    }
  }
  trace3("ZFILTER::tr_review", long_label(), _sim->_time0, _time_by._event);
  return _time_by;
}
/*--------------------------------------------------------------------------*/
void ZFILTER::tr_unload()
{ untested();
  incomplete();
}
/*--------------------------------------------------------------------------*/
double ZFILTER::tr_amps() const
{
  return _output;
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
