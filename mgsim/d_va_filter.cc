/*                     -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023, 2024 Felix Salfelder
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
#include <e_hsparam.h>
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
    case mTRAPGEAR:untested();
      incomplete();
      // fall through
    case mGEAR:
      assert(OPT::_keep_time_steps >= 3);
       return FPOLY1(q[0].x,
 		    (3./2.) * i[0].f0 + q[1].f0 * dt - (1./2.) * (q[1].f0 + q[2].f0) * (time[1] - time[2]),
 		    q[0].f1 * (2./3.) * dt);
    case mTRAPEULER:untested();
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
  COMPLEX _acout; // TODO: use _vi*?
  int	   _n_ports{-1};
  double   _load_time{0.};
  std::vector<std::string> _current_port_names;
  std::vector<ELEMENT const*> _input;
private:
  node_t* _nN{nullptr};
protected:
  explicit DEV_CPOLY_CAP(const DEV_CPOLY_CAP& p);
public:
  explicit DEV_CPOLY_CAP();
  ~DEV_CPOLY_CAP();
protected: // override virtual
  char	   id_letter()const override	{ untested();unreachable(); return '\0';}
  std::string value_name()const override{return "";}
  std::string dev_type()const override	{ untested();unreachable(); return "cpoly_cap";}
  int	   max_nodes()const override	{return net_nodes();}
  int	   min_nodes()const override	{return net_nodes();}
  int	   matrix_nodes()const override	{return _n_ports*2;}
  int	   net_nodes()const override	{return _n_ports*2 - int(_current_port_names.size());}
  int	   ext_nodes()const override	{return _n_ports*2;}
  CARD*	   clone()const override        { untested();unreachable();return new DEV_CPOLY_CAP(*this);}
  void	   tr_iwant_matrix()override	{tr_iwant_matrix_extended();}
  void     precalc_last() override;
  bool	   tr_needs_eval()const override;
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_unload()override;
  double   tr_involts()const override	{ untested();return dn_diff(n_(IN1).v0(), n_(IN2).v0());}
  double   tr_involts_limited()const override { untested();return volts_limited(n_(IN1),n_(IN2));}
  double   tr_amps()const override;
  void	   ac_iwant_matrix()override	{ac_iwant_matrix_extended();}
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}

  node_t& n_(int i)const override {
    assert(_nN); assert(i>=0); assert(i<matrix_nodes()); return _nN[i];
  }

  std::string port_name(int)const override {untested();
    incomplete();
    unreachable();
    return "";
  }

  void expand()override;
  void expand_last()override;
  void expand_current_port(int i);

  void set_port_by_index(int i, /*const*/ std::string& s) override {
    if(i>=0){ untested();
      ELEMENT::set_port_by_index(i, s);
    }else{
      obsolete_set_current_port_by_index(-i-1, s);
    }
  }
private:
  int first_current_port()const { return (_n_ports - int(_current_port_names.size()))*2; }
  int last_current_port()const { return 2*_n_ports - int(_current_port_names.size()); }
  bool node_is_connected(int i)const override {
    if(i < first_current_port()){
      return ELEMENT::node_is_connected(i);
    }else if(i < last_current_port()) {
      return true; // no names set // BUG.
      return _current_port_names[i-_n_ports*2] != "";
    }else{ untested();
      return false;
    }
  }
  void obsolete_set_current_port_by_index(int i, const std::string& s) {
    if(i==0){ untested();
      // _self_is_current = true;
    }else if(i<=int(_current_port_names.size())){
      _current_port_names[i-1] = s;
    }else{ untested();
      throw Exception_Too_Many(i, int(_current_port_names.size()), 0);
    }
  }
public:
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
protected:
  bool do_tr_con_chk_and_q();
private:
  double tr_probe_num(const std::string& x) const override;
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
class DEV_DDT : public DEV_CPOLY_CAP {
private:
  explicit DEV_DDT(const DEV_DDT& p)
    :DEV_CPOLY_CAP(p) {}
public:
  explicit DEV_DDT() :DEV_CPOLY_CAP() {}
  // explicit DEV_DDT(isrc) :DEV_CPOLY_CAP() {...} // TODO
private: // override virtual
  char	   id_letter()const override	{ untested();unreachable(); return '\0';}
  std::string dev_type()const override	{ untested();unreachable(); return "ddt";}
  CARD*	   clone()const override	{return new DEV_DDT(*this);}
  bool	   do_tr()override;
  void	   ac_load()override; // TODO: ac_eval
  double   tr_amps()const override      { return DEV_CPOLY_CAP::tr_amps(); }
  void	   tr_begin()override;
  void	   tr_advance()override;
  void tr_load()override{
    trace5("DDT::tr_load", _sim->_time0, _vi0[0], _vi0[1], _vi0[2], long_label());
    DEV_CPOLY_CAP::tr_load();
  }
  TIME_PAIR tr_review()override; //		{ untested();return _time_by.reset();}//BUG//review(_i0.f0, _it1.f0);}
}p4;
DISPATCHER<CARD>::INSTALL d_ddt(&device_dispatcher, "va_ddt", &p4);
// DEV_DDT p4i(I);
// DISPATCHER<CARD>::INSTALL d_ddt_i(&device_dispatcher, "va_ddt_i", &p4i); // TODO
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_DDT::tr_review()
{
  COMPONENT::tr_review(); // skip ELEMENT
  if (_method_a == mEULER) { itested();
    // Backward Euler, no step control, take it as it comes
  }else{
    double timestep = tr_review_trunc_error(_y);
    _time_by.min_error_estimate(tr_review_check_and_convert(timestep));
  }

      // time_future = _time[1] + timestep;
  return _time_by;
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
bool DEV_CPOLY_CAP::tr_needs_eval()const
{
  /*assert(!is_q_for_eval());*/

  if(_loss0 == 0.){
    // more work to do...
    return true;
  }else{
    return true;
    return false;
  }
}
/*--------------------------------------------------------------------------*/
class DEV_IDT : public DEV_CPOLY_CAP {
  bool _reset{false};
private:
  explicit DEV_IDT(const DEV_IDT& p)
    :DEV_CPOLY_CAP(p) {}
public:
  explicit DEV_IDT() :DEV_CPOLY_CAP() {}
private: // override virtual
  char	   id_letter()const override	{ untested();unreachable(); return '\0';}
  std::string dev_type()const override	{ untested();unreachable(); return "idt";}
  CARD*	   clone()const override	{return new DEV_IDT(*this);}
  bool	   do_tr()override;
  double   tr_amps()const override      { return DEV_CPOLY_CAP::tr_amps(); }
/*--------------------------------------------------------------------------*/
  void	   tr_begin()override;
  void	   tr_advance()override;
  void	   ac_load()override;
  TIME_PAIR tr_review()override;
public:
  void set_param_by_index(int i, std::string& v, int j)override {
    trace2("DEV_IDT reset", long_label(), _sim->_time0);
    if(i==123456){
      // BUG. need node_p or something to pass signals.
      _reset = j;
    }else{ untested();
      return DEV_CPOLY_CAP::set_param_by_index(i, v, j);
    }
  }
}p1;
DISPATCHER<CARD>::INSTALL
  d1(&device_dispatcher, "va_idt", &p1);
/*--------------------------------------------------------------------------*/
void DEV_IDT::ac_load()
{
 // do_ac.
  _acout = 1./_sim->_jomega;
  if(_loss0) {
    _acout *= - _loss0;
  }else{
  }
		   //
  DEV_CPOLY_CAP::ac_load();
}
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_IDT::tr_review()
{
  COMPONENT::tr_review(); // skip ELEMENT
  if (_method_a == mEULER) {
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
   _nN(_nodes)
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

  // configure output nodes
  _loss1 = _loss0 = 1.;
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
   _nN(_nodes)
{
}
/*--------------------------------------------------------------------------*/
DEV_CPOLY_CAP::~DEV_CPOLY_CAP()
{
  delete [] _vy1;
  delete [] _vi0;
  delete [] _vi1;
  if (net_nodes() > NODES_PER_BRANCH) {
    delete [] _nN;
  }else{
    // it is part of a base class
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_CPOLY_CAP::do_tr_con_chk_and_q()
{
  if(!_sim->_v0){ untested();
  }else
 // if(_load_time != _sim->_time0)
  {
    trace1("DEV_CPOLY_CAP::q_load", _sim->_time0);
    q_load();
 // }else{ untested();
  }

  assert(_vy1);
  set_converged(conchk(_load_time, _sim->_time0));
  _load_time = _sim->_time0;
  for (int i=0; converged() && i<=_n_ports; ++i) {
    set_converged(conchk(_vy1[i], _vy0[i]));
  }
  set_converged();
//  trace2("DEV_CPOLY_CAP::do_tr_con_chk_and_q", long_label(), converged());
//  trace3("DEV_CPOLY_CAP::do_tr_con_chk_and_q done", long_label(), _y[0].f0, _y[1].f0);
  return converged();
}
/*--------------------------------------------------------------------------*/
bool DEV_CPOLY_CAP::do_tr()
{ untested();
  unreachable();

#if 0
  if(_loss0){ untested();
    _m0 = - _loss0 * CPOLY1(0., _vi0[0], _vi0[1]);
  }else{ untested();
    _m0 = CPOLY1(0., _vi0[0], _vi0[1]);
  }
#endif
 bool ret = do_tr_con_chk_and_q();

 _m0 = CPOLY1(0., _vi0[0], 0.); // _vi0[1]);

  trace2("DEV_CPOLY_CAP::do_tr", long_label(), tr_amps());
  return ret;
}
/*--------------------------------------------------------------------------*/
void DEV_DDT::tr_advance()
{
  trace2("DEV_DDT::tr_advance", _sim->_time0, long_label());
   if(_sim->_last_time == 0.) {
     // breaks idt..
     _y[0].x = tr_outvolts();
     _y[0].f0 = _vy0[0]; // state, from owner, "charge".
   }else{
   }
  STORAGE::tr_advance();
}
/*--------------------------------------------------------------------------*/
void DEV_DDT::tr_begin()
{
  DEV_CPOLY_CAP::tr_begin();
  trace4("DEV_DDT::tr_begin", long_label(), _y[0].f0, _y[1].f0, LINEAR);
}
/*--------------------------------------------------------------------------*/
bool DEV_DDT::do_tr()
{
  double tramps1 = tr_amps();
  assert((_time[0] == 0) || (_vy0[0] == _vy0[0]));

  if(_sim->_v0) {
    _y[0].x = tr_outvolts();
  }else{ untested();
    _y[0].x = 0.;
  }
  _y[0].f0 = _vy0[0]; // state, from owner, "charge".
  // assert(_vy0[1] == 0.); // mfactor abuse.
  _y[0].f1 = 0; // _vy0[1]; // "self" capacity. later.
  
  trace4("DEV_DDT::do_tr0", _sim->_time0, _y[0].f0, _y[1].f0, _y1.f0);
  trace3("DEV_DDT::do_tr", long_label(), _sim->iteration_tag(), _sim->_time0);
  if(_sim->is_advance_iteration()){ itested();
    // return true;
  }else if(_sim->_v0){
    _i[0] = differentiate(_y, _i, _time, _method_a);
  }else{ untested();
  }
  trace4("DIFFD", _i[0].f0, _i[0].f1, _y[0].f0, _sim->_time0);
  assert(_i[0].f0 < 1e99);
  if(_loss0){
    assert(_loss0 == 1); // for now.
    _vi0[0] = - _loss0 * _i[0].f0;
    _vi0[1] = - _loss0 * _i[0].f1;
  }else{
    _vi0[0] = _i[0].f0;
    _vi0[1] = _i[0].f1;
  }
  assert(_vi0[0] == _vi0[0]);
  
  if(_sim->_v0){
    int i = 2;
    for (; i<=_n_ports - int(_input.size()); ++i) {
      if(_loss0){
	_vi0[i] = -_loss0 * tr_c_to_g(_vy0[i], _vi0[i]);
      }else{
	_vi0[i] = tr_c_to_g(_vy0[i], _vi0[i]);
      }

      trace4("DEV_DDT::do_tr", i, _vi0[0], volts_limited(n_(2*i-2),n_(2*i-1)), _vi0[i]);
      _vi0[0] -= volts_limited(n_(2*i-2),n_(2*i-1)) * _vi0[i];
      assert(_vi0[i] == _vi0[i]);
      assert(_vi0[0] == _vi0[0]);
    }

    assert( i == _n_ports - int(_input.size()) + 1);

    // current port input
    for (; int(i)<=_n_ports; ++i) {
      int k = int(i)-int(_n_ports - _input.size() + 1);
      assert(_input[k]->has_iv_probe()); // for now.
      // _m0.c0 += _y[0].f1 * _input->_m0.c0;
      // _m0.c1  = _y[0].f1 * (_input->_loss0 + _input->_m0.c1);
      double scale = _input[k]->_loss0 + _input[k]->_m0.c1 ;
      if(_input[k]->_m0.c1){ untested();
      }else{
      }
      if(_input[k]->_loss0){
      }else{ untested();
      }

     // _vi0[i] = tr_c_to_g(scale*_vy0[i], _vi0[i]);
      if(_loss0){
	_vi0[i] = -_loss0 * scale * tr_c_to_g(_vy0[i], _vi0[i]);
      }else{ untested();
	_vi0[i] = scale * tr_c_to_g(_vy0[i], _vi0[i]);
      }

      _vi0[0] -= volts_limited(n_(2*i-2),n_(2*i-1)) * _vi0[i];
    }

    for (int ii=0; ii<=_n_ports; ++ii) {
      assert(_vi0[ii] == _vi0[ii]);
    }
  }else{ untested();
  }

  if( CKT_BASE::_sim->analysis_is_static() ) {
    if(_vi0[0]){ untested();
//      incomplete(); // TODO: MOD_*::tr_begin?
      _vi0[0] = 0.;
    }
    if(_vi0[1]){ untested();
//      incomplete(); // TODO: MOD_*::tr_begin?
      _vi0[1] = 0.;
    }
  }else{
  }

  trace4("trampsdbg", _sim->_time0, tramps1, tr_amps(), _i[0].f0);
  _m0 = CPOLY1(0., _vi0[0], 0.); // _vi0[1]);

  // q_load();
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void DEV_IDT::tr_begin()
{
  DEV_CPOLY_CAP::tr_begin();
}
/*--------------------------------------------------------------------------*/
void DEV_IDT::tr_advance()
{
  trace3("DEV_IDT::tr_advance", long_label(), _sim->_time0, _reset);
  if(_reset){
    _reset = false;
    _i[0] = _i[1] = FPOLY1(0., 0., 0.);
  }else{
  }

  STORAGE::tr_advance();

#if 0 // later
  if(_sim->_v0){ untested();
    for (int i=2; i<=_n_ports; ++i) { untested();
      _vi0[i] = tr_l_to_g(_vy0[i], _vi0[i], _time, _method_a, _dt);
      if(_loss0){ untested();
	_vi0[i] *= -_loss0;
      }else{ untested();
      }
      _vi0[0] -= volts_limited(n_(2*i-2],n_(2*i-1]) * _vi0[i];
      assert(_vi0[i] == _vi0[i]);
      assert(_vi0[0] == _vi0[0]);
    }
    for (int i=0; i<=_n_ports; ++i) { untested();
      assert(_vi0[i] == _vi0[i]);
    }
  }else{ untested();
  }
#endif

   // if(_sim->_last_time == 0.) { untested();
   //   // breaks idt..
   //   _y[0].x = tr_outvolts();
   //   _y[0].f0 = _vy0[0]; // state, from owner, "charge".
   // }else{ untested();
   // }
}
/*--------------------------------------------------------------------------*/
bool DEV_IDT::do_tr()
{
  assert((_time[0] == 0) || (_vy0[0] == _vy0[0]));

  if(_sim->_v0){
    _y[0].x = tr_outvolts();
  }else{ untested();
    _y[0].x = 0.;
  }
  _y[0].f0 = _vy0[0]; // state, from owner, "input voltage".
  // assert(_vy0[1] == 0.);
  _y[0].f1 = 0; // _vy0[1]; // another state, capacity.?
  
  _i[0] = integrate(_y, _i, _time, _method_a, _dt);
  trace3("idt do_tr", _y[0].f0, _i[0].f0, _i[1].f0 );
 
  _vi0[0] = _i[0].f0;
  _vi0[1] = _i[0].f1;

  assert(_vi0[0] == _vi0[0]);
  
  if(_sim->_v0){
    for (int i=2; i<=_n_ports; ++i) {
      _vi0[i] = tr_l_to_g(_vy0[i], _vi0[i], _time, _method_a, _dt);
      _vi0[0] -= volts_limited(n_(2*i-2),n_(2*i-1)) * _vi0[i];

      assert(_vi0[i] == _vi0[i]);
      assert(_vi0[0] == _vi0[0]);
    }
    for (int i=0; i<=_n_ports; ++i) {
      assert(_vi0[i] == _vi0[i]);
    }
  }else{ untested();
  }

  if(_loss0){
   // _vi0[0] *= - _loss0;
   // _vi0[1] *= - _loss0;
  }else{
  }
  if(_loss0){
    for (int i=0; i<=_n_ports; ++i) {
      _vi0[i] *= -_loss0;
    }
  }else{
  }

  //trace4("idttramp", oldtramps, tr_amps(), n_(0).v0(), n_(1).v0());
  //trace4("idttramp", oldtramps, tr_amps(), n_(0).m_(), n_(1).m_());
  //_m0 = CPOLY1(0., _vi0[0], _vi0[1]);
//  _vi0[0] = 0;
//  _vi0[1] = 0;
				 //
  if( CKT_BASE::_sim->analysis_is_static() ) {
    if(_vi0[0]){ untested();
//      incomplete(); // TODO: MOD_*::tr_begin?
      _vi0[0] = 0.;
    }
    if(_vi0[1]){ untested();
//      incomplete(); // TODO: MOD_*::tr_begin?
      _vi0[1] = 0.;
    }
  }else{
  }

  _m0 = CPOLY1(0., _vi0[0], 0.); // _vi0[1]);

  return do_tr_con_chk_and_q();
} // IDT::d0_tr
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::tr_load()
{
 // assert(_loaditer != _sim->iteration_tag()); // double load

  tr_load_shunt(); // 4 pt +- loss
  for (int i=0; i<=_n_ports; ++i) {
    assert(_vi0[i] == _vi0[i]);
  }
  tr_load_passive();
  _vi1[0] = _vi0[0];
  _vi1[1] = _vi0[1];
  for (int i=2; i<=_n_ports; ++i) {
    if( CKT_BASE::_sim->analysis_is_static() ) {
      if(_vi0[i]){ untested();
//	incomplete(); // TODO: MOD_*::tr_begin?
//	_vi0[i] = 0.;
//	_vi1[i] = 0.;
      }
    }else{
    }
    trace4("DEV_CPOLY_CAP::tr_load", _sim->_time0, _vi0[i], long_label(), i);
    tr_load_extended(n_(OUT1), n_(OUT2), n_(2*i-2), n_(2*i-1), &(_vi0[i]), &(_vi1[i]));
  }

 // assert(_loaditer == _sim->iteration_tag()); // double load
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
double DEV_CPOLY_CAP::tr_amps() const
{
  // return fixzero((_loss0 * tr_outvolts() + _m0.c1 * tr_involts() + _m0.c0), _m0.c0);
  double amps = _m0.c0;
  for (int i=2; i<=_n_ports; ++i) {
    amps += dn_diff(n_(2*i-2).v0(),n_(2*i-1).v0()) * _vi0[i];
  }

  if(_loss0){
    // a voltage source. tr_amps abuse, return the filter output...
    assert(_loss0 == 1.); // for now.
    amps = -amps;
  }else{
  }

  trace3("DEV_CPOLY_CAP::tr_amps", long_label(), _sim->_time0, amps);
  return amps;
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::ac_load()
{
  ac_load_shunt(); // 4 pt +- loss
		   //
  if(_acg==0.){
  }else{ untested();
  }
  ac_load_passive();
  for (int i=2; i<=_n_ports; ++i) {
    trace2("CAP::ac_load", i, _vy0[i]);
    ac_load_extended(n_(OUT1), n_(OUT2), n_(2*i-2), n_(2*i-1), _vy0[i] * _acout);
  }
}
/*--------------------------------------------------------------------------*/
void DEV_DDT::ac_load()
{
 // return DEV_CPOLY_CAP::ac_load();

  _acout = _sim->_jomega;
  if(_loss0) {
    _acout *= - _loss0;
  }else{
  }

  trace6("CAP::ac_load", long_label(), _vy0[0], _vy0[1], _loss0, _loss1, _acg);

  DEV_CPOLY_CAP::ac_load();
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
    _current_port_names.resize(n_states - 1 - n_nodes/2);
    _input.resize(n_states - 1 - n_nodes/2);
    _n_ports = n_nodes/2; // sets num_nodes() = _n_ports*2
    trace3("DEV_CPOLY_CAP::set_parameters", _n_ports, n_nodes, n_states);

    _n_ports = n_states-1; // set net_nodes
    // assert(_n_ports == n_states-1);
    assert(size_t(_n_ports) == n_nodes/2 + _current_port_names.size());

    assert(!_vy1);
    assert(!_vi0);
    assert(!_vi1);
    _vy1 = new double[n_states]; 
    _vi0 = new double[n_states];
    _vi1 = new double[n_states];

    if (net_nodes() > NODES_PER_BRANCH) {
      // allocate a bigger node list
      _nN = new node_t[matrix_nodes()];
    }else{
      // use the default node list, already set
    }      
  }else{
    assert(_n_ports == n_states-1);
    assert(_vy1);
    assert(_vi0);
    assert(_vi1);
    assert(net_nodes() == n_nodes + int(_current_port_names.size()));
    // assert could fail if changing the number of nodes after a run
  }

  _vy0 = states;
  std::fill_n(_vy0, n_states, 0.);
  std::fill_n(_vy1, n_states, 0.);
  std::fill_n(_vi0, n_states, 0.);
  std::fill_n(_vi1, n_states, 0.);

  trace4("set_parameters", n_nodes, net_nodes(), _n_ports, _vy0[1]);
  notstd::copy_n(nodes, n_nodes, _nN);
  assert(ext_nodes() == _n_ports * 2);
  if(n_(0).is_short_to(n_(1))){
  }else{
    _vy0[1] = 1.; // mfactor hack.
    _vy1[1] = 1.; // mfactor hack.
  }
}
/*--------------------------------------------------------------------------*/
double DEV_CPOLY_CAP::tr_probe_num(const std::string& x)const
{
  if (Umatch(x, "loss ")) { untested();
    return _loss0;
  }else if (Umatch(x, "conv ")) { untested();
    return converged();
  }else if (Umatch(x, "st0 ")) {
    return _vy0[0];
  }else if (Umatch(x, "v0 ")) { untested();
    return n_(0).v0();
  }else{
    return STORAGE::tr_probe_num(x);
  }
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::precalc_last()
{
  trace2("DCC::pl", long_label(), hsparam());
  HS_PARAM* c;
  if(common()){
    COMMON_COMPONENT* cc = mutable_common()->mutable_clone();
    c = prechecked_cast<HS_PARAM*>(cc);
  }else{
    // incomplete();
    c = new HS_PARAM();
  }

  if(_vy0[1]){
  }else{
  }
  c->set_mfactor(_vy0[1]); // HACK
  attach_common(c);

  STORAGE::precalc_last();
  trace1("DCC::pl", _method_a);
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::expand()
{
  STORAGE::expand();
  if(_current_port_names.size()){
    q_expand_last();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DEV_CPOLY_CAP::expand_last()
{
  ELEMENT::expand_last();
  for(int i=0; i<int(_current_port_names.size()); ++i){
    expand_current_port(i);
    // expand_current_port(&_input[i], _current_port_names[i], this); // or so.
  }
}
/*--------------------------------------------------------------------------*/
// !! duplicate in d_va.h
void DEV_CPOLY_CAP::expand_current_port(int i)
{
  std::string const& input_label = _current_port_names[i];
  ELEMENT const*& input = _input[i];

  int in1 = ext_nodes() - 2*int(_current_port_names.size()) + 2*i;
  // int in1 = first_current_port() + 2*i;
  int in2 = in1 + 1;

  assert (input_label != "");
  CARD const* e = find_in_my_scope(input_label);
  input = dynamic_cast<const ELEMENT*>(e);

  if (!e) {untested();
    throw Exception(long_label() + ": " + input_label + " does not exist");
  }else if (!input) {untested();
    throw Exception(long_label() + ": " + input_label + " cannot be used as current probe");
  }else if (input->subckt()) {untested();
    throw Exception(long_label() + ": " + input_label
		    + " has a subckt, cannot be used as current probe");
  }else if (input->has_inode()) {untested();
    n_(in1) = input->n_(input()->ext_nodes());
    n_(in2).set_to_ground(nullptr);
  }else if (input->has_iv_probe()) {
    n_(in1) = input->n_(OUT1);
    n_(in2) = input->n_(OUT2);
  }else{ untested();
    throw Exception(long_label() + ": " + input_label + " cannot be used as current probe");
  }
}
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
