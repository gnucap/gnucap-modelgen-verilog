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
 * VAFLOW : DEV_CPOLY_G, flow contribution
 */
#include "d_va.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class VAFLOW : public DEV_CPOLY_G {
protected:
  explicit VAFLOW(const VAFLOW& p) : DEV_CPOLY_G(p) {}
public:
  explicit VAFLOW() : DEV_CPOLY_G() {}
  ~VAFLOW() {}
protected: // override virtual
  CARD*	   clone()const override	{return new VAFLOW(*this);}
  void	   tr_iwant_matrix()override	{tr_iwant_matrix_extended();}
  bool	   do_tr()override;
  void	   tr_load()override;
  double   tr_involts()const override	{ untested();unreachable(); return NOT_VALID;}
  double   tr_involts_limited()const override { untested();unreachable(); return NOT_VALID;}
  double   tr_amps()const override;
  void	   ac_iwant_matrix()override	{ac_iwant_matrix_extended();}
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}

  bool has_iv_probe()const override { untested();return true;}

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
    return cv->flow_abstol();
  }
  bool do_tr_con_chk_and_q();
}f; // VAFLOW;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "flow_src|va_flow", &f);
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
bool VAFLOW::do_tr_con_chk_and_q()
{
  trace1("VAFLOW::do_tr_con_chk", long_label());
  if(!_sim->_v0){ untested();
  }else if(_loaditer != _sim->iteration_tag()){

    q_load();
  }else{ untested();
  }

  assert(_old_values);
  set_converged(conchk(_time, _sim->_time0));
  _time = _sim->_time0;
  if(converged()){
    set_converged(conchk(_old_values[0], _values[0], abstol()));
  }else{
  }
  for (int i=1; converged() && i<=_n_ports; ++i) {
    set_converged(conchk(_old_values[i], _values[i] /*, 0.?*/ ));
  }
  return converged();
}
/*--------------------------------------------------------------------------*/
bool VAFLOW::do_tr()
{
  assert(_values);
  assert(!_loss0);
  trace4("VAFLOW::do_tr", _values, long_label(), _values[0], _values[1]);

  _m0 = CPOLY1(0., _values[0], _values[1]);
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
void VAFLOW::tr_load()
{
  trace5("VAFLOW::tr_load", long_label(), _values[0], _values[1], _n_ports, matrix_nodes());
  tr_load_passive();
  _old_values[0] = _values[0];
  _old_values[1] = _values[1];
  for (int i=2; i<=_n_ports; ++i) {
    // trace4("VAFLOW::tr_load", long_label(), i, _values[i], _old_values[i]);
    tr_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], &(_values[i]), &(_old_values[i]));
  }
}
/*--------------------------------------------------------------------------*/
double VAFLOW::tr_amps()const
{
  double amps = _m0.c0;
  for (int i=1; i<=_n_ports; ++i) {
    amps += dn_diff(_n[2*i-2].v0(), _n[2*i-1].v0()) * _values[i];
  }
  return amps;
}
/*--------------------------------------------------------------------------*/
void VAFLOW::ac_load()
{
  _acg = _values[1];
  ac_load_passive();
  for (int i=2; i<=_n_ports; ++i) {
    trace4("acload", long_label(), i, _values[i], _old_values[i]);
    ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _values[i]);
  }
}
/*--------------------------------------------------------------------------*/
/* set: set parameters, used in model building
 */
void VAFLOW::set_parameters(const std::string& Label, CARD *Owner,
				 COMMON_COMPONENT *Common, double Value,
				 int n_states, double states[],
				 int n_nodes, const node_t nodes[])
  //				 const double* inputs[])
{
  bool first_time = (net_nodes() == 0);

  set_label(Label);
  trace4("VAFLOW::set_parameters", long_label(), n_nodes, n_states, first_time);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);

  if (first_time) {
    _current_port_names.resize(n_states - 1 - n_nodes/2);
    _input.resize(n_states - 1 - n_nodes/2);
    _n_ports = n_states-1; // set net_nodes
    assert(size_t(_n_ports) == n_nodes/2 + _current_port_names.size());

    assert(!_old_values);
    _old_values = new double[n_states];

    if (net_nodes() > NODES_PER_BRANCH) {
      // allocate a bigger node list
      _n = new node_t[net_nodes()];
    }else{
      // use the default node list, already set
    }      
  }else{
    assert(_n_ports == n_states-1);
    assert(_old_values);
    assert(int(_input.size()) == n_states - 1 - n_nodes/2);
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
class DEV_FPOLY_G : public VAFLOW {
private:
  explicit DEV_FPOLY_G(const DEV_FPOLY_G& p)
    :VAFLOW(p) {}
public:
  explicit DEV_FPOLY_G() :VAFLOW() {}
private: // override virtual
  char	   id_letter()const override { untested();unreachable(); return '\0';}
  std::string dev_type()const override { untested();unreachable(); return "va_fpoly_g";}
  CARD*	   clone()const override { return new DEV_FPOLY_G(*this);}
  bool	   do_tr() override;
}ff;
DISPATCHER<CARD>::INSTALL dff(&device_dispatcher, "va_fpoly_g", &ff);
/*--------------------------------------------------------------------------*/
bool DEV_FPOLY_G::do_tr()
{
  assert(_values);
  double c0 = _values[0];
  trace4("DEV_FPOLY_G::do_tr", long_label(), _values[0], _values[1], _values[_n_ports]);
  // incomplete current input?
  // if (_inputs) {untested();
  //   untested();
  //   for (int i=1; i<=_n_ports; ++i) {untested();
  //     c0 -= *(_inputs[i]) * _values[i];
  //     trace4("", i, *(_inputs[i]), _values[i], *(_inputs[i]) * _values[i]);
  //   }
  // }else
  {
    for (int i=1; i<=_n_ports; ++i) {
      c0 -= volts_limited(_n[2*i-2],_n[2*i-1]) * _values[i];
      trace4("", i, volts_limited(_n[2*i-2],_n[2*i-1]), _values[i],
	     volts_limited(_n[2*i-2],_n[2*i-1]) * _values[i]);
    }
  }
  trace2("", _values[0], c0);
  _m0 = CPOLY1(0., c0, _values[1]);

  return do_tr_con_chk_and_q();
}
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
