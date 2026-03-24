/*$Id: d_logic.cc  $ -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2026 Felix Salfelder
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
//testing=script,sparse 2023.11.22
#include "e_logicmod.h"
#include "globals.h"
#include "e_subckt.h"
#include "u_xprobe.h"
#include "e_logic.h"
#include "e_elemnt.h"
#include "e_logicnode.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
class DEV_LOGIC : public ELEMENT {
public:
  enum {OUTNODE=0,BEGIN_IN=1}; //node labels
  enum {PORTS_PER_GATE = 10};
private:
  int		_lastchangenode;
  int		_quality;
  std::string	_failuremode;
  smode_t	_oldgatemode;
  smode_t	_gatemode;
  static int	_count;
  mutable node_l _nodes[PORTS_PER_GATE];	/* PORTS_PER_GATE <= PORTSPERSUBCKT */
public:
  explicit	DEV_LOGIC(COMMON_COMPONENT* c=nullptr);
  explicit	DEV_LOGIC(const DEV_LOGIC& p);
		~DEV_LOGIC()		{--_count;}
private: // override virtuals
  char	   id_letter()const override	{return 'U';}
  std::string value_name()const override{return "";}
  bool	      print_type_in_spice()const override{return true;}
  std::string dev_type()const override{assert(has_common()); return common()->name();}
  int	   tail_size()const override {return 2;}
  int	   max_nodes()const override {return PORTS_PER_GATE;}
  int	   min_nodes()const override {return BEGIN_IN+1;}
  int	   matrix_nodes()const override	{ untested();return 2;}
  int	   net_nodes()const override {return _net_nodes;}
  CARD*	   clone()const override {return new DEV_LOGIC(*this);}
  void	   precalc_first()override {ELEMENT::precalc_first(); if (subckt()) {subckt()->precalc_first();}}
  void	   expand()override;
  void	   precalc_last() override;
  //void   map_nodes();

  void	   tr_iwant_matrix()override;
  void	   tr_begin()override;
  void	   tr_restore()override;
  void	   dc_advance()override;
  void	   tr_advance()override;
  void	   tr_regress()override;
  bool	   tr_needs_eval()const override;
  void	   tr_queue_eval()override;
  bool	   do_tr()override;
  void	   tr_load()override;
  void	   tr_unload()override;
  TIME_PAIR tr_review()override;
  void	   tr_accept()override;
  double   tr_involts()const override	{ untested();unreachable(); return 0;}
  //double tr_input()const		//ELEMENT
  double   tr_involts_limited()const override { untested();unreachable(); return 0;}
  //double tr_input_limited()const	//ELEMENT
  //double tr_amps()const		//ELEMENT
  double   tr_probe_num(const std::string&)const override;

  void	   ac_iwant_matrix()override;
  void	   ac_begin()override;
  void	   do_ac()override	{untested();  assert(subckt());  subckt()->do_ac();}
  void	   ac_load()override	{untested();  assert(subckt());  subckt()->ac_load();}
  COMPLEX  ac_involts()const override	{ untested();unreachable(); return 0.;}
  COMPLEX  ac_amps()const override	{ untested();unreachable(); return 0.;}
  XPROBE   ac_probe_ext(const std::string&)const override;

  node_l& n_(int i)const override {
    assert(_nodes); assert(i>=0); assert(i<PORTS_PER_GATE); return _nodes[i];
  }
  std::string port_name(int i)const override {
    assert(i >= 0);
    assert(i < PORTS_PER_GATE);
    const COMMON_LOGIC* c = dynamic_cast<const COMMON_LOGIC*>(common());
    assert(c);
    return c->port_name(i);
    //static std::string names[PORTS_PER_GATE] = {"out",
    //    "in1", "in2", "in3", "in4", "in5", "in6", "in7", "in8", "in9"};
    //return names[i];
  }
public:
  static int count()			{untested();return _count;}
private:
  bool	   tr_eval_digital();
  bool	   want_analog()const;
  bool	   want_digital()const;
public:
  void set_parameters(const std::string& Label, CARD* Parent,
		      COMMON_COMPONENT* Common, double Value,
		      int state_count, double state[],
		      int node_count, const node_t nodes[])override;
};
/*--------------------------------------------------------------------------*/
DEV_LOGIC::DEV_LOGIC(COMMON_COMPONENT* c)
  :ELEMENT(c),
   _lastchangenode(0),
   _quality(qGOOD),
   _failuremode("ok"),
   _oldgatemode(moUNKNOWN),
   _gatemode(moUNKNOWN)   
{
  ++_count;
}
/*--------------------------------------------------------------------------*/
DEV_LOGIC::DEV_LOGIC(const DEV_LOGIC& p)
  :ELEMENT(p),
   _lastchangenode(0),
   _quality(qGOOD),
   _failuremode("ok"),
   _oldgatemode(moUNKNOWN),
   _gatemode(moUNKNOWN)   
{
  assert(max_nodes() == PORTS_PER_GATE);
  for (int ii = 0;  ii < max_nodes();  ++ii) {
    _nodes[ii] = p._nodes[ii];
  }
  ++_count;
}
/*--------------------------------------------------------------------------*/
/* set: set parameters, used in model building
 */
void DEV_LOGIC::set_parameters(const std::string& Label, CARD *Owner,
			       COMMON_COMPONENT *Common, double Value,
			       int , double [],
			       int node_count, const node_t Nodes[])
{
  set_label(Label);
  set_owner(Owner);
  if(Value==0.){
  }else if(Common){
    Common = Common->clone();
    Common->set_param_by_name("delay", to_string(Value));
  }else{ untested();
    incomplete();
  }
  attach_common(Common);

  assert(node_count <= max_nodes());
  _net_nodes = short(node_count);
  if(node_count){
    std::copy_n(Nodes, node_count, (node_t*)&n_(0));
  }else{itested();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::precalc_last()
{
  ELEMENT::precalc_last();
  if (subckt()) { untested();
    subckt()->precalc_last();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::expand()
{
  ELEMENT::expand();
  attach_model();

  const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);

  const MODEL_LOGIC* m = dynamic_cast<const MODEL_LOGIC*>(c->model());
  if (!m) { untested();
    throw Exception_Model_Type_Mismatch(long_label(), c->modelname(), "logic family (LOGIC)");
  }else{
  }

  std::string subckt_name(c->modelname()+c->name()+to_string(net_nodes()-BEGIN_IN));
  try {
    const CARD* model = find_looking_out(subckt_name);
    
    if(!dynamic_cast<const BASE_SUBCKT*>(model)) {untested();
      error(((!_sim->is_first_expand()) ? (bDEBUG) : (bWARNING)),
	    long_label() + ": " + subckt_name + " is not a subckt, forcing digital\n");
    }else{ untested();
      _gatemode = OPT::mode;    
      renew_subckt(model, nullptr/*&(c->_params)*/);    
      subckt()->expand();
    }
  }catch (Exception_Cant_Find&) {
    error(((!_sim->is_first_expand()) ? (bDEBUG) : (bWARNING)), 
	  long_label() + ": can't find subckt: " + subckt_name + ", forcing digital\n");
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::tr_iwant_matrix()
{
  if (subckt()) { untested();
    subckt()->tr_iwant_matrix();
  }else{
  }
  ////////tr_iwant_matrix_passive();
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::tr_begin()
{
  ELEMENT::tr_begin();
  if (!subckt()) {
    _gatemode = moDIGITAL;
    n_(OUTNODE)->set_mode(_gatemode);
    _oldgatemode = _gatemode;
  }else{ untested();
    _gatemode = (OPT::mode==moMIXED) ? moANALOG : OPT::mode;
    n_(OUTNODE)->set_mode(_gatemode);
    _oldgatemode = _gatemode;
    subckt()->tr_begin();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::tr_restore()
{untested();
  ELEMENT::tr_restore();
  if (!subckt()) {untested();
    _gatemode = moDIGITAL;
  }else{untested();
    _gatemode = (OPT::mode==moMIXED) ? moANALOG : OPT::mode;
    subckt()->tr_restore();
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::dc_advance()
{
  ELEMENT::dc_advance();

  if (_gatemode != _oldgatemode) {untested();
    tr_unload();
    n_(OUTNODE)->set_mode(_gatemode);
    _oldgatemode = _gatemode;
  }else{
  }
  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:   unreachable(); break;
  case moANALOG:untested();
    assert(subckt());
    subckt()->dc_advance();
    break;
  case moDIGITAL:
    if (n_(OUTNODE)->in_transit()) {
      //q_eval(); evalq is not used for DC
      n_(OUTNODE)->propagate();
    }else{
    }
    break;
  }
}
/*--------------------------------------------------------------------------*/
/* tr_advance: the first to run on a new time step.
 * It sets up preconditions for the new time.
 */
void DEV_LOGIC::tr_advance()
{
  ELEMENT::tr_advance();

  if (_gatemode != _oldgatemode) { untested();
    tr_unload();
    n_(OUTNODE)->set_mode(_gatemode);
    _oldgatemode = _gatemode;
  }else{
  }
  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:   unreachable(); break;
  case moANALOG:untested();
    assert(subckt());
    subckt()->tr_advance();
    break;
  case moDIGITAL: 
    if (n_(OUTNODE)->in_transit()) {
      q_eval();
      if (_sim->_time0 >= n_(OUTNODE)->final_time()) {
	n_(OUTNODE)->propagate();
      }else{
	// not ready to propagate.
      }
    }else{
    }
    break;
  }
}
void DEV_LOGIC::tr_regress()
{
  ELEMENT::tr_regress();

  if (_gatemode != _oldgatemode) {untested();
    tr_unload();
    n_(OUTNODE)->set_mode(_gatemode);
    _oldgatemode = _gatemode;
  }else{
  }
  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:   unreachable(); break;
  case moANALOG:  untested();
    assert(subckt());
    subckt()->tr_regress();
    break;
  case moDIGITAL:
    q_eval();
    if (n_(OUTNODE)->last_change_time() > _sim->_time0) {
      n_(OUTNODE)->unpropagate();
      assert(_sim->_time0 < n_(OUTNODE)->final_time());
    }else if (_sim->_time0 >= n_(OUTNODE)->final_time()) {untested();
      n_(OUTNODE)->propagate();
    }else{
    }
    break;
  }
}
/*--------------------------------------------------------------------------*/
/* tr_needs_eval
 * in digital mode ... DC always returns true, to queue it.
 * tran always returns false, already queued by tr_advance if needed
 */
bool DEV_LOGIC::tr_needs_eval()const
{
  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:   unreachable(); break;
  case moDIGITAL:
    //assert(!is_q_for_eval());
    if (_sim->analysis_is_restore()) {untested();
    }else if (_sim->analysis_is_static()) {
    }else{
    }
    return (_sim->analysis_is_static() || _sim->analysis_is_restore());
  case moANALOG:untested();
    
    assert(!is_q_for_eval());
    assert(subckt());
    return subckt()->tr_needs_eval();
  }
  unreachable();
  return false;
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::tr_queue_eval()
{
  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:	  unreachable(); break;
  case moDIGITAL: ELEMENT::tr_queue_eval(); break;
  case moANALOG:  assert(subckt()); subckt()->tr_queue_eval(); break;
  }
}
/*--------------------------------------------------------------------------*/
bool DEV_LOGIC::tr_eval_digital()
{
  assert(_gatemode == moDIGITAL);
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
  _y[0].f1 = n_(OUTNODE)->to_analog(m);
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
bool DEV_LOGIC::do_tr()
{  
  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:   unreachable(); break;
  case moDIGITAL: set_converged(tr_eval_digital()); break;
  case moANALOG:  assert(subckt()); set_converged(subckt()->do_tr()); break;
  }
  return converged();
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::tr_load()
{
  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:   unreachable(); break;
  case moDIGITAL:
    tr_load_diagonal_point(n_(OUTNODE), &_m0.c1, &_m1.c1);
    tr_load_source_point(n_(OUTNODE), &_m0.c0, &_m1.c0);
    break;
  case moANALOG:  assert(subckt()); subckt()->tr_load(); break;
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::tr_unload()
{ untested();
  if (subckt()) { untested();
    switch (_gatemode) {
    case moUNKNOWN: unreachable(); break;
    case moMIXED:   unreachable(); break;
    case moDIGITAL:		   break;
    case moANALOG:  untested();    break;
    }
    subckt()->tr_unload();
    assert(_m0.c0 == 0.);
    assert(_m0.c1 == 0.);
  }else{untested();
    switch (_gatemode) {
    case moUNKNOWN: unreachable(); break;
    case moMIXED:   unreachable(); break;
    case moDIGITAL: untested(); break;
    case moANALOG:  untested(); break;
    }
    _m0.c0 = _m0.c1 = 0.;
    _sim->mark_inc_mode_bad();
    tr_load();
  }
}
/*--------------------------------------------------------------------------*/
TIME_PAIR DEV_LOGIC::tr_review()
{
  // not calling ELEMENT::tr_review();

  q_accept();
  //digital mode queues events explicitly in tr_accept

  switch (_gatemode) {
  case moUNKNOWN: unreachable(); break;
  case moMIXED:   unreachable(); break;
  case moDIGITAL: _time_by.reset(); break;
  case moANALOG:  assert(subckt()); _time_by = subckt()->tr_review(); break;
  }
  return _time_by;
}
/*--------------------------------------------------------------------------*/
/* tr_accept: This runs after everything has passed "review".
 * It sets up and queues transitions, and sometimes determines logic states.
 */
void DEV_LOGIC::tr_accept()
{
  assert(_gatemode == moDIGITAL || _gatemode == moANALOG);
  const COMMON_LOGIC* c = prechecked_cast<const COMMON_LOGIC*>(common());
  assert(c);
  const MODEL_LOGIC* m = prechecked_cast<const MODEL_LOGIC*>(c->model());
  assert(m);
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
  /* Check quality and get node info to local array. */
  /* side effect --- generate digital values for analog nodes */
  assert(PORTS_PER_GATE == max_nodes());
  {
    n_(OUTNODE)->to_logic(m);
    _quality = n_(OUTNODE)->quality();  /* the worst quality on this device */
    _failuremode = n_(OUTNODE)->failure_mode();    /* what is wrong with it? */
    _lastchangenode = OUTNODE;		/* which node changed most recently */
    int lastchangeiter=n_(OUTNODE)->d_iter();/* iteration # when it changed */
    trace0(long_label().c_str());
    trace2(n_(OUTNODE)->failure_mode().c_str(), OUTNODE, n_(OUTNODE)->quality());
    
    for (int ii = BEGIN_IN;  ii < net_nodes();  ++ii) {
      n_(ii)->to_logic(m);
      if (n_(ii)->quality() < _quality) { untested();
	_quality = n_(ii)->quality();
	_failuremode = n_(ii)->failure_mode();
      }else{
      }
      if (n_(ii)->d_iter() >= lastchangeiter) {
	lastchangeiter = n_(ii)->d_iter();
	_lastchangenode = ii;
      }else{ untested();
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
  if (want_analog()) { untested();
    if (_gatemode == moDIGITAL) {untested();
      error(bTRACE, "%s:%u:%g switch to analog, %s\n", long_label().c_str(),
	    _sim->iteration_tag(), _sim->_time0, _failuremode.c_str());
      _oldgatemode = _gatemode;
      _gatemode = moANALOG;
    }else{ untested();
    }
    assert(_gatemode == moANALOG);
  }else{
    assert(want_digital());
    if (_gatemode == moANALOG) { untested();
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
    }else{ untested();
    }
    n_(OUTNODE)->store_old_last_change_time();
    n_(OUTNODE)->store_old_lv(); // needed? yes
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::ac_iwant_matrix()
{
  if (subckt()) { untested();
    subckt()->ac_iwant_matrix();
  }else{
  }
}
/*--------------------------------------------------------------------------*/
void DEV_LOGIC::ac_begin()
{untested();
  if (subckt()) {untested();
    subckt()->ac_begin();
  }else{untested();
    error(bWARNING, long_label() + ": no logic in AC analysis\n");
  }
}
/*--------------------------------------------------------------------------*/
double DEV_LOGIC::tr_probe_num(const std::string& what)const
{ untested();
  return n_(OUTNODE)->tr_probe_num(what);
}
/*--------------------------------------------------------------------------*/
XPROBE DEV_LOGIC::ac_probe_ext(const std::string& what)const
{untested();
  return n_(OUTNODE)->ac_probe_ext(what);
}
/*--------------------------------------------------------------------------*/
bool DEV_LOGIC::want_analog()const
{
  return subckt() &&
    ((OPT::mode == moANALOG) || (OPT::mode == moMIXED && _quality != qGOOD));
}
/*--------------------------------------------------------------------------*/
bool DEV_LOGIC::want_digital()const
{
  return !subckt() ||
    ((OPT::mode == moDIGITAL) || (OPT::mode == moMIXED && _quality == qGOOD));
}
/*--------------------------------------------------------------------------*/
int DEV_LOGIC::_count = -1;
/*--------------------------------------------------------------------------*/
static DEV_LOGIC p1(nullptr);
static DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "__assign", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
