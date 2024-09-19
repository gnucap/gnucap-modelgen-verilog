/*                          -*- C++ -*-
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
 * "wave" class, for transmission lines and delays
 */
#ifndef M_WAVE_H
#define M_WAVE_H
#include <l_denoise.h>
#include <m_interp.h>
/*--------------------------------------------------------------------------*/
class WAVE {
private:
  std::deque<DPAIR> _w;
  double _delay;
public:
  typedef std::deque<DPAIR>::iterator iterator;
  typedef std::deque<DPAIR>::const_iterator const_iterator;

  explicit WAVE(double d=0);
  explicit WAVE(const WAVE&);
	  ~WAVE() {}
  WAVE&	   set_delay(double d);
  WAVE&	   initialize();
  WAVE&	   push(double t, double v);
  FPOLY1   v_out(double t)const;
  double   v_reflect(double t, double v_total)const;
  WAVE&	   operator+=(const WAVE& x);
  WAVE&	   operator+=(double x);
  WAVE&	   operator*=(const WAVE& x);
  WAVE&	   operator*=(double x);
  const_iterator begin()const { untested();return _w.begin();}
  const_iterator end()const { untested();return _w.end();}
  size_t size()const {return _w.size();}

  double last_time()const {
    if(_w.size()){
      return _w.back().first;
    }else{ untested();
      return NEVER;
    }
  }
  double time()const {
    if(_w.size()){
      return _w.begin()->first;
    }else{ untested();
      return NEVER;
    }
  }
  double next_time()const { untested();
    if(_w.size()>1){ untested();
      return (_w.begin()+1)->first;
    }else{ untested();
      return NEVER;
    }
  }
  void new_transition(double when, double rt, double ft, double fv);
  double cleanup(double until);
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// push: insert a signal on the "input" end.
// args: t = the time now
//       v = the value to push
//
inline WAVE& WAVE::push(double t, double v)
{
  _w.push_back(DPAIR(t+_delay, v));
  return *this;
}
/*--------------------------------------------------------------------------*/
// initialize: remove all info, fill it with all 0.
//
inline WAVE& WAVE::initialize()
{
  _w.clear();
  return *this;
}
/*--------------------------------------------------------------------------*/
inline WAVE::WAVE(const WAVE& w)
  :_w(w._w),
   _delay(w._delay)
{ untested();
}
/*--------------------------------------------------------------------------*/
// constructor -- argument is the delay
//
inline WAVE::WAVE(double d)
  :_w(),
   _delay(d)
{
  initialize();
}
/*--------------------------------------------------------------------------*/
inline WAVE& WAVE::set_delay(double d) 
{
  _delay = d; 
  return *this;
}
/*--------------------------------------------------------------------------*/
// v_out: return the value at the "output" end
// args: t = the time now
//
inline FPOLY1 WAVE::v_out(double t)const
{
  return interpolate(_w.begin(), _w.end(), t, 0., 0.);
}
/*--------------------------------------------------------------------------*/
// reflect: calculate a reflection
// args: t = the time now
//       v_total = actual voltage across the termination
// returns: the value (voltage) to send back as the reflection
//
inline double WAVE::v_reflect(double t, double v_total)const
{ untested();
  // return (v_total*2 - v_out(t)); // de-noised
  return dn_diff(v_total*2, v_out(t).f0);
}
/*--------------------------------------------------------------------------*/
inline WAVE& WAVE::operator+=(const WAVE& x)
{ untested();
  untested();
  for (std::deque<DPAIR>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    untested();
    (*i).second += x.v_out((*i).first).f0;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
inline WAVE& WAVE::operator+=(double x)
{ untested();
  untested();
  for (std::deque<DPAIR>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    untested();
    (*i).second += x;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
inline WAVE& WAVE::operator*=(const WAVE& x)
{ untested();
  untested();
  for (std::deque<DPAIR>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    untested();
    (*i).second *= x.v_out((*i).first).f0;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
inline WAVE& WAVE::operator*=(double x)
{ untested();
  untested();
  for (std::deque<DPAIR>::iterator
	 i = _w.begin(); i != _w.end(); ++i) { untested();
    untested();
    (*i).second *= x;
  }
  return *this;
}
/*--------------------------------------------------------------------------*/
inline double WAVE::cleanup(double start)
{
  auto b = _w.begin();
  double pending = NEVER; // b->first;

  while(_w.size() > 1 && _w.front().first <= start){
    trace3("cleanup front", _w.size(), _w.back().first, start);
    ++b;
    if(b->first == start){
      _w.pop_front();
    }else if(b->first <= start){
      _w.pop_front();
    }else{
      pending = b->first;
      break;
    }
    assert(_w.size());
  }
  for(auto w : _w){
    trace2("clean", w.first, w.second);
  }
  trace3("cleaned", _w.size(), pending, start);
  return pending;
}
/*--------------------------------------------------------------------------*/
template <typename T>
int sgn(T val)
{
  return (T(0) < val) - (val < T(0));
}
/*--------------------------------------------------------------------------*/
inline void WAVE::new_transition(double t, double rt, double ft, double new_fv)
{
  double start = t; // + _delay...
  trace5("nt0", start, rt, ft, new_fv, _w.size());
  for(auto w : _w){
    trace2("nt0", w.first, w.second);
  }
  assert(_w.size());
  DPAIR old_pair = DPAIR(NEVER, _w.back().second);
  while(start < _w.back().first){
    trace3("cleanup", _w.size(), _w.back().first, start);
    old_pair = _w.back();
    _w.pop_back();
  }

  double vcur;
  if (start == 0. && !_w.size()){ untested();
    _w.push_back(DPAIR(start, old_pair.second));
    vcur = old_pair.second;
  }else{
    DPAIR oo[2] = { _w.back(), old_pair };
    FPOLY1 v_int = interpolate(oo, oo+2, start, 0., 0.);
    vcur = v_int.f0;
    // old_slope = v_int.f1?
  }
  for(auto w : _w){
    trace2("nt pruned", w.first, w.second);
  }
  trace2("nt old", old_pair.first, old_pair.second);

  double old_origin = _w.back().second;
  double old_ft = old_pair.first;
  double old_fv = old_pair.second;


  double new_origin = old_fv;
  double new_slope = (new_fv - old_fv);
  double abs_slope = (new_fv - vcur);

  double new_tt = NEVER;
  if(new_slope > 0) {
     new_tt = rt;
  }else if(new_slope < 0) {
     new_tt = ft;
  }else{ untested();
  }

  new_slope /= new_tt;
  abs_slope /= new_tt;
  // double last_sample = _w.back().first;
  double final_time = old_pair.first;

  trace6("nt0", start, new_tt, old_fv, new_fv, new_origin, _w.size());
  if(new_slope == 0.){ untested();
    trace3("nt flat", start, old_ft, new_slope);
    incomplete();
  }else if(start > old_pair.first) { untested();
    // hold until start.
    trace3("nt new_transition hold1", start, _w.back().second, new_fv);
    assert(_w.back().first <= start);
    _w.push_back(DPAIR(start, _w.back().second));
  }else if(old_pair.first == NEVER) {
    // hold until start.
    // possibly move sample to start?
    trace3("nt new_transition hold2", start, _w.back().second, new_fv);
    assert(_w.back().first <= start);
    _w.push_back(DPAIR(start, _w.back().second));
  }else if(old_fv == _w.back().second) { untested();
    trace3("nt stable", start, old_ft, new_slope);
    // stable when new transition starts, nothing to do.
  }else if(_w.back().first < start && start < old_pair.first){
    // in transition @ start.
    double old_tstart = _w.back().first;
    double old_tt = old_pair.first - old_tstart;
    double old_slope = (old_fv - old_origin) / old_tt;

    trace3("nt slopes", new_slope, old_slope, abs_slope);

    if(0. < old_slope && old_slope < abs_slope){
      trace5("nt1b rise faster", start, old_ft, old_slope, new_slope, vcur);
      _w.push_back(DPAIR(start, vcur));
    }else if(0. > old_slope && old_slope > abs_slope){
      // fall faster
      _w.push_back(DPAIR(start, vcur));
    }else if(fabs(new_fv) > fabs(old_fv) && new_fv*old_fv > 0 ){
      trace5("nt1d keep?", start, new_fv, old_fv, old_slope, new_slope);
      if(0 <= old_slope){
	assert(old_slope >= new_slope);
      }else{
	assert(old_slope <= new_slope);
      }
      _w.push_back(old_pair);
    }else{
      int sign = sgn(old_slope);
      if(0 < old_slope){
      }else if(0 > old_slope){
      }else{ untested();
      }
      trace3("nt2", sign, new_fv, old_fv);
      trace2("nt2", old_slope - new_slope, new_fv*old_fv);
      assert(sign*old_slope >= sign*abs_slope);
      assert(sign*new_fv <= sign*old_fv);

      double t_int = (start * new_slope - old_ft * old_slope ) / ( new_slope - old_slope );
      trace5("nt2", start, old_ft, old_slope, new_slope, t_int);
      double new_ft = start + new_tt;
      assert(t_int <= new_ft);
      assert(start <= t_int);

      double v_int = old_fv + new_slope * (t_int - start);
      trace5("nt2 push", start, old_ft, old_slope, new_slope, v_int);
      _w.push_back(DPAIR(t_int, v_int));
    }
  }else if(start == final_time){ untested();
    trace3("nt incomplete?", start, _w.back().second, new_fv);
    assert(_w.back().first <= final_time);
    _w.push_back(DPAIR(old_pair));
  }else{

  }

  trace3("new_transition tail", start, new_tt, new_fv);
  assert(_w.size());

  assert(_w.back().first != start+new_tt);

  _w.push_back(DPAIR(start+new_tt, new_fv));

  for(auto w : _w){
    trace2("nt done", w.first, w.second);
  }
  trace0("nt done===========");
}
/*--------------------------------------------------------------------------*/
#endif

// vim:ts=8:sw=2:noet:
