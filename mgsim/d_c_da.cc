/*$Id: d_d_a.cc  $ -*- C++ -*-
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
 * D to A connect module
 * event triggered, analog out
 */
//testing=none
#include "globals.h"
#include "e_logicnode.h"
#include "e_logicmod.h"
#include "e_logic.h"
#include "u_xprobe.h"
#include "e_elemnt.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_D_A : public ELEMENT {
public:
  enum {UPPER=0, LOWER=1, OUTNODE=UPPER, INNODE=LOWER}; //node labels
private:
  static int	_count;
public:
  explicit DEV_D_A(COMMON_COMPONENT* c=nullptr);
  explicit DEV_D_A(const DEV_D_A& p);
	   ~DEV_D_A()			{--_count;}
private: // override virtuals
  char	   id_letter()const override	{untested();return '\0';}
  std::string value_name()const override{untested();return "";}
  bool	   print_type_in_spice()const override{untested();return true;}
  std::string dev_type()const override	{return "c_d_a";}
  int	   tail_size()const override	{untested();return 2;}
  int	   max_nodes()const override	{return 2;}
  int	   min_nodes()const override	{return 2;}
  int	   matrix_nodes()const override	{untested();return 2;}
  int	   net_nodes()const override	{return _net_nodes;}

  CARD*	   clone()const override	{return new DEV_D_A(*this);}
  void	   precalc_first()override	{ELEMENT::precalc_first();}
  void	   expand()override;
  void	   precalc_last() override	{ELEMENT::precalc_last();}
  void	   tr_iwant_matrix()override	{}
  void	   tr_begin()override		{ELEMENT::tr_begin();}
  void	   tr_restore()override		{untested();ELEMENT::tr_restore();}
  void	   dc_advance()override;
  void	   tr_advance()override;
  void	   tr_regress()override;
  bool	   tr_needs_eval()const override;
  void	   tr_queue_eval()override	{ELEMENT::tr_queue_eval();}
  double to_analog(node_l& n, MODEL_LOGIC const* f);
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_unload()override;
  TIME_PAIR tr_review()override;
  void	   tr_accept()override;
  double   tr_involts()const override	{untested();unreachable(); return 0;}
  //double tr_input()const		//ELEMENT
  double   tr_involts_limited()const override {untested();unreachable(); return 0;}
  //double tr_input_limited()const	//ELEMENT
  //double tr_amps()const		//ELEMENT
  double   tr_probe_num(const std::string& what)const override
					{untested(); return n_(OUTNODE)->tr_probe_num(what);}

  void	   ac_iwant_matrix()override	{}
  void	   ac_begin()override
		{untested(); error(bWARNING, long_label() + ": no logic in AC analysis\n");}
  void	   do_ac()override		{untested();}
  void	   ac_load()override		{untested();}
  COMPLEX  ac_involts()const override	{unreachable(); return 0.;}
  COMPLEX  ac_amps()const override	{unreachable(); return 0.;}
  XPROBE   ac_probe_ext(const std::string& what)const override
					{return n_(OUTNODE)->ac_probe_ext(what);}

  std::string port_name(int i)const override {
    assert(i >= 0);
    assert(i < 2);
    static std::string names[] = {"out", "in"};
    return names[i];
  }
public:
  static int count()			{untested();return _count;}
private:
  node_l& n_(int i)const override {
    return reinterpret_cast<node_l&>(ELEMENT::n_(i));
  }
  MODEL_LOGIC const* process()const {
    const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
    assert(c);
    const MODEL_LOGIC* m = dynamic_cast<const MODEL_LOGIC*>(c->model());
    assert(m);
    return m;
  }
};
/*--------------------------------------------------------------------------*/
DEV_D_A::DEV_D_A(COMMON_COMPONENT* c)
  :ELEMENT(c)
{
  ++_count;
}
/*--------------------------------------------------------------------------*/
DEV_D_A::DEV_D_A(const DEV_D_A& p)
  :ELEMENT(p)
{
  assert(max_nodes() == 2);
  for (int ii = 0;  ii < max_nodes();  ++ii) {
    assert(_nodes[ii] == p._nodes[ii]);
  }
  ++_count;
}
/*--------------------------------------------------------------------------*/
void DEV_D_A::expand()
{
  ELEMENT::expand();
  const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);

  attach_model();
  c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);
  const MODEL_LOGIC* m = dynamic_cast<const MODEL_LOGIC*>(c->model());
  if (!m) {untested();
    throw Exception_Model_Type_Mismatch(long_label(), c->modelname(), "logic family (LOGIC)");
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DEV_D_A::dc_advance()
{
  ELEMENT::dc_advance();
  if (n_(INNODE)->in_transit()) {untested();
    //q_eval(); evalq is not used for DC
    n_(INNODE)->propagate();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
/* tr_advance: the first to run on a new time step.
 * It sets up preconditions for the new time.
 */
void DEV_D_A::tr_advance()
{
  ELEMENT::tr_advance();

  if (n_(INNODE)->in_transit()) {
    q_eval();
    if (_sim->_time0 >= n_(INNODE)->final_time()) {untested();untested();
      n_(INNODE)->propagate();
    }else{
      // not ready to propagate.
    }
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DEV_D_A::tr_regress()
{untested();
  ELEMENT::tr_regress();

  q_eval();
  if (n_(INNODE)->last_change_time() > _sim->_time0) {untested();
    n_(INNODE)->unpropagate();
    assert(_sim->_time0 < n_(INNODE)->final_time());
  }else if (_sim->_time0 >= n_(INNODE)->final_time()) {untested();
    n_(INNODE)->propagate();
  }else{untested();
  }
}
/*--------------------------------------------------------------------------*/
/* tr_needs_eval
 * in digital mode ... DC always returns true, to queue it.
 * tran always returns false, already queued by tr_advance if needed
 */
bool DEV_D_A::tr_needs_eval()const
{
  return true;
  //assert(!is_q_for_eval());
  if (_sim->analysis_is_restore()) {untested();
  }else if (_sim->analysis_is_static()) {untested();
  }else{untested();
  }
  return (_sim->analysis_is_static() || _sim->analysis_is_restore());
}
/*--------------------------------------------------------------------------*/
double DEV_D_A::to_analog(node_l& n, MODEL_LOGIC const* f)
{
  assert(f);
//  assert(process());
//  assert(process() == f);
  //if (process() && process() != f) {untested();
  //  error(bWARNING, "node " + long_label()
  //	  + " logic process mismatch\nis it " + process()->long_label()
  //	  + " or " + f->long_label() + "?\n");
  //}
  //set_process(f);

  double start = NOT_VALID;
  double end = NOT_VALID;
  double risefall = NOT_VALID;
  switch (n->lv()) {
  case lvSTABLE0:
    return process()->vmin;
  case lvRISING:
    start = process()->vmin;
    end = process()->vmax;
    risefall = process()->rise;
    break;
  case lvFALLING:
    start = process()->vmax;
    end = process()->vmin;
    risefall = process()->fall;
    break;
  case lvSTABLE1:
    return process()->vmax;
  case lvUNKNOWN:
    return process()->unknown;
  }
  assert(start != NOT_VALID);
  assert(end   != NOT_VALID);
  assert(risefall != NOT_VALID);

  if (_sim->_time0 <= (n->final_time()-risefall)) {
    return start;
  }else if (_sim->_time0 >= n->final_time()) { untested();
    return end;
  }else{
    return end - ((end-start) * (n->final_time()-_sim->_time0) / risefall);
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_D_A::do_tr()
{
  if (_sim->analysis_is_restore()) {untested();
  }else if (_sim->analysis_is_static()) {
  }else{
  }
  if (_sim->analysis_is_static() || _sim->analysis_is_restore()) {
    tr_accept();
  }else{
    assert(_sim->analysis_is_tran_dynamic());
  }
  
  const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);
  const MODEL_LOGIC* m = prechecked_cast<const MODEL_LOGIC*>(c->model());
  assert(m);
  _y[0].x = 0.;
  _y[0].f1 = to_analog(n_(INNODE), m);
  _y[0].f0 = 0.;
  _m0.x = 0.;
  _m0.c1 = 1./m->rs;
  _m0.c0 = _y[0].f1 / m->rs;
  set_converged(conv_check());
  store_values();
  q_load();
  
  return converged();
}
/*--------------------------------------------------------------------------*/
void DEV_D_A::tr_load()
{
  tr_load_diagonal_point(n_(OUTNODE), &_m0.c1, &_m1.c1);
  tr_load_source_point(n_(OUTNODE), &_m0.c0, &_m1.c0);
}
/*--------------------------------------------------------------------------*/
void DEV_D_A::tr_unload()
{untested();
  _m0.c0 = _m0.c1 = 0.;
  _sim->mark_inc_mode_bad();
  tr_load();
}
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_D_A::tr_review()
{
  // not calling ELEMENT::tr_review();
  q_accept();
  //digital mode queues events explicitly in tr_accept
  _time_by.reset();
  return _time_by;
}
/*--------------------------------------------------------------------------*/
/* tr_accept: This runs after everything has passed "review".
 * It sets up and queues transitions, and sometimes determines logic states.
 */
void DEV_D_A::tr_accept()
{
  const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);
  const MODEL_LOGIC* m = prechecked_cast<const MODEL_LOGIC*>(c->model());
  assert(m);

  //LOGICVAL future_state = lvSTABLE0;
  //n_(OUTNODE)->set_event(c->_real_delay, future_state);

#if 0
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /* Check quality and get node info to local array. */
  /* side effect --- generate digital values for analog nodes */
  {
    n_(OUTNODE)->to_logic(m, n_(OUTNODE)->v0());
    int lastchangeiter=n_(OUTNODE)->d_iter();/* iteration # when it changed */
    trace0(long_label().c_str());
    trace2(n_(OUTNODE)->failure_mode().c_str(), OUTNODE, n_(OUTNODE)->quality());
    
    for (int ii = BEGIN_IN;  ii < net_nodes();  ++ii) {
      n_(ii)->to_logic(m, n_(ii)->v0());
      if (n_(ii)->quality() < _quality) {
	_quality = n_(ii)->quality();
	_failuremode = n_(ii)->failure_mode();
      }else{
      }
      if (n_(ii)->d_iter() >= lastchangeiter) {
	lastchangeiter = n_(ii)->d_iter();
	_lastchangenode = ii;
      }else{
      }
      trace2(n_(ii)->failure_mode().c_str(), ii, n_(ii)->quality());
    }
    /* If _lastchangenode == OUTNODE, no new changes, bypass may be ok.
     * Otherwise, an input changed.  Need to evaluate.
     * If all quality are good, can evaluate as digital.
     * Otherwise need to evaluate as analog.
     */
    trace3(_failuremode.c_str(), _lastchangenode, lastchangeiter, _quality);
  }
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */  
#endif
#if 0
  if (want_analog()) {
    if (_gatemode == moDIGITAL) {untested();
      error(bTRACE, "%s:%u:%g switch to analog, %s\n", long_label().c_str(),
	    _sim->iteration_tag(), _sim->_time0, _failuremode.c_str());
      _oldgatemode = _gatemode;
      _gatemode = moANALOG;
    }else{
    }
    assert(_gatemode == moANALOG);
  }else{
    assert(want_digital());
    if (_gatemode == moANALOG) {
      error(bTRACE, "%s:%u:%g switch to digital\n",
	    long_label().c_str(), _sim->iteration_tag(), _sim->_time0);
      _oldgatemode = _gatemode;
      _gatemode = moDIGITAL;
    }else{
    }
    assert(_gatemode == moDIGITAL);
    if (_sim->analysis_is_restore()) {untested();
    }else if (_sim->analysis_is_static()) {
    }else{
    }
    if (!_sim->_bypass_ok
	|| _lastchangenode != OUTNODE
	|| _sim->analysis_is_static()
	|| _sim->analysis_is_restore()) {
      LOGICVAL future_state = c->logic_eval(&n_(BEGIN_IN), net_nodes()-BEGIN_IN);
      //		         ^^^^^^^^^^
      if ((n_(OUTNODE)->is_unknown()) &&
	  (_sim->analysis_is_static() || _sim->analysis_is_restore())) {
	n_(OUTNODE)->force_initial_value(future_state);
	n_(OUTNODE)->store_old_lv();
	/* This happens when initial DC is digital.
	 * Answers could be wrong if order in netlist is reversed 
	 */
      }else if (future_state != n_(OUTNODE)->lv()) {
	assert(future_state != lvUNKNOWN);
	switch (future_state) {
	case lvSTABLE0:	/*nothing*/		break;
	case lvRISING:  future_state=lvSTABLE0;	break;
	case lvFALLING: future_state=lvSTABLE1;	break;
	case lvSTABLE1:	/*nothing*/		break;
	case lvUNKNOWN: unreachable();		break;
	}
	/* This handling of rising and falling may seem backwards.
	 * These states occur when the value has been contaminated 
	 * by another pending action.  The "old" value is the
	 * value without this contamination.
	 * This code is planned for replacement as part of VHDL/Verilog
	 * conversion, so the kluge stays in for now.
	 */
	assert(future_state.lv_old() == future_state.lv_future());
	if (n_(OUTNODE)->lv() == lvUNKNOWN
	    || future_state.lv_future() != n_(OUTNODE)->lv_future()) {
	  n_(OUTNODE)->set_event(c->_real_delay, future_state);
	  //assert(future_state == n_(OUTNODE).lv_future());
	  if (_lastchangenode == OUTNODE) {untested();
	    unreachable();
	    error(bDANGER, "%s:%u:%g non-event state change\n",
		  long_label().c_str(), _sim->iteration_tag(), _sim->_time0);
	  }else{
	  }
	}else{
	}
      }else{
      }
    }else{
    }
    n_(OUTNODE)->store_old_last_change_time();
    n_(OUTNODE)->store_old_lv(); // needed? yes
  }
#endif
}
/*--------------------------------------------------------------------------*/
int DEV_D_A::_count = -1;
/*--------------------------------------------------------------------------*/
class LOGIC_NONE : public COMMON_LOGIC {
private:
  explicit LOGIC_NONE(const LOGIC_NONE&p):COMMON_LOGIC(p){++_count;}
  COMMON_COMPONENT* clone()const override {return new LOGIC_NONE(*this);}
public:
  explicit LOGIC_NONE(int c=0)		  :COMMON_LOGIC(c) {}
  LOGICVAL logic_eval(const node_l*, int)const override {untested();
    return lvUNKNOWN;
  }
  std::string name()const override	  {untested();return "error";}
};
/*--------------------------------------------------------------------------*/
LOGIC_NONE c_buf(CC_STATIC);
DEV_D_A d_buf(&c_buf);
DISPATCHER<CARD>::INSTALL dd_buf(&device_dispatcher, "c_d_a", &d_buf);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
