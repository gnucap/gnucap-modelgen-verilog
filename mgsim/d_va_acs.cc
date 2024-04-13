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
 * VAACS : ac_stim device
 */
#include "d_va.h"
#include "u_xprobe.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class VAACS : public DEV_CPOLY_G {
protected:
  explicit VAACS(const VAACS& p) : DEV_CPOLY_G(p) {
    _loss1 = _loss0 = 1.;
  }
public:
  explicit VAACS() : DEV_CPOLY_G() {}
  ~VAACS() {}
protected: // override virtual
  CARD*	   clone()const override	{return new VAACS(*this);}
  std::string dev_type()const override{ return "va_acs"; }
//  int      net_nodes()const override {untested(); return _net_nodes;}
//  int      max_nodes()const override {untested(); return 4;}
//   void set_port_by_index(int i, std::string& s) override { untested();
//     incomplete();
//   }
  void	   tr_iwant_matrix()override	{tr_iwant_matrix_extended();}
  double   tr_involts()const override	{untested(); return tr_outvolts();}
  double   tr_involts_limited()const override {return tr_outvolts_limited();}
  double   tr_amps()const override{return 0.;}
  void	   ac_iwant_matrix()override	{ac_iwant_matrix_extended();}
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}
  bool	   do_tr()override { return true; }
  bool	   tr_needs_eval()const override { return false; }

  bool has_iv_probe()const override{return true;}
public:
  int set_param_by_name(std::string n, std::string v)override { untested();
    if(n=="mag"){ untested();
    incomplete();
      return 0;
    }else{ untested();
      return DEV_CPOLY_G::set_param_by_name(n, v);
    }
  }
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
  //		      const double* inputs[]=0);
protected:
  double abstol() const{ untested();
    return 0.;
    auto cv = prechecked_cast<COMMON_VASRC const*>(common());
    assert(cv); // TODO: give feedback
    return cv->potential_abstol();
  }
  bool do_tr_con_chk_and_q();
  XPROBE   ac_probe_ext(const std::string& x)const override{
    CS cmd(CS::_STRING, x);
    if (Umatch(x, "loss ")) {
      return XPROBE(_loss0);
    }else{ untested();
      return DEV_CPOLY_G::ac_probe_ext(x);
    }
  }
}d;
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
//// void VAACS::ac_load()
//// { untested();
////   if(_loss0){ untested();
////     ac_load_shunt(); // 4 pt +- loss
////   }else{ untested();
////   }
////   if(_current_port_names.size()){ untested();
////     incomplete();
////   }else{ untested();
////   }
////   _acg = 1; // _values[1];
////   // ac_load_passive();
////   ac_load_source();
//// }
void VAACS::ac_load()
{
  ac_load_shunt(); // 4 pt +- loss
  _acg = 1; // _vy0[1] * _sim->_jomega;
  ac_load_source();

  if(_loss0){
  //  ac_load_passive();
  }else{
  }
  // for (int i=2; i<=_n_ports; ++i) { untested();
  //   trace2("load", i, _vy0[i]);
  //   ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _vy0[i] * _sim->_jomega);
  // }
}
/*--------------------------------------------------------------------------*/
/* set: set parameters, used in model building
 */
void VAACS::set_parameters(const std::string& Label, CARD *Owner,
				 COMMON_COMPONENT *Common, double Value,
				 int n_states, double states[],
				 int n_nodes, const node_t nodes[])
  //				 const double* inputs[])
{
  bool first_time = (net_nodes() == 0);

  set_label(Label);
  trace3("VAACS::set_parameters", long_label(), n_nodes, n_states);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);

  if (first_time) {
    _current_port_names.resize(n_states - 1 - n_nodes/2);
    _input.resize(n_states - 1 - n_nodes/2);
    _n_ports = n_states-1; // set net_nodes
    assert(size_t(_n_ports) == n_nodes/2 + _current_port_names.size());

    assert(!_old_values);
    // _adj_values = new double[n_states];
    _old_values = new double[n_states];

    if (matrix_nodes() > NODES_PER_BRANCH) { untested();
      // allocate a bigger node list
      _n = new node_t[matrix_nodes()];
    }else{
      // use the default node list, already set
    }      
  }else{
    assert(_n_ports == n_states-1);
    assert(_old_values);
    assert(net_nodes() == n_nodes);
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
/*--------------------------------------------------------------------------*/
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_acs", &d);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
