/*                       -*- C++ -*-
 * Copyright (C) 2025 Felix Salfelder
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
 */
#include "u_nodemap.h"
#include "u_lang.h"
#include "u_opt.h"
#include "globals.h"
#include "c_comand.h"
#include "d_dot.h"
#include "d_coment.h"
#include "e_subckt.h"
#include "e_model.h"
#include "e_node_type.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
const int MAX_DISCIPLINES = 1000;
class CONNECT_RULES : public BASE_SUBCKT {
  mutable node_t _nodes[MAX_DISCIPLINES];
public:
  explicit CONNECT_RULES() : BASE_SUBCKT() {
    new_subckt();
  }
  explicit CONNECT_RULES(CONNECT_RULES const& p) : BASE_SUBCKT(p) {
    new_subckt();
  }
  ~CONNECT_RULES() {
    if(OPT::connect_rules == this){
      OPT::connect_rules = nullptr;
    }else{
    }
  }

private:
  node_t& n_(int i)const override {assert(i<MAX_DISCIPLINES); return _nodes[i];}
  int max_nodes()const override {return MAX_DISCIPLINES;}
  bool node_is_connected(int i)const override {return n_(i).n_();}
  std::string port_name(int)const override {unreachable(); return "";}
  const std::string port_value(int i)const override { return n_(i).n_()?n_(i).n_()->short_label():"??";}
  CARD* clone()const override {return new CONNECT_RULES(*this);}
  bool is_device()const override {return false;}
  bool print_type_in_spice()const override {return false;}
  std::string dev_type()const override {return "connectrules";}
  CARD_LIST* scope()override {untested(); return subckt();}
  CARD_LIST const* scope()const override { return subckt();}
  void expand()override;
  void map_nodes()override {}
}p1;
DISPATCHER<CARD>::INSTALL d1(&device_dispatcher, "connectrules", &p1);
/*--------------------------------------------------------------------------*/
void CONNECT_RULES::expand()
{
  trace2("build_connect_rules", long_label(), net_nodes());
  for(int i = 0; i< MAX_DISCIPLINES; ++i){
    n_(i).clear();
  }
  // create index map to disciplines
  for(auto p : node_dispatcher) {
    std::string name = p.first;
    NODE_TYPE* disc = prechecked_cast<NODE_TYPE*>(p.second);
    if(disc) {
      disc->set_owner(this);
      int id = disc->type_number();
//      disc->clear();
      trace2("disc_dot", id, name);
      set_port_by_index(id, name);
      disc->n_(id) = n_(id) = disc;
      // assert(port_value(id) == name);
    }else{
    }
  }

  for(CARD const* card : *subckt()){
    if(auto dot = dynamic_cast<DEV_DOT const*>(card)){
      trace1("disc_dot", dot->s());
      CS connect(CS::_STRING, dot->s());
      if(connect >> "connect"){
      }else{ untested();
	unreachable();
      }
      NODE const* n = node_dispatcher[connect];
      if(!n){ untested();
	throw Exception("bogus connectrules");
      }else{ untested();
      }
      int i1 = n->user_number();
      n = node_dispatcher[connect];
      if(!n){ untested();
	throw Exception("bogus connectrules");
      }else{ untested();
      }
      int i2 = n->user_number();

      if(connect >> "resolveto"){
      }else{ untested();
	incomplete();
      }

      n = node_dispatcher[connect];
      if(!n){ untested();
	throw Exception("bogus connectrules");
      }else{ untested();
      }
      int r = n->user_number();

      trace3("resolveto", i1, i2, r);
      n_(i1).n_()->n_(i2) = n_(r).n_();
      n_(i2).n_()->n_(i1) = n_(r).n_();
    }
  }
}
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
// vim:ts=8:sw=2:noet:
