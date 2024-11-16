/*                        -*- C++ -*-
 * Copyright (C) 2023 Felix Salfelder
 * Author: Felix Salfelder
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
 */

#ifndef GNUCAP_E_VA_H
#define GNUCAP_E_VA_H
/*--------------------------------------------------------------------------*/
#include <e_compon.h>
/*--------------------------------------------------------------------------*/
class NATURE {
public:
  virtual double abstol()const{ untested();return 0.;}
};
/*--------------------------------------------------------------------------*/
class DISCIPLINE {
public:
  virtual NATURE const* flow()const { untested();return nullptr;}
  virtual NATURE const* potential()const { untested();return nullptr;}
};
/*--------------------------------------------------------------------------*/
// tmp kludge. nodes don't have disciplines yet...
class COMMON_VASRC : public COMMON_COMPONENT {
public:
  explicit COMMON_VASRC(int i) : COMMON_COMPONENT(i) {}
  ~COMMON_VASRC() {}
public:
  virtual DISCIPLINE const* discipline()const{return nullptr;};
  double flow_abstol() const{
    if(!discipline()){ untested();
      return OPT::abstol;
    }else if(!discipline()->flow()){ untested();
      return OPT::abstol;
    }else{
      return discipline()->flow()->abstol();
    }
  }
  double potential_abstol() const{
    if(!discipline()){ untested();
      return OPT::abstol;
    }else if(!discipline()->potential()){ untested();
      return OPT::abstol;
    }else{
      return discipline()->potential()->abstol();
    }
  }
};
/*--------------------------------------------------------------------------*/
class COMMON_FILT : public COMMON_COMPONENT {
public:
  explicit COMMON_FILT(int i=CC_STATIC) : COMMON_COMPONENT(i) { }
  ~COMMON_FILT() { }
public:
  //virtual DISCIPLINE const* discipline()const{untested(); return nullptr;};
  virtual int args(int) const{ untested(); return 0; }
};
/*--------------------------------------------------------------------------*/
inline void e_val(double* p, const double& x, const CARD_LIST*)
{ untested();
  assert(p);
  *p = x;
}
/*--------------------------------------------------------------------------*/
struct LOGICVAL;
class INTERFACE COMMON_UDP : public COMMON_COMPONENT {
  PARAMETER<vInteger> _delay;
protected:
  explicit	COMMON_UDP(int c=0) :COMMON_COMPONENT(c) { _modelname = "dummy_tmp"; }
  explicit	COMMON_UDP(const COMMON_UDP& p) :COMMON_COMPONENT(p) { _modelname = "dummy_tmp"; }
public:
		~COMMON_UDP()			{}
  bool operator==(const COMMON_COMPONENT&x)const override {
    auto p = dynamic_cast<const COMMON_UDP*>(&x);
    bool rv = p
      && _delay == p->_delay
      && COMMON_COMPONENT::operator==(x);
    return rv;
  }
  virtual LOGICVAL logic_eval(const node_t*, int)const	= 0;

  void		set_param_by_index(int, std::string& x, int)override {_delay=x;}
  bool		param_is_printable(int i)const override {return i==0;}
  using COMMON_COMPONENT::param_name;
  std::string	param_name(int)const override {return "delay";}
  std::string	param_value(int)const override {return "42";}
  int param_count()const override {return (1 + COMMON_COMPONENT::param_count());}
};
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet:
