/*$Id: e_logicnode.h $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
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
 * circuit node class
 */
//testing=script,sparse 2006.07.11
#ifndef E_LOGICNODE_H_
#define E_LOGICNODE_H_
#include "e_logicval.h"
#include "e_logicnode.h"
#include "e_cardlist.h" // cm_hack
/*--------------------------------------------------------------------------*/
class INTERFACE LOGIC_NODE : public LOGIC_NODE_BASE {
private: // inhibited
  explicit LOGIC_NODE(const LOGIC_NODE&) : LOGIC_NODE_BASE(){ untested();incomplete();unreachable();}
public: // general use
  explicit LOGIC_NODE(std::string const& s) : LOGIC_NODE_BASE(s) { untested();}
  explicit LOGIC_NODE(int i=-1) : LOGIC_NODE_BASE(i) { set_mode(moDIGITAL); }
	   ~LOGIC_NODE() {}
  LOGIC_NODE_BASE* clone()const override { untested();return new LOGIC_NODE(*this);}
public:
//  double tr_probe_num(const std::string& x) const override;
public:
  bool is_digital()const override { untested();return true;}
  bool is_analog()const {return false;}
  void set_event(double delay, LOGICVAL v, CARD* Device);
  bool just_reached_stable()const;
  void set_d_iter()			{ LOGIC_NODE_BASE::set_d_iter(); }
  void set_last_change_time()		{ LOGIC_NODE_BASE::set_last_change_time(); }
  void set_last_change_time(double t)	{ LOGIC_NODE_BASE::set_last_change_time(t); }
};
/*--------------------------------------------------------------------------*/
void LOGIC_NODE::set_event(double delay, LOGICVAL v, CARD* Device)
{
  if (Device){
  }else{ itested();
  }
  _lv.set_in_transition(v);
  if (_sim->analysis_is_tran_dynamic()  &&  in_transit()) {
    /* set_bad_quality("race"); */
  }else{
    // normal good quality event
    // leaving quality as it was
  }
  set_d_iter();
  double ft = CKT_BASE::_sim->new_event(_sim->_time0 + delay, Device?:this);
  trace4("set_event", _sim->_time0, delay, ft, _sim->_dtmin);
  set_final_time(ft);
  if (OPT::picky <= bTRACE) { untested();
    error(bTRACE, "%s:%u:%g new event\n",
	  long_label().c_str(), d_iter(), final_time());
  }
  set_last_change_time();
  if(!subckt()){
  }else if(is_cm_hack()){
    subckt()->tr_accept();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
struct node_l : node_t {
  LOGIC_NODE*       operator->()      { return &data(); }
  LOGIC_NODE const* operator->()const { return &data(); }
private:
  LOGIC_NODE& data() const {
    if(n_()->m_()==0) { untested();
      // tmp hack.
      // think of it as an autoinserted connect module
      // mimics traditional behaviour but probably wrong.
      static LOGIC_NODE lg(0);
      return lg;
    }else{
      auto n = prechecked_cast<LOGIC_NODE const*>(n_());
      assert(n);
      return *const_cast<LOGIC_NODE*>(n);
    }
  }
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet:
