/*                               -*- C++ -*-
 * Copyright (C) 2001 Albert Davis
 *               2024 Felix Salfelder
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
 * Transmission line, RLGC and length. baseline model WIP: tran
 */
#include "globals.h"
#include "m_wave.h"
#include "e_elemnt.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
enum {NUM_INIT_COND = 4};
/*--------------------------------------------------------------------------*/
class COMMON_TRANSLINE : public COMMON_COMPONENT {
public:
  PARAMETER<double> len;	/* length multiplier */
  PARAMETER<double> R;
  PARAMETER<double> L;
  PARAMETER<double> G;
  PARAMETER<double> C;
  double   ic[NUM_INIT_COND];	/* initial conditions: v1, i1, v2, i2 */
  int	   icset;		/* flag: initial condition set */
public:
  double real_z0;
  double real_td;
  double real_loss;
private:
  explicit COMMON_TRANSLINE(const COMMON_TRANSLINE& p);
public:
  explicit COMMON_TRANSLINE(int c=0);
  bool		operator==(const COMMON_COMPONENT&)const override;
  COMMON_COMPONENT* clone()const override {return new COMMON_TRANSLINE(*this);}

  void		set_param_by_index(int, std::string&, int)override;
  bool		param_is_printable(int)const override;
  std::string	param_name(int)const override;
  std::string	param_name(int,int)const override;
  std::string	param_value(int)const override;
  int param_count()const override {return (5 + COMMON_COMPONENT::param_count());}
public:
  void		precalc_last(const CARD_LIST*)override;
  std::string	name()const override		{untested(); return "transline";}
};
/*--------------------------------------------------------------------------*/
class DEV_TRANSLINE : public ELEMENT {
private:
  WAVE	 _forward;
  WAVE	 _reflect;
  double _if0; // value of current source representing incident wave
  double _ir0; // value of current source representing reflected wave
  double _if1; // val of cs rep incident wave, one load ago
  double _ir1; // val of cs rep reflected wave, one load ago
  COMPLEX _y11;// AC equiv ckt
  COMPLEX _y12;// AC equiv ckt
private:
  explicit	DEV_TRANSLINE(const DEV_TRANSLINE& p)
    :ELEMENT(p), _forward(), _reflect(), _if0(0), _ir0(0), _if1(0), _ir1(0) {}
public:
  explicit	DEV_TRANSLINE(COMMON_COMPONENT* c=NULL);
private: // override virtual
  char		id_letter()const override	{return 'T';}
  std::string   value_name()const override	{return "#";}
  std::string	dev_type()const override	{itested(); return "tline";}
  int		max_nodes()const override	{return 4;}
  int		min_nodes()const override	{return 4;}
  int		matrix_nodes()const override	{return 4;}
  int		net_nodes()const override	{return 4;}
  CARD*		clone()const override		{return new DEV_TRANSLINE(*this);}
  void		precalc_last()override;
  void		tr_iwant_matrix()override;
  void		tr_begin()override;
  void		dc_advance()override;
  void		tr_advance()override;
  void		tr_regress()override;
  bool		tr_needs_eval()const override;
  bool		do_tr()override;
  void		tr_load()override;
  TIME_PAIR 	tr_review()override;
  void		tr_accept()override;
  void		tr_unload()override;
  double	tr_involts()const override;
  double	tr_involts_limited()const override;
  void		ac_iwant_matrix()override	{ac_iwant_matrix_extended();}
  void		do_ac()override;
  void		ac_load()override;
  COMPLEX	ac_involts()const override;

  std::string port_name(int i)const override {itested();
    assert(i >= 0);
    assert(i < 4);
    static std::string names[] = {"t1", "b1", "t2", "b2"};
    return names[i];
  }
private:
  void		setinitcond(CS&);
};
/*--------------------------------------------------------------------------*/
inline bool DEV_TRANSLINE::tr_needs_eval()const
{ untested();
  assert(!is_q_for_eval());
  return (_if0!=_if1 || _ir0!=_ir1);
}
/*--------------------------------------------------------------------------*/
inline double DEV_TRANSLINE::tr_involts()const
{ untested();
  return dn_diff(_n[IN1].v0(), _n[IN2].v0());
}
/*--------------------------------------------------------------------------*/
inline double DEV_TRANSLINE::tr_involts_limited()const
{ untested();
  unreachable();
  return volts_limited(_n[IN1],_n[IN2]);
}
/*--------------------------------------------------------------------------*/
inline COMPLEX DEV_TRANSLINE::ac_involts()const
{untested();
  return _n[IN1]->vac() - _n[IN2]->vac();
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
const double _default_len (1);
const double _default_R   (0);
const double _default_L   (NOT_INPUT);
const double _default_G   (0);
const double _default_C   (NOT_INPUT);
const double LINLENTOL = .000001;
/*--------------------------------------------------------------------------*/
COMMON_TRANSLINE::COMMON_TRANSLINE(int c)
  :COMMON_COMPONENT(c),
   len(_default_len),
   R(_default_R),
   L(_default_L),
   G(_default_G),
   C(_default_C),
   icset(false)
{ untested();
  for (int i = 0;  i < NUM_INIT_COND;  ++i) { untested();
    ic[i] = 0.;
  }
}
/*--------------------------------------------------------------------------*/
COMMON_TRANSLINE::COMMON_TRANSLINE(const COMMON_TRANSLINE& p)
  :COMMON_COMPONENT(p),
   len(p.len),
   R(p.R),
   L(p.L),
   G(p.G),
   C(p.C),
   icset(p.icset)
{ untested();
  for (int i = 0;  i < NUM_INIT_COND;  ++i) { untested();
    ic[i] = p.ic[i];
  }
}
/*--------------------------------------------------------------------------*/
bool COMMON_TRANSLINE::operator==(const COMMON_COMPONENT& x)const
{ untested();
  const COMMON_TRANSLINE* p = dynamic_cast<const COMMON_TRANSLINE*>(&x);
  bool rv = p
    && len == p->len
    && R == p->R
    && L == p->L
    && G == p->G
    && C == p->C
    && icset == p->icset
    && COMMON_COMPONENT::operator==(x);
  if (rv) { untested();
    for (int i=0; i<NUM_INIT_COND; ++i) { untested();
      rv &= ic[i] == p->ic[i];
    }
  }else{ untested();
  }
  return rv;
}
/*--------------------------------------------------------------------------*/
void COMMON_TRANSLINE::set_param_by_index(int I, std::string& Value, int Offset)
{ untested();
  trace2("spbn", I, Value);
  switch (COMMON_TRANSLINE::param_count() - 1 - I) {
  case 0:  len = Value; break;
  case 1:  R = Value; break;
  case 2:  L = Value; break;
  case 3:  G = Value; break;
  case 4:  C = Value; break;
  default: COMMON_COMPONENT::set_param_by_index(I, Value, Offset); break;
  }
  //BUG// does not print IC
}
/*--------------------------------------------------------------------------*/
bool COMMON_TRANSLINE::param_is_printable(int I)const
{ untested();
  switch (COMMON_TRANSLINE::param_count() - 1 - I) {
  case 0:  return len.has_hard_value();
  case 1:  return R.has_hard_value();
  case 2:  return L.has_hard_value();
  case 3:  return G.has_hard_value();
  case 4:  return C.has_hard_value();
  default: return COMMON_COMPONENT::param_is_printable(I);
  }
  //BUG// does not print IC
#if 0
  if (icset) {untested();
    o << " IC=";
    for (int i = 0;  i < NUM_INIT_COND;  ++i) {untested();
      o << ic[i] << ' ';
    }
  }else{ untested();
  }
#endif
}
/*--------------------------------------------------------------------------*/
std::string COMMON_TRANSLINE::param_name(int I)const
{ untested();
  switch (COMMON_TRANSLINE::param_count() - 1 - I) {
  case 0:  return "len";
  case 1:  return "r";
  case 2:  return "l";
  case 3:  return "g";
  case 4:  return "c";
  default: return COMMON_COMPONENT::param_name(I);
  }
  //BUG// does not print IC
}
/*--------------------------------------------------------------------------*/
std::string COMMON_TRANSLINE::param_name(int I, int j)const
{ untested();
  if (j == 0) { untested();
    return param_name(I);
  }else if (I >= COMMON_COMPONENT::param_count()) { untested();
    switch (COMMON_TRANSLINE::param_count() - 1 - I) {
    default: return "";
    }
  }else{ untested();
    return COMMON_COMPONENT::param_name(I, j);
  }
  //BUG// does not print IC
}
/*--------------------------------------------------------------------------*/
std::string COMMON_TRANSLINE::param_value(int I)const
{ untested();
  switch (COMMON_TRANSLINE::param_count() - 1 - I) {
  case 0:  return len.string();
  case 1:  return R.string();
  case 2:  return L.string();
  case 3:  return G.string();
  case 4:  return C.string();
  default: return COMMON_COMPONENT::param_value(I);
  }
  //BUG// does not print IC
}
/*--------------------------------------------------------------------------*/
void COMMON_TRANSLINE::precalc_last(const CARD_LIST* Scope)
{ untested();
  assert(Scope);
  COMMON_COMPONENT::precalc_last(Scope);

  len.e_val(_default_len, Scope);
  R.e_val(_default_R, Scope);
  L.e_val(_default_L, Scope);
  G.e_val(_default_G, Scope);
  C.e_val(_default_C, Scope);

  // incomplete. how about transient?
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
DEV_TRANSLINE::DEV_TRANSLINE(COMMON_COMPONENT* c)
  :ELEMENT(c),
   _forward(), _reflect(),
   _if0(0), _ir0(0), _if1(0), _ir1(0),
   _y11(), _y12()
{ untested();
}
/*--------------------------------------------------------------------------*/
void DEV_TRANSLINE::precalc_last()
{ untested();
  ELEMENT::precalc_last();
  const COMMON_TRANSLINE* c=prechecked_cast<const COMMON_TRANSLINE*>(common());
  assert(c);
  _forward.set_delay(c->real_td);
  _reflect.set_delay(c->real_td);
  set_converged();
  assert(!is_constant());
}
/*--------------------------------------------------------------------------*/
void DEV_TRANSLINE::tr_iwant_matrix()
{ untested();
  _sim->_aa.iwant(_n[OUT1].m_(),_n[OUT2].m_());
  _sim->_aa.iwant(_n[IN1].m_(), _n[IN2].m_());
  _sim->_lu.iwant(_n[OUT1].m_(),_n[OUT2].m_());
  _sim->_lu.iwant(_n[IN1].m_(), _n[IN2].m_());
}
/*--------------------------------------------------------------------------*/
/* first setup, initial dc, empty the lines
 */
void DEV_TRANSLINE::tr_begin()
{ untested();
  ELEMENT::tr_begin();
  const COMMON_TRANSLINE* c=prechecked_cast<const COMMON_TRANSLINE*>(common());
  assert(c);
  _forward.initialize().push(0.-c->real_td, 0.).push(0., 0.);
  _reflect.initialize().push(0.-c->real_td, 0.).push(0., 0.);
}
/*--------------------------------------------------------------------------*/
/* before anything else .. see what is coming out
 * _if0 = output current ..
 * The "wave" class stores voltages, but we need currents,
 * because the simulator uses the Norton equivalent circuit.
 * This makes the Thevenin to Norton conversion.
 */
void DEV_TRANSLINE::dc_advance()
{ untested();
  ELEMENT::dc_advance();
  const COMMON_TRANSLINE* c=prechecked_cast<const COMMON_TRANSLINE*>(common());
  assert(c);
  _if0 = 0.; incomplete(); // _forward.v_out(_sim->_time0).f0/c->real_z0;
  _ir0 = 0.;  // _reflect.v_out(_sim->_time0).f0/c->real_z0;
}
void DEV_TRANSLINE::tr_advance()
{ untested();
  ELEMENT::tr_advance();
  const COMMON_TRANSLINE* c=prechecked_cast<const COMMON_TRANSLINE*>(common());
  assert(c);
  _if0 = 0.; incomplete(); // _forward.v_out(_sim->_time0).f0/c->real_z0;
  _ir0 = 0.;  // _reflect.v_out(_sim->_time0).f0/c->real_z0;
}
void DEV_TRANSLINE::tr_regress()
{ untested();
  ELEMENT::tr_regress();
  const COMMON_TRANSLINE* c=prechecked_cast<const COMMON_TRANSLINE*>(common());
  assert(c);
  _if0 = 0.; incomplete(); // _forward.v_out(_sim->_time0).f0/c->real_z0;
  _ir0 = 0.;  // _reflect.v_out(_sim->_time0).f0/c->real_z0;
}
/*--------------------------------------------------------------------------*/
/* usually nothing, always converged.  It is all done in advance and accept.
 * UNLESS ... it is a very short line .. then we fake it here.
 * very short line means delay is less than internal time step.
 */
bool DEV_TRANSLINE::do_tr()
{ untested();
  incomplete(); // won't work.
  // code to deal with short lines goes here.
  //if (_if0 != _if1  ||  _ir0 != _ir1) { untested();
  if (!conchk(_if0, _if1, OPT::abstol, OPT::reltol*.01)
       || !conchk(_ir0, _ir1, OPT::abstol, OPT::reltol*.01)) { untested();
    q_load();
  }else{ untested();
  }
  assert(converged());
  return true;
}
/*--------------------------------------------------------------------------*/
void DEV_TRANSLINE::tr_load()
{ untested();
  incomplete(); // won't work.
		return;
  //BUG// explicit mfactor
  double lvf = NOT_VALID; // load value, forward
  double lvr = NOT_VALID; // load value, reflected
  if (!_sim->is_inc_mode()) { untested();
    const COMMON_TRANSLINE* c = prechecked_cast<const COMMON_TRANSLINE*>(common());
    assert(c);
    _sim->_aa.load_symmetric(_n[OUT1].m_(), _n[OUT2].m_(), mfactor()/c->real_z0);
    _sim->_aa.load_symmetric(_n[IN1].m_(),  _n[IN2].m_(),  mfactor()/c->real_z0);
    lvf = _if0;
    lvr = _ir0;
  }else{ untested();
    lvf = dn_diff(_if0, _if1);
    lvr = dn_diff(_ir0, _ir1);
  }
  if (lvf != 0.) { untested();
    if (_n[OUT1].m_() != 0) { untested();
      _n[OUT1].i() += mfactor() * lvf;
    }else{untested();
    }
    if (_n[OUT2].m_() != 0) {untested();
      _n[OUT2].i() -= mfactor() * lvf;
    }else{ untested();
    }
  }else{ untested();
  }
  if (lvr != 0.) { untested();
    if (_n[IN1].m_() != 0) { untested();
      _n[IN1].i() += mfactor() * lvr;
    }else{untested();
    }
    if (_n[IN2].m_() != 0) {untested();
      _n[IN2].i() -= mfactor() * lvr;
    }else{ untested();
    }
  }else{ untested();
  }
  _if1 = _if0;
  _ir1 = _ir0;
}
/*--------------------------------------------------------------------------*/
/* limit the time step to no larger than a line length.
 */
TIME_PAIR DEV_TRANSLINE::tr_review()
{ untested();
  q_accept();
  const COMMON_TRANSLINE* c=prechecked_cast<const COMMON_TRANSLINE*>(common());
  assert(c);
  return TIME_PAIR(_sim->_time0 + c->real_td, NEVER); // ok to miss the spikes, for now
}
/*--------------------------------------------------------------------------*/
/* after this step is all done, determine the reflections and send them on.
 */
void DEV_TRANSLINE::tr_accept()
{ untested();
  trace1(short_label().c_str(), _sim->_time0);
  // incomplete
//  _reflect.push(_sim->_time0, _forward.v_reflect(_sim->_time0, tr_outvolts()));
//  _forward.push(_sim->_time0, _reflect.v_reflect(_sim->_time0, tr_involts()));
}
/*--------------------------------------------------------------------------*/
void DEV_TRANSLINE::tr_unload()
{untested();
}
/*--------------------------------------------------------------------------*/
void DEV_TRANSLINE::do_ac()
{ untested();
  const COMMON_TRANSLINE*c=prechecked_cast<const COMMON_TRANSLINE*>(common());
  assert(c);
#if 0
  double lenth = _sim->_freq * c->real_td * 4;  /* length in quarter waves */

  double dif = lenth - floor(lenth+.5);	/* avoid divide by zero if close to */
  if (std::abs(dif) < LINLENTOL) {	/* resonance by tweeking a little */
    error(bDEBUG,
	  long_label() + ": transmission line too close to resonance\n");
    lenth = (dif<0.) ? floor(lenth+.5)-LINLENTOL : floor(lenth+.5)+LINLENTOL;
  }else{ untested();
  }
#endif
  /* starting point, from
  https://eng.libretexts.org/Bookshelves/Electrical_Engineering/Electronics/Microwave_and_RF_Design_II_-_Transmission_Lines_(Steer)/02%3A_Transmission_Lines/2.02%3A_Transmission_Line_Theory
  */
  double R = c->R;
  double G = c->G;
  double C = c->C;
  double L = c->L;
  double len = c->len;

  COMPLEX jwL = _sim->_jomega * L;
  COMPLEX jwC = _sim->_jomega * C;
  COMPLEX gamma = std::sqrt((R+jwL)*(G+jwC)); // ~ jw sqrt(L/C)?
  COMPLEX z0 = (R+jwL) / gamma;

  trace4("trln:ac", _sim->_jomega/_sim->_freq, z0, gamma, len);

  // lenth = _sim->_freq * len * sqrt(L*C) * 4 * M_PI_2
  // COMPLEX arg(loss, lenth); // == gamma*l == (alpha + i beta) * l

  _y12 = 1. / (z0 * sinh(gamma*len));
  _y11 = 1. / (z0 * tanh(gamma*len));
  trace3("trln:ac", _y12, _y11, tanh(gamma*len));
}
/*--------------------------------------------------------------------------*/
void DEV_TRANSLINE::ac_load()
{ untested();
  //BUG// explicit mfactor
  _sim->_acx.load_symmetric(_n[OUT1].m_(), _n[OUT2].m_(), mfactor()*_y11);
  _sim->_acx.load_symmetric(_n[IN1].m_(), _n[IN2].m_(),  mfactor()*_y11);
  _sim->_acx.load_asymmetric(_n[OUT1].m_(),_n[OUT2].m_(), _n[IN2].m_(), _n[IN1].m_(),
			     mfactor()*_y12);
  _sim->_acx.load_asymmetric(_n[IN1].m_(), _n[IN2].m_(), _n[OUT2].m_(), _n[OUT1].m_(),
			     mfactor()*_y12);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
COMMON_TRANSLINE Default_TRANSLINE(CC_STATIC);
DEV_TRANSLINE p1(&Default_TRANSLINE);
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "ltra", &p1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
