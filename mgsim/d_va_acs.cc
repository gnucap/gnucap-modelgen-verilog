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
 * VAACS : ac_stim device. missing phase..
 */
#include "d_va.h"
#include "u_xprobe.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class VAACS : public DEV_CPOLY_G /* COMPONENT? */ {
protected:
  explicit VAACS(const VAACS& p) : DEV_CPOLY_G(p) {
    _loss1 = _loss0 = 1.; // -> set_parameters?
  }
public:
  explicit VAACS() : DEV_CPOLY_G() {}
  ~VAACS() {}
protected: // override virtual
  CARD*	   clone()const override	{return new VAACS(*this);}
  std::string dev_type()const override{ untested(); return "va_acs"; }
  std::string value_name()const override{ return "mag"; }
//  int      net_nodes()const override {untested(); return _net_nodes;}
//  int      max_nodes()const override {untested(); return 4;}
//   void set_port_by_index(int i, std::string& s) override { untested();
//     incomplete();
//   }
  void	   tr_iwant_matrix()override	{tr_iwant_matrix_extended();}
  double   tr_involts()const override	{untested(); return tr_outvolts();}
  double   tr_involts_limited()const override { untested();return tr_outvolts_limited();}
  double   tr_amps()const override{return 0.;}
  void	   ac_iwant_matrix()override	{ac_iwant_matrix_extended();}
  void	   ac_load()override;
  COMPLEX  ac_involts()const override	{itested(); return NOT_VALID;}
  COMPLEX  ac_amps()const override	{itested(); return NOT_VALID;}
  bool	   do_tr()override { return true; }
  bool	   tr_needs_eval()const override { return false; }

  bool has_iv_probe()const override{ untested();return true;}
public:
 // int set_param_by_name(std::string n, std::string v)override { untested();
 //   if(n=="mag"){ untested();
 //   incomplete();
 //     return 0;
 //   }else{ untested();
 //     return DEV_CPOLY_G::set_param_by_name(n, v);
 //   }
 // }
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
void VAACS::ac_load()
{
  ac_load_shunt(); // 4 pt +- loss
  trace2("VAACS::ac_load", long_label(), value());
  _acg = value(); // 1; // _vy0[1] * _sim->_jomega; BUG. value?
  ac_load_source();

  if(_loss0){
  //  ac_load_passive();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DISPATCHER<CARD>::INSTALL d2(&device_dispatcher, "va_acs", &d);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
