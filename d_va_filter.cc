/*                     -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023 Felix Salfelder
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
 * DEV_CPOLY_CAP
 * number of nodes = 2*n_ports
 * number of val, ov = n_ports+1
 * val[0] is the constant part, val[1] is self admittance,
 *   val[2+] are transadmittances, up to n_ports
 * node[0] and node[1] are the output.
 * node[2] up are inputs.
 * node[2*i] and node[2*i+1] correspond to val[i+1]
 */
#include <globals.h>
#include <e_storag.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
FPOLY1 integrate(const FPOLY1* q, const FPOLY1* i, double* time,
		     METHOD method, double dt)
{
  if (time[1] == 0) {
    method = mEULER;	// Bogus current in previous step.  Force Euler.
  }else{
  }

  if( CKT_BASE::_sim->analysis_is_static() ) {
    assert(time[0] == 0.);
    return FPOLY1(q[0].x, 0., 0.);
  } else {
    assert (!CKT_BASE::_sim->analysis_is_ac());
    trace3("integrate", dt, q[0].x,  i[0].x);

    switch (method) {
    case mTRAPGEAR:
      incomplete();
      // fall through
    case mGEAR:
      assert(OPT::_keep_time_steps >= 3);
      incomplete();
      // fall through
    case mTRAPEULER:
      incomplete();
      // fall through
    case mEULER:
      return FPOLY1(q[0].x,
		    i[1].f0 + q[0].f0 * dt,
		    q[0].f1 * dt /*?*/);
    case mTRAP:
      assert(OPT::_keep_time_steps >= 2);
      return FPOLY1(q[0].x,
		    i[1].f0 + (q[0].f0 + q[1].f0) * dt * .5,
		    q[0].f1 * dt /*?*/);
      assert(OPT::_keep_time_steps >= 2);
    }
    unreachable();
    return FPOLY1();
  }
}
/*--------------------------------------------------------------------------*/
double tr_l_to_g(double l, double g, double const* time, METHOD method_a, double dt )
{
  if (CKT_BASE::_sim->analysis_is_static()) {
    assert(time[0] == 0.);
    return 0.;
  }else if (CKT_BASE::_sim->analysis_is_restore()) {itested();
    assert(time[0] > 0);
    return g;
    // no change, fake
  }else{
    assert(CKT_BASE::_sim->analysis_is_tran_dynamic());
    METHOD method;
    if (time[1] == 0) {
      method = mEULER; // Bogus current in previous step.  Force Euler.
    }else{
      method = method_a;
    }
    g = dt * l;
    switch (method) {
    case mTRAPGEAR: incomplete();
      // fall through
    case mGEAR:	 g /= 3./2.;	break;
    case mTRAPEULER: incomplete();
      // fall through
    case mEULER: /* g *= 1 */	break;
    case mTRAP:	 g /= 2;	break;
    }
    return g;
  }
}
/*--------------------------------------------------------------------------*/
class DEV_CPOLY_CAP : public STORAGE {
protected:
  double*  _vy0; // vector form of _y0 _values; charge, capacitance
  double*  _vy1; // vector form of _y1 _old_values;
  double*  _vi0; // vector form of _i0; current, difference conductance
  double*  _vi1; // vector form of _i1
  int	   _n_ports;
  double   _load_time;
  const double** _inputs;
protected:
  explicit DEV_CPOLY_CAP(const DEV_CPOLY_CAP& p);
public:
  explicit DEV_CPOLY_CAP();
  ~DEV_CPOLY_CAP();
protected: // override virtual
  char	   id_letter()const override	{unreachable(); return '\0';}
  std::string value_name()const override{incomplete(); return "";}
  std::string dev_type()const override	{unreachable(); return "cpoly_cap";}
  int	   max_nodes()const override	{return net_nodes();}
  int	   min_nodes()const override	{return net_nodes();}
  int	   matrix_nodes()const override	{return _n_ports*2;}
  int	   net_nodes()const override	{return _n_ports*2;}
  CARD*	   clone()const override        {unreachable();return new DEV_CPOLY_CAP(*this);}
  void	   tr_iwant_matrix()override	{tr_iwant_matrix_extended();}
  bool	   tr_needs_eval()const override	{/*assert(!is_q_for_eval());*/ return false;}
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_unload()override;
  double   tr_involts()const override	{unreachable(); return NOT_VALID;}
  double   tr_involts_limited()const override {unreachable(); return NOT_VALID;}
  double   tr_amps()const override;
  void	   ac_iwant_matrix()override	{ac_iwant_matrix_extended();}
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}

  std::string port_name(int)const override {untested();
    incomplete();
    unreachable();
    return "";
  }
public:
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
  //		      const double* inputs[]=0);
protected:
  bool do_tr_con_chk_and_q();
private:
  double tr_probe_num(const std::string& x) const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DEV_DDT : public DEV_CPOLY_CAP {
private:
  explicit DEV_DDT(const DEV_DDT& p)
    :DEV_CPOLY_CAP(p) {}
public:
  explicit DEV_DDT() :DEV_CPOLY_CAP() {}
private: // override virtual
  char	   id_letter()const override	{unreachable(); return '\0';}
  std::string dev_type()const override	{unreachable(); return "ddt";}
  CARD*	   clone()const override	{return new DEV_DDT(*this);}
  bool	   do_tr()override;
  void	   tr_begin()override;
  TIME_PAIR tr_review()override; //		{return _time_by.reset();}//BUG//review(_i0.f0, _it1.f0);}
}p4;
DISPATCHER<CARD>::INSTALL d4(&device_dispatcher, "va_ddt|f_ddt0", &p4);
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_DDT::tr_review()
{
  COMPONENT::tr_review(); // skip ELEMENT
  if (_method_a == mEULER) { untested();
    // Backward Euler, no step control, take it as it comes
  }else{
    double timestep = tr_review_trunc_error(_y);
    _time_by.min_error_estimate(tr_review_check_and_convert(timestep));
  }
  return _time_by;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DEV_IDT : public DEV_CPOLY_CAP {
private:
  explicit DEV_IDT(const DEV_IDT& p)
    :DEV_CPOLY_CAP(p) {}
public:
  explicit DEV_IDT() :DEV_CPOLY_CAP() {}
private: // override virtual
  char	   id_letter()const override	{unreachable(); return '\0';}
  std::string dev_type()const override	{unreachable(); return "idt";}
  CARD*	   clone()const override	{return new DEV_IDT(*this);}
  bool	   do_tr()override;
  void	   tr_begin()override;
  void	   ac_load()override;
  TIME_PAIR tr_review()override;
}p1;
DISPATCHER<CARD>::INSTALL
  d1(&device_dispatcher, "va_idt|f_idt0", &p1);
/*--------------------------------------------------------------------------*/
void DEV_IDT::ac_load()
{ untested();
  ac_load_shunt(); // 4 pt +- loss
  _acg = _vy0[1] * _sim->_jomega;
  trace4("load", _vy0[0], _vy0[1], _loss0, _loss1);
  ac_load_passive();
  for (int i=2; i<=_n_ports; ++i) { untested();
    trace2("load", i, _vy0[i]);
    ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _vy0[i] / _sim->_jomega);
  }
}
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_IDT::tr_review()
{
  COMPONENT::tr_review(); // skip ELEMENT
  if (_method_a == mEULER) { untested();
    // Backward Euler, no step control, take it as it comes
  }else{
    double timestep = tr_review_trunc_error(_i);
    _time_by.min_error_estimate(tr_review_check_and_convert(timestep));
  }
  return _time_by;
}
/*--------------------------------------------------------------------------*/
DEV_CPOLY_CAP::DEV_CPOLY_CAP(const DEV_CPOLY_CAP& p)
  :STORAGE(p),
   _vy0(NULL),
   _vy1(NULL),
   _vi0(NULL),
   _vi1(NULL),
   _n_ports(p._n_ports),
   _load_time(NOT_VALID),
   _inputs(NULL)
{
  // not really a copy .. only valid to copy a default
  // too lazy to do it right, and that's all that is being used
  // to do it correctly requires a deep copy
  // just filling in defaults is better than a shallow copy, hence this:
  assert(!p._vy0);
  assert(!p._vy1);
  assert(!p._vi0);
  assert(!p._vi1);
  assert(p._n_ports == 0);
  assert(!p._inputs);
}
/*--------------------------------------------------------------------------*/
DEV_CPOLY_CAP::DEV_CPOLY_CAP()
  :STORAGE(),
   _vy0(NULL),
   _vy1(NULL),
   _vi0(NULL),
   _vi1(NULL),
   _n_ports(0),
   _load_time(NOT_VALID),
   _inputs(NULL)
{
}
/*--------------------------------------------------------------------------*/
DEV_CPOLY_CAP::~DEV_CPOLY_CAP()
{
  delete [] _vy1;
  delete [] _vi0;
  delete [] _vi1;
  if (net_nodes() > NODES_PER_BRANCH) { untested();
    delete [] _n;
  }else{
    // it is part of a base class
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_CPOLY_CAP::do_tr_con_chk_and_q()
{
  if(_load_time != _sim->_time0){
    q_load();
  }else{
  }

  assert(_vy1);
  set_converged(conchk(_load_time, _sim->_time0));
  _load_time = _sim->_time0;
  for (int i=0; converged() && i<=_n_ports; ++i) {
    set_converged(conchk(_vy1[i], _vy0[i]));
  }
  set_converged();
  trace2("DEV_CPOLY_CAP::do_tr_con_chk_and_q", long_label(), converged());
  trace3("DEV_CPOLY_CAP::do_tr_con_chk_and_q done", long_label(), _y[0].f0, _y[1].f0);
  return converged();
}
/*--------------------------------------------------------------------------*/
bool DEV_CPOLY_CAP::do_tr()
{untested();
  incomplete();
  _m0 = CPOLY1(0., _vi0[0], _vi0[1]);
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
void DEV_DDT::tr_begin()
{
  DEV_CPOLY_CAP::tr_begin();
  _loss1 = _loss0 = 1.;
  trace4("DEV_DDT::tr_begin", long_label(), _y[0].f0, _y[1].f0, LINEAR);
}
/*--------------------------------------------------------------------------*/
void DEV_IDT::tr_begin()
{
  DEV_CPOLY_CAP::tr_begin();
  _loss1 = _loss0 = 1.;
}
/*--------------------------------------------------------------------------*/
bool DEV_DDT::do_tr()
{
  assert((_time[0] == 0) || (_vy0[0] == _vy0[0]));

  _y[0].x  = tr_outvolts();
  _y[0].f0 = _vy0[0]; // state, from owner, "charge".
  assert(_vy0[1] == 0.);
  _y[0].f1 = 0; // _vy0[1]; // another state, capacity.?
  
  trace4("DEV_DDT::do_tr", long_label(), _y[0].f0, _y[1].f0, _y1.f0);
  trace3("DEV_DDT::do_tr", long_label(), _sim->iteration_tag(), _sim->_time0);
  if(_sim->is_advance_iteration()){
    // return true;
  }else{
    _i[0] = differentiate(_y, _i, _time, _method_a);
  }
  trace4("DIFFD", _i[0].f0, _i[0].f1, _y[0].f0, _sim->_time0);
  assert(_i[0].f0 < 1e99);
  _vi0[0] = _i[0].f0;
  _vi0[1] = _i[0].f1;
  assert(_vi0[0] == _vi0[0]);
  
  for (int i=2; i<=_n_ports; ++i) {
    _vi0[i] = tr_c_to_g(_vy0[i], _vi0[i]);
    trace4("DEV_DDT::do_tr", i, _vi0[0], volts_limited(_n[2*i-2],_n[2*i-1]), _vi0[i]);
    _vi0[0] -= volts_limited(_n[2*i-2],_n[2*i-1]) * _vi0[i];
    assert(_vi0[i] == _vi0[i]);
    assert(_vi0[0] == _vi0[0]);
  }
  for (int i=0; i<=_n_ports; ++i) {
    assert(_vi0[i] == _vi0[i]);
  }
  
  _m0 = CPOLY1(0., _vi0[0], _vi0[1]);
  trace3("DEV_DDT::do_tr done", _m0.x, _m0.c0, _m0.c1);
  trace3("DEV_DDT::do_tr done", long_label(), _y[0].f0, _y[1].f0);
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
bool DEV_IDT::do_tr()
{
  assert((_time[0] == 0) || (_vy0[0] == _vy0[0]));

  _y[0].x  = tr_outvolts();
  _y[0].f0 = _vy0[0]; // state, from owner, "input voltage".
  assert(_vy0[1] == 0.);
  _y[0].f1 = 0; // _vy0[1]; // another state, capacity.?
  
  _i[0] = integrate(_y, _i, _time, _method_a, _dt);
  trace3("idt do_tr", _y[0].f0, _i[0].f0, _i[1].f0 );
 
  _vi0[0] = _i[0].f0;
  _vi0[1] = _i[0].f1;
  assert(_vi0[0] == _vi0[0]);
  
  for (int i=2; i<=_n_ports; ++i) {
    _vi0[i] = tr_l_to_g(_vy0[i], _vi0[i], _time, _method_a, _dt);
    _vi0[0] -= volts_limited(_n[2*i-2],_n[2*i-1]) * _vi0[i];
    assert(_vi0[i] == _vi0[i]);
    assert(_vi0[0] == _vi0[0]);
  }
  for (int i=0; i<=_n_ports; ++i) {
    assert(_vi0[i] == _vi0[i]);
  }
  
  _m0 = CPOLY1(0., _vi0[0], _vi0[1]);
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::tr_load()
{
  trace2("tr_load", long_label(), _sim->iteration_tag());
  tr_load_shunt(); // 4 pt +- loss
  for (int i=0; i<=_n_ports; ++i) {
    assert(_vi0[i] == _vi0[i]);
  }
  if( CKT_BASE::_sim->analysis_is_static() ) {
    assert(!_vi0[0]);
    assert(!_vi0[1]);
  }else{
  }
  tr_load_passive();
  _vi1[0] = _vi0[0];
  _vi1[1] = _vi0[1];
  for (int i=2; i<=_n_ports; ++i) {
    if( CKT_BASE::_sim->analysis_is_static() ) {
      assert(!_vi0[i]);
    }else{
    }
    tr_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], &(_vi0[i]), &(_vi1[i]));
  }
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::tr_unload()
{untested();
  std::fill_n(_vi0, _n_ports+1, 0.);
  _m0.c0 = _m0.c1 = 0.;
  _sim->mark_inc_mode_bad();
  tr_load();
}
/*--------------------------------------------------------------------------*/
double DEV_CPOLY_CAP::tr_amps()const
{
  assert(_i[0].f0 == _i[0].f0);
  assert(_i[0].f0 < 1e99);
  return _i[0].f0;
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::ac_load()
{
  ac_load_shunt(); // 4 pt +- loss
  _acg = _vy0[1] * _sim->_jomega;
  trace4("load", _vy0[0], _vy0[1], _loss0, _loss1);
  ac_load_passive();
  for (int i=2; i<=_n_ports; ++i) {
    trace2("load", i, _vy0[i]);
    ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _vy0[i] * _sim->_jomega);
  }
}
/*--------------------------------------------------------------------------*/
/* set: set parameters, used in model building
 */
void DEV_CPOLY_CAP::set_parameters(const std::string& Label, CARD *Owner,
				   COMMON_COMPONENT *Common, double Value,
				   int n_states, double states[],
				   int n_nodes, const node_t nodes[])
  //				   const double* inputs[])
{
  bool first_time = (net_nodes() == 0);

  set_label(Label);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);

  if (first_time) {
    _n_ports = n_nodes/2; // sets num_nodes() = _n_ports*2
    assert(_n_ports == n_states-1);

    assert(!_vy1);
    assert(!_vi0);
    assert(!_vi1);
    _vy1 = new double[n_states]; 
    _vi0 = new double[n_states];
    _vi1 = new double[n_states];

    if (net_nodes() > NODES_PER_BRANCH) { untested();
      // allocate a bigger node list
      _n = new node_t[net_nodes()];
    }else{
      // use the default node list, already set
    }      
  }else{itested();
    assert(_n_ports == n_states-1);
    assert(_vy1);
    assert(_vi0);
    assert(_vi1);
    assert(net_nodes() == n_nodes);
    // assert could fail if changing the number of nodes after a run
  }

  //_inputs = inputs;
  _inputs = 0;
  _vy0 = states;
  std::fill_n(_vy0, n_states, 0.);
  std::fill_n(_vy1, n_states, 0.);
  std::fill_n(_vi0, n_states, 0.);
  std::fill_n(_vi1, n_states, 0.);
  notstd::copy_n(nodes, net_nodes(), _n);
  assert(net_nodes() == _n_ports * 2);
}
/*--------------------------------------------------------------------------*/
double DEV_CPOLY_CAP::tr_probe_num(const std::string& x)const
{
  if (Umatch(x, "loss ")) {
    return _loss0;
  }else if (Umatch(x, "conv ")) { untested();
    return converged();
  }else{
    return STORAGE::tr_probe_num(x);
  }
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
