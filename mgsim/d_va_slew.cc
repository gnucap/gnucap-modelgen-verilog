/*                     -*- C++ -*-
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
 * DEV_SLEW: slew filter stub
 */
#include <globals.h>
#include <e_storag.h>
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_SLEW : public ELEMENT {
  double* _states{NULL};
  int _n_states{0};
  FPOLY1   _i[OPT::_keep_time_steps];
private:
  explicit DEV_SLEW(const DEV_SLEW& p)
    :ELEMENT(p) {}
public:
  explicit DEV_SLEW() :ELEMENT() {}
private: // override virtual
  char	   id_letter()const override	{ untested();unreachable(); return '\0';}
  std::string value_name()const override { untested();unreachable(); return "";}
  std::string dev_type()const override	{ untested();unreachable(); return "slew";}
  CARD*	   clone()const override	{return new DEV_SLEW(*this);}
  std::string port_name(int)const override { untested();unreachable(); return "";}
  int max_nodes()const override {return 0.;}
  int min_nodes()const override {return 0.;}
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
  void	   tr_iwant_matrix() override {}
  void	   ac_iwant_matrix() override {}
  bool	   do_tr()override;
  void	   tr_begin()override {
    ELEMENT::tr_begin();
    // _method_a = method_select[OPT::method][_method_u];
    for (int i = 0;  i < OPT::_keep_time_steps;  ++i) {
      _i[i] = FPOLY1(0., 0., 0.);
    }
    // _m1 = _m0 = CPOLY1(0., 0., 0.);
  }
/*--------------------------------------------------------------------------*/
  TIME_PAIR tr_review()override; //		{ untested();return _time_by.reset();}//BUG//review(_i0.f0, _it1.f0);}
				 //
  double  tr_involts()const override { untested();unreachable(); return 0.;}
  double  tr_involts_limited()const override { untested();unreachable(); return 0.;}
  COMPLEX ac_involts()const override { untested();unreachable(); return 0.;}
  void dc_advance() override {
    ELEMENT::dc_advance();

    for (int i = 1;  i < OPT::_keep_time_steps;  ++i) {
      _i[i] = _i[0];
    }
  }
  void tr_advance() override {
    ELEMENT::tr_advance();

    for (int i=OPT::_keep_time_steps-1; i>0; --i) {
      _i[i] = _i[i-1];
    }
  }
/*--------------------------------------------------------------------------*/
}slew;
DISPATCHER<CARD>::INSTALL d4(&device_dispatcher, "va_slew", &slew);
/*--------------------------------------------------------------------------*/
void DEV_SLEW::set_parameters(const std::string& Label, CARD *Owner,
				   COMMON_COMPONENT *Common, double Value,
				   int n_states, double states[],
				   int, const node_t [])
{
  bool first_time = !_states;
  _n_states = n_states;

  set_label(Label);
  set_owner(Owner);
  set_value(Value);
  attach_common(Common);

  if (first_time) {
  }else{itested();
    assert(net_nodes() == 0);
    // assert could fail if changing the number of nodes after a run
  }

  _states = states;
}
/*--------------------------------------------------------------------------*/
bool DEV_SLEW::do_tr()
{
  _y[0].f1 = 0; // _vy0[1]; // another state, capacity.?
  
  trace4("DEV_SLEW::do_tr", _sim->_time0, _y[0].f0, _y[1].f0, _y1.f0);

  if(_sim->is_advance_iteration()){ untested();
    // return true;
  }else if( CKT_BASE::_sim->analysis_is_static() ) {
    assert(_sim->_time0 == 0.);
    // std::fill_n(_states+1, _n_states-1, 0.);
    _states[0] = _states[2];
    _states[1] = 1.;
  } else {
    assert (!CKT_BASE::_sim->analysis_is_ac());
    double slope = ( _states[2] - _i[1].f0 ) / _dt;
    trace3("DEV_SLEW::do_tr", _sim->_time0, _states[2], _dt);

    double sl_min, sl_max;
    if(_n_states > 4){
      sl_max = _states[3];
      sl_min = _states[4];
    }else if(_n_states > 3){
      sl_max = _states[3];
      sl_min = -sl_max;
    }else{ untested();
      sl_max = 1e99;
      sl_min = -sl_max;
    }
    trace4("DEV_SLEW::do_tr", sl_min, sl_max, slope, _n_states);

    if(_n_states <= 3){ untested();
    }else if(slope > sl_max){
      _states[0] = _i[1].f0 /* out?! */ + sl_max * _dt;
      _states[1] = 0.;
    }else if(slope < sl_min){
      _states[0] = _i[1].f0 + sl_min * _dt;
      _states[1] = 0.;
    }else{
      _states[0] = _states[2];
      _states[1] = 1.;
    }

    _y[0].f1 = slope;
  }
  _i[0].f0 = _states[0]; // accept?
  _y[0].f0 = _states[2];
  return true;//do_tr_con_chk_and_q();
}
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_SLEW::tr_review()
{
  ELEMENT::tr_review();
  double sl_min, sl_max;
  if(_n_states > 4){
    sl_max = _states[3];
    sl_min = _states[4];
  }else if(_n_states > 3){
    sl_max = _states[3];
    sl_min = -sl_max;
  }else{ untested();
    sl_max = 1e99;
    sl_min = -sl_max;
  }

  double slope_in = ( _y[0].f0 - _y[1].f0 ) / _dt;

  if(_n_states <= 3){ untested();
  }else if(_time[0] == 0){
  }else if(_i[0].f0 < _y[0].f0){
    if(slope_in - sl_max < 0){ untested();
      double cross = (_y[0].f0 - _i[0].f0) / (sl_max - slope_in);
      assert(cross>0);
      double new_evt = _time[0] + cross;
      _time_by.min_event(new_evt);
    }else{
    }
  }else if(_i[0].f0 > _y[0].f0){
    if(slope_in - sl_min > 0){
      double cross = (_y[0].f0 - _i[0].f0) / (sl_min - slope_in);
      assert(cross>0);
      double new_evt = _time[0] + cross;
      _time_by.min_event(new_evt);
    }else{
    }
  }else if(slope_in > sl_max){
  }else if(slope_in < sl_min){
  }else{
  }

  return _time_by;
}
/*--------------------------------------------------------------------------*/
} // namespace
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
