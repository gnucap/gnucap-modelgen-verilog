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
#define NATURE NATURE_
#include <e_node_type.h>
#undef NATURE
#include <bm.h> // BUG
#include <e_cardlist.h>
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
  COMMON_COMPONENT* clone()const override { untested();unreachable(); return nullptr;}
  bool operator==(COMMON_COMPONENT const&p)const override {
    return dynamic_cast<COMMON_VASRC const*>(&p)
      && EVAL_BM_BASE::operator==(p);
  }
  double flow_abstol() const{ untested();
    if(!discipline()){ untested();
      return OPT::abstol;
    }else if(!discipline()->flow()){ untested();
      return OPT::abstol;
    }else{ untested();
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
class CURRENT_CTRL : public NODE {
  mutable node_t _nn;
public:
  explicit CURRENT_CTRL(std::string const& n) : NODE(), _nn(this) {
    set_label(n);
  }
  virtual ELEMENT const* get(COMPONENT const*)const {untested();
    unreachable();
    return nullptr;
  }
  node_t const& nn() {return _nn;}
  node_t& n_(int)const override {return _nn;}
};
/*--------------------------------------------------------------------------*/
void set_pot_source(CARD* e)
{
  std::string x;
  e->set_param_by_index(123457, x, 1);
}
/*--------------------------------------------------------------------------*/
void unset_pot_source(CARD* e)
{
  std::string x;
  e->set_param_by_index(123457, x, 0);
}
/*--------------------------------------------------------------------------*/
void map_sckt_nodes(CARD_LIST* l, node_t* nodes)
{
  assert(l);
  for (CARD_LIST::iterator ci = l->begin(); ci != l->end(); ++ci) {
    assert ((**ci).is_device());
    for (int ii = 0;  ii < (**ci).net_nodes();  ++ii) {
      auto& cni = (**ci).n_(ii);
      if(cni.e_() == INVALID_NODE){
      // }else if(nodes[cni.e_()].is_grounded()){ untested();
      }else if(dynamic_cast<CURRENT_CTRL const*>(nodes[cni.e_()].n_())){
	trace2("msn0a", (**ci).long_label(), ii);
	cni = nodes[cni.e_()].n_(); // HACK, later.
      }else{
        cni.map_subckt_node(nodes, nullptr);
      }
    }
  }
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
#endif
// vim:ts=8:sw=2:noet:
