/*                                 -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2023 Felix Salfelder
 * Author: Albert Davis <aldavis@gnu.org>
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
 * VA_BREQN : DEV_CPOLY_G. generic branch equation
 */
#include "d_va.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class VA_BREQN : public DEV_CPOLY_G {
  double _one0, _one1;
protected:
  explicit VA_BREQN(const VA_BREQN& p) : DEV_CPOLY_G(p) {}
public:
  explicit VA_BREQN() : DEV_CPOLY_G() {}
  ~VA_BREQN() {}
protected: // override virtual
  int	   matrix_nodes()const override	{return _n_ports*2 + 1;}
  int      int_nodes()const override	{return 1;}
  CARD*	   clone()const override	{return new VA_BREQN(*this);}
  void	   tr_iwant_matrix()override;
  void	   tr_iwant_matrix_extended_branch();
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_unload_ones();
  void	   tr_load_ones();
  void	   ac_load_ones();
  void	   tr_begin()override{
    _loss0 = 1./OPT::shortckt;
    _one0 = 1.;
    _one1 = 1.;
  }

  bool is_vs() const{
    return bool(_loss0); // HACK.
  }

  double   tr_involts()const override	{unreachable(); return NOT_VALID;}
  double   tr_involts_limited()const override {unreachable(); return NOT_VALID;}
  double   tr_amps()const override;
  void	   ac_iwant_matrix()override;
  void	   ac_iwant_matrix_extended_branch();
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{untested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{untested(); return NOT_VALID;}

  bool has_iv_probe()const override{incomplete(); return false;}
  void expand()override;

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
  int BR() const{
    assert(ext_nodes());
    return ext_nodes();
  }
}d_breqn;
/*--------------------------------------------------------------------------*/
void VA_BREQN::expand()
{
  assert(BR());
//  if (!(_n[BR()].n_())) 
  if (_sim->is_first_expand()) {
    _n[BR()].new_model_node( long_label() + ".br", this);
    trace2("newmodelnode", long_label(), _n[BR()].t_());
  }else{ untested();
    trace3("no newmodelnode", BR(), long_label(), _n[BR()].t_());
  }
  DEV_CPOLY_G::expand();
}
/*--------------------------------------------------------------------------*/
void VA_BREQN::tr_iwant_matrix_extended_branch()
{

  for (int ii = 0;  ii < matrix_nodes();  ++ii) {
      trace2("ELEMENT::tr_iwant_matrix_extended", ii, _n[ii].m_() );
  }
  for (int ii = 0;  ii < matrix_nodes();  ++ii) {
    // connect all to branch..
    _sim->_aa.iwant(_n[BR()].m_(),_n[ii].m_());
    _sim->_lu.iwant(_n[BR()].m_(),_n[ii].m_());

    // is this too much?
    if (_n[ii].m_()  != INVALID_NODE) {
      for (int jj = 2;  jj < ii ;  ++jj) {
	_sim->_aa.iwant(_n[ii].m_(),_n[jj].m_());
	_sim->_lu.iwant(_n[ii].m_(),_n[jj].m_());
      }
    }else{ untested();
      trace3("eek", ii, _n[ii].m_(), long_label() );
      // node 1 is grounded or invalid
    }
  }
}
/*--------------------------------------------------------------------------*/
void VA_BREQN::tr_iwant_matrix()
{
  trace3("tr_iwant_matrix", long_label(), matrix_nodes(), BR());
  assert(is_device());
  assert(ext_nodes() + int_nodes() == matrix_nodes());

  assert(!subckt());
  tr_iwant_matrix_extended_branch();

  _sim->_aa.iwant(_n[BR()].m_(),_n[OUT1].m_());
  _sim->_lu.iwant(_n[BR()].m_(),_n[OUT1].m_());
  _sim->_aa.iwant(_n[BR()].m_(),_n[OUT2].m_());
  _sim->_lu.iwant(_n[BR()].m_(),_n[OUT2].m_());
}
/*--------------------------------------------------------------------------*/
bool VA_BREQN::do_tr_con_chk_and_q()
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
    trace2("pot?", _loss1, _loss0);
//    set_converged(_loss1 == _loss0);
  }else{
  }
  for (int i=1; converged() && i<=_n_ports; ++i) {
    set_converged(conchk(_old_values[i], _values[i]) /*,0.?*/);
  }
  return converged();
}
/*--------------------------------------------------------------------------*/
bool VA_BREQN::do_tr()
{
  assert(_values);

  if(is_vs()){
    trace2("loss", _values[0], _values[1]);
    _m0.x = 0.;
    _m0.c0 = -_loss0 * _values[0]; // -> rhs
    _m0.c1 = 0; // -_loss0 * _values[1];
  }else{
    // current source.
    _m0 = CPOLY1(0., _values[0], _values[1]);
  }
  return do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
inline void VA_BREQN::tr_unload_ones()
{
  _one0 = 0;
  tr_load_ones();
}
/*--------------------------------------------------------------------------*/
inline void VA_BREQN::tr_load_ones()
{
  double d = dampdiff(&_one0, _one1);
  if (d != 0.) {
//     tr_load_shunt(); // 4 pt +- loss
// 		     return;
    _sim->_aa.load_asymmetric(_n[OUT1].m_(), _n[OUT2].m_(), _n[BR()].m_(), 0,  d);

    if(is_vs()){
      trace4("BREQN::tr_load_ones vs", long_label(), _one0, _one1, d);
      _sim->_aa.load_asymmetric(_n[BR()].m_(), 0, _n[OUT1].m_(), _n[OUT2].m_(), d);
    }else{
      trace4("BREQN::tr_load_ones cs", long_label(), _one0, _one1, d);
      _sim->_aa.load_diagonal_point(_n[BR()].m_(), d);
    }
  }else{
  }
  _one1 = _one0;
}
/*--------------------------------------------------------------------------*/
void VA_BREQN::tr_load()
{
  node_t gnd(&ground_node);
  if(_loss0 != _loss1){
      std::swap(_loss1, _loss0);
      tr_unload_ones();
      std::swap(_loss1, _loss0);
      _one0 = 1;
      _loss1 = _loss0;
  }else{
  }
  tr_load_ones();

  if(is_vs()){
    // is_voltage_source
    trace1("VA_BREQN::tr_load vs", _values[0]);
    tr_load_source_point(_n[BR()], &_values[0], &_old_values[0]); // rhs.

    for (int i=1; i<=_n_ports; ++i) {
      trace2("VA_BREQN::tr_load vs", i, _values[i]);
      tr_load_extended(gnd, _n[BR()], _n[2*i-2], _n[2*i-1], &(_values[i]), &(_old_values[i]));
    }
  }else{
    trace2("VA_BREQN::tr_load I", _values[0], _values[1]);

    for (int i=1; i<=_n_ports; ++i) {
      trace4("tr_load", long_label(), i, _values[i], _old_values[i]);
      tr_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], &(_values[i]), &(_old_values[i]));
    }
  }
}
/*--------------------------------------------------------------------------*/
double VA_BREQN::tr_amps()const
{ untested();
  return _n[BR()].v0();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void VA_BREQN::ac_iwant_matrix_extended_branch()
{
  for (int ii = 2;  ii < matrix_nodes();  ++ii) {
    // connect all to branch..
    _sim->_acx.iwant(_n[BR()].m_(),_n[ii].m_());

    // is this too much?
    if (_n[ii].m_()  != INVALID_NODE) {
      for (int jj = 2;  jj < ii ;  ++jj) {
	_sim->_acx.iwant(_n[ii].m_(),_n[jj].m_());
      }
    }else{ untested();
      trace3("eek", ii, _n[ii].m_(), long_label() );
      // node 1 is grounded or invalid
    }
  }
}
/*--------------------------------------------------------------------------*/
void VA_BREQN::ac_iwant_matrix()
{
  trace3("tr_iwant_matrix", long_label(), matrix_nodes(), BR());
  assert(is_device());
  assert(ext_nodes() + int_nodes() == matrix_nodes());

  assert(!subckt());
  ac_iwant_matrix_extended_branch();

  _sim->_acx.iwant(_n[BR()].m_(),_n[OUT1].m_());
  _sim->_acx.iwant(_n[BR()].m_(),_n[OUT2].m_());
}
/*--------------------------------------------------------------------------*/
inline void VA_BREQN::ac_load_ones()
{
  double d = 1.;
  _sim->_acx.load_asymmetric(_n[OUT1].m_(), _n[OUT2].m_(),_n[BR()].m_(), 0,  d);

  if(is_vs()){
    _sim->_acx.load_asymmetric(_n[BR()].m_(), 0, _n[OUT1].m_(), _n[OUT2].m_(), d);
  }else{
    _sim->_acx.load_diagonal_point(_n[BR()].m_(), d);
  }
}
/*--------------------------------------------------------------------------*/
void VA_BREQN::ac_load()
{
  node_t gnd(&ground_node);
  ac_load_ones();
  if(is_vs()){
    for (int i=1; i<=_n_ports; ++i) {
      trace2("VA_BREQN::ac_load", i, _values[i]);
      ac_load_extended(gnd, _n[BR()], _n[2*i-2], _n[2*i-1], _values[i]);
    }
  }else{
    for (int i=1; i<=_n_ports; ++i) {
      ac_load_extended(_n[OUT1], _n[OUT2], _n[2*i-2], _n[2*i-1], _values[i]);
    }
  }
  if(_current_port_names.size()){ untested();
    incomplete();
  }else{
  }
  _acg = _values[1];
//  ac_load_passive();

}
/*--------------------------------------------------------------------------*/
/* set: set parameters, used in model building
 */
void VA_BREQN::set_parameters(const std::string& Label, CARD *Owner,
				 COMMON_COMPONENT *Common, double Value,
				 int n_states, double states[],
				 int n_nodes, const node_t nodes[])
  //				 const double* inputs[])
{
  bool first_time = (net_nodes() == 0);

  set_label(Label);
  trace3("VA_BREQN::set_parameters", long_label(), n_nodes, n_states);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);
  _current_port_names.resize(n_states - 1 - n_nodes/2);
  _input.resize(n_states - 1 - n_nodes/2);

  if (first_time) {
    _n_ports = n_states-1; // set net_nodes
    assert(size_t(_n_ports) == n_nodes/2 + _current_port_names.size());

    assert(!_old_values);
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
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_pot_br", &d_breqn);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
