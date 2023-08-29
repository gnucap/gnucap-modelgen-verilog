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
protected:
  explicit VAPOT(const VAPOT& p) : DEV_CPOLY_G(p) {}
public:
  explicit VAPOT() : DEV_CPOLY_G() {}
  ~VAPOT() {}
protected: // override virtual
  CARD*	   clone()const override	{return new VAPOT(*this);}
  void	   tr_iwant_matrix()override	{tr_iwant_matrix_extended();}
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_begin()override{
    _loss0 = 1./OPT::shortckt;
  }
  double   tr_involts()const override	{unreachable(); return NOT_VALID;}
  double   tr_involts_limited()const override {unreachable(); return NOT_VALID;}
  double   tr_amps()const override;
  void	   ac_iwant_matrix()override	{ac_iwant_matrix_extended();}
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}

  bool has_iv_probe()const override{return true;}
public:
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
  //		      const double* inputs[]=0);
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
    set_converged(conchk(_old_values[0], _values[0], abstol()));
  }else{
  }
  if(converged()){
    set_converged(_loss1 == _loss0);
    trace3("pot?", _loss1, _loss0, converged());
  }else{
  }
  for (int i=1; converged() && i<=_n_ports; ++i) {
    if(_loss0){
      set_converged(conchk(_old_values[i], _adj_values[i]) /*,0.?*/);
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
  }else if(_self_is_current && fabs(_values[1]) > OPT::shortckt){
    // loss but CS mode.
    _values[0] /= - _values[1];
    _adj_values[1] = 1./ _values[1];

    for (int i=2; i<=_n_ports; ++i) {
      _adj_values[i] = _values[i]; //  *_loss0;
    }
    _m0.x = 0.;
    _m0.c0 = 0.; // - _values[0]; // *_loss0;
    _m0.c1 = _adj_values[1]; // really?
    _loss0 = 0.;
  }else{
    for (int i=1; i<=_n_ports; ++i) {
      _adj_values[i] = -_values[i] * _loss0;
    }
    _m0.x = 0.;
    _m0.c0 = -_loss0 * _values[0];
    _m0.c1 = 0.; // really?
  }
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
void VAPOT::tr_load()
{
  _old_values[0] = _values[0];
  if(!_loss0){
    if(_loss1){
      tr_unload_shunt(); // 4 pt +- loss
    }else{
    }

    tr_load_passive();
    _old_values[1] = _values[1];
    for (int i=2; i<=_n_ports; ++i) {
      trace4("tr_load", long_label(), i, _values[i], _old_values[i]);
      tr_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], &(_values[i]), &(_old_values[i]));
    }

  }else if(_self_is_current && fabs(_values[1]) > OPT::shortckt){ untested();
    // loss but CS mode.
    //
    if(_loss1){
      tr_unload_shunt(); // 4 pt +- loss
    }else{
    }
			
    tr_load_passive();
    _old_values[1] = _values[1];
    for (int i=2; i<=_n_ports; ++i) {
      trace4("tr_load", long_label(), i, _values[i], _old_values[i]);
      tr_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], &(_values[i]), &(_old_values[i]));
    }

  }else{
    tr_load_shunt(); // 4 pt +- loss
    trace3("CPG.. ", long_label(), _loss0, _loss1);
    tr_load_source();

    trace2("VAPOT::tr_load", _values[0], _values[1]);
    _old_values[1] = _adj_values[1];
    for (int i=2; i<=_n_ports; ++i) {
      trace2("VAPOT::tr_load control", i, _values[i]);
      tr_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], &(_adj_values[i]), &(_old_values[i]));
    }
  }
}
/*--------------------------------------------------------------------------*/
double VAPOT::tr_amps()const
{
  double amps = _m0.c0 + _loss0 * tr_outvolts();
  for (int i=1; i<=_n_ports; ++i) {
    amps += dn_diff(_n[2*i-2].v0(), _n[2*i-1].v0()) * _values[i];
  }
  return amps;
}
/*--------------------------------------------------------------------------*/
void VAPOT::ac_load()
{
  if(_loss0){
    ac_load_shunt(); // 4 pt +- loss
  }else{
  }
  if(_current_port_names.size()){ untested();
    incomplete();
  }else{
  }
  _acg = _values[1];
  ac_load_passive();

  for (int i=2; i<=_n_ports; ++i) {
    if(_loss0){
      ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], - _values[i] * _loss0);
    }else{
      ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _values[i]);
    }
  }
}
/*--------------------------------------------------------------------------*/
/* set: set parameters, used in model building
 */
void VAPOT::set_parameters(const std::string& Label, CARD *Owner,
				 COMMON_COMPONENT *Common, double Value,
				 int n_states, double states[],
				 int n_nodes, const node_t nodes[])
  //				 const double* inputs[])
{
  bool first_time = (net_nodes() == 0);

  set_label(Label);
  trace3("VAPOT::set_parameters", long_label(), n_nodes, n_states);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);
  _current_port_names.resize(n_states - 1 - n_nodes/2);
  _input.resize(n_states - 1 - n_nodes/2);

  if (first_time) {
    _n_ports = n_states-1; // set net_nodes
    assert(size_t(_n_ports) == n_nodes/2 + _current_port_names.size());

    assert(!_old_values);
    _adj_values = new double[n_states];
    _old_values = new double[n_states];

    if (matrix_nodes() > NODES_PER_BRANCH) {
      // allocate a bigger node list
      _n = new node_t[matrix_nodes()];
    }else{
      // use the default node list, already set
    }      
  }else{
    assert(_n_ports == n_states-1);
    assert(_old_values);
    assert(net_nodes() == n_nodes);
    // assert could fail if changing the number of nodes after a run
  }

  _values = states;
  std::fill_n(_values, n_states, 0.);
  std::fill_n(_old_values, n_states, 0.);
  assert(n_nodes <= net_nodes());
  notstd::copy_n(nodes, n_nodes, _n); // copy more in expand_last
  assert(net_nodes() == _n_ports * 2);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_pot|pot_src", &d);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
