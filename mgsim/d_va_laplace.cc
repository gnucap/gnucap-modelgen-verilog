/*                        -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
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
 */
#include <globals.h>
#include <e_compon.h>
#include <e_subckt.h>
#include <e_node.h>
#include <e_elemnt.h>
#include <u_nodemap.h>
#include "e_va.h"
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
class LAPLACE;
class COMMON_LAPLACE :public COMMON_COMPONENT{
  typedef LAPLACE MOD;
public:
  explicit COMMON_LAPLACE(const COMMON_LAPLACE& p);
  explicit COMMON_LAPLACE(int c=0);
           ~COMMON_LAPLACE();
  bool     operator==(const COMMON_COMPONENT&)const override;
  COMMON_LAPLACE* clone()const override {return new COMMON_LAPLACE(*this);}
  void     set_param_by_index(int, std::string&, int)override;
  int      set_param_by_name(std::string, std::string)override;
  bool     is_valid()const;
  bool     param_is_printable(int)const override;
  std::string param_name(int)const override;
  std::string param_name(int,int)const override;
  std::string param_value(int)const override;
  int param_count()const override {
	  return int(_p_num.size() + _p_den.size())
			  + COMMON_COMPONENT::param_count();
  }
  void precalc_first(const CARD_LIST*)override;
  void expand(const COMPONENT*)override;
  void precalc_last(const CARD_LIST*)override;
  void tr_eval_analog(LAPLACE*)const;
//  void precalc_analog(LAPLACE*)const;
  std::string name()const override {itested();return "va_laplace_nd";}
  static int  count() {return _count;}
private: // strictly internal
  static int _count;
public: // input parameters
//  PARAMETER<int> _p_vs;
  std::vector<PARAMETER<double> > _p_num;
  std::vector<PARAMETER<double> > _p_den;
}cl; //COMMON_LAPLACE
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
  explicit LAPLACE();
  ~LAPLACE() {
    if (net_nodes() > NODES_PER_BRANCH) { untested();
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
    // if(_set_parameters){
    // }else{
    //   delete[] _st_b_in_;
    //   _st_b_in_ = NULL;
    // }
  }
  CARD* clone()const override;
  std::string dev_type()const override{assert(has_common()); return common()->name();}
private: // ELEMENT, pure
  void tr_iwant_matrix() override { return COMPONENT::tr_iwant_matrix(); }
  void ac_iwant_matrix() override { return COMPONENT::ac_iwant_matrix(); }
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
  void	  ac_begin()override	{assert(subckt()); subckt()->ac_begin();}
  void	  do_ac()override	{assert(subckt()); subckt()->do_ac();}
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
    int dens = int(c->_p_den.size());
    int num_num = int(c->_p_num.size());
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
  int pivot()const;
private: // impl
  friend class COMMON_LAPLACE;
}; // LAPLACE
/*--------------------------------------------------------------------------*/
COMMON_LAPLACE::COMMON_LAPLACE(int c)
  :COMMON_COMPONENT(c),
   _p_num(/*default*/),
   _p_den(/*default*/)
{
  ++_count;
}
/*--------------------------------------------------------------------------*/
COMMON_LAPLACE::COMMON_LAPLACE(const COMMON_LAPLACE& p)
  :COMMON_COMPONENT(p),
   _p_num(p._p_num),
   _p_den(p._p_den)
{
  ++_count;
}
/*--------------------------------------------------------------------------*/
COMMON_LAPLACE::~COMMON_LAPLACE()
{
  --_count;
}
/*--------------------------------------------------------------------------*/
bool COMMON_LAPLACE::operator==(const COMMON_COMPONENT& x)const
{
  const COMMON_LAPLACE* p = dynamic_cast<const COMMON_LAPLACE*>(&x);
  return (p
    && _p_num == p->_p_num
    && _p_den == p->_p_den
    && COMMON_COMPONENT::operator==(x));
}
/*--------------------------------------------------------------------------*/
void COMMON_LAPLACE::set_param_by_index(int I, std::string& Value, int Offset)
{ untested();
	incomplete();
  switch (COMMON_LAPLACE::param_count() - 1 - I) {
  default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset);
  }
}
/*--------------------------------------------------------------------------*/
int COMMON_LAPLACE::set_param_by_name(std::string Name, std::string Value)
{
  if(Name == "$mfactor"){ untested();
    incomplete();
    Name = "m";
  }else{
  }
  trace2("spbn", Name, Value);
  if(Name[0] == 'd'){
    size_t idx = atoi(Name.substr(1).c_str());
    assert(idx<20);
    if(idx < _p_den.size()){
    }else{
      _p_den.resize(idx + 1);
    }
    _p_den[idx] = Value;
  }else if(Name[0] == 'n'){
    size_t idx = atoi(Name.substr(1).c_str());
    assert(idx<20);
    if(idx < _p_num.size()){
    }else{
      _p_num.resize(idx + 1);
    }
    _p_num[idx] = Value;
  }else{ untested();
  }

  return 0; // incomplete();
}
/*--------------------------------------------------------------------------*/
bool COMMON_LAPLACE::param_is_printable(int i)const
{
  size_t idx = COMMON_LAPLACE::param_count() - 1 - i;
  if(idx < _p_num.size() + _p_den.size()){
    return true;
  }else{
    return COMMON_COMPONENT::param_is_printable(i);
  }
}
/*--------------------------------------------------------------------------*/
std::string COMMON_LAPLACE::param_name(int i)const
{
  size_t idx = COMMON_LAPLACE::param_count() - 1 - i;
  if(idx < _p_num.size()){
    return "n" + to_string(int(idx));
  }else if(idx < _p_num.size() + _p_den.size()){
    return "d" + to_string(int(idx - _p_num.size()));
  }else{ untested();
    return COMMON_COMPONENT::param_name(i);
  }
}
/*--------------------------------------------------------------------------*/
std::string COMMON_LAPLACE::param_name(int i, int j)const
{ untested();
  if(j==0){ untested();
    return param_name(i);
  }else{ untested();
    return "";
  }
}
/*--------------------------------------------------------------------------*/
std::string COMMON_LAPLACE::param_value(int i)const
{
  size_t idx = COMMON_LAPLACE::param_count() - 1 - i;
  if (idx < _p_num.size()) {
	  return _p_num[idx].string();
  }else if (idx < (_p_num.size() + _p_den.size())) {
	  return _p_den[idx - _p_num.size()].string();
  }else{ untested();
	  return COMMON_COMPONENT::param_value(i);
  }
}
/*--------------------------------------------------------------------------*/
bool COMMON_LAPLACE::is_valid() const
{ untested();
  return true; //COMMON_COMPONENT::is_valid();
}
/*--------------------------------------------------------------------------*/
void COMMON_LAPLACE::expand(const COMPONENT* d)
{
  COMMON_COMPONENT::expand(d);
}
/*--------------------------------------------------------------------------*/
void COMMON_LAPLACE::precalc_first(const CARD_LIST* par_scope)
{
  assert(par_scope);
  COMMON_COMPONENT::precalc_first(par_scope);
  for(auto &pp : _p_num) {
    e_val(&(pp), 0., par_scope);
  }
  for(auto &pp : _p_den) {
    e_val(&(pp), 0., par_scope);
  }
}
/*--------------------------------------------------------------------------*/
void COMMON_LAPLACE::precalc_last(const CARD_LIST* par_scope)
{
  assert(par_scope);
  COMMON_COMPONENT::precalc_last(par_scope);
  for(auto &pp : _p_num) {
    e_val(&(pp),     0. , par_scope);
  }
  for(auto &pp : _p_den) {
    e_val(&(pp),     0. , par_scope);
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
int COMMON_LAPLACE::_count = -1;
static COMMON_LAPLACE Default_test_lap2(CC_STATIC);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
double LAPLACE::tr_probe_num(std::string const& n) const
{
  COMMON_LAPLACE const* c = prechecked_cast<COMMON_LAPLACE const*>(common());
  assert(c);
  int dens = int(c->_p_den.size());

  if(n[0] == 's'){
    int idx = atoi(n.substr(1).c_str());
    if(idx < dens){
      return _n[net_nodes() + idx].v0();
    }else{ untested();
    }
  }else{
  }

  if(n == "vin") {
    return _st_b_in_[0];
  }else if(n == "conv") { untested();
    return converged();
  }else{
    return ELEMENT::tr_probe_num(n);
  }
}
/*--------------------------------------------------------------------------*/
LAPLACE::LAPLACE() : ELEMENT()
{
  attach_common(&Default_test_lap2);
  // build netlist
  // ports:2
  // overrides
}
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
  int dens = int(c->_p_den.size());

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
LAPLACE m_test_lap2;
DISPATCHER<CARD>::INSTALL d0(&device_dispatcher, "va_laplace_nd", &m_test_lap2);
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
    assert(int_nodes() + max_nodes() <= NODES_PER_BRANCH); // not expanded yet.
//    _n = new node_t[int_nodes() + max_nodes()];
//    for (int ii = 0; ii < p.net_nodes(); ++ii) {
//      _n[ii] = p._n[ii];
//    }
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
int LAPLACE::pivot() const
{
//  return 0;
  COMMON_LAPLACE const* c = prechecked_cast<COMMON_LAPLACE const*>(common());
  assert(c);
  int dens = int(c->_p_den.size());
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
  auto c = static_cast</*const*/ COMMON_LAPLACE*>(mutable_common());
  assert(c);
  int dens = int(c->_p_den.size());
  assert(dens);
  _pivot = pivot();
  if(_pivot==0){
  }else if(_pivot==1){
  }else if(_pivot==2){
  }else if(_pivot==3){ untested();
  }else if(_pivot==4){ untested();
  }else{ untested();
  }

  int num_num = int(c->_p_num.size());
  int num_s = std::max(dens, num_num);
  node_t gnd;
  gnd.set_to_ground(this);

  trace2("expand", long_label(), _pivot);
  ELEMENT::expand();
  assert(_n);
  if (!subckt()) {
    new_subckt();
  }else{ untested();
  }

  _s_.resize(num_s);
  trace2("expand", long_label(), c->_p_den.size());

  auto nn = new node_t[net_nodes() + num_s];
  notstd::copy_n(_n, net_nodes(), nn);
  _n = nn;
  if(net_nodes()==4){
  }else{
  }

  if(_state){ untested();
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
  trace3("expand2", long_label(), c->_p_den.size(), int_nodes());

  if (_sim->is_first_expand()) {

    int instate = 0;
    if(_st_b_in_){
    }else{
      instate = 3;
    }

    if(_st_s){ untested();
    }else{
      // s00 s01 s1 s2 .. sk | s1 X s0 | s2 X s1 ....
      _st_s = new double[1 + num_s + (num_s-1)*3 + instate];
    }
    if(_st_b_out_){ untested();
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
      }else{ untested();
      }

      std::vector<node_t> nodes(num_num*2 + 2);
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

      _output->set_parameters("out", this, &_C_V_electrical, 0., /*states:*/2 + num_num, _st_b_out_, 2+2*num_num, nodes.data());
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
	  throw Exception("Cannot find va_pot. Load module?");
	}else{
	}
	_input = dynamic_cast<ELEMENT*>(input_dev->clone());
	if(!_input){ untested();
	  throw Exception("Cannot use va_pot: wrong type");
	}else{
	}
	subckt()->push_front(_input);
      }else{ untested();
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
      }else{ untested();
      }
      std::vector<node_t> nodes(num_s*2);

//      nodes[2] = _n[n_in0];
//      nodes[3] = _n[n_in1];

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
      }else{ untested();
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
      }else{ untested();
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
}
/*--------------------------------------------------------------------------*/
void LAPLACE::precalc_last()
{
  ELEMENT::precalc_last();
  auto c = static_cast<COMMON_LAPLACE*>(mutable_common());
  assert(c);

  LAPLACE* m = this;
  LAPLACE const* p = this;
  assert(p);

  int dens = int(c->_p_den.size());
  int num_num = int(c->_p_num.size());
  int num_s = std::max(dens, num_num);
  trace1("LAPLACE::precalc_last", _pivot);

  int num_s0_states = 1 + dens;

  _st_b_out_[0] = 0;
  _st_b_out_[1] = 0;

  double piv = c->_p_den[_pivot];

  for(int jj=0; jj<num_num; ++jj){
    _st_b_out_[2+jj] = c->_p_num[jj] / piv;
    trace2("preclast", jj, _st_b_out_[jj+2]);
  }

  assert(_st_b_in_);
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
}
/*--------------------------------------------------------------------------*/
void LAPLACE::set_parameters(const std::string& Label, CARD *Owner,
				   COMMON_COMPONENT *Common, double Value,
				   int n_states, double states[],
				   int n_nodes, const node_t nodes[])
  //				   const double* inputs[])
{
  trace2("LAPLACE::set_parameters", n_states, n_nodes);
  bool first_time = !_set_parameters;
  _set_parameters = true;
  auto p = prechecked_cast<COMMON_FILT const*>(Common);
  assert(p);
  int dens = p->args(2);
  int nums = p->args(1);
  int num_s = std::max(dens, nums);

  auto cc = cl.clone();
  _n_ports = n_nodes/2;
  _st_b_in_ = states;

  cc->_p_den.resize(dens);
  cc->_p_num.resize(nums);
  trace2("L", nums, dens);

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
  ac_load_shunt();
  assert(subckt());
  subckt()->ac_load();
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
  trace1("LAPLACE::tr_amps", r);
  return r;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet
