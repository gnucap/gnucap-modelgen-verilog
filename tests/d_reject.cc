/*                            -*- C++ -*-
 * Copyright (C) 2024 Felix Salfelder
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
 * reject a time step. for testing.
 */
#include "globals.h"
#include "e_compon.h"
#include "u_xprobe.h"
#include "e_node.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class REJECT : public COMPONENT {
	PARAMETER<double> _rejecttime;
	bool _done{false};
	double _time0{0.};
	double _time1{0.};
//	PARAMETER<double> _rejectto; .5
private:
  explicit REJECT(const REJECT& p) :COMPONENT(p) {
	  trace1("REJECT", _net_nodes);
	  for(int i = 0; i<_net_nodes; ++i){ untested();
		  _n[i] = p._n[i];
	  }
  }
public:
  explicit REJECT()		:COMPONENT() {}
private: // override virtual
  char	   id_letter()const override	{ untested();return '\0';}
  std::string value_name()const override { untested();return "";}
  std::string dev_type()const override	{return "reject";}
  int	   max_nodes()const override		{return 0;}
  int	   min_nodes()const override		{return 0;}
  int	   net_nodes()const override		{return _net_nodes;}
  CARD*	   clone()const override		{return new REJECT(*this);}
  TIME_PAIR     tr_review()override;
  // bool     tr_needs_eval()override;
  void precalc_last()override {
	  COMPONENT::precalc_last();
	  _rejecttime.e_val(1., scope());
  }
  void tr_advance()override{ _time1 = _time0; _time0 = _sim->_time0; }

  double   tr_probe_num(const std::string&)const override;
  bool print_type_in_spice()const override{ untested();return false;}
  std::string port_name(int)const { untested();unreachable(); return "";}

  int param_count()const override{
	  return COMPONENT::param_count() + 1;
  }
  int set_param_by_name(std::string N, std::string V)override {
	  if(N=="time"){
		  _rejecttime = V;
		  return 0;
	  }else{ untested();
		  return COMPONENT::set_param_by_name(N, V);
	  }
  }
  std::string param_name(int I, int J)const override { untested();
	  if (J == 0) {
		  return param_name(I);
	  }else{
		  return "";
	  }
  }
  std::string param_name(int i)const override { untested();
	  int idx = REJECT::param_count() - 1 - i;
	  if(idx==0){ untested();
		  return "time";
	  }else{ untested();
		  return "unreachable";
	  }
  }
}p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "reject", &p1);
/*--------------------------------------------------------------------------*/
TIME_PAIR REJECT::tr_review()
{
	TIME_PAIR t;
	if(_done){
	}else if(_sim->_time0 > _rejecttime){
		trace2("reject", _sim->_time0, _time1);
		t.min_event((_sim->_time0 + _time1)*.5);
		_done = true;
	}
	return t;
}
/*--------------------------------------------------------------------------*/
double REJECT::tr_probe_num(const std::string& x)const
{ untested();
	return COMPONENT::tr_probe_num(x);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
