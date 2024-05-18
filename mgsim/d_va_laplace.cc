/*                        -*- C++ -*-
 * Copyright (C) 2023, 2024 Felix Salfelder
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
 * laplace filter
 * BUG1: pivot not reconsidered after parameter change.
 * BUG2: when instanciated from analog block
 *       highest index is pivot.
 * BUG3: current input not currently implemented
 */
#include <globals.h>
#include <e_compon.h>
#include <e_subckt.h>
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
bool ac_use_sckt = false;
/*--------------------------------------------------------------------------*/
class NATURE_current : public NATURE {
  double abstol()const override {return 1e-12;}
}_N_current;
class NATURE_voltage : public NATURE {
  double abstol()const override {return 1e-6;}
}_N_voltage;
class DISCIPLINE_electrical : public DISCIPLINE {
public:
  NATURE const* flow()const override{
    return &_N_current;
  }
  NATURE const* potential()const override{
    return &_N_voltage;
  }
}_D_electrical;
/*--------------------------------------------------------------------------*/
class _COMMON_VASRC_electrical : public COMMON_VASRC {
public:
  _COMMON_VASRC_electrical(int i) : COMMON_VASRC(i){}
private:
  _COMMON_VASRC_electrical(_COMMON_VASRC_electrical const&p)     : COMMON_VASRC(p){}
  COMMON_COMPONENT* clone()const override{
    return new _COMMON_VASRC_electrical(*this);
  }
  std::string name()const override{untested(); return "electrical";}
  DISCIPLINE const* discipline()const override {return &_D_electrical;}
public:
};
static _COMMON_VASRC_electrical _C_V_electrical(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_LAPLACE : public COMMON_RF_BASE {
public:
  PARAMETER<double> _tolerance; // {NOT_INPUT};
  std::string name()const override {return "va_laplace";}
public:
  ~COMMON_LAPLACE() {}
  COMMON_LAPLACE(int x) : COMMON_RF_BASE(x) {}
  COMMON_LAPLACE(COMMON_LAPLACE const& x) :
    COMMON_RF_BASE(x), _tolerance(x._tolerance) {}
  COMMON_LAPLACE* clone()const override {return new COMMON_LAPLACE(*this);}

  virtual int pivot()const;

  bool operator==(const COMMON_COMPONENT& x)const override;
  void set_param_by_index(int I, std::string& Value, int Offset)override;
  void precalc_last(const CARD_LIST* par_scope)override;
}; //COMMON_LAPLACE
COMMON_LAPLACE cl(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_LAPLACE_ND : public COMMON_LAPLACE {
  std::string name()const override {return "va_laplace_nd";}
public:
  ~COMMON_LAPLACE_ND() {}
  COMMON_LAPLACE_ND(int x) : COMMON_LAPLACE(x) {set_xd(); set_nx();}
  COMMON_LAPLACE_ND(COMMON_LAPLACE_ND const& x) : COMMON_LAPLACE(x) {}
  COMMON_LAPLACE_ND* clone()const override {return new COMMON_LAPLACE_ND(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{
    COMMON_LAPLACE::precalc_last(par_scope);
    convert_nd();
  }
  virtual int num_states()const override{
    trace2("num_states", _arg1, _arg2);
    if(_arg1 || _arg2){
      // set_parameters
      return std::max(_arg1, _arg2);
    }else{
      // direct use. figure out in expand
      return 0;
    }
  }
}; //COMMON_LAPLACE_ND
COMMON_LAPLACE_ND cl_nd(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_LAPLACE_ZD : public COMMON_LAPLACE {
  std::string name()const override {return "va_laplace_zd";}
public:
  ~COMMON_LAPLACE_ZD() {}
  COMMON_LAPLACE_ZD(int x) : COMMON_LAPLACE(x) { set_xd(); set_zx(); }
  COMMON_LAPLACE_ZD(COMMON_LAPLACE_ZD const& x) : COMMON_LAPLACE(x) {}
  COMMON_LAPLACE_ZD* clone()const override {return new COMMON_LAPLACE_ZD(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{ untested();
    COMMON_LAPLACE::precalc_last(par_scope);
    convert_nd();
  }
}; //COMMON_LAPLACE_ZD
COMMON_LAPLACE_ZD cl_zd(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_LAPLACE_NP : public COMMON_LAPLACE {
  std::string name()const override {return "va_laplace_np";}
public:
  ~COMMON_LAPLACE_NP() {}
  COMMON_LAPLACE_NP(int x) : COMMON_LAPLACE(x) { set_nx(); set_xp(); }
  COMMON_LAPLACE_NP(COMMON_LAPLACE_NP const& x) : COMMON_LAPLACE(x) {}
  COMMON_LAPLACE_NP* clone()const override {return new COMMON_LAPLACE_NP(*this);}

  void precalc_last(const CARD_LIST* par_scope)override{
    COMMON_LAPLACE::precalc_last(par_scope);
    convert_nd();
  }
  void precalc_first(const CARD_LIST* par_scope)override {
    COMMON_LAPLACE::precalc_first(par_scope);
    convert_nd();
  }
  int pivot()const override {
    if(_p_den.size()) {
      return 1;
    }else{ untested();
      return 0;
    }
  }
  virtual int num_states()const override{
    trace2("num_states", _arg1, _arg2);
    if(_arg1 || _arg2){
      // set_parameters
      return std::max(_arg1, (_arg2+1)/2 + 1);
    }else{
      // direct use. figure out in expand
      return 0;
    }
  }
}; //COMMON_LAPLACE_NP
COMMON_LAPLACE_NP cl_np(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_LAPLACE_ZP : public COMMON_LAPLACE {
  std::string name()const override {return "va_laplace_zp";}
public:
  ~COMMON_LAPLACE_ZP() {}
  COMMON_LAPLACE_ZP(int x) : COMMON_LAPLACE(x) { set_zx(); set_xp();
    assert(!_p_den.size());
    assert(!_p_num.size());
  }
  COMMON_LAPLACE_ZP(COMMON_LAPLACE_ZP const& x) : COMMON_LAPLACE(x) {}
  COMMON_LAPLACE_ZP* clone()const override {return new COMMON_LAPLACE_ZP(*this);}

  void precalc_last(const CARD_LIST* par_scope)override {
    COMMON_LAPLACE::precalc_last(par_scope);
    if(!_p_num.size()){
      set_zx();
    }else{
    }
    if(!_p_den.size()){
      set_xp();
    }else{
    }
    convert_nd();
  }
  int pivot()const override {
    if(_p_den.size()) {
      return 1;
    }else{
      return 0;
    }
  }
  virtual int num_states()const override{
    trace2("num_states", _arg1, _arg2);
    if(_arg1 || _arg2){
      // set_parameters
      return (std::max(_arg1, _arg2)+1)/2 + 1;
    }else{
      // direct use. figure out in expand
      return 0;
    }
  }
}; //COMMON_LAPLACE_ZP
COMMON_LAPLACE_ZP cl_zp(CC_STATIC);
/*--------------------------------------------------------------------------*/
class COMMON_LAPLACE_RP : public COMMON_LAPLACE {
public:
  ~COMMON_LAPLACE_RP() {}
  COMMON_LAPLACE_RP(int x) : COMMON_LAPLACE(x) {set_rp();}
  COMMON_LAPLACE_RP(COMMON_LAPLACE_RP const& x) : COMMON_LAPLACE(x) {}
  COMMON_LAPLACE_RP* clone()const override {return new COMMON_LAPLACE_RP(*this);}

  void precalc_last(const CARD_LIST* par_scope)override {
    COMMON_LAPLACE::precalc_last(par_scope);
    convert_nd();
  }
  int pivot()const override {
    if(_p_den.size()) {
      return 1;
    }else{ untested();
      return 0;
    }
  }
 // void precalc_first(const CARD_LIST* par_scope)override {
 //   COMMON_LAPLACE::precalc_first(par_scope);
 //   convert_nd();
 // }
}; //COMMON_LAPLACE_RP
COMMON_LAPLACE_RP cl_rp(CC_STATIC);
/*--------------------------------------------------------------------------*/
class LAPLACE : public ELEMENT {
private:
  int _n_ports{2};
public: // netlist
  ELEMENT* _input{NULL};
  ELEMENT* _output{NULL};
  std::vector<ELEMENT*> _s_;
private:
  double* _state{NULL};
  double* _st_b_in_{NULL};
  double* _st_b_out_{NULL};
  double* _st_s{NULL}; // s0: 1+dens, s1 .. sk:  3 each.
  bool _set_parameters{false};
  int _pivot{-1};
private: // construct
  explicit LAPLACE(LAPLACE const&);
public:
  explicit LAPLACE(COMMON_COMPONENT* c) : ELEMENT(c) {}
  explicit LAPLACE();
  ~LAPLACE() {
    if (net_nodes() > NODES_PER_BRANCH) {
      delete [] _n;
    }else{
      // it is part of a base class
    }
    delete[] _st_s;
    _st_s = NULL;
    delete[] _st_b_out_;
    _st_b_out_ = NULL;
    delete[] _state;
    _state = NULL;
    // if(_set_parameters){ untested();
    // }else{ untested();
    //   delete[] _st_b_in_;
    //   _st_b_in_ = NULL;
    // }
  }
  CARD* clone()const override;
  std::string dev_type()const override{assert(has_common()); return common()->name();}
private: // ELEMENT, pure
  void tr_iwant_matrix() override { return COMPONENT::tr_iwant_matrix(); }
  void ac_iwant_matrix() override;
  double tr_involts()const override{ untested();
    assert(_input);
    return _input->tr_outvolts();
  }
  double  tr_involts_limited()const override { incomplete(); return 0.; }
  COMPLEX ac_involts()const override { incomplete(); return 0.; }
private: // BASE_SUBCKT
  void	  tr_begin()override	{assert(subckt()); subckt()->tr_begin(); ELEMENT::tr_begin();
      _s_[0]->_loss0 = 1.;
      _s_[0]->_loss1 = 1.;
//      _output->_loss0 = _loss0;
//      _output->_loss1 = _loss1;
  }
  void	  tr_restore()override	{assert(subckt()); subckt()->tr_restore(); ELEMENT::tr_restore();}
  void	  dc_advance()override; //{set_not_converged(); /*really?*/ assert(subckt()); subckt()->dc_advance();}
  void	  tr_advance()override;	// {assert(subckt()); subckt()->tr_advance();}
  void	  tr_regress()override {
    set_not_converged();
    assert(subckt()); subckt()->tr_regress();
    ELEMENT::tr_regress();
  }
  bool	  tr_needs_eval()const override; // {assert(subckt()); return subckt()->tr_needs_eval();}
  // void	  tr_queue_eval()override {assert(subckt()); subckt()->tr_queue_eval();}
  void    tr_queue_eval()override {if(tr_needs_eval()){q_eval();}else{} }
  bool	  do_tr()override; // {assert(subckt());set_converged(subckt()->do_tr());return converged();}
  void	  tr_load()override;
  TIME_PAIR tr_review()override	{assert(subckt()); return _time_by = subckt()->tr_review();}
  void	  tr_accept()override	{assert(subckt()); subckt()->tr_accept(); ELEMENT::tr_accept();}
  void	  tr_unload()override;
  void	  ac_begin()override;
  void	  do_ac()override;
  void	  ac_load()override;
private: // overrides
  double tr_amps()const override;
  bool is_valid()const override;
  void precalc_first()override;
  void expand()override;
  void precalc_last()override;
  double tr_probe_num(std::string const&)const override;
    //void    ac_load();           //BASE_SUBCKT
    //XPROBE  ac_probe_ext(CS&)const;//CKT_BASE/nothing
  int max_nodes()const override {return std::max(4,_n_ports*2);}
  int net_nodes()const override	{return _n_ports*2;}
  int min_nodes()const override {return 4;}
  int int_nodes()const override {
    COMMON_LAPLACE const* c = prechecked_cast<COMMON_LAPLACE const*>(common());
    assert(c);
    int dens = int(c->den_size());
    int num_num = int(c->num_size());
    int num_s = std::max(dens, num_num);

    return num_s;
  }
//  int matrix_nodes()const override { untested();
//    return int_nodes() + max_nodes();
//  }
  std::string value_name()const override {itested(); return "";}
  bool print_type_in_spice()const override {itested(); return false;}
  std::string port_name(int i)const override;
  void set_port_by_index(int Index, std::string& Value)override {
    if(!_n){ untested();
      trace2("spbi", long_label(), matrix_nodes());
      _n = new node_t[max_nodes()];
    }else{
      trace2("spbi -", long_label(), matrix_nodes());
    }
    // COMPONENT::set_port_by_index(Index, Value);
    ELEMENT::set_port_by_index(Index, Value);
  }
/*--------------------------------------------------------------------------*/
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
  node_t const& state_node(int k)const {return _n[net_nodes()+k];}
  node_t& state_node(int k) {return _n[net_nodes()+k];}
private: // impl
  friend class COMMON_LAPLACE;
}; // LAPLACE
/*--------------------------------------------------------------------------*/
bool COMMON_LAPLACE::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_LAPLACE* p = dynamic_cast<const COMMON_LAPLACE*>(&x);
  return (p
    && _tolerance == p->_tolerance
    && COMMON_RF_BASE::operator==(x));
}
/*--------------------------------------------------------------------------*/
void COMMON_LAPLACE::precalc_last(const CARD_LIST* par_scope)
{
  COMMON_RF_BASE::precalc_last(par_scope);
  e_val(&_tolerance, 0. , par_scope);
}
/*--------------------------------------------------------------------------*/
void COMMON_LAPLACE::set_param_by_index(int I, std::string& Value, int Offset)
{
	incomplete();
  switch (COMMON_LAPLACE::param_count() - 1 - I) {
  default: COMMON_RF_BASE::set_param_by_index(I, Value, Offset);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
static COMMON_LAPLACE Default_test_lap2(CC_STATIC);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void LAPLACE::ac_iwant_matrix()
{
  if(ac_use_sckt){ untested();
    COMPONENT::ac_iwant_matrix();
  }else{
    // ELEMENT::ac_iwant_matrix_passive();
    // ac_iwant_matrix_extended()
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
  }
}
/*--------------------------------------------------------------------------*/
double LAPLACE::tr_probe_num(std::string const& n) const
{
  COMMON_LAPLACE const* c = prechecked_cast<COMMON_LAPLACE const*>(common());
  assert(c);
  int dens = int(c->den_size());

  if(n[0] == 's'){
    int idx = atoi(n.substr(1).c_str());
    if(idx < dens){
      return _n[net_nodes() + idx].v0();
    }else{ untested();
    }
  }else{ untested();
  }

  if(n == "vin") { untested();
    return _st_b_in_[0];
  }else if(n == "conv") { untested();
    return converged();
  }else{ untested();
    return ELEMENT::tr_probe_num(n);
  }
}
/*--------------------------------------------------------------------------*/
// LAPLACE::LAPLACE() : ELEMENT()
// { untested();
//   attach_common(&Default_test_lap2);
//   // build netlist
//   // ports:2
//   // overrides
// }
/*--------------------------------------------------------------------------*/
// seq blocks
/*--------------------------------------------------------------------------*/
bool LAPLACE::tr_needs_eval()const
{
  trace1("test_lap2::needs_eval?", long_label());
  node_t gnd(&ground_node);
  if (is_q_for_eval()) { untested();
    return false;
  }else if (!converged()) {
    return true;
  }else   if(_sim->is_first_iteration()){ untested();
    return true;
  }else{
    for(int k=0; k<int_nodes(); ++k){
      if( !conchk(_state[k], state_node(k).v0(), 1e-6)){
	return true;
      }else{
      }
    }
    return false;
  }
};
/*--------------------------------------------------------------------------*/
bool LAPLACE::do_tr()
{
  trace2("test_lap2::do_tr", this, long_label());
  COMMON_LAPLACE const* c = prechecked_cast<COMMON_LAPLACE const*>(common());
  assert(c);
  set_converged();
  int dens = int(c->den_size());

  node_t gnd(&ground_node);

  for(int k=0; k<int_nodes(); ++k){
    _state[k] = volts_limited(state_node(k), gnd);
  }
  // _involts = volts_limited(_n[n_in0], _n[n_in1]);

  // common?
  assert(_st_s);
  // _st_s[0] = ?
  int num_s0_states = 1 + dens;

  for (int k=_pivot; k < int_nodes()-1; ++k){
    trace2("test_lap2::do_tr", k, volts_limited(state_node(k), gnd));
    _st_s[num_s0_states + 3*k] = _state[k]; // volts_limited(_n[n_s0+k], gnd);
  }

  for (int k=0; k < _pivot; ++k){
    trace2("test_lap2::do_tr", k, volts_limited(state_node(k), gnd));
    _st_s[num_s0_states + 3*k] = _state[k+1]; // volts_limited(_n[n_s0+k], gnd);
  }

  _st_s[0] = 0; // LINEAR;

  assert(subckt());
  set_converged(subckt()->do_tr() && converged());
  return converged();
}
/*--------------------------------------------------------------------------*/
// typedef LAPLACE::ddouble ddouble;
/*--------------------------------------------------------------------------*/
inline void LAPLACE::dc_advance()
{
  set_not_converged();
  assert(subckt());
  subckt()->dc_advance();
  return ELEMENT::dc_advance();
}
/*--------------------------------------------------------------------------*/
inline void LAPLACE::tr_advance()
{
  set_not_converged();
  assert(subckt());
  subckt()->tr_advance();
  return ELEMENT::tr_advance();
}
/*--------------------------------------------------------------------------*/
std::string LAPLACE::port_name(int i)const
{
	assert(i >= 0);
	assert(i < max_nodes());
	static std::string names[] = {"out0", "out1", "in0", "in1" };
	return names[i];
}
/*--------------------------------------------------------------------------*/
LAPLACE laplace(&cl);
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, "va_laplace", &laplace);
LAPLACE laplace_nd(&cl_nd);
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "va_laplace_nd", &laplace_nd);
LAPLACE laplace_zd(&cl_zd);
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_laplace_zd", &laplace_zd);
LAPLACE laplace_zp(&cl_zp);
DISPATCHER<CARD>::INSTALL d3(&device_dispatcher, "va_laplace_zp", &laplace_zp);
LAPLACE laplace_np(&cl_np);
DISPATCHER<CARD>::INSTALL d4(&device_dispatcher, "va_laplace_np", &laplace_np);
LAPLACE laplace_rp(&cl_rp);
DISPATCHER<CARD>::INSTALL d5(&device_dispatcher, "va_laplace_rp", &laplace_rp);
/*--------------------------------------------------------------------------*/
CARD* LAPLACE::clone()const
{
  LAPLACE* new_instance = new LAPLACE(*this);
  assert(!new_instance->subckt());
  return new_instance;
}
/*--------------------------------------------------------------------------*/
LAPLACE::LAPLACE(LAPLACE const&p) : ELEMENT(p)
{
  if(p._n){
    trace2("laplace", int_nodes(), max_nodes());
    assert(_n);
    if(int_nodes() + max_nodes() <= NODES_PER_BRANCH){
       // not expanded yet...?
    }else{
      _n = new node_t[int_nodes() + max_nodes()];
      for (int ii = 0; ii < p.net_nodes(); ++ii) {
	_n[ii] = p._n[ii];
      }
    }
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void LAPLACE::precalc_first()
{
  ELEMENT::precalc_first();
  trace2("test_lap2::pf", long_label(), mfactor());
  auto c = static_cast<COMMON_LAPLACE*>(mutable_common());
  assert(c);
  auto cc = c->clone();
  if(subckt()){
    attach_common(NULL);
    attach_common(cc);
    subckt()->precalc_first();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
bool LAPLACE::is_valid()const
{ untested();
  COMMON_LAPLACE const* c = prechecked_cast<COMMON_LAPLACE const*>(common());
  assert(c);
  return c->is_valid();
}
/*--------------------------------------------------------------------------*/
int COMMON_LAPLACE::pivot() const
{
//  return 0;
  COMMON_LAPLACE const* c = this;
  assert(c);
  if(c->den_is_p()){
    return c->num_states()-1;
  }else{
  }
  int dens = int(c->den_size());
  double p = 0;
  int r = 0;
  for(int i=0; i<dens; ++i){
    double pi = fabs(c->_p_den[i]);
    trace2("piv", i, pi);
    if(pi>p){
      p = pi;
      r = i;
    }else{
    }
  }
  trace3("pivot", c->den_size(), p, dens); 

  if(p == 0.){
    // elaboration order fallback hack.
    return dens-1;
  }else{
    return r;
  }
}
/*--------------------------------------------------------------------------*/
void LAPLACE::expand()
{
  assert(common());
  auto c = prechecked_cast</*const*/ COMMON_LAPLACE*>(mutable_common());
  assert(c);
  int dens = int(c->den_size());
  assert(dens);
  _pivot = c->pivot();
  assert(_pivot >= 0);
  trace2("expand", dens, _pivot);
  if(_pivot==0){
  }else if(_pivot==1){
  }else if(_pivot==2){
  }else if(_pivot==3){ untested();
  }else if(_pivot==4){ untested();
  }else{ untested();
  }

  int num_num = int(c->num_size());
  int num_s = std::max(dens, num_num);
  assert(c->num_states() == num_s || !c->num_states());
  if(c->num_states()>num_s){
    num_s = c->num_states();
  }else{
  }
  node_t gnd;
  gnd.set_to_ground(this);

  trace4("expand", short_label(), c->_p_num.size(), c->num_size(), _pivot);
  trace3("expand", short_label(), c->_p_den.size(), c->den_size());
  trace5("expand", short_label(), _pivot, num_num, num_s, c->is_rp());

  ELEMENT::expand();
  assert(_n);
  if (!subckt()) {
    new_subckt();
  }else{
  }

  _s_.resize(num_s);

  auto nn = new node_t[net_nodes() + num_s];
  notstd::copy_n(_n, net_nodes(), nn);
  _n = nn;
  if(net_nodes()==4){
  }else{
  }

  if(_state){
  }else{
    _state = new double[int_nodes()];
    std::fill_n(_state, int_nodes(), 0.);
  }

  // node_t* _ni = _n + 4;
  for(int jj = 0; jj < num_s; ++jj) {
    if (!(state_node(jj).n_())) {
      state_node(jj).new_model_node("." + long_label() + ".s" + to_string(jj), this);
    }else{ untested();
	//_n[n_s].new_model_node("s." + long_label(), this);
    }
  }

  if (_sim->is_first_expand()) {
    trace4("expand first", short_label(), int_nodes(), num_s, num_num);

    int instate = 0;
    if(_st_b_in_){
    }else{
      instate = 3;
    }

    if(_st_s){
    }else{
      // s00 s01 s1 s2 .. sk | s1 X s0 | s2 X s1 ....
      _st_s = new double[1 + num_s + (num_s-1)*3 + instate];
    }
    if(_st_b_out_){
    }else{
      // s00 s01 s1 s2 .. sk | s1 X s0 | s2 X s1 ....
      _st_b_out_ = new double[2 + num_num];
    }

    if(_st_b_in_){
    }else{
      _st_b_in_ =  _st_s + 1 + num_s + (num_s-1)*3;
    }

    trace2("expand2.", long_label(), c->_p_den.size());
    precalc_first();
    c = static_cast</*const*/ COMMON_LAPLACE*>(mutable_common());
    assert(c);


    trace2("expand2a", long_label(), c->_p_den.size());

    std::string output_elt_type = "va_flow";
    std::string d_ddt_type = "va_ddt";
    std::string d_idt_type = "va_idt";

//    output_elt_type = "va_pot";
//    s_elt_type = "va_idt"; needs reverse order num coeffs

    {
      if (!_output) {
	const CARD* p = device_dispatcher[output_elt_type];
	if(!p){ untested();
	  throw Exception("Cannot find "+output_elt_type+". Load module?");
	}else{
	}
	trace2("expand2b", long_label(), c->_p_den.size());
	_output = dynamic_cast<ELEMENT*>(p->clone());
	if(!_output){ untested();
	  throw Exception("Cannot use "+output_elt_type+": wrong type");
	}else{
	}
	subckt()->push_front(_output);
      }else{
      }

      assert(num_s >= num_num);
      std::vector<node_t> nodes(num_s*2 + 2);
      nodes[0] = _n[n_out0];
      nodes[1] = _n[n_out1];
      int jj;
      for(jj = 0; jj < _pivot; ++jj) {
	nodes[2+jj*2] = state_node(jj);
	nodes[2+jj*2 + 1] = gnd;
      }

//      jj = num_num-_pivot

      for(; jj < num_num; ++jj) {
	nodes[2+jj*2] = state_node(jj);
	nodes[2+jj*2 + 1] = gnd;
      }

      _output->set_parameters("out", this, &_C_V_electrical, 0., /*states:*/2 + num_num, _st_b_out_,
	   2+2*num_num, nodes.data());
    }

    const CARD* ddt = device_dispatcher[d_ddt_type];
    if(!ddt){ untested();
      throw Exception("Cannot find "+d_ddt_type+". Load module?");
    }else{
    }
    const CARD* idt = device_dispatcher[d_idt_type];
    if(!idt){ untested();
      throw Exception("Cannot find "+d_idt_type+". Load module?");
    }else{
    }

    int n_inputs = _n_ports - 1;
    {
      std::string input_dev_type;
      if(_set_parameters) {
	input_dev_type = "va_fpoly_g";
      }else{
	input_dev_type = "va_flow";
      }
      if (!_input) {
	const CARD* input_dev = device_dispatcher[input_dev_type];
	if(!input_dev){ untested();
	  throw Exception("Cannot find " + input_dev_type + ". Load module?");
	}else{
	}
	_input = dynamic_cast<ELEMENT*>(input_dev->clone());
	if(!_input){ untested();
	  throw Exception("Cannot use " + input_dev_type + ": wrong type");
	}else{
	}
	subckt()->push_front(_input);
      }else{
      }
      std::vector<node_t> nodes(dens*2 + 2 + 2*n_inputs);
      nodes[1] = state_node(_pivot);
      nodes[0] = gnd;

      for(int k=0; k<2*n_inputs; ++k){
	nodes[2+k] = _n[n_in0+k];
      }

      trace2("expand4 input", long_label(), c->_p_den.size());
      // TODO: current inputs?
      assert(_input);
      _input->set_parameters("in", this, &_C_V_electrical, 0., /*states:*/2+n_inputs, _st_b_in_, 2+2*n_inputs, nodes.data());
    }

    int num_s0_states = 1 + dens;
    trace1("expand", num_s0_states);
    {
      if (!_s_[0]) {
	const CARD* s0_dev = device_dispatcher["va_pot"];
	if(!s0_dev){ untested();
	  throw Exception("Cannot find va_pot. Load module?");
	}else{
	}
	_s_[0] = dynamic_cast<ELEMENT*>(s0_dev->clone());
	if(!_s_[0]){ untested();
	  throw Exception("Cannot use va_pot: wrong type");
	}else{
	}
	subckt()->push_front(_s_[0]);
      }else{
      }
      std::vector<node_t> nodes(num_s*2);

      nodes[0] = state_node(_pivot);
      nodes[1] = gnd;
      int jj;
      for(jj = 0; jj < _pivot; ++jj) {
	nodes[jj*2 + 2] = state_node(jj);
	nodes[jj*2 + 3] = gnd;
      }
      for(++jj; jj < dens; ++jj) {
	nodes[jj*2] = state_node(jj);
	nodes[jj*2 + 1] = gnd;
      }

      trace2("expand4 s0", long_label(), c->_p_den.size());
      _s_[0]->set_parameters("s0", this, &_C_V_electrical, 0., num_s0_states, _st_s, 2*dens, nodes.data());
      _s_[0]->_loss0 = 1.;
      _s_[0]->_loss1 = 1.;
    }
    for(int jj = _pivot+1; jj < num_s; ++jj) {
      assert(jj < int(_s_.size()));
      if (!_s_[jj]) {
	_s_[jj] = dynamic_cast<ELEMENT*>(ddt->clone());
	if(!_s_[jj]){ untested();
	  throw Exception("Cannot use va_ddt wrong type");
	}else{
	}
	subckt()->push_front(_s_[jj]);
      }else{
      }
      {
	node_t nodes[] = {state_node(jj), gnd, state_node(jj-1), gnd};
	_s_[jj]->set_parameters("s" + to_string(jj), this, _s_[jj]->mutable_common(), 0.,
	                          3, &_st_s[3*(jj-1)+num_s0_states], 4, nodes);
	_s_[jj]->_loss0 = 1.;
	_s_[jj]->_loss1 = 1.;
      }
    }

    for(int jj = 0; jj < _pivot; ++jj) {
      trace2("build int", long_label(), jj);
      assert(jj < int(_s_.size()));
      if (!_s_[jj+1]) {
	_s_[jj+1] = dynamic_cast<ELEMENT*>(idt->clone());
	if(!_s_[jj+1]){ untested();
	  throw Exception("Cannot use va_idt wrong type");
	}else{
	}
	subckt()->push_front(_s_[jj+1]);
      }else{
      }
      {
	node_t nodes[] = {state_node(jj), gnd, state_node(jj+1), gnd};
	_s_[jj+1]->set_parameters("s" + to_string(jj), this, _s_[jj+1]->mutable_common(), 0.,
	                          3, &_st_s[3*(jj)+num_s0_states], 4, nodes);
	_s_[jj+1]->_loss0 = 1.;
	_s_[jj+1]->_loss1 = 1.;
      }
    }
  }else{ untested();
  }

  subckt()->expand();
  //subckt()->precalc();
  assert(!is_constant());
  for(CARD_LIST::iterator i=subckt()->begin(); i!=subckt()->end(); ++i){
    CARD* d = (*i)->deflate();
    if(d == (*i)) {
    }else{ untested();
      assert(d->owner() == this);
      delete *i;
      *i = d;
    }
  }

  // after precalc_last
//  assert( c->_p_num.size());
//  assert( c->_p_den.size());
} // expand
/*--------------------------------------------------------------------------*/
void LAPLACE::precalc_last()
{
  {
    auto c = static_cast<COMMON_LAPLACE*>(mutable_common());
    assert(c);
    trace1("LAPLACE::precalc_last0a", c->_type);
  }
  ELEMENT::precalc_last();
  auto c = static_cast<COMMON_LAPLACE*>(mutable_common());
  assert(c);
  trace1("LAPLACE::precalc_last0b", c->_type);

  LAPLACE* m = this;
  LAPLACE const* p = this;
  assert(p);

  int dens = int(c->den_size());
  int num_num = int(c->num_size());
  int num_s = std::max(dens, num_num);
  trace4("LAPLACE::precalc_last", num_s, _pivot, num_num, c->_type);

  int num_s0_states = 1 + dens;

  _st_b_out_[0] = 0;
  _st_b_out_[1] = 0; // _st_b_in_[1];

  double piv = c->_p_den[_pivot];
  if(fabs(piv)<OPT::shortckt){ untested();
    error(bDANGER, long_label() + ": pivot too small\n");
    piv = OPT::shortckt;
  }else{
  }

  for(int jj=0; jj<num_num; ++jj){
    _st_b_out_[2+jj] = c->_p_num[jj] / piv;
    trace2("preclast", jj, _st_b_out_[jj+2]);
  }

  assert(_st_b_in_);
  double mhack = _st_b_in_[1];
  _st_b_in_[1] = 0.;
  if(_set_parameters){
  }else{
    _st_b_in_[0] = 0.;
    _st_b_in_[2] = 1.;
  }

  _st_s[0] = 0.;
  _st_s[1] = 0.;
  int jj;
  for(jj=0; jj<_pivot; ++jj){
    // _st_b_in_[jj+2] = - c->_p_den[jj] / piv;
    _st_s[jj+2] = - c->_p_den[jj] / piv;
  }
  for(++jj; jj<num_s; ++jj){
    // _st_b_in_[jj+2] = - c->_p_den[jj] / piv;
    _st_s[jj+1] = - c->_p_den[jj] / piv;
  }

  for(jj=0; jj<_pivot; ++jj){
    _st_s[num_s0_states + 0 + (jj)*3] = 0.; // initial state
    _st_s[num_s0_states + 1 + (jj)*3] = 1.; // mfactor hack
    _st_s[num_s0_states + 2 + (jj)*3] = 1.; // deriv.
  }
  for(++jj; jj<num_s; ++jj){
    _st_s[num_s0_states + 0 + (jj-1)*3] = 0.; // initial state
    _st_s[num_s0_states + 1 + (jj-1)*3] = 1.; // mfactor hack
    _st_s[num_s0_states + 2 + (jj-1)*3] = 1.; // deriv.
  }

  assert(m->_st_s);
  if(subckt()){
    subckt()->precalc_last();
  }else{untested();
  }

  trace1("LAPLACE::precalc_last mfactor?", _st_b_in_[1]);
    // _st_b_in_[1] = 1.;
  if(_set_parameters) {
    assert(_output);
    ((COMPONENT*)_output)->set_param_by_name("$mfactor", "");
    ((COMPONENT*)_output)->set_param_by_name("$mfactor", to_string(mhack));
    _output->COMPONENT::precalc_first();
  }else{
  }
  trace5("LAPLACE::precalc_last2", num_s, _pivot, num_num, c->_type, mfactor());
} // precalc_last
/*--------------------------------------------------------------------------*/
void LAPLACE::set_parameters(const std::string& Label, CARD *Owner,
				   COMMON_COMPONENT *Common, double Value,
				   int n_states, double states[],
				   int n_nodes, const node_t nodes[])
{
  bool first_time = !_set_parameters;
  _set_parameters = true;
  auto p = prechecked_cast<COMMON_FILT const*>(Common);
  assert(p);
  int dens = p->args(2);
  int nums = p->args(1);

  trace4("LAPLACE::set_parameters", n_states, n_nodes, nums, dens);

  assert(common());
  _n_ports = n_nodes/2;
  _st_b_in_ = states;

  auto ccc = common()->clone();
  auto cc = prechecked_cast<COMMON_LAPLACE*>(ccc);
  assert(cc);

  // HACK
  cc->set_args(1, p->args(1));
  cc->set_args(2, p->args(2));
  if(first_time){
    cc->_p_den.resize(dens);
    cc->_p_num.resize(nums);
  }else{
  }
  int num_s = cc->num_states();

  set_label(Label);
  set_owner(Owner);
  set_value(Value);
  attach_common(cc);

  if (first_time) {
    assert(n_states - 1 - n_nodes/2 == 0); // for now.
//    _current_port_names.resize(n_states - 1 - n_nodes/2);
    // _input.resize(n_states - 1 - n_nodes/2);
 //   _n_ports = n_nodes/2; // sets num_nodes() = _n_ports*2
    _n = new node_t[net_nodes() + num_s];
  }else{untested();
    assert(net_nodes() == n_nodes);
    // assert could fail if changing the number of nodes after a run
  }

  // _vy0 = states;
  assert(net_nodes() == n_nodes);
  trace4("setnodes", net_nodes(), nodes, dens, n_nodes);
  notstd::copy_n(nodes, net_nodes(), _n);
  _loss1 = _loss0 = 1.;

}
/*--------------------------------------------------------------------------*/
void LAPLACE::ac_load()
{
  //ac_load_shunt(); // BUG. explicit mfactor.
  _sim->_acx.load_symmetric(_n[OUT1].m_(), _n[OUT2].m_(), mfactor() * _output->mfactor() * _loss0);
  if(ac_use_sckt){ untested();
    assert(subckt());
    subckt()->ac_load();
  }else{
//    _acg = _values[1];
  //  ac_load_source();
   auto _values = _st_b_in_;
    for (int i=2; i<=_n_ports; ++i) {
      ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _values[i] * _acg);
    }

  }
}
/*--------------------------------------------------------------------------*/
void LAPLACE::do_ac()
{
  if(ac_use_sckt){ untested();
    assert(subckt());
    subckt()->do_ac();
  }else{
    auto c = prechecked_cast<COMMON_LAPLACE const*>(common());
    assert(c);
    assert( c->_p_num.size());
    assert( c->_p_den.size());

    COMPLEX z = _sim->_jomega;
    COMPLEX num = evalp(z, c->_p_num.begin(), c->_p_num.size());
    COMPLEX den = evalp(z, c->_p_den.begin(), c->_p_den.size());

    // BUG: explicit mfactor.
    trace1("do_ac mfactor hack", _output->mfactor());
    _acg = _output->mfactor() * num/den;
  }
}
/*--------------------------------------------------------------------------*/
void LAPLACE::ac_begin()
{
  if(ac_use_sckt){ untested();
    assert(subckt());
    subckt()->ac_begin();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void LAPLACE::tr_load()
{
  tr_load_shunt();
  assert(subckt());
  trace2("LAPLACE::tr_load", _loaditer, _sim->iteration_tag());
  subckt()->tr_load();
}
/*--------------------------------------------------------------------------*/
void LAPLACE::tr_unload()
{ untested();
  incomplete();
  subckt()->tr_unload();
}
/*--------------------------------------------------------------------------*/
double LAPLACE::tr_amps() const
{
  assert(_output);
  double r = _output->tr_amps();
  if(_loss0){
    assert(_loss0==1.);
    incomplete();
    // r = -r;
  }else{ untested();
  }
  trace2("LAPLACE::tr_amps", r, _loss0);
  return r;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
