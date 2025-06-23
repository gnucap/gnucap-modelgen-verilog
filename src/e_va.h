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
#include <e_logic.h>
#include <m_union.h>
#include <e_node.h>
#include <bm.h> // BUG
/*--------------------------------------------------------------------------*/
extern NODE ground_node;
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
class COMMON_VASRC : public EVAL_BM_BASE {
public:
  explicit COMMON_VASRC(int i) : EVAL_BM_BASE(i) {}
  explicit COMMON_VASRC(COMMON_VASRC const& i) : EVAL_BM_BASE(i) {}
  ~COMMON_VASRC() {}
public:
  virtual DISCIPLINE const* discipline()const{return nullptr;};
  COMMON_COMPONENT* clone()const override {unreachable(); return nullptr;}
  bool operator==(COMMON_COMPONENT const&p)const override {
    return dynamic_cast<COMMON_VASRC const*>(&p)
      && EVAL_BM_BASE::operator==(p);
  }
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
  bool operator==(COMMON_COMPONENT const&)const override {untested(); return false;}
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
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet:
