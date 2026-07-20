/*$Id: d_logic.cc  $ -*- C++ -*-
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
 * logic model and device.
 * netlist syntax:
 * device:  mxxxx  out gnd vdd in1 in2 ... family gatetype
 * model:   .model mname LOGIC <args>
 */
//testing=none
#include "globals.h"
#include "e_logicnode.h"
#include "e_subckt.h"
#include "e_logicmod.h"
#include "e_logic.h"
#include "u_xprobe.h"
#include "e_elemnt.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_A_D : public ELEMENT {
public:
  enum {UPPER=0, LOWER=1, OUTNODE=UPPER, INNODE=LOWER}; //node labels
private:
  static int	_count;
public:
  explicit DEV_A_D(COMMON_COMPONENT* c=nullptr);
  explicit DEV_A_D(const DEV_A_D& p);
	   ~DEV_A_D()			{--_count;}
private: // override virtuals
  char	   id_letter()const override	{return '\0';}
  std::string value_name()const override{return "";}
  bool	   print_type_in_spice()const override{return true;}
  std::string dev_type()const override	{return "c_a_d";}
  int	   tail_size()const override	{return 2;}
  int	   max_nodes()const override	{return 2;}
  int	   min_nodes()const override	{return 2;}
  int	   matrix_nodes()const override	{return 2;}
  int	   net_nodes()const override	{return _net_nodes;}

  CARD*	   clone()const override	{return new DEV_A_D(*this);}
  void	   precalc_first()override	{ELEMENT::precalc_first();}
  void	   expand()override;
  void	   precalc_last() override	{ELEMENT::precalc_last();}
  void	   tr_iwant_matrix()override	{}
  void	   tr_begin()override		{ELEMENT::tr_begin();}
  void	   tr_restore()override		{ELEMENT::tr_restore();}
  void	   dc_advance()override;
  void	   tr_advance()override;
  void	   tr_regress()override;
  bool	   tr_needs_eval()const override;
  void	   tr_queue_eval()override	{ELEMENT::tr_queue_eval();}
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_unload()override;
  TIME_PAIR tr_review()override;
  void     to_logic(node_l&, MODEL_LOGIC const* f, double v);
  void	   tr_accept()override;
  double   tr_involts()const override	{ untested();unreachable(); return 0;}
  //double tr_input()const		//ELEMENT
  double   tr_involts_limited()const override { untested();unreachable(); return 0;}
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
DEV_A_D::DEV_A_D(COMMON_COMPONENT* c)
  :ELEMENT(c)
{
  ++_count;
}
/*--------------------------------------------------------------------------*/
DEV_A_D::DEV_A_D(const DEV_A_D& p)
  :ELEMENT(p)
{
  assert(max_nodes() == 2);
  for (int ii = 0;  ii < max_nodes();  ++ii) {
    assert(_nodes[ii] == p._nodes[ii]);
  }
  ++_count;
}
/*--------------------------------------------------------------------------*/
void DEV_A_D::expand()
{
  ELEMENT::expand();
  const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);

  attach_model();
  c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);
  const MODEL_LOGIC* m = dynamic_cast<const MODEL_LOGIC*>(c->model());
  if (!m) {
    throw Exception_Model_Type_Mismatch(long_label(), c->modelname(), "logic family (LOGIC)");
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DEV_A_D::dc_advance()
{
  ELEMENT::dc_advance();
}
/*--------------------------------------------------------------------------*/
/* tr_advance: the first to run on a new time step.
 * It sets up preconditions for the new time.
 */
void DEV_A_D::tr_advance()
{
  ELEMENT::tr_advance();
}
/*--------------------------------------------------------------------------*/
void DEV_A_D::tr_regress()
{
  ELEMENT::tr_regress();
}
/*--------------------------------------------------------------------------*/
/* tr_needs_eval
 * in digital mode ... DC always returns true, to queue it.
 * tran always returns false, already queued by tr_advance if needed
 */
bool DEV_A_D::tr_needs_eval()const
{
  assert(!is_q_for_eval());
  return false;
}
/*--------------------------------------------------------------------------*/
bool DEV_A_D::do_tr()
{  
  return converged();
}
/*--------------------------------------------------------------------------*/
void DEV_A_D::tr_load()
{
}
/*--------------------------------------------------------------------------*/
void DEV_A_D::tr_unload()
{
}
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_A_D::tr_review()
{
  // not calling ELEMENT::tr_review();
  q_accept();
  //digital mode queues events explicitly in tr_accept
  _time_by.reset();
  return _time_by;
}
/*--------------------------------------------------------------------------*/
void DEV_A_D::to_logic(node_l& n, MODEL_LOGIC const* f, double input)
{
//  if (n->is_analog()){
    n->set_a_iter();
//  }else{ untested();
//    unreachable();
//  }

  assert(f);
  if (process() && process() != f) {untested();
    unreachable(); // yes it still is..
//    o->set_bad_quality("logic process mismatch");
//    error(bWARNING, "node " + long_label() 
//	  + " logic process mismatch\nis it " + process()->long_label() 
//	  + " or " + f->long_label() + "?\n");
  }
  // set_process(f);

  if (/*n->is_analog() && */  n->d_iter() < n->a_iter()) {
    if (_sim->analysis_is_restore()) {untested();
    }else if (_sim->analysis_is_static()) {
    }else{
    }
    if (_sim->analysis_is_static() || _sim->analysis_is_restore()) {
      n->set_last_change_time(0);
      n->store_old_last_change_time();
      n->set_lv(lvUNKNOWN);
    }else{
    }
    double dt = _sim->_time0 - n->last_change_time();
    if (dt < 0.) {untested();
      error(bPICKY, "time moving backwards.  was %g, now %g\n",
	    n->last_change_time(), _sim->_time0);
      dt = _sim->_time0 - n->old_last_change_time();
      if (dt <= 0.) {untested();
	throw Exception("internal error: time moving backwards, can't recover");
      }else{untested();
      }
      assert(dt > 0.);
      n->set_lv(n->old_lv());			/* skip back one */
    }else{
      n->store_old_last_change_time();
      n->store_old_lv();			/* save to see if it changes */
    }
    
    double sv = input / process()->range;	/* new scaled voltage */
    if (sv >= process()->th1) {		/* logic 1 */
      switch (n->lv()) {
      case lvSTABLE0: break; // n->dont_set_quality("stable 0 to stable 1");	break;
      case lvRISING: untested(); break; //n->dont_set_quality("begin stable 1");	break;
      case lvFALLING: untested(); n->set_quality(qBAD); break; // untested();n->set_bad_quality("rising to stable 0");	break;
      case lvSTABLE1: break; // n->dont_set_quality("continuing stable 1");	break;
      case lvUNKNOWN: n->set_quality(qGOOD);		break;
      }
      n->set_lv(lvSTABLE1);
    }else if (sv <= process()->th0) {	/* logic 0 */
      switch (n->lv()) {
      case lvSTABLE0: break; // n->dont_set_quality("continuing stable 0");	break;
      case lvRISING: untested(); n->set_quality(qBAD); break; // untested();n->set_bad_quality("rising to stable 0");	break;
      case lvFALLING: untested(); break; // n->dont_set_quality("begin stable 0");	break;
      case lvSTABLE1: break; // n->dont_set_quality("stable 1 to stable 0");	break;
      case lvUNKNOWN: n->set_quality(qGOOD); break;
      }
      n->set_lv(lvSTABLE0);
    }else{				/* transition region */
      double oldsv = n->vt1() / process()->range;/* old scaled voltage */
      double diff  = sv - oldsv;
      if (diff > 0) {	/* rising */
	switch (n->lv()) {
	case lvSTABLE0:
	 //  n->dont_set_quality("begin good rise");
	  break;
	case lvRISING:
	  if (diff < dt/(process()->mr * process()->rise)) { untested();
	    n->set_quality(qBAD);
	  }else{
	    // n->dont_set_quality("continuing good rise");
	  }
	  break;
	case lvFALLING:untested();
	  n->set_quality(qBAD); // //( "positive glitch in fall");
	  break;
	case lvSTABLE1:untested();
	  n->set_quality(qBAD); // //( "negative glitch in 1");
	  break;
	case lvUNKNOWN: untested();
	  n->set_quality(qBAD); //( "initial rise");
	  break;
	}
	n->set_lv(lvRISING);
      }else if (diff < 0) {	/* falling */
	switch (n->lv()) {
	case lvSTABLE0:untested();
	  n->set_quality(qBAD); // "positive glitch in 0");
	  break;
	case lvRISING: untested();
	  n->set_quality(qBAD); // "negative glitch in rise");
	  break;
	case lvFALLING:
	  if (-diff < dt/(process()->mf * process()->fall)) { untested();
	    n->set_quality(qBAD); // ("slow fall");
	  }else{
	    // n->dont_set_quality("continuing good fall");
	  }
	  break;
	case lvSTABLE1:
	  // n->dont_set_quality("begin good fall");
	  break;
	case lvUNKNOWN:untested();
	  n->set_quality(qBAD); // ("initial fall");
	  break;
	}
	n->set_lv(lvFALLING);
      }else{untested();			/* hanging up in transition */
	error(bDANGER, "inflection???\n");
	n->set_quality(qBAD); // "in transition but no change");
	/* state (rise/fall)  unchanged */
      }
    }
    if (sv > 1.+process()->over || sv < -process()->over) { untested(); /* out of range */
      n->set_quality(qBAD); // "out of range");
    }
    if (n->just_reached_stable()) { /* A bad node gets a little better */
      n->improve_quality();	/* on every good transition.	   */
    }				/* Eventually, it is good enough.  */
				/* A good transition is defined as */
				/* entering a stable state from    */
				/* a transition state.		   */
    n->set_d_iter();
    n->set_last_change_time();
//    trace3(_failure_mode.c_str(), _lastchange, _quality, _lv);
  }else{ untested();
  }
}
/* tr_accept: This runs after everything has passed "review".
 * It sets up and queues transitions, and sometimes determines logic states.
 */
void DEV_A_D::tr_accept()
{
  const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);
  const MODEL_LOGIC* m = prechecked_cast<const MODEL_LOGIC*>(c->model());
  assert(m);

  bool oldstate = n_(OUTNODE)->lv_future();
  to_logic(n_(OUTNODE), m, n_(INNODE)->v0());
  bool future_state = n_(OUTNODE)->lv_future();
  if(future_state == oldstate) {
  }else if(future_state){
    n_(OUTNODE)->set_event(1e-20, lvSTABLE1);
    n_(OUTNODE)->set_quality(qGOOD);
  }else{
    n_(OUTNODE)->set_event(1e-20, lvSTABLE0);
    n_(OUTNODE)->set_quality(qGOOD);
  }
}
/*--------------------------------------------------------------------------*/
int DEV_A_D::_count = -1;
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
DEV_A_D d_buf(&c_buf);
DISPATCHER<CARD>::INSTALL dd_buf(&device_dispatcher, "c_a_d", &d_buf);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
