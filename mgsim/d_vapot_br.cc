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

  double   tr_involts()const override	{ untested();unreachable(); return NOT_VALID;}
  double   tr_involts_limited()const override { untested();unreachable(); return NOT_VALID;}
  double   tr_amps()const override;
  void	   ac_iwant_matrix()override;
  void	   ac_iwant_matrix_extended_branch();
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{untested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{untested(); return NOT_VALID;}
  node_t& n_(int i)const override {
    assert(_nN); assert(i>=0); assert(i<matrix_nodes()); return _nN[i];
  }

  bool has_iv_probe()const override{ untested();incomplete(); return false;}
  void expand()override;
 // void expand_last()override{assert(0);}

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
  DEV_CPOLY_G::expand();
  if (_sim->is_first_expand()) {
    for(int i=net_nodes(); i<ext_nodes()+int_nodes(); ++i){
      n_(i).clear();
    }
  }else{ untested();
  }
  assert(BR());
  if (_sim->is_first_expand()) {
    n_(BR()).new_model_node( long_label() + ".br", this);
    assert(BR() < int_nodes()+ext_nodes());
  }else{ untested();
  }
}
/*--------------------------------------------------------------------------*/
void VA_BREQN::tr_iwant_matrix_extended_branch()
{

  for (int ii = 0;  ii < matrix_nodes();  ++ii) {
      trace2("ELEMENT::tr_iwant_matrix_extended", ii, n_(ii).m_() );
  }
  for (int ii = 0;  ii < matrix_nodes();  ++ii) {
    // connect all to branch..
    _sim->_aa.iwant(n_(BR()).m_(),n_(ii).m_());
    _sim->_lu.iwant(n_(BR()).m_(),n_(ii).m_());

    // is this too much?
    if (n_(ii).m_()  != INVALID_NODE) {
      for (int jj = 2;  jj < ii ;  ++jj) {
	_sim->_aa.iwant(n_(ii).m_(),n_(jj).m_());
	_sim->_lu.iwant(n_(ii).m_(),n_(jj).m_());
      }
    }else{ untested();
      trace3("eek", ii, n_(ii).m_(), long_label() );
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

  _sim->_aa.iwant(n_(BR()).m_(),n_(OUT1).m_());
  _sim->_lu.iwant(n_(BR()).m_(),n_(OUT1).m_());
  _sim->_aa.iwant(n_(BR()).m_(),n_(OUT2).m_());
  _sim->_lu.iwant(n_(BR()).m_(),n_(OUT2).m_());
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
    if(_values[0] || _values[1]){ untested();
       // possibly incomplete();
    }else{
    }
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
    if(is_vs()){
      trace4("BREQN::tr_load_ones vs", long_label(), _one0, _one1, d);
      // like tr_load inode, but use branch index.
      _sim->_aa.load_couple(n_(OUT1).m_(), n_(BR()).m_(), -d);
      _sim->_aa.load_couple(n_(OUT2).m_(), n_(BR()).m_(),  d);
    }else{
      trace4("BREQN::tr_load_ones cs", long_label(), _one0, _one1, d);
      _sim->_aa.load_asymmetric(n_(OUT1).m_(), n_(OUT2).m_(), n_(BR()).m_(), 0,  d);
      _sim->_aa.load_diagonal_point(n_(BR()).m_(), d);
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

  tr_load_source_point(n_(BR()), &_values[0], &_old_values[0]);
  if(_current_port_names.size()){ untested();
    incomplete();
  }else{
  }

  if(is_vs()){
    // is_voltage_source
    trace1("VA_BREQN::tr_load vs", _values[0]);

    for (int i=1; i<=_n_ports; ++i) {
      trace2("VA_BREQN::tr_load vs", i, _values[i]);
      tr_load_extended(gnd, n_(BR()), n_(2*i-2), n_(2*i-1), &(_values[i]), &(_old_values[i]));
    }
  }else{
    trace2("VA_BREQN::tr_load I", _values[0], _values[1]);

    for (int i=1; i<=_n_ports; ++i) { untested();
      tr_load_extended(gnd, n_(BR()), n_(2*i-2), n_(2*i-1), &(_values[i]), &(_old_values[i]));
    }
  }
}
/*--------------------------------------------------------------------------*/
double VA_BREQN::tr_amps()const
{ untested();
  return n_(BR()).v0();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void VA_BREQN::ac_iwant_matrix_extended_branch()
{
  for (int ii = 2;  ii < matrix_nodes();  ++ii) {
    // connect all to branch..
    _sim->_acx.iwant(n_(BR()).m_(),n_(ii).m_());

    // is this too much?
    if (n_(ii).m_()  != INVALID_NODE) {
      for (int jj = 2;  jj < ii ;  ++jj) {
	_sim->_acx.iwant(n_(ii).m_(),n_(jj).m_());
      }
    }else{ untested();
      trace3("eek", ii, n_(ii).m_(), long_label() );
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

  _sim->_acx.iwant(n_(BR()).m_(),n_(OUT1).m_());
  _sim->_acx.iwant(n_(BR()).m_(),n_(OUT2).m_());
}
/*--------------------------------------------------------------------------*/
inline void VA_BREQN::ac_load_ones()
{
  double d = 1.;

  if(is_vs()){
//    _sim->_acx.load_asymmetric(n_(OUT1).m_(), n_(OUT2).m_(),n_(BR()).m_(), 0,  d);
//    _sim->_acx.load_asymmetric(n_(BR()).m_(), 0, n_(OUT1).m_(), n_(OUT2).m_(), d);
    _sim->_acx.load_couple(n_(OUT1).m_(), n_(BR()).m_(), -d);
    _sim->_acx.load_couple(n_(OUT2).m_(), n_(BR()).m_(),  d);
  }else{
    _sim->_acx.load_asymmetric(n_(OUT1).m_(), n_(OUT2).m_(),n_(BR()).m_(), 0,  d);
    _sim->_acx.load_diagonal_point(n_(BR()).m_(), d);
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
      ac_load_extended(gnd, n_(BR()), n_(2*i-2), n_(2*i-1), _values[i]);
    }
  }else{
    for (int i=1; i<=_n_ports; ++i) {
      ac_load_extended(n_(OUT1), n_(OUT2), n_(2*i-2), n_(2*i-1), _values[i]);
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
/*--------------------------------------------------------------------------*/
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_pot_br", &d_breqn);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
