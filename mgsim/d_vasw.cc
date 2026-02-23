/*                                 -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
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
 * VAPOT : DEV_CPOLY_G, potential contribution (and switch branch).
 */
#include "d_va.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class VAPOT : public DEV_CPOLY_G {
  double*  _m0_{nullptr};
  double*  _m1_{nullptr};
protected:
  explicit VAPOT(const VAPOT& p) : DEV_CPOLY_G(p) {}
public:
  explicit VAPOT() : DEV_CPOLY_G() {}
  ~VAPOT() {
    delete [] _m0_;
    delete [] _m1_;
  }
protected: // override virtual
  CARD*	   clone()const override	{return new VAPOT(*this);}
 // void	   tr_iwant_matrix()override// CPOLY
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_begin()override{
    DEV_CPOLY_G::tr_begin();
    _loss0 = 1./OPT::shortckt;
  }
  double   tr_involts()const override	{return tr_outvolts();}
  double   tr_involts_limited()const override {return tr_outvolts_limited();}
  double   tr_amps()const override;
 // void	   ac_iwant_matrix()override // CPOLY
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}

  bool has_iv_probe()const override{return true;}
public:
  void set_parameters(const std::string& Label, CARD* Parent,
                     COMMON_COMPONENT* Common, double Value,
                     int n_states, double state[],
                     int node_count, const node_t nodes[])override {
    DEV_CPOLY_G::set_parameters(Label, Parent, Common, Value,
	n_states, state, node_count, nodes);
    if(_sim->is_first_expand()){
      assert(n_states > 1);
      _m0_ = new double[n_states-2];
#ifndef NDEBUG
      std::fill_n(_m0_, n_states-2, 0.);
#endif
      _m1_ = new double[n_states-2];
      std::fill_n(_m1_, _n_ports-1, 0.);
    }
  }
/*--------------------------------------------------------------------------*/
protected:
  double abstol() const{
    auto cv = prechecked_cast<COMMON_VASRC const*>(common());
    assert(cv); // TODO: give feedback
    return cv->potential_abstol();
  }
  bool do_tr_con_chk_and_q();
}d;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
bool VAPOT::do_tr_con_chk_and_q()
{
  q_load();

  assert(_old_values);
  set_converged(conchk(_time, _sim->_time0));
  _time = _sim->_time0;
  if(converged()){
    set_converged(conchk(_m0.c0, _m1.c0, abstol()));
  }else{
  }
  if(converged()){
    set_converged(conchk(_m0.c1, _m1.c1));
  }else{
  }
  if(converged()){
    set_converged(_loss1 == _loss0);
    trace3("pot?", _loss1, _loss0, converged());
  }else{
  }
  for (int i=2; converged() && i<=_n_ports; ++i) {
    if(_loss0){
      set_converged(conchk(_old_values[i], _values[i]) /*,0.?*/);
    }else{
      set_converged(conchk(_old_values[i], _values[i]) /*,0.?*/);
    }
    trace3("ctrl ", _loss1, _loss0, converged());
  }
  return converged();
}
/*--------------------------------------------------------------------------*/
bool VAPOT::do_tr()
{
  assert(_values);

  if(!_loss0){
    _m0 = CPOLY1(0., _values[0], _values[1]);
    for (int i=2; i<=_n_ports; ++i) {
      _m0_[i-2] = _values[i];
    }
  }else if(p0_is_cc() && (fabs(_values[1]) > OPT::shortckt)){
    // loss but switch to CS mode.
    // V(br) <+ f(I(br) ...) = v0 + I * v1
    _loss0 = 0.;
//    double amps = tr_amps();

    _m0.x = tr_involts_limited(); // like d_admit...?
    _m0.c1 = 1./_values[1];
    _m0.c0 = - _values[0] * _m0.c1;

     // _values[1] = _m0.c1;
    for (int i=2; i<=_n_ports; ++i) {
      _m0_[i-2] = _values[i];
    }

  }else{
    for (int i=2; i<=_n_ports; ++i) {
      _m0_[i-2] = - _values[i] * _loss0;
    }
    _m0.x = 0.;
    // _m0.c0 = -_loss0 * _y[0].f1; // d_vs.
    _m0.c0 = -_loss0 * _values[0];
    _m0.c1 = 0.; // really?
    assert(_m0.c1 == 0.); // d_vs
  }
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
void VAPOT::tr_load()
{
  for (int i=0; i<=_n_ports; ++i) {
    _old_values[i] = _values[i];
  }
  if(!_loss0){
    if(_loss1){
      tr_unload_shunt(); // 4 pt +- loss
    }else{
    }

    tr_load_passive();
  }else if(p0_is_cc() && fabs(_values[1]) > OPT::shortckt){ untested();
    // loss but CS mode.
    //
    if(_loss1){ untested();
      tr_unload_shunt(); // 4 pt +- loss
    }else{ untested();
    }
    tr_load_passive();
  }else{
    tr_load_shunt(); // 4 pt +- loss
    trace3("CPG.. ", long_label(), _loss0, _loss1);
    tr_load_source();
    _m1.c1 = _m0.c1;
  }

  for (int i=2; i<=_n_ports; ++i) {
    tr_load_extended(n_(OUT1), n_(OUT2), n_(2*i-2), n_(2*i-1), &(_m0_[i-2]), &(_m1_[i-2]));
  }
}
/*--------------------------------------------------------------------------*/
double VAPOT::tr_amps()const
{
  double amps = 0.;
  if(_loss0){
    // voltage src mode
    // d_vs: fixzero((_loss0 * tr_outvolts() + _m0.c1 * tr_involts()(==0) + _m0.c0), _m0.c0);
    // amps = fixzero((_loss0 * tr_outvolts() + _m0.c1 * tr_involts() + _m0.c0), _m0.c0);
    amps = _loss0 * tr_outvolts() + _m0.c1 * tr_involts() + _m0.c0;
    trace3("VAPOT::tr_amps", tr_outvolts(), _loss0, amps);
  }else{
    trace3("VAPOT::tr_amps no loss", tr_outvolts(), _loss0, amps);
    // amps = _m0.c0 + _m0.c1 * tr_outvolts();
    amps = fixzero((_m0.c1 * tr_involts() + _m0.c0), _m0.c0);
  }
  trace4("VAPOT::tr_amps self", tr_outvolts(), _loss0, _input.size(), _p0_is_cc);

  int i=2;
  for (; i<=int(_n_ports - _input.size() + _p0_is_cc); ++i) {
    trace3("VAPOT::tr_amps", tr_outvolts(), _loss0, _m0_[i-2]);
    amps += dn_diff(n_(2*i-2).v0(), n_(2*i-1).v0()) * _m0_[i-2];
  }
  trace4("VAPOT::tr_amps h", tr_outvolts(), _loss0, amps, _n_ports);
  for (; i<=_n_ports; ++i) { untested();
    assert(0); // later.
    int k = i-int(_n_ports - _input.size() + 1);
    double scale = _input[k]->_loss0 + _input[k]->_m0.c1;
    amps += scale * dn_diff(n_(2*i-2).v0(), n_(2*i-1).v0()) * _m0_[i-2];
  }
  trace3("VAPOT::tr_amps done", tr_outvolts(), _loss0, amps);
  return amps;
}
/*--------------------------------------------------------------------------*/
void VAPOT::ac_load()
{
  if(_loss0){
    ac_load_shunt(); // 4 pt +- loss
  }else{
  }
  if(_n_current_inputs){
    incomplete();
  }else{
  }
  _acg = _m0.c1;
  ac_load_passive();

  for (int i=2; i<=_n_ports; ++i) {
    ac_load_extended(n_(OUT1), n_(OUT2), n_(2*i-2), n_(2*i-1), _m0_[i-2]);
    if(_loss0){
    }else{
    }
  }
}
/*--------------------------------------------------------------------------*/
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_sw", &d);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
